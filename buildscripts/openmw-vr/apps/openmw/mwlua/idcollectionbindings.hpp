#ifndef MWLUA_IDCOLLECTIONBINDINGS_H
#define MWLUA_IDCOLLECTIONBINDINGS_H

#include <functional>

#include <components/esm/refid.hpp>
#include <components/lua/luastate.hpp>

namespace MWLua
{
    struct Identity
    {
        template <typename T>
        constexpr T&& operator()(T&& t) const noexcept
        {
            return std::forward<T>(t);
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
