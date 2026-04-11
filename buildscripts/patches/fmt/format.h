// <format> compatibility shim for Android NDK libc++ (no std::format support)
// Maps std::format to fmt::format (fmtlib 8.x, same API)
#pragma once

#include <fmt/format.h>
#include <string>
#include <string_view>

namespace std {
    using ::fmt::format;
    using ::fmt::format_to;
    using ::fmt::format_to_n;
    using ::fmt::formatted_size;
    using ::fmt::vformat;
    using ::fmt::make_format_args;
    using ::fmt::formatter;

    template <typename... Args>
    using format_string = ::fmt::format_string<Args...>;
}
