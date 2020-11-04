#include <dmsdk/sdk.h>
#include <dmsdk/dlib/json.h>
#include <dmsdk/dlib/webserver.h>
#include "json.h"

#define LIB_NAME "poco"
#define HTTP_PATH "/poco"



namespace dmPoco
{

struct PocoContext
{
    char*       m_Buffer;
    uint32_t    m_BufferSize;
    uint32_t    m_BufferCapacity;
    dmJson::Document m_DocReceived; // The current request
    bool        m_Initialized;
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

// static const char* GetString(dmJson::Document* doc, dmJson::Node* n, const char*)
// {
//     if (n->m_Type != dmJson::TYPE_STRING)
//         return 0;


// }


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

static dmExtension::Result AppInitialize(dmExtension::AppParams* params)
{
    memset(&g_Poco, 0, sizeof(g_Poco));

    if (!params->m_WebServer) {
        dmLogWarning("Engine is built with extension '%s', but there is no webserver available. Is this a release build?", LIB_NAME);
        return dmExtension::RESULT_OK;
    }

    dmWebServer::HandlerParams handler_params;
    handler_params.m_Userdata = (void*)&g_Poco;
    handler_params.m_Handler = HttpHandler;
    dmWebServer::Result result = dmWebServer::AddHandler(params->m_WebServer, HTTP_PATH, &handler_params);
    if (dmWebServer::RESULT_OK != result)
    {
        dmLogError("Failed to register http handler for '%s'", HTTP_PATH);
        return dmExtension::RESULT_OK; // it's not a fatal error
    }

    dmLogInfo("Registered %s extension", LIB_NAME);
    g_Poco.m_Initialized = true;
    return dmExtension::RESULT_OK;
}


static dmExtension::Result Initialize(dmExtension::Params* params)
{
    if (!g_Poco.m_Initialized)
        return dmExtension::RESULT_OK;

    return dmExtension::RESULT_OK;
}

static dmExtension::Result AppFinalize(dmExtension::AppParams* params)
{
    if (g_Poco.m_Initialized)
    {
        dmWebServer::RemoveHandler(params->m_WebServer, HTTP_PATH);
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

    return dmExtension::RESULT_OK;
}

}

DM_DECLARE_EXTENSION(Poco, "poco", dmPoco::AppInitialize, dmPoco::AppFinalize, dmPoco::Initialize, dmPoco::OnUpdate, 0, dmPoco::Finalize)

#undef LIB_NAME
#undef HTTP_PATH
