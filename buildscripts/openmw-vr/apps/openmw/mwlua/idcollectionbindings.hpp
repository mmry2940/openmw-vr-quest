#ifndef MWLUA_IDCOLLECTIONBINDINGS_H
#define MWLUA_IDCOLLECTIONBINDINGS_H

#include <functional>
#include <utility>

#include <components/esm/refid.hpp>
#include <components/lua/luastate.hpp>

namespace MWLua
{
    struct Identity
    {
<<<<<<< HEAD
        template <typename T>
        constexpr T&& operator()(T&& t) const noexcept
        {
            return std::forward<T>(t);
=======
        template <class T>
        constexpr T&& operator()(T&& value) const noexcept
        {
            return std::forward<T>(value);
>>>>>>> 3ecc687e950b13580a4e709d17a2dd7170894a4e
        }
    };

    template <class C, class P = Identity>
    sol::table createReadOnlyRefIdTable(lua_State* lua, const C& container, P projection = {})
    {
        sol::table res(lua, sol::create);
        for (const auto& element : container)
        {
            ESM::RefId id = projection(element);
            if (!id.empty())
                res.add(id.serializeText());
        }
        return LuaUtil::makeReadOnly(res);
    }
}

#endif
