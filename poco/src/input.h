#pragma once

#include <dmsdk/dlib/hash.h>
#include <dmsdk/hid/hid.h>

namespace dmPoco
{
    void InitInput();

    void Click(dmHID::MouseButton button, int32_t x, int32_t y);
    void LongClick(int32_t x, int32_t y, float duration);
    void Swipe(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float duration);
    void KeyEvent(const char* keycodes);

    void UpdateInput(dmHID::HContext context, float dt);

    // private
    void InitInputTables();

    dmHID::Key HashToKey(dmhash_t key_name);
    dmHID::Key AsciiToKey(char c);
}