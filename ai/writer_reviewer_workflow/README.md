# Writer-Reviewer Multi-Agent Workflow (Python)

This app implements a multi-agent workflow using Microsoft Agent Framework SDK:

1. Writer creates initial content from a user request.
2. Reviewer provides concise, actionable feedback.
3. Writer refines content using that feedback.

The final workflow output is plain text: the refined content.

Both Writer and Reviewer are output executors in the workflow, so streaming output includes:
- Writer initial draft
- Reviewer concise actionable feedback
- Writer final refined content (final result)

## Prerequisites

- Python 3.10+
- A Foundry project with a deployed model
- Auth configured for `DefaultAzureCredential` (for example with `az login`)

## Install

```bash
cd ai/writer_reviewer_workflow
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
pip install --pre agent-dev-cli
```

## Configure model

Create `.env` from `.env.example`:

```bash
cp .env.example .env
```

Required variables:

- `FOUNDRY_PROJECT_ENDPOINT`
- `FOUNDRY_MODEL_DEPLOYMENT_NAME`

## Run

HTTP server mode (default):

```bash
python main.py
```

CLI mode (single run):

```bash
python main.py --cli --prompt "Write a launch email for a new budget EV."
```

## VS Code run/debug

- Use task `writer-reviewer: server` for HTTP mode
- Use task `writer-reviewer: cli` for one-shot CLI mode
- Use launch config `Writer Reviewer: HTTP Server` or `Writer Reviewer: CLI` to debug with breakpoints

## Notes

- Agent Framework is currently preview; version pins are intentional.
- You can change model/deployment later by editing `.env`.