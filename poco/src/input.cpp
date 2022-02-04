#include "input.h"
#include <dmsdk/dlib/array.h>
#include <dmsdk/dlib/hash.h>
#include <dmsdk/dlib/math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <dmsdk/dlib/log.h>

namespace dmPoco
{

enum InputEventType
{
    INPUT_EVENT_MOUSE,
    INPUT_EVENT_KEY
};

struct InputEvent
{
    InputEvent() {
        memset(this, 0, sizeof(*this));
    }
    // Mouse event
    float   m_X1;
    float   m_Y1;
    float   m_X2;
    float   m_Y2;
    float   m_Elapsed;
    float   m_Duration;
    int32_t m_MouseButton;

    // Key event
    const char* m_KeyCodes;
    uint32_t    m_KeyCodesLen;
    uint32_t    m_KeyCodesIndex;

    int32_t m_Type;
};

dmArray<InputEvent> g_PocoInputEvents;

void InitInput()
{
    InitInputTables();
}


static void DoTouch(dmHID::HMouse mouse, dmHID::HTouchDevice device, int32_t x, int32_t y, bool pressed, bool released)
{
    if (dmHID::INVALID_TOUCH_DEVICE_HANDLE != device)
    {
        uint32_t touch_id = 0;
        dmHID::Phase phase = dmHID::PHASE_MOVED;
        if (pressed)
            phase = dmHID::PHASE_BEGAN;
        if (released)
            phase = dmHID::PHASE_ENDED;
        dmHID::AddTouch(device, x, y, touch_id, phase);
    }

    // We also have a "fast path" for the "touch" action, which is implemented via the mouse button 1
    // See g_MouseEmulationTouch in android_init.c in Defold source
    if (dmHID::INVALID_MOUSE_HANDLE != mouse)
    {
        dmHID::SetMouseButton(mouse, dmHID::MOUSE_BUTTON_LEFT, released?false:true);
        dmHID::SetMousePosition(mouse, x, y);
    }
}

static void AddMouseInput(dmHID::MouseButton button, int32_t x1, int32_t y1, int32_t x2, int32_t y2, float duration)
{
    if (g_PocoInputEvents.Full())
        g_PocoInputEvents.OffsetCapacity(8);

    InputEvent ev;
    ev.m_Type = INPUT_EVENT_MOUSE;
    ev.m_MouseButton = button;
    ev.m_X1 = x1;
    ev.m_Y1 = y1;
    ev.m_X2 = x2;
    ev.m_Y2 = y2;
    ev.m_Duration = duration;
    ev.m_Elapsed = 0;
    g_PocoInputEvents.Push(ev);
}

static void AddKeyInput(const char* keycodes)
{
    if (!keycodes)
        return;
    int len = strlen(keycodes);
    if (!len)
        return;

    if (g_PocoInputEvents.Full())
        g_PocoInputEvents.OffsetCapacity(8);

    InputEvent ev;
    ev.m_Type = INPUT_EVENT_KEY;
    ev.m_KeyCodes = strdup(keycodes);
    ev.m_KeyCodesLen = len;
    ev.m_KeyCodesIndex = 0;
    g_PocoInputEvents.Push(ev);
}

static void ReleaseKeyInput(InputEvent& ev)
{
    if (ev.m_Type == INPUT_EVENT_KEY)
        free((void*)ev.m_KeyCodes);
    ev.m_KeyCodes = 0;
}


void Click(dmHID::MouseButton button, int32_t x, int32_t y)
{
    AddMouseInput(button, x, y, x, y, 0);
}

void LongClick(int32_t x, int32_t y, float duration)
{
    AddMouseInput(dmHID::MOUSE_BUTTON_LEFT, x, y, x, y, duration);
}

void Swipe(int32_t x1, int32_t y1, int32_t x2, int32_t y2, float duration)
{
    AddMouseInput(dmHID::MOUSE_BUTTON_LEFT, x1, y1, x2, y2, duration);
}

void KeyEvent(const char* keycodes)
{
    AddKeyInput(keycodes);
}

static bool UpdateMouseEvent(dmHID::HMouse mouse, dmHID::HTouchDevice device, float dt, InputEvent& ev)
{
    float t = ev.m_Duration > 0 ? ev.m_Elapsed / ev.m_Duration : 0.0f;
    t = dmMath::Clamp(t, 0.0f, 1.0f);
    float x = ev.m_X1 + (ev.m_X2 - ev.m_X1) * t;
    float y = ev.m_Y1 + (ev.m_Y2 - ev.m_Y1) * t;

    bool pressed = ev.m_Elapsed == 0;
    bool released = ev.m_Elapsed >= ev.m_Duration;
    ev.m_Elapsed += dt;

    // Since the action.released will be called automatically, it will also use the current mouse cursor
    // which may not be where we expect it to be
    // So we split it up into two frames
    if (pressed && released)
        released = false;

    DoTouch(mouse, device, (int32_t)x, (int32_t)y, pressed, released);

    return released;
}

// Notes about Airtest keyevents:
// https://airtest.doc.io.netease.com/en/IDEdocs/faq/3_api_faq/#3-keyevent

// Parse strings of the form "{KEY_F1}"
static const char* ParseKey(const char* str, uint32_t* len)
{
    if (*str != '{')
        return 0;
    const char* pend = str + *len;
    const char* c = str;
    while ((++c) < pend)
    {
        if (*c == '}')
        {
            break;
        }
    }

    if (c == pend || *c != '}')
        return 0;

    *len = c - (str+1);

    return *len ? str+1 : 0;
}


static bool UpdateKeyEvent(dmHID::HContext context, dmHID::HKeyboard keyboard, float dt, InputEvent& ev)
{
    char c = ev.m_KeyCodes[ev.m_KeyCodesIndex];
    uint32_t key_string_len = ev.m_KeyCodesLen - ev.m_KeyCodesIndex;
    const char* key_string = ParseKey(&ev.m_KeyCodes[ev.m_KeyCodesIndex], &key_string_len);

    dmHID::Key key = dmHID::MAX_KEY_COUNT;
    if (key_string)
    {
        dmhash_t key_hash = dmHashBuffer64((const void *)key_string, key_string_len);
        key = HashToKey(key_hash);

        if (key != dmHID::MAX_KEY_COUNT)
        {
            dmHID::SetKey(keyboard, key, true);
            ev.m_KeyCodesIndex += key_string_len + 2; // '{' + text + '}'
            return ev.m_KeyCodesIndex == ev.m_KeyCodesLen;
        }
    }

    ev.m_KeyCodesIndex++;
    dmHID::AddKeyboardChar(context, c);
    return ev.m_KeyCodesIndex == ev.m_KeyCodesLen;
}

void UpdateInput(dmHID::HContext context, float dt)
{
    dmHID::HMouse mouse = dmHID::GetMouse(context, 0);
    dmHID::HTouchDevice device = dmHID::INVALID_TOUCH_DEVICE_HANDLE;
    dmHID::HKeyboard keyboard = dmHID::GetKeyboard(context, 0);
#if defined(DM_PLATFORM_IOS) || defined(DM_PLATFORM_ANDROID) || defined(DM_PLATFORM_SWITCH)
    device = dmHID::GetTouchDevice(context, 0);
#endif

    for (uint32_t i = 0; i < g_PocoInputEvents.Size(); ++i)
    {
        InputEvent& ev = g_PocoInputEvents[i];

        bool released;
        switch(ev.m_Type)
        {
        case INPUT_EVENT_MOUSE: released = UpdateMouseEvent(mouse, device, dt, ev); break;
        case INPUT_EVENT_KEY:   released = UpdateKeyEvent(context, keyboard, dt, ev); break;
        }

        if (released)
            g_PocoInputEvents.EraseSwap(i);
    }
}

}