#include "windows_window.h"
#include "math.h"
#include <Windows.h>

static Key key_from_windows_key_code(WPARAM key, LPARAM flags)
{
    switch (key)
    {
        case VK_SHIFT:
        {
            auto left_shift = MapVirtualKeyW(VK_LSHIFT, MAPVK_VK_TO_VSC);
            auto shift_check_code = (flags & (0xFF << 16)) >> 16;
            return shift_check_code == left_shift ? Key::LShift : Key::RShift;
        }

        case VK_MENU:       return (HIWORD(flags) & KF_EXTENDED) ? Key::RAlt : Key::LAlt;
        case VK_CONTROL:    return (HIWORD(flags) & KF_EXTENDED) ? Key::RControl : Key::LControl;
        case VK_LWIN:       return Key::LSystem;
        case VK_RWIN:       return Key::RSystem;
        case VK_APPS:       return Key::Menu;
        case VK_OEM_1:      return Key::SemiColon;
        case VK_OEM_2:      return Key::Slash;
        case VK_OEM_PLUS:   return Key::Equal;
        case VK_OEM_MINUS:  return Key::Dash;
        case VK_OEM_4:      return Key::LBracket;
        case VK_OEM_6:      return Key::RBracket;
        case VK_OEM_COMMA:  return Key::Comma;
        case VK_OEM_PERIOD: return Key::Period;
        case VK_OEM_7:      return Key::Quote;
        case VK_OEM_5:      return Key::BackSlash;
        case VK_OEM_3:      return Key::Tilde;
        case VK_ESCAPE:     return Key::Escape;
        case VK_SPACE:      return Key::Space;
        case VK_RETURN:     return Key::Return;
        case VK_BACK:       return Key::BackSpace;
        case VK_TAB:        return Key::Tab;
        case VK_PRIOR:      return Key::PageUp;
        case VK_NEXT:       return Key::PageDown;
        case VK_END:        return Key::End;
        case VK_HOME:       return Key::Home;
        case VK_INSERT:     return Key::Insert;
        case VK_DELETE:     return Key::Delete;
        case VK_ADD:        return Key::Add;
        case VK_SUBTRACT:   return Key::Subtract;
        case VK_MULTIPLY:   return Key::Multiply;
        case VK_DIVIDE:     return Key::Divide;
        case VK_PAUSE:      return Key::Pause;
        case VK_F1:         return Key::F1;
        case VK_F2:         return Key::F2;
        case VK_F3:         return Key::F3;
        case VK_F4:         return Key::F4;
        case VK_F5:         return Key::F5;
        case VK_F6:         return Key::F6;
        case VK_F7:         return Key::F7;
        case VK_F8:         return Key::F8;
        case VK_F9:         return Key::F9;
        case VK_F10:        return Key::F10;
        case VK_F11:        return Key::F11;
        case VK_F12:        return Key::F12;
        case VK_F13:        return Key::F13;
        case VK_F14:        return Key::F14;
        case VK_F15:        return Key::F15;
        case VK_LEFT:       return Key::Left;
        case VK_RIGHT:      return Key::Right;
        case VK_UP:         return Key::Up;
        case VK_DOWN:       return Key::Down;
        case VK_NUMPAD0:    return Key::Numpad0;
        case VK_NUMPAD1:    return Key::Numpad1;
        case VK_NUMPAD2:    return Key::Numpad2;
        case VK_NUMPAD3:    return Key::Numpad3;
        case VK_NUMPAD4:    return Key::Numpad4;
        case VK_NUMPAD5:    return Key::Numpad5;
        case VK_NUMPAD6:    return Key::Numpad6;
        case VK_NUMPAD7:    return Key::Numpad7;
        case VK_NUMPAD8:    return Key::Numpad8;
        case VK_NUMPAD9:    return Key::Numpad9;
        case 'A':           return Key::A;
        case 'Z':           return Key::Z;
        case 'E':           return Key::E;
        case 'R':           return Key::R;
        case 'T':           return Key::T;
        case 'Y':           return Key::Y;
        case 'U':           return Key::U;
        case 'I':           return Key::I;
        case 'O':           return Key::O;
        case 'P':           return Key::P;
        case 'Q':           return Key::Q;
        case 'S':           return Key::S;
        case 'D':           return Key::D;
        case 'F':           return Key::F;
        case 'G':           return Key::G;
        case 'H':           return Key::H;
        case 'J':           return Key::J;
        case 'K':           return Key::K;
        case 'L':           return Key::L;
        case 'M':           return Key::M;
        case 'W':           return Key::W;
        case 'X':           return Key::X;
        case 'C':           return Key::C;
        case 'V':           return Key::V;
        case 'B':           return Key::B;
        case 'N':           return Key::N;
        case '0':           return Key::Num0;
        case '1':           return Key::Num1;
        case '2':           return Key::Num2;
        case '3':           return Key::Num3;
        case '4':           return Key::Num4;
        case '5':           return Key::Num5;
        case '6':           return Key::Num6;
        case '7':           return Key::Num7;
        case '8':           return Key::Num8;
        case '9':           return Key::Num9;
    }

    return Key::Unknown;
}

static LRESULT window_proc(HWND window_handle, UINT message, WPARAM wparam, LPARAM lparam)
{
    WindowsWindow* window = (WindowsWindow*)GetWindowLongPtr(window_handle, GWLP_USERDATA);

    if (window == nullptr)
        return DefWindowProc(window_handle, message, wparam, lparam);

    WindowState* state = &window->state;

    switch(message)
    {
    case WM_QUIT:
    case WM_CLOSE:
        state->closed = true;
        return 0;
    case WM_KEYDOWN:
        if (state->key_pressed_callback != nullptr)
        {
            state->key_pressed_callback(key_from_windows_key_code(wparam, lparam));
        }
        return 0;
    case WM_KEYUP:
        if (state->key_released_callback != nullptr)
        {
            state->key_released_callback(key_from_windows_key_code(wparam, lparam));
        }
        return 0;
    case WM_INPUT:
        {
            unsigned size;
            GetRawInputData((HRAWINPUT)lparam, RID_INPUT, NULL, &size, sizeof(RAWINPUTHEADER));
            unsigned char* lpb = new unsigned char[size];

            if (lpb == nullptr)
                return 0;

            if (GetRawInputData((HRAWINPUT)lparam, RID_INPUT, lpb, &size, sizeof(RAWINPUTHEADER)) != size)
                return 0;

            RAWINPUT* raw = (RAWINPUT*)lpb;

            if (raw->header.dwType == RIM_TYPEMOUSE && state->mouse_moved_callback && (raw->data.mouse.lLastX != 0 || raw->data.mouse.lLastY != 0))
                state->mouse_moved_callback({raw->data.mouse.lLastX, raw->data.mouse.lLastY});

            return 0;
        }
    }

    return DefWindowProc(window_handle, message, wparam, lparam);
}

void windows_create_window(WindowsWindow* w, const char* title, unsigned width, unsigned height)
{
    HINSTANCE instance_handle = GetModuleHandle(nullptr);
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance_handle;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = title;

    RegisterClassEx(&wc);
    RECT window_rect = {0, 0, (int)width, (int)height};
    AdjustWindowRect(&window_rect, WS_OVERLAPPEDWINDOW, false);
    HWND handle = CreateWindowEx(
        0,
        title,
        title,
        WS_OVERLAPPEDWINDOW,
        300,
        300,
        window_rect.right - window_rect.left,
        window_rect.bottom - window_rect.top,
        nullptr,
        nullptr,
        instance_handle,
        nullptr);
    w->handle = handle;
    ShowWindow(handle, true);
    SetWindowLongPtr(handle, GWLP_USERDATA, (LONG_PTR)w);
    RAWINPUTDEVICE rid = {};
    rid.usUsagePage = 0x01; 
    rid.usUsage = 0x02; 
    rid.hwndTarget = 0;
    RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE));
}

void windows_process_all_window_messsages()
{
    MSG message;
    while(PeekMessage(&message, nullptr, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
}
