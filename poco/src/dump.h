#pragma once

#include <dmsdk/sdk.h>

namespace dmGameObject
{
    /// Component type and game object register
    typedef struct Register* HRegister;
}

namespace dmPoco
{
    void DumpToLuaTable(dmGameObject::HRegister regist, int32_t screen_width, int32_t screen_height, lua_State* L);
}