#pragma once

#include <dmsdk/sdk.h>

namespace dmGameObject
{
    /// Component type and game object register
    typedef struct Register* HRegister;
}

namespace dmPoco
{
    void DumpToLuaTable(dmGameObject::HRegister regist, lua_State* L);
}