#ifndef OPENMW_COMPONENTS_MISC_CONCEPTS_H
#define OPENMW_COMPONENTS_MISC_CONCEPTS_H

#include <type_traits>

namespace Misc
{
    // Note: std::same_as from <concepts> unavailable with NDK r21/Clang 9;
    // semantically equivalent replacement using <type_traits>.
    template <class T, class U>
    concept SameAsWithoutCvref = std::is_same_v<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;
}

#endif
