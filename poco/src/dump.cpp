#include "dump.h"
#include <dmsdk/gameobject/gameobject.h>
#include <dmsdk/dlib/math.h>

namespace dmPoco
{
    // static dmhash_t hash_id = dmHashString64("id");
    // static dmhash_t hash_name = dmHashString64("name");
    // static dmhash_t hash_world_position = dmHashString64("world_position");
    // static dmhash_t hash_world_size = dmHashString64("world_size");
    // static dmhash_t hash_position = dmHashString64("position");
    // static dmhash_t hash_pos = dmHashString64("pos");
    // static dmhash_t hash_size = dmHashString64("size");
    // static dmhash_t hash_pivot = dmHashString64("pivot");
    // static dmhash_t hash_anchor_point = dmHashString64("anchorPoint");



    // The gui nodes have their position at the pivot
    // We need to calculate the anchor point so that the click() coordinate
    // will end up in the [(0,0, (1,1)] area
    static void GetPivotFromHash(dmhash_t pivot, float& px, float& py)
    {
        static dmhash_t hash_PIVOT_NW = dmHashString64("PIVOT_NW");
        static dmhash_t hash_PIVOT_N = dmHashString64("PIVOT_N");
        static dmhash_t hash_PIVOT_NE = dmHashString64("PIVOT_NE");
        static dmhash_t hash_PIVOT_W = dmHashString64("PIVOT_W");
        static dmhash_t hash_PIVOT_CENTER = dmHashString64("PIVOT_CENTER");
        static dmhash_t hash_PIVOT_E = dmHashString64("PIVOT_E");
        static dmhash_t hash_PIVOT_SW = dmHashString64("PIVOT_SW");
        static dmhash_t hash_PIVOT_S = dmHashString64("PIVOT_S");
        static dmhash_t hash_PIVOT_SE = dmHashString64("PIVOT_SE");

        const dmhash_t hashes[] = {
            hash_PIVOT_NW, hash_PIVOT_N, hash_PIVOT_NE, hash_PIVOT_W,
            hash_PIVOT_CENTER, hash_PIVOT_E, hash_PIVOT_SW, hash_PIVOT_S, hash_PIVOT_SE,
        };

        // The Poco coordinate origin is in the top left corner
        const float points[][2] = {
            { 0.0f,  0.0f}, // PIVOT_NW
            { 0.5f,  0.0f}, // PIVOT_N
            { 1.0f,  0.0f}, // PIVOT_NE
            { 0.0f,  0.5f}, // PIVOT_W
            { 0.5f,  0.5f}, // PIVOT_CENTER
            { 1.0f,  0.5f}, // PIVOT_E
            { 0.0f,  1.0f}, // PIVOT_SW
            { 0.5f,  1.0f}, // PIVOT_S
            { 1.0f,  1.0f}, // PIVOT_SE
        };

        for (int i = 0; i < DM_ARRAY_SIZE(hashes); ++i)
        {
            if (hashes[i] == pivot)
            {
                px = points[i][0];
                py = points[i][1];
                return;
            }
        }

        px = 0;
        py = 0;
    }

    static void OutputProperty(lua_State* L, const dmVMath::Matrix4& view_proj, dmGameObject::SceneNodeProperty* property)
    {
        static dmhash_t hash_id = dmHashString64("id");
        static dmhash_t hash_name = dmHashString64("name");
        static dmhash_t hash_world_position = dmHashString64("world_position");
        static dmhash_t hash_world_size = dmHashString64("world_size");
        static dmhash_t hash_position = dmHashString64("position");
        static dmhash_t hash_pos = dmHashString64("pos");
        static dmhash_t hash_size = dmHashString64("size");
        static dmhash_t hash_pivot = dmHashString64("pivot");
        static dmhash_t hash_anchor_point = dmHashString64("anchorPoint");

        dmhash_t name = property->m_NameHash;

        // Overwritten anyways
        if (name == hash_size)
            return;

        if (name == hash_world_position) {
            name = hash_pos; // the poco api needs it to be called "pos"
        } else if (name == hash_id) {
            name = hash_name; // the poco api needs it to be called "name"
        } else if (name == hash_pivot) {
            name = hash_anchor_point;
        } else if (name == hash_world_size) {
            name = hash_size;
        }

        lua_pushstring(L, dmHashReverseSafe64(name));

        char buffer[128];
        buffer[0] = 0;

        switch(property->m_Type)
        {
        case dmGameObject::SCENE_NODE_PROPERTY_TYPE_HASH:

            if (name == hash_anchor_point)
            {
                float px, py;
                GetPivotFromHash(property->m_Value.m_Hash, px, py);

                lua_newtable(L);
                    lua_pushnumber(L, px);
                    lua_rawseti (L, -2, 1);
                    lua_pushnumber(L, py);
                    lua_rawseti (L, -2, 2);
            }
            else
            {
                lua_pushstring(L, dmHashReverseSafe64(property->m_Value.m_Hash));
            }
            break;
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

                float v[4] = { property->m_Value.m_V4[0], property->m_Value.m_V4[1], property->m_Value.m_V4[2], property->m_Value.m_V4[3]};

                if (name == hash_pos)
                {
                    // Convert from engine world space to defold screen space [(-1, -1), (1,1)]
                    dmVMath::Vector4 p = view_proj * dmVMath::Point3(v[0], v[1], v[2]);

                    // Convert to poco space [(0,0), (1,1)]
                    v[0] = p.getX() * 0.5f + 0.5f;
                    v[1] = p.getY() * 0.5f + 0.5f;

                    // flip Y
                    v[1] = 1.0f - v[1];

                    // Certain operations like "pinch" unpacks all values,
                    // and then we cannot return a list with more than 2 elements
                    vsize = 2;
                }
                else if (name == hash_size)
                {
                    dmVMath::Vector4 p = view_proj * dmVMath::Vector3(v[0], v[1], v[2]);

                    // Convert to poco space [(0,0), (1,1)]
                    v[0] = p.getX() * 0.5f;
                    v[1] = p.getY() * 0.5f;

                    // Certain operations like "pinch" unpacks all values,
                    // and then we cannot return a list with more than 2 elements
                    vsize = 2;
                }

                for (int i = 0; i < vsize; ++i)
                {
                    lua_pushnumber(L, v[i]);
                    lua_rawseti (L, -2, i+1);
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

    static void GetNodeInfo(dmGameObject::SceneNode* node, dmhash_t& name, bool& clickable, bool& isguitype)
    {
        static dmhash_t hash_id = dmHashString64("id");
        static dmhash_t hash_type = dmHashString64("type");
        static dmhash_t hash_world_position = dmHashString64("world_position");
        static dmhash_t hash_position = dmHashString64("position");

        dmGameObject::SceneNodePropertyIterator pit = TraverseIterateProperties(node);
        while(dmGameObject::TraverseIteratePropertiesNext(&pit))
        {
            if (pit.m_Property.m_NameHash == hash_type)
            {
                const char* type_string = dmHashReverseSafe64(pit.m_Property.m_Value.m_Hash);
                isguitype = strstr(type_string, "gui_node_") == type_string;
            }
            else if (pit.m_Property.m_NameHash == hash_id)
            {
                name = pit.m_Property.m_Value.m_Hash;
            }
        }
    }

    static void SceneGraphToLua(lua_State* L, const dmVMath::Matrix4& view_proj, const dmVMath::Matrix4& gui_view_proj,
                                    dmGameObject::SceneNode* node)
    {
        static dmhash_t hash_id = dmHashString64("id");

        bool clickable = false;
        bool isguinode = false;
        dmhash_t name = 0;
        GetNodeInfo(node, name, clickable, isguinode);

        lua_pushstring(L, dmHashReverseSafe64(name));
        lua_setfield(L, -2, "name");

        lua_pushstring(L, "payload");
        lua_newtable(L);

            bool visible = IsVisible(node);

            lua_pushboolean(L, visible);
            lua_setfield(L, -2, "visible");

            lua_pushboolean(L, clickable);
            lua_setfield(L, -2, "clickable");

            // add a default position value
            lua_pushstring(L, "pos");
            lua_newtable(L);
                for (int v = 0; v < 3; ++v)
                {
                    lua_pushnumber(L, 0);
                    lua_rawseti (L, -2, v+1);
                }
            lua_settable(L, -3);

            // add a default anchorPoint value (top left corner of the items' area)
            lua_pushstring(L, "anchorPoint");
            lua_newtable(L);
                for (int v = 0; v < 2; ++v)
                {
                    //lua_pushnumber(L, 0);
                    lua_pushnumber(L, 0.5f);
                    lua_rawseti (L, -2, v+1);
                }
            lua_settable(L, -3);

            dmGameObject::SceneNodePropertyIterator pit = TraverseIterateProperties(node);
            while(dmGameObject::TraverseIteratePropertiesNext(&pit))
            {
                OutputProperty(L, isguinode ? gui_view_proj : view_proj, &pit.m_Property);
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

                    SceneGraphToLua(L, view_proj, gui_view_proj, &it.m_Node);

                lua_settable(L, -3);
            }

        lua_settable(L, -3);

    }

    // Follow the structure from
    // https://poco-chinese.readthedocs.io/en/latest/source/poco.sdk.AbstractDumper.html?highlight=abstractdumper#poco.sdk.AbstractDumper.IDumper.dumpHierarchy
    void DumpToLuaTable(lua_State* L, dmGameObject::HRegister regist,
                                    const dmVMath::Matrix4& view_proj, const dmVMath::Matrix4& gui_view_proj)
    {
        DM_LUA_STACK_CHECK(L, 1);

        dmGameObject::SceneNode root;
        if (!dmGameObject::TraverseGetRoot(regist, &root))
        {
            lua_pushnil(L);
            return;
        }

        lua_newtable(L);
        dmVMath::Matrix4 identity = dmVMath::Matrix4::identity();
        SceneGraphToLua(L, view_proj, gui_view_proj, &root);
    }
}