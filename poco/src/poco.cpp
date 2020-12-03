#include <dmsdk/sdk.h>
#include <dmsdk/dlib/json.h>
#include <dmsdk/dlib/math.h>
#include <dmsdk/dlib/webserver.h>
#include <dmsdk/hid/hid.h>
#include "json.h"
#include "dump.h"
#include "input.h"

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
    dmJson::Document    m_DocReceived; // The current request
    bool                m_Initialized;
    uint64_t            m_LastTime;

    dmGameObject::HRegister m_Register;
    dmHID::HContext         m_HidContext;
} g_Poco;

static dmJson::Document* ParseRequest(PocoContext* ctx)
{
    dmJson::Result result = dmJson::Parse((const char*)ctx->m_Buffer, ctx->m_BufferSize, &ctx->m_DocReceived);
    if (dmJson::RESULT_OK != result)
    {
        dmLogError("Failed to parse json request: %d", result);
        return 0;
    }
    return &ctx->m_DocReceived;
}

static dmWebServer::Result ReadHttpContent(PocoContext* ctx, dmWebServer::Request* request)
{
    char buf[256];
    uint32_t total_recv = 0;

    if (request->m_ContentLength > ctx->m_BufferCapacity)
    {
        ctx->m_Buffer = (char*)realloc(ctx->m_Buffer, request->m_ContentLength+1);
        ctx->m_BufferCapacity = request->m_ContentLength;
    }

    ctx->m_BufferSize = 0;
    while (total_recv < request->m_ContentLength)
    {
        uint32_t recv_bytes = 0;
        uint32_t to_read = dmMath::Min(ctx->m_BufferCapacity - ctx->m_BufferSize, request->m_ContentLength - total_recv);
        dmWebServer::Result r = dmWebServer::Receive(request, ctx->m_Buffer + ctx->m_BufferSize, to_read, &recv_bytes);
        if (r != dmWebServer::RESULT_OK)
            return r;
        total_recv += recv_bytes;
    }
    ctx->m_BufferSize = total_recv;
    return dmWebServer::RESULT_OK;
}

static void HttpHandler(void* user_data, dmWebServer::Request* request)
{
    PocoContext* ctx = (PocoContext*)user_data;
    printf("request: %s  '%s'  size: %u\n", request->m_Method, request->m_Resource, request->m_ContentLength);


    if (strcmp("POST", request->m_Method) != 0)
        return;

    uint32_t content_length = request->m_ContentLength;
    if (content_length > 0)
    {
        dmWebServer::Result result = ReadHttpContent(ctx, request);
        // ctx->m_BufferSize = 0;
        // dmWebServer::Result result = dmWebServer::Receive(request, ctx->m_Buffer, ctx->m_BufferCapacity, &ctx->m_BufferSize);
        // ctx->m_Buffer[ctx->m_BufferCapacity] = 0;
        if (dmWebServer::RESULT_OK != result)
        {
            dmLogError("Failed to receive %u bytes for request '%s %s': %d", content_length, request->m_Method, request->m_Resource, result);
            return;
        }
        ctx->m_Buffer[content_length] = 0;

        // If the entire string is wrapped in a format with extra \\\", and also extra quotation marks around it all '\"PAYLOAD\"''
        if (strstr(ctx->m_Buffer, "\"{\\\"") == ctx->m_Buffer)
        {
            printf("Replacing '\\\"' with '\"'\n");
            uint32_t writeoffset = 0; // where to stire
            uint32_t readoffset = 1; // skip the first '\"'

            const char* end = ctx->m_Buffer + content_length - 1;
            const char* c = ctx->m_Buffer + readoffset;
            content_length = 0;
            while (c < end)
            {
                if ((c+1) < end)
                {
                    if (c[0] == '\\' && c[1] == '\"')
                    {
                        c += 1;
                        continue; // skip
                    }
                }

                ctx->m_Buffer[writeoffset++] = *c++;
                content_length++;
            }
            ctx->m_Buffer[content_length] = 0;
        }
    }

    if (strcmp(HTTP_PATH, request->m_Resource) == 0)
    {
        dmJson::Document* doc = ParseRequest(ctx);
        if (!doc)
        {
            const char* msg = "Failed to parse json";
            dmWebServer::SetStatusCode(request, 500);
            dmWebServer::Send(request, msg, strlen(msg));
            return;
        }

        printf("\nDEBUGPRINT:\n");
        dmJson::DebugPrint(doc, 0);
        printf("\nEND\n");


// The current response
        const char* json =
"{\n"
"    \"id\": \"%s\",\n"
"    \"result\": {\n"
"        \"name\": \"OctopusArea\",\n"
"        \"payload\": {\n"
"            \"name\": \"OctopusArea\",\n"
"            \"type\": \"GameObject\",\n"
"            \"visible\": true,\n"
"            \"clickable\": true,\n"
"            \"zOrders\": {\n"
"                \"global\": 0,\n"
"                \"local\": -10\n"
"            },\n"
"            \"scale\": [\n"
"                1,\n"
"                1\n"
"            ],\n"
"            \"anchorPoint\": [\n"
"                0.5,\n"
"                0.5\n"
"            ],\n"
"            \"pos\": [\n"
"                0.130729169,\n"
"                0.44907406\n"
"            ],\n"
"            \"size\": [\n"
"                0.0859375,\n"
"                0.125\n"
"            ]\n"
"        }\n"
"    }\n"
"}\n\n\n\n"
;

        const dmJson::Node* j_root = dmJson::GetRootNode(doc);
        const dmJson::Node* j_id = dmJson::GetNode(doc, j_root, "id");
        if (!j_id)
        {
            dmJson::Free(doc);
            dmLogError("Couldn't find id in request");
            dmWebServer::SetStatusCode(request, 500);
            return;
        }


        uint32_t j_id_value_len = 0;
        const char* j_id_value = dmJson::GetString(doc, j_id, &j_id_value_len);

        char id_value[40];
        uint32_t id_value_size = j_id_value_len+1 < sizeof(id_value) ? j_id_value_len+1 : sizeof(id_value);
        dmStrlCpy(id_value, j_id_value, j_id_value_len+1);

        dmJson::Free(doc);


        printf("ID: '%s'  %u\n", id_value, j_id_value_len);



        ctx->m_BufferSize = dmSnPrintf(ctx->m_Buffer, ctx->m_BufferCapacity, json, id_value);
        if (ctx->m_BufferSize >= ctx->m_BufferCapacity)
        {
            const uint32_t id_size = 40;
            const uint32_t size = strlen(json)+id_size+1;
            ctx->m_Buffer = (char*)realloc(ctx->m_Buffer, size);
            ctx->m_BufferCapacity = size;

            ctx->m_BufferSize = dmSnPrintf(ctx->m_Buffer, ctx->m_BufferCapacity, json, id_value);
            if (ctx->m_BufferSize >= ctx->m_BufferCapacity)
            {
                dmLogError("Couldn't write response to too small buffer");
                dmWebServer::SetStatusCode(request, 500);
                return;
            }
        }

        printf("JSON:\n'%s'\n\nlen: %u", ctx->m_Buffer, ctx->m_BufferSize);

        dmWebServer::SetStatusCode(request, 200);
        dmWebServer::SendAttribute(request, "Access-Control-Allow-Origin", "*");
        dmWebServer::SendAttribute(request, "Content-Type", "application/json");

        dmWebServer::Result r = dmWebServer::Send(request, ctx->m_Buffer, ctx->m_BufferSize);
        printf("Write result: %d\n", r);
        return;
    }

    const char* response = "HELLO WORLD!";
    dmWebServer::Result r = dmWebServer::Send(request, response, strlen(response));

    dmWebServer::SetStatusCode(request, 404);
}

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

static int Poco_Click(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);

    lua_Number x = luaL_checknumber(L, 1);
    lua_Number y = luaL_checknumber(L, 2);

    dmPoco::Click((int32_t)x, (int32_t)y);

    return 0;
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

// Functions exposed to Lua
static const luaL_reg Poco_module_methods[] =
{
    {"dump", Poco_Dump},
    {"click", Poco_Click},
    {"long_click", Poco_LongClick},
    {"swipe", Poco_Swipe},
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
    dmWebServer::HServer web_server = dmEngine::GetWebServer(params);

    memset(&g_Poco, 0, sizeof(g_Poco));

    if (!web_server) {
        dmLogWarning("Engine is built with extension '%s', but there is no webserver available. Is this a release build?", LIB_NAME);
        return dmExtension::RESULT_OK;
    }
    else {
        dmWebServer::HandlerParams handler_params;
        handler_params.m_Userdata = (void*)&g_Poco;
        handler_params.m_Handler = HttpHandler;
        dmWebServer::Result result = dmWebServer::AddHandler(web_server, HTTP_PATH, &handler_params);
        if (dmWebServer::RESULT_OK != result)
        {
            dmLogError("Failed to register http handler for '%s'", HTTP_PATH);
            return dmExtension::RESULT_OK; // it's not a fatal error
        }
    }

    g_Poco.m_Register = dmEngine::GetGameObjectRegister(params);
    g_Poco.m_HidContext = dmEngine::GetHIDContext(params);
    g_Poco.m_LastTime = dmTime::GetTime();
    g_Poco.m_Initialized = true;

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
    if (g_Poco.m_Initialized)
    {
        dmWebServer::RemoveHandler(dmEngine::GetWebServer(params), HTTP_PATH);
    }
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
