#pragma once

#include <dmsdk/hid/hid.h>

namespace dmPoco
{
    void Click(int32_t x, int32_t y);
    void LongClick(int32_t x, int32_t y, float duration);
    void Swipe(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float duration);

    void UpdateInput(dmHID::HContext context, float dt);
}