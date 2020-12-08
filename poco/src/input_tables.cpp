#include "input.h"
#include <stdio.h>
#include <dmsdk/dlib/array.h>
#include <dmsdk/dlib/hash.h>


namespace dmPoco
{

struct SHashToKey
{
    dmhash_t    m_Hash;
    dmHID::Key  m_Key;
};

dmArray<SHashToKey> g_PocoHashToKey;


static void MapKeyHash(const char* const name, dmHID::Key key)
{
    if (g_PocoHashToKey.Full())
        g_PocoHashToKey.OffsetCapacity(8);

    SHashToKey s;
    s.m_Hash = dmHashString64(name);
    s.m_Key = key;
    g_PocoHashToKey.Push(s);
}

dmHID::Key HashToKey(dmhash_t key_name)
{
    uint32_t size = g_PocoHashToKey.Size();
    for (uint32_t i = 0; i < size; ++i)
    {
        const SHashToKey& key = g_PocoHashToKey[i];
        if (key.m_Hash == key_name)
        {
            return key.m_Key;
        }
    }
    return dmHID::MAX_KEY_COUNT;
}


void InitInputTables()
{
    if (g_PocoHashToKey.Empty())
    {
        g_PocoHashToKey.SetCapacity(dmHID::MAX_KEY_COUNT);

#define MAP_KEY_HASH(_NAME) MapKeyHash( #_NAME, dmHID:: _NAME)

        MAP_KEY_HASH(KEY_SPACE);
        MAP_KEY_HASH(KEY_EXCLAIM);
        MAP_KEY_HASH(KEY_QUOTEDBL);
        MAP_KEY_HASH(KEY_HASH);
        MAP_KEY_HASH(KEY_DOLLAR);
        MAP_KEY_HASH(KEY_AMPERSAND);
        MAP_KEY_HASH(KEY_QUOTE);
        MAP_KEY_HASH(KEY_LPAREN);
        MAP_KEY_HASH(KEY_RPAREN);
        MAP_KEY_HASH(KEY_ASTERISK);
        MAP_KEY_HASH(KEY_PLUS);
        MAP_KEY_HASH(KEY_COMMA);
        MAP_KEY_HASH(KEY_MINUS);
        MAP_KEY_HASH(KEY_PERIOD);
        MAP_KEY_HASH(KEY_SLASH);

        MAP_KEY_HASH(KEY_0);
        MAP_KEY_HASH(KEY_1);
        MAP_KEY_HASH(KEY_2);
        MAP_KEY_HASH(KEY_3);
        MAP_KEY_HASH(KEY_4);
        MAP_KEY_HASH(KEY_5);
        MAP_KEY_HASH(KEY_6);
        MAP_KEY_HASH(KEY_7);
        MAP_KEY_HASH(KEY_8);
        MAP_KEY_HASH(KEY_9);

        MAP_KEY_HASH(KEY_COLON);
        MAP_KEY_HASH(KEY_SEMICOLON);
        MAP_KEY_HASH(KEY_LESS);
        MAP_KEY_HASH(KEY_EQUALS);
        MAP_KEY_HASH(KEY_GREATER);
        MAP_KEY_HASH(KEY_QUESTION);
        MAP_KEY_HASH(KEY_AT);

        MAP_KEY_HASH(KEY_A);
        MAP_KEY_HASH(KEY_B);
        MAP_KEY_HASH(KEY_C);
        MAP_KEY_HASH(KEY_D);
        MAP_KEY_HASH(KEY_E);
        MAP_KEY_HASH(KEY_F);
        MAP_KEY_HASH(KEY_G);
        MAP_KEY_HASH(KEY_H);
        MAP_KEY_HASH(KEY_I);
        MAP_KEY_HASH(KEY_J);
        MAP_KEY_HASH(KEY_K);
        MAP_KEY_HASH(KEY_L);
        MAP_KEY_HASH(KEY_M);
        MAP_KEY_HASH(KEY_N);
        MAP_KEY_HASH(KEY_O);
        MAP_KEY_HASH(KEY_P);
        MAP_KEY_HASH(KEY_Q);
        MAP_KEY_HASH(KEY_R);
        MAP_KEY_HASH(KEY_S);
        MAP_KEY_HASH(KEY_T);
        MAP_KEY_HASH(KEY_U);
        MAP_KEY_HASH(KEY_V);
        MAP_KEY_HASH(KEY_W);
        MAP_KEY_HASH(KEY_X);
        MAP_KEY_HASH(KEY_Y);
        MAP_KEY_HASH(KEY_Z);

        MAP_KEY_HASH(KEY_LBRACKET);
        MAP_KEY_HASH(KEY_BACKSLASH);
        MAP_KEY_HASH(KEY_RBRACKET);
        MAP_KEY_HASH(KEY_CARET);
        MAP_KEY_HASH(KEY_UNDERSCORE);
        MAP_KEY_HASH(KEY_BACKQUOTE);

        MAP_KEY_HASH(KEY_LBRACE);
        MAP_KEY_HASH(KEY_PIPE);
        MAP_KEY_HASH(KEY_RBRACE);
        MAP_KEY_HASH(KEY_TILDE);

        MAP_KEY_HASH(KEY_ESC);
        MAP_KEY_HASH(KEY_F1);
        MAP_KEY_HASH(KEY_F2);
        MAP_KEY_HASH(KEY_F3);
        MAP_KEY_HASH(KEY_F4);
        MAP_KEY_HASH(KEY_F5);
        MAP_KEY_HASH(KEY_F6);
        MAP_KEY_HASH(KEY_F7);
        MAP_KEY_HASH(KEY_F8);
        MAP_KEY_HASH(KEY_F9);
        MAP_KEY_HASH(KEY_F10);
        MAP_KEY_HASH(KEY_F11);
        MAP_KEY_HASH(KEY_F12);
        MAP_KEY_HASH(KEY_UP);
        MAP_KEY_HASH(KEY_DOWN);
        MAP_KEY_HASH(KEY_LEFT);
        MAP_KEY_HASH(KEY_RIGHT);
        MAP_KEY_HASH(KEY_LSHIFT);
        MAP_KEY_HASH(KEY_RSHIFT);
        MAP_KEY_HASH(KEY_LCTRL);
        MAP_KEY_HASH(KEY_RCTRL);
        MAP_KEY_HASH(KEY_LALT);
        MAP_KEY_HASH(KEY_RALT);
        MAP_KEY_HASH(KEY_TAB);
        MAP_KEY_HASH(KEY_ENTER);
        MAP_KEY_HASH(KEY_BACKSPACE);
        MAP_KEY_HASH(KEY_INSERT);
        MAP_KEY_HASH(KEY_DEL);
        MAP_KEY_HASH(KEY_PAGEUP);
        MAP_KEY_HASH(KEY_PAGEDOWN);
        MAP_KEY_HASH(KEY_HOME);
        MAP_KEY_HASH(KEY_END);
        MAP_KEY_HASH(KEY_KP_0);
        MAP_KEY_HASH(KEY_KP_1);
        MAP_KEY_HASH(KEY_KP_2);
        MAP_KEY_HASH(KEY_KP_3);
        MAP_KEY_HASH(KEY_KP_4);
        MAP_KEY_HASH(KEY_KP_5);
        MAP_KEY_HASH(KEY_KP_6);
        MAP_KEY_HASH(KEY_KP_7);
        MAP_KEY_HASH(KEY_KP_8);
        MAP_KEY_HASH(KEY_KP_9);
        MAP_KEY_HASH(KEY_KP_DIVIDE);
        MAP_KEY_HASH(KEY_KP_MULTIPLY);
        MAP_KEY_HASH(KEY_KP_SUBTRACT);
        MAP_KEY_HASH(KEY_KP_ADD);
        MAP_KEY_HASH(KEY_KP_DECIMAL);
        MAP_KEY_HASH(KEY_KP_EQUAL);
        MAP_KEY_HASH(KEY_KP_ENTER);
        MAP_KEY_HASH(KEY_KP_NUM_LOCK);
        MAP_KEY_HASH(KEY_CAPS_LOCK);
        MAP_KEY_HASH(KEY_SCROLL_LOCK);
        MAP_KEY_HASH(KEY_PAUSE);
        MAP_KEY_HASH(KEY_LSUPER);
        MAP_KEY_HASH(KEY_RSUPER);
        MAP_KEY_HASH(KEY_MENU);
        MAP_KEY_HASH(KEY_BACK);

#undef MAP_KEY_HASH
    }
}

dmHID::Key AsciiToKey(char c)
{
    printf("AsciiToKey: c: '%c'  ", c);
    switch(c)
    {
    case dmHID::KEY_SPACE: // ' '
    case dmHID::KEY_EXCLAIM: // '!'
    case dmHID::KEY_QUOTEDBL: // '"'
    case dmHID::KEY_HASH: // '#'
    case dmHID::KEY_DOLLAR: // '$'
    case dmHID::KEY_AMPERSAND: // '&'
    case dmHID::KEY_QUOTE: // '\''
    case dmHID::KEY_LPAREN: // '('
    case dmHID::KEY_RPAREN: // ')'
    case dmHID::KEY_ASTERISK: // '*'
    case dmHID::KEY_PLUS: // '+'
    case dmHID::KEY_COMMA: // ','
    case dmHID::KEY_MINUS: // '-'
    case dmHID::KEY_PERIOD: // '.'
    case dmHID::KEY_SLASH: // '/'
    case dmHID::KEY_0: // '0'
    case dmHID::KEY_1: // '1'
    case dmHID::KEY_2: // '2'
    case dmHID::KEY_3: // '3'
    case dmHID::KEY_4: // '4'
    case dmHID::KEY_5: // '5'
    case dmHID::KEY_6: // '6'
    case dmHID::KEY_7: // '7'
    case dmHID::KEY_8: // '8'
    case dmHID::KEY_9: // '9'

    case dmHID::KEY_COLON: // ':'
    case dmHID::KEY_SEMICOLON: // ';'
    case dmHID::KEY_LESS: // '<'
    case dmHID::KEY_EQUALS: // '='
    case dmHID::KEY_GREATER: // '>'
    case dmHID::KEY_QUESTION: // '?'
    case dmHID::KEY_AT: // '@'

    case dmHID::KEY_A: // 'A'
    case dmHID::KEY_B: // 'B'
    case dmHID::KEY_C: // 'C'
    case dmHID::KEY_D: // 'D'
    case dmHID::KEY_E: // 'E'
    case dmHID::KEY_F: // 'F'
    case dmHID::KEY_G: // 'G'
    case dmHID::KEY_H: // 'H'
    case dmHID::KEY_I: // 'I'
    case dmHID::KEY_J: // 'J'
    case dmHID::KEY_K: // 'K'
    case dmHID::KEY_L: // 'L'
    case dmHID::KEY_M: // 'M'
    case dmHID::KEY_N: // 'N'
    case dmHID::KEY_O: // 'O'
    case dmHID::KEY_P: // 'P'
    case dmHID::KEY_Q: // 'Q'
    case dmHID::KEY_R: // 'R'
    case dmHID::KEY_S: // 'S'
    case dmHID::KEY_T: // 'T'
    case dmHID::KEY_U: // 'U'
    case dmHID::KEY_V: // 'V'
    case dmHID::KEY_W: // 'W'
    case dmHID::KEY_X: // 'X'
    case dmHID::KEY_Y: // 'Y'
    case dmHID::KEY_Z: // 'Z'

    case dmHID::KEY_LBRACKET: // '['
    case dmHID::KEY_BACKSLASH: // '\\'
    case dmHID::KEY_RBRACKET: // ']'
    case dmHID::KEY_CARET: // '^'
    case dmHID::KEY_UNDERSCORE: // '_'
    case dmHID::KEY_BACKQUOTE: // '`'

    case dmHID::KEY_LBRACE: // '{'
    case dmHID::KEY_PIPE: // '|'
    case dmHID::KEY_RBRACE: // '}'
    case dmHID::KEY_TILDE: // '~'
        break;
    default:
    printf("<not found>:  Char: %d 0x%x   dmHID::Key:  %d 0x%x\n", c, c, dmHID::KEY_H, dmHID::KEY_H);
        return dmHID::MAX_KEY_COUNT;
    }
    printf("%d\n", (dmHID::Key)c);
    return (dmHID::Key)c;
}

}