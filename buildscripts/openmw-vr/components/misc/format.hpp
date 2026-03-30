#pragma once

#if __has_include(<format>)
#include <components/misc/format.hpp>
#else

#ifndef FMT_HEADER_ONLY
#define FMT_HEADER_ONLY 1
#endif

#include <fmt/format.h>

#include <string>
#include <string_view>
#include <utility>

namespace std
{
    template <class... Args>
    inline string format(string_view fmtString, Args&&... args)
    {
        return fmt::vformat(fmtString, fmt::make_format_args(std::forward<Args>(args)...));
    }
}

#endif
