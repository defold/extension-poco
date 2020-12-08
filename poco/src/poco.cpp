#include <dmsdk/sdk.h>
#include <dmsdk/dlib/math.h>
#include <dmsdk/hid/hid.h>
#include "dump.h"
#include "input.h"

#include <stdlib.h> // realloc

#define MODULE_NAME Poco
#define LIB_NAME "poco_helper"
#define HTTP_PATH "/poco"


namespace dmPoco
{

struct PocoContext
{
    char*               m_Buffer;
    uint32_t            m_BufferSize;
    uint32_t            m_BufferCapacity;
    bool                m_Initialized;
    uint64_t            m_LastTime;

    dmGameObject::HRegister m_Register;
    dmHID::HContext         m_HidContext;
} g_Poco;

static int Poco_Dump(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);

    bool only_visible_node = false;
    if (lua_isboolean(L, 1))
        only_visible_node = lua_toboolean(L, 1);
    else
        return DM_LUA_ERROR("Expected boolean for argument 1");

    lua_Number screen_width = luaL_checknumber(L, 2);
    lua_Number screen_height = luaL_checknumber(L, 3);

    // Creates a table
    dmPoco::DumpToLuaTable(g_Poco.m_Register, screen_width, screen_height, L);

    return 1;
}

static int Click(lua_State* L, dmHID::MouseButton button)
{
    DM_LUA_STACK_CHECK(L, 0);

    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);

    dmPoco::Click(button, (int32_t)x, (int32_t)y);

    return 0;
}

static int Poco_Click(lua_State* L)
{
    return Click(L, dmHID::MOUSE_BUTTON_LEFT);
}

static int Poco_RightClick(lua_State* L)
{
    return Click(L, dmHID::MOUSE_BUTTON_RIGHT);
}

static int Poco_LongClick(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);
    lua_Number duration = luaL_checknumber(L, 3);
    if (duration <= 0.0f)
        return DM_LUA_ERROR("Duration must be > 0.0. Duration: %f", duration);

    dmPoco::LongClick(x, y, duration);

    return 0;
}

static int Poco_Swipe(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    lua_Number x1 = luaL_checknumber(L, 1);
    lua_Number y1 = luaL_checknumber(L, 2);
    lua_Number x2 = luaL_checknumber(L, 3);
    lua_Number y2 = luaL_checknumber(L, 4);
    lua_Number duration = luaL_checknumber(L, 5);
    if (duration <= 0.0f)
        return DM_LUA_ERROR("Duration must be > 0.0. Duration: %f", duration);

    dmPoco::Swipe(x1, y1, x2, y2, duration);

    return 0;
}

static int Poco_KeyEvent(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    const char* str = luaL_checkstring(L, 1);

    dmPoco::KeyEvent(str);

    return 0;
}

// Functions exposed to Lua
static const luaL_reg Poco_module_methods[] =
{
    {"dump", Poco_Dump},
    {"click", Poco_Click},
    {"right_click", Poco_RightClick},
    {"long_click", Poco_LongClick},
    {"swipe", Poco_Swipe},
    {"keyevent", Poco_KeyEvent},
    {0, 0}
};

static void LuaInit(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    luaL_register(L, LIB_NAME, Poco_module_methods);
    lua_pop(L, 1);
}


static dmExtension::Result AppInitialize(dmExtension::AppParams* params)
{
    memset(&g_Poco, 0, sizeof(g_Poco));

    g_Poco.m_Register = dmEngine::GetGameObjectRegister(params);
    g_Poco.m_HidContext = dmEngine::GetHIDContext(params);
    g_Poco.m_LastTime = dmTime::GetTime();
    g_Poco.m_Initialized = true;

    dmPoco::InitInput();

    return dmExtension::RESULT_OK;
}


static dmExtension::Result Initialize(dmExtension::Params* params)
{
    if (!g_Poco.m_Initialized)
        return dmExtension::RESULT_OK;

    LuaInit(params->m_L);
    dmLogInfo("Registered %s extension", LIB_NAME);

    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalize(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result Finalize(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

static dmExtension::Result OnUpdate(dmExtension::Params* params)
{
    if (!g_Poco.m_Initialized)

        return dmExtension::RESULT_OK;

    uint64_t time = dmTime::GetTime();
    double dt = (time - g_Poco.m_LastTime) / 1000000.0f;
    g_Poco.m_LastTime = time;

    dmPoco::UpdateInput(g_Poco.m_HidContext, dt);

    return dmExtension::RESULT_OK;
}

}

DM_DECLARE_EXTENSION(MODULE_NAME, LIB_NAME, dmPoco::AppInitialize, dmPoco::AppFinalize, dmPoco::Initialize, dmPoco::OnUpdate, 0, dmPoco::Finalize)

#undef LIB_NAME
#undef HTTP_PATH
