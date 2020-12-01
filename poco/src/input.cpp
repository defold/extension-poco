#include "input.h"
#include <dmsdk/dlib/array.h>
#include <stdio.h>

namespace dmPoco
{

struct InputEvent
{
    float   m_X1;
    float   m_Y1;
    float   m_X2;
    float   m_Y2;
    float   m_Elapsed;
    float   m_Duration;
};

dmArray<InputEvent> g_InputEvents;


#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_SWITCH)
static void DoTouch(dmHID::HTouchDevice device, int32_t x, int32_t y, bool pressed, bool released)
{
    uint32_t touch_id = 0;
    dmHID::Phase phase = dmHID::PHASE_MOVED;
    if (pressed)
        phase = dmHID::PHASE_BEGAN;
    if (released)
        phase = dmHID::PHASE_ENDED;
    dmHID::AddTouch(device, x, y, touch_id, phase);
}
#else
static void DoTouch(dmHID::HMouse mouse, int32_t x, int32_t y, bool pressed, bool released)
{
    printf("mouse: %d %d\n", x, y);
    dmHID::SetMouseButton(mouse, dmHID::MOUSE_BUTTON_LEFT, true);
    dmHID::SetMousePosition(mouse, x, y);
}
#endif


static void AddInput(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float duration)
{
    if (g_InputEvents.Full())
        g_InputEvents.OffsetCapacity(8);

    InputEvent ev;
    ev.m_X1 = x1;
    ev.m_Y1 = y1;
    ev.m_X2 = x2;
    ev.m_Y2 = y2;
    ev.m_Duration = duration;
    ev.m_Elapsed = 0;
    g_InputEvents.Push(ev);
}


void Click(int32_t x, int32_t y)
{
    printf("Click %d %d\n", x, y);
    AddInput(x, y, x, y, 0);
}

void LongClick(int32_t x, int32_t y, float duration)
{
    printf("Long Click %d %d  %f\n", x, y, duration);
    AddInput(x, y, x, y, duration);
}

void Swipe(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float duration)
{
    printf("Swipe %d %d -> %d %d  %f\n", x1, y1, x2, y2, duration);
    AddInput(x1, y1, x2, y2, duration);
}

void UpdateInput(dmHID::HContext context, float dt)
{

#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_SWITCH)
    dmHID::HTouchDevice device = dmHID::GetTouchDevice(context, 0);
#else
    dmHID::HMouse device = dmHID::GetMouse(context, 0);
#endif

    for (uint32_t i = 0; i < g_InputEvents.Size(); ++i)
    {
        InputEvent& ev = g_InputEvents[i];

        bool pressed = ev.m_Elapsed == 0;
        ev.m_Elapsed += dt;

        float t = ev.m_Duration > 0 ? ev.m_Elapsed / ev.m_Duration : 0;
        float x = ev.m_X1 + (ev.m_X2 - ev.m_X1) * t;
        float y = ev.m_Y1 + (ev.m_Y2 - ev.m_Y1) * t;

        bool released = ev.m_Elapsed >= ev.m_Duration;
        if (ev.m_Elapsed >= ev.m_Duration)
            g_InputEvents.EraseSwap(i);

        DoTouch(device, (int32_t)x, (int32_t)y, pressed, released);

        printf("update: %d  duration: %f  elapsed: %f\n", i, ev.m_Duration, ev.m_Elapsed);
    }
}

}