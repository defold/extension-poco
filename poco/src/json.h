#include <dmsdk/dlib/json.h>
#include <stdint.h>

// Until it's part of the dmSDK
namespace dmMath
{
    template <class T>
    const T Min(const T a, const T b)
    {
        return (a < b) ? a : b;
    }
}

namespace dmJson
{
    const dmJson::Node* GetRootNode(const dmJson::Document* doc);
    // Get's the value node of associated with the name, from the list of children of the "parent" node
    const dmJson::Node* GetNode(const dmJson::Document* doc, const dmJson::Node* parent, const char* name);
    // Gets the text value associate with the node.
    const char*         GetString(const dmJson::Document* doc, const dmJson::Node* node, uint32_t* size);

    int DebugPrint(dmJson::Document* doc, int index);
}

