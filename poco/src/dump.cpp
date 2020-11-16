#include "dump.h"
#include <dmsdk/gameobject/gameobject.h>

namespace dmPoco
{
    struct GameObjectIteratorCtx
    {
        lua_State*              m_L;
        // we need these variables to recreate the tree structure
        dmArray<uint32_t>       m_Stack; // A stack of indices to keep track of the traversal tree (i.e. parents)
        uint32_t                m_Index;
        bool                    m_FirstCollection;
    };

    static bool SendGameObjectData(GameObjectIteratorCtx* ctx, dmGameObject::HInstance instance, dmhash_t id, dmhash_t resource_id, uint32_t index, uint32_t parent, const char* type)
    {
        lua_State* L = ctx->m_L;

        const char* str_id = dmHashReverseSafe64(id);

        bool is_gameobject = strcmp(type, "goc") == 0;

        //printf("NODE: '%s'  type: %s\n", str_id, type);

        lua_pushstring(L, str_id);
        lua_setfield(L, -2, "name");

        lua_pushstring(L, dmHashReverseSafe64(resource_id));
        lua_setfield(L, -2, "resource");

        lua_pushstring(L, type);
        lua_setfield(L, -2, "resource_type");

        lua_pushboolean(L, true);
        lua_setfield(L, -2, "visible");

        bool clickable = is_gameobject;

        lua_pushboolean(L, clickable);
        lua_setfield(L, -2, "clickable");

        // TODO: Figure out if they're in local or world space
        if (instance)
        {
            Vectormath::Aos::Point3 pos = dmGameObject::GetPosition(instance);
            lua_pushstring(L, "pos");
            lua_newtable(L);
                lua_pushnumber(L, pos.getX());
                lua_rawseti (L, -2, 1);
                lua_pushnumber(L, pos.getY());
                lua_rawseti (L, -2, 2);
            lua_settable(L, -3);
        }

        return true;
    }

    static bool ComponentIteratorFunction(const dmGameObject::IteratorComponent* iterator, void* user_ctx)
    {
        GameObjectIteratorCtx* ctx = (GameObjectIteratorCtx*)user_ctx;
        lua_State* L = ctx->m_L;
        uint32_t parent = ctx->m_Stack.Back();

        //lua_pushstring(L, dmHashReverseSafe64(iterator->m_NameHash));
        lua_pushinteger(L, iterator->m_Number+1);
        lua_newtable(L);

            lua_pushstring(L, dmHashReverseSafe64(iterator->m_NameHash));
            lua_setfield(L, -2, "name");

            lua_pushstring(L, "payload");
            lua_newtable(L);

                SendGameObjectData(ctx, iterator->m_Instance, iterator->m_NameHash, iterator->m_Resource, ctx->m_Index++, parent, iterator->m_Type);

            lua_settable(L, -3);

            // currently no children
            // lua_pushstring(L, "children");
            // lua_newtable(L);

            // lua_settable(L, -3);

        lua_settable(L, -3);

        return true;
    }

    static bool GameObjectIteratorFunction(const dmGameObject::IteratorGameObject* iterator, void* user_ctx)
    {
        GameObjectIteratorCtx* ctx = (GameObjectIteratorCtx*)user_ctx;
        lua_State* L = ctx->m_L;
        uint32_t parent = ctx->m_Stack.Back();
        uint32_t index = ctx->m_Index++;
        if (ctx->m_Stack.Full())
            ctx->m_Stack.OffsetCapacity(16);
        ctx->m_Stack.Push(index);

        dmhash_t id = dmGameObject::GetIdentifier(iterator->m_Instance);

        //lua_pushstring(L, dmHashReverseSafe64(id));
        lua_pushinteger(L, iterator->m_Number+1);
        lua_newtable(L);

            lua_pushstring(L, dmHashReverseSafe64(id));
            lua_setfield(L, -2, "name");

            lua_pushstring(L, "payload");
            lua_newtable(L);

                SendGameObjectData(ctx, iterator->m_Instance, id, iterator->m_Resource, index, parent, "goc");

            lua_settable(L, -3);

            lua_pushstring(L, "children");
            lua_newtable(L);

                bool result = dmGameObject::IterateComponents(iterator->m_Instance, ComponentIteratorFunction, user_ctx);

            lua_settable(L, -3);

        lua_settable(L, -3);

        uint32_t lastindex = ctx->m_Stack.Back();
        ctx->m_Stack.Pop();
        assert(lastindex == index);

        return result;
    }

    static bool CollectionIteratorFunction(const dmGameObject::IteratorCollection* iterator, void* user_ctx)
    {
        GameObjectIteratorCtx* ctx = (GameObjectIteratorCtx*)user_ctx;
        lua_State* L = ctx->m_L;

        uint32_t parent = ctx->m_Stack.Back();
        uint32_t index = ctx->m_Index++;
        ctx->m_Stack.Push(index);

        printf("collection: %d\n", ctx->m_Index);

        bool first_collection = ctx->m_FirstCollection;
        ctx->m_FirstCollection = false;

        // If it's not the top level collection, add the name to
        if (!first_collection)
        {
        lua_pushstring(L, dmHashReverseSafe64(iterator->m_NameHash));
        lua_newtable(L);
        }

            lua_pushstring(L, dmHashReverseSafe64(iterator->m_NameHash));
            lua_setfield(L, -2, "name");

            lua_pushstring(L, "payload");
            lua_newtable(L);

                SendGameObjectData(ctx, 0, iterator->m_NameHash, iterator->m_Resource, index, parent, "collectionc");

            lua_settable(L, -3);

            lua_pushstring(L, "children");
            lua_newtable(L);

            bool result = dmGameObject::IterateGameObjects(iterator->m_Collection, GameObjectIteratorFunction, user_ctx);

            lua_settable(L, -3);

        if (!first_collection)
        {
        lua_settable(L, -3);
        }

        uint32_t lastindex = ctx->m_Stack.Back();
        ctx->m_Stack.Pop();
        assert(lastindex == index);

        return result;
    }

    // Follow the structure from
    // https://poco-chinese.readthedocs.io/en/latest/source/poco.sdk.AbstractDumper.html?highlight=abstractdumper#poco.sdk.AbstractDumper.IDumper.dumpHierarchy
    void DumpToLuaTable(dmGameObject::HRegister regist, lua_State* L)
    {
        DM_LUA_STACK_CHECK(L, 1);

        GameObjectIteratorCtx ctx;
        ctx.m_L = L;
        ctx.m_Index = 0;
        ctx.m_Stack.SetCapacity(32);   // the depth of the tree
        ctx.m_Stack.Push(ctx.m_Index);
        ctx.m_Index++;
        ctx.m_FirstCollection = true;

        lua_newtable(L);
        bool result = dmGameObject::IterateCollections(regist, CollectionIteratorFunction, &ctx);
    }
}