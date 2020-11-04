#include "json.h"
#include <dmsdk/dlib/log.h>
#include <dmsdk/dlib/dstrings.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <float.h> // DBL_MANT_DIG DBL_MIN_EXP

namespace dmJson
{

/*
     * @name Node
     * @member m_Type [type:Type] Node type
     * @member m_Start [type:int] Start index inclusive into document json-data
     * @member m_End [type:int] End index exclusive into document json-data
     * @member m_Size [type:int] Size. Only applicable for arrays and objects
     * @member m_Sibling [type:int] Sibling index. -1 if no sibling
    struct Node
    {
        Type    m_Type;
        int     m_Start;
        int     m_End;
        int     m_Size;
        int     m_Sibling;
    };

    # Json document
     * Holds a full json document
     *
     * @struct
     * @name Document
     * @member m_Nodes [type:Node*] Array of nodes. First node is root
     * @member m_NodeCount [type:int] Total number of nodes
     * @member m_Json [type:char*] Json-data (unescaped)
     * @member m_UserData [type:void*] User-data
     *
    struct Document
    {
        Node*   m_Nodes;
        int     m_NodeCount;
        char*   m_Json;
        void*   m_UserData;
    };
*/

const dmJson::Node* GetRootNode(const dmJson::Document* doc)
{
    return doc ? &doc->m_Nodes[0] : 0;
}


// Get's the value node of associated with the name, from the list of children of the "parent" node
const dmJson::Node* GetNode(const dmJson::Document* doc, const dmJson::Node* parent, const char* name)
{
    // Gets the text value associate with the node.
    int nodeindex = (int)(parent - &doc->m_Nodes[0]);
    int size = parent->m_Size;

    // {1 2 3} is a valid object according to the jsmn parser, we need
    // to protect against this to avoid reading random memory.
    if ((size % 2) == 0)
    {
        int sibling = nodeindex + 1;
        for (int i = 0; i < size; i += 2)
        {
            dmJson::Node* child = &doc->m_Nodes[sibling];

            uint32_t childnamelen = child->m_End - child->m_Start;
            const char* childname = doc->m_Json + child->m_Start;
            if (strncmp(childname, name, childnamelen) == 0)
            {
                return child + 1;
            }

            sibling = child->m_Sibling;
        }
    }
    return 0;
}

const char* GetString(const dmJson::Document* doc, const dmJson::Node* node, uint32_t* size)
{
    if (node->m_Type == dmJson::TYPE_STRING)
    {
        *size = (uint32_t)(node->m_End - node->m_Start);
        return doc->m_Json + node->m_Start;
    }
    return 0;
}

int DebugPrint(dmJson::Document* doc, int index)
{
    // The maximum length of a IEEE 754 double (+ \0)
    const uint32_t buffer_len = 3 + DBL_MANT_DIG - DBL_MIN_EXP + 1;

    if (index >= doc->m_NodeCount)
    {
        dmLogError("Unexpected JSON index, unable to parse content.");
        return -1;
    }

    const dmJson::Node& n = doc->m_Nodes[index];
    const char* json = doc->m_Json;
    uint32_t l = n.m_End - n.m_Start;
    switch (n.m_Type)
    {
    case dmJson::TYPE_PRIMITIVE:
        if (l == 4 && memcmp(json + n.m_Start, "null", 4) == 0)
        {
            printf("null");
        }
        else if (l == 4 && memcmp(json + n.m_Start, "true", 4) == 0)
        {
            printf("true");
        }
        else if (l == 5 && memcmp(json + n.m_Start, "false", 5) == 0)
        {
            printf("false");
        }
        else
        {
            char buffer[buffer_len] = { 0 };
            memcpy(buffer, json + n.m_Start, dmMath::Min(buffer_len - 1, l));

            uint32_t bytes_read = 0;
            double value = 0.0f;
            int result = sscanf(buffer, "%lf%n", &value, &bytes_read);

#if defined(__NX__)
            if (result == 1 && bytes_read == l+1 && value == 0)
            {
                // for some reason, if the value happens to be a 0, or -0 it seems the sscanf code is stepping
                // one character too far. However, as long as the result is ok,
                // and since the last character is a \0 , we'll let it slide
                bytes_read--;
            }
#endif
            if (result == 1 && bytes_read == dmMath::Min(buffer_len - 1, l))
            {
                printf("\"%lf\"", value);
            }
            else
            {
                dmLogError("Invalid JSON primitive: %s", buffer);
                return -1;
            }
        }
        return index + 1;

    case dmJson::TYPE_STRING:
        printf("\"%.*s\"", l, json + n.m_Start);
        return index + 1;

    case dmJson::TYPE_ARRAY:
        printf("[");
        ++index;
        for (int i = 0; i < n.m_Size; ++i)
        {
            index = DebugPrint(doc, index);
            if (index < 0)
                return -1;
            if (i != n.m_Size-1)
                printf(", ");
        }
        printf("]");
        return index;

    case dmJson::TYPE_OBJECT:
        // {1 2 3} is a valid object according to the jsmn parser, we need
        // to protect against this to avoid reading random memory.
        if ((n.m_Size % 2) == 0)
        {
            printf("{\n");
            ++index;
            for (int i = 0; i < n.m_Size; i += 2)
            {
                index = DebugPrint(doc, index);
                if (index < 0)
                    return -1;

                printf(": ");
                index = DebugPrint(doc, index);
                if (index < 0)
                    return -1;
                if (i != n.m_Size-1)
                    printf(", ");
                printf("\n");
            }

            printf("}\n");
            return index;
        }
        else
        {
            char buffer[buffer_len] = { 0 };
            memcpy(buffer, json + n.m_Start, dmMath::Min(buffer_len - 1, l));
            dmLogError("Incomplete JSON object: %s", buffer);
            return -1;
        }
    }

    dmLogError("Unsupported JSON type (%d), unable to parse content.", n.m_Type);
    return -1;
}

} // namespace
