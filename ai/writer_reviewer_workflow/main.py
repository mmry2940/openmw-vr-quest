import argparse
import asyncio
import os
from dataclasses import dataclass
from typing import Any

from agent_framework import AgentResponseUpdate, Content, Executor, Message, WorkflowBuilder, WorkflowContext, handler
from agent_framework.azure import AzureAIClient
from azure.ai.agentserver.agentframework import from_agent_framework
from azure.identity.aio import DefaultAzureCredential
from dotenv import load_dotenv
from typing_extensions import Never


@dataclass
class DraftPayload:
    user_request: str
    draft_content: str


@dataclass
class ReviewPayload:
    user_request: str
    draft_content: str
    reviewer_feedback: str


def _require_env(name: str) -> str:
    value = os.getenv(name)
    if not value:
        raise ValueError(f"Missing required environment variable: {name}")
    return value


def _safe_text(value: Any) -> str:
    if hasattr(value, "text") and isinstance(value.text, str):
        return value.text.strip()
    if isinstance(value, str):
        return value.strip()
    return str(value).strip()


def _extract_user_prompt(messages: list[Message]) -> str:
    for message in reversed(messages):
        if message.role == "user":
            if isinstance(message.content, list) and message.content:
                return _safe_text(message.content[0])
            return str(message.content).strip()
    return "Please write a short, clear blog post about the topic you choose."


def _as_output(author_name: str, text: str) -> AgentResponseUpdate:
    return AgentResponseUpdate(
        contents=[Content("text", text=text)],
        role="assistant",
        author_name=author_name,
    )


def _output_text(update: AgentResponseUpdate) -> str:
    contents = getattr(update, "contents", None)
    if isinstance(contents, list) and contents:
        first = contents[0]
        return _safe_text(first)
    return _safe_text(update)


class WriterDraftExecutor(Executor):
    def __init__(self, writer_agent, id: str = "writer_draft"):
        super().__init__(id=id)
        self._writer_agent = writer_agent

    @handler
    async def create_draft(
        self,
        messages: list[Message],
        ctx: WorkflowContext[DraftPayload, AgentResponseUpdate],
    ) -> None:
        user_request = _extract_user_prompt(messages)
        writer_prompt = (
            "You are the Writer. Create an initial draft that is clear, structured, and concise.\n\n"
            f"User request:\n{user_request}\n"
        )
        writer_result = await self._writer_agent.run([Message("user", [writer_prompt])])
        draft_content = _safe_text(writer_result)

        await ctx.yield_output(_as_output("Writer", draft_content))
        await ctx.send_message(DraftPayload(user_request=user_request, draft_content=draft_content))


class ReviewerExecutor(Executor):
    def __init__(self, reviewer_agent, id: str = "reviewer"):
        super().__init__(id=id)
        self._reviewer_agent = reviewer_agent

    @handler
    async def review(
        self,
        payload: DraftPayload,
        ctx: WorkflowContext[ReviewPayload, AgentResponseUpdate],
    ) -> None:
        reviewer_prompt = (
            "You are the Reviewer. Provide concise, actionable feedback for the draft. "
            "Return 3-6 bullet points that focus on clarity, structure, and usefulness.\n\n"
            f"User request:\n{payload.user_request}\n\n"
            f"Draft content:\n{payload.draft_content}\n"
        )
        review_result = await self._reviewer_agent.run([Message("user", [reviewer_prompt])])
        reviewer_feedback = _safe_text(review_result)

        await ctx.yield_output(_as_output("Reviewer", reviewer_feedback))
        await ctx.send_message(
            ReviewPayload(
                user_request=payload.user_request,
                draft_content=payload.draft_content,
                reviewer_feedback=reviewer_feedback,
            )
        )


class WriterRefineExecutor(Executor):
    def __init__(self, writer_agent, id: str = "writer_refine"):
        super().__init__(id=id)
        self._writer_agent = writer_agent

    @handler
    async def refine(
        self,
        payload: ReviewPayload,
        ctx: WorkflowContext[Never, AgentResponseUpdate],
    ) -> None:
        refine_prompt = (
            "You are the Writer. Revise the draft using the reviewer feedback.\n"
            "Output only the final refined content as plain text.\n\n"
            f"User request:\n{payload.user_request}\n\n"
            f"Original draft:\n{payload.draft_content}\n\n"
            f"Reviewer feedback:\n{payload.reviewer_feedback}\n"
        )
        refined_result = await self._writer_agent.run([Message("user", [refine_prompt])])
        refined_content = _safe_text(refined_result)
        await ctx.yield_output(_as_output("Writer", refined_content))


def build_workflow_agent():
    # Preserve process environment values and only fill missing values from .env.
    load_dotenv(override=False)
    endpoint = _require_env("FOUNDRY_PROJECT_ENDPOINT")
    deployment_name = _require_env("FOUNDRY_MODEL_DEPLOYMENT_NAME")

    writer_client = AzureAIClient(
        endpoint=endpoint,
        deployment_name=deployment_name,
        credential=DefaultAzureCredential(),
    )
    reviewer_client = AzureAIClient(
        endpoint=endpoint,
        deployment_name=deployment_name,
        credential=DefaultAzureCredential(),
    )

    writer_agent = writer_client.as_agent(
        name="WriterAgent",
        instructions=(
            "You are a strong content writer. Produce clear, well-structured drafts and revisions."
        ),
    )
    reviewer_agent = reviewer_client.as_agent(
        name="ReviewerAgent",
        instructions=(
            "You are a strict but constructive reviewer. Keep feedback concise and actionable."
        ),
    )

    writer_draft = WriterDraftExecutor(writer_agent=writer_agent)
    reviewer = ReviewerExecutor(reviewer_agent=reviewer_agent)
    writer_refine = WriterRefineExecutor(writer_agent=writer_agent)

    workflow = (
        WorkflowBuilder(start_executor=writer_draft)
        .add_edge(writer_draft, reviewer)
        .add_edge(reviewer, writer_refine)
        .build()
    )
    return workflow.as_agent()


async def run_cli(prompt: str) -> None:
    workflow_agent = build_workflow_agent()
    response_text: str = ""

    async for event in workflow_agent.run(Message("user", [prompt]), stream=True):
        if event.type == "output" and isinstance(event.data, AgentResponseUpdate):
            update = event.data
            if update.author_name and update.author_name.lower() == "writer":
                response_text = _output_text(update)

    print(response_text)


async def run_server() -> None:
    workflow_agent = build_workflow_agent()
    await from_agent_framework(workflow_agent).run_async()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Writer-Reviewer multi-agent workflow app")
    parser.add_argument("--cli", action="store_true", help="Run once in CLI mode")
    parser.add_argument(
        "--prompt",
        default="Write a concise product announcement for a budget-friendly electric SUV.",
        help="Prompt used for --cli mode",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if args.cli:
        asyncio.run(run_cli(prompt=args.prompt))
        return
    asyncio.run(run_server())


if __name__ == "__main__":
    main()