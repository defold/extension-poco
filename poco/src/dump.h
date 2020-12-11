#pragma once

#include <dmsdk/sdk.h>

namespace dmGameObject
{
    /// Component type and game object register
    typedef struct Register* HRegister;
}

namespace dmPoco
{
    void DumpToLuaTable(lua_State* L, dmGameObject::HRegister regist,
                                const dmVMath::Matrix4& view_proj, const dmVMath::Matrix4& gui_view_proj);
}