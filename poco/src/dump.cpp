#include "dump.h"
#include <dmsdk/gameobject/gameobject.h>

namespace dmPoco
{
    static void OutputProperty(lua_State* L, dmGameObject::SceneNodeProperty* property)
    {
        static dmhash_t hash_id = dmHashString64("id");
        static dmhash_t hash_name = dmHashString64("name");
        static dmhash_t hash_name_position = dmHashString64("position");
        static dmhash_t hash_name_pos = dmHashString64("pos");

        dmhash_t name = property->m_NameHash;
        if (name == hash_name_position) // the poco api needs it to be called "pos"
        {
            name = hash_name_pos;
        } else if (name == hash_id)
        {
            name = hash_name;
        }

        lua_pushstring(L, dmHashReverseSafe64(name));

        char buffer[128];
        buffer[0] = 0;

        switch(property->m_Type)
        {
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_HASH: lua_pushstring(L, dmHashReverseSafe64(property->m_Value.m_Hash)); break;
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_NUMBER: lua_pushnumber(L, property->m_Value.m_Number); break;
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_BOOLEAN: lua_pushboolean(L, property->m_Value.m_Bool); break;
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_URL: lua_pushstring(L, property->m_Value.m_URL); break;
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_TEXT: lua_pushstring(L, property->m_Value.m_Text); break;
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_VECTOR3:
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_VECTOR4:
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_QUAT:
            {
                lua_newtable(L);
                int vsize = property->m_Type == dmGameObject::SCENE_NODE_PROPERTY_TYPE_VECTOR3 ? 3 : 4;
                for (int v = 0; v < vsize; ++v)
                {
                    lua_pushnumber(L, property->m_Value.m_V4[v]);
                    lua_rawseti (L, -2, v+1);
                }
            }
            break;
        default:
            lua_pushnil(L); break;
        }


        lua_settable(L, -3);
    }

    static bool IsVisible(dmGameObject::SceneNode* node)
    {
        return true;
    }

    static bool IsClickable(dmGameObject::SceneNode* node)
    {
        return true;
    }

    static void SceneGraphToLua(lua_State* L, dmGameObject::SceneNode* node)
    {
        static dmhash_t hash_id = dmHashString64("id");
        static dmhash_t hash_name_position = dmHashString64("position");
        dmhash_t name = 0;

        lua_pushstring(L, "payload");
        lua_newtable(L);

            bool visible = IsVisible(node);
            bool clickable = IsClickable(node);

            lua_pushboolean(L, visible);
            lua_setfield(L, -2, "visible");

            lua_pushboolean(L, clickable);
            lua_setfield(L, -2, "clickable");

            bool has_position = false;
            dmGameObject::SceneNodePropertyIterator pit = TraverseIterateProperties(node);
            while(dmGameObject::TraverseIteratePropertiesNext(&pit))
            {
                OutputProperty(L, &pit.m_Property);

                if (pit.m_Property.m_NameHash == hash_id)
                    name = pit.m_Property.m_Value.m_Hash;
                else if (pit.m_Property.m_NameHash == hash_name_position)
                    has_position = true;
            }

            if (!has_position)
            {
                lua_pushstring(L, "pos");  // add a default position value
                lua_newtable(L);
                    for (int v = 0; v < 3; ++v)
                    {
                        lua_pushnumber(L, 0);
                        lua_rawseti (L, -2, v+1);
                    }
                lua_settable(L, -3);
            }

        lua_settable(L, -3);

        lua_pushstring(L, "children");
        lua_newtable(L);

            int counter = 0;
            dmGameObject::SceneNodeIterator it = dmGameObject::TraverseIterateChildren(node);
            while(dmGameObject::TraverseIterateNext(&it))
            {
                lua_pushinteger(L, 1+counter++);
                lua_newtable(L);

                    SceneGraphToLua(L, &it.m_Node);

                lua_settable(L, -3);
            }

        lua_settable(L, -3);

        lua_pushstring(L, dmHashReverseSafe64(name));
        lua_setfield(L, -2, "name");
    }

    // Follow the structure from
    // https://poco-chinese.readthedocs.io/en/latest/source/poco.sdk.AbstractDumper.html?highlight=abstractdumper#poco.sdk.AbstractDumper.IDumper.dumpHierarchy
    void DumpToLuaTable(dmGameObject::HRegister regist, lua_State* L)
    {
        DM_LUA_STACK_CHECK(L, 1);

        dmGameObject::SceneNode root;
        if (!dmGameObject::TraverseGetRoot(regist, &root))
        {
            lua_pushnil(L);
            return;
        }

        lua_newtable(L);
        SceneGraphToLua(L, &root);
    }
}