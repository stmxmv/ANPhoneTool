//
// Created by aojoie on 9/13/2023.
//

#include "KeyboardHandler.h"
#include "Utilities/Log.h"

#ifdef _WIN32
#include <objbase.h>
#include <Windows.h>
#include <oleacc.h>

#pragma comment(lib, "Oleacc.lib")

#endif

void KeyboardHandler::sendArrowKey(int id)
{
    INPUT input[2];
    ZeroMemory(input, sizeof(input));

    input[0].type = INPUT_KEYBOARD;

    switch (id)
    {
        case 0:
            input[0].ki.wVk = VK_UP;
            break;
        case 1:
            input[0].ki.wVk = VK_DOWN;
            break;
        case 2:
            input[0].ki.wVk = VK_LEFT;
            break;
        case 3:
            input[0].ki.wVk = VK_RIGHT;
            break;
        default:
            return;
    }

    input[1] = input[0];
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT uSent = SendInput(std::size(input), input, sizeof(INPUT));
    if (uSent != std::size(input))
    {
        AN_LOG(Error, "Send Keyboard Fail");
    }
}

void KeyboardHandler::sendPaste()
{
    INPUT input[4];
    ZeroMemory(&input, sizeof(input));

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = VK_LCONTROL;

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 'V';

    input[2].type = INPUT_KEYBOARD;
    input[2].ki.wVk = VK_LCONTROL;
    input[2].ki.dwFlags = KEYEVENTF_KEYUP;

    input[3].type = INPUT_KEYBOARD;
    input[3 ].ki.wVk = 'V';
    input[3].ki.dwFlags = KEYEVENTF_KEYUP;

    UINT uSent = SendInput(std::size(input), input, sizeof(INPUT));
    if (uSent != std::size(input))
    {
        AN_LOG(Error, "Send paste key Fail");
    }
}

bool KeyboardHandler::isUserEditing()
{
    HWND hwndFocused = GetFocus();
    // Initialize an Accessibility Object
    IAccessible* pAccessible;
    if (AccessibleObjectFromWindow(hwndFocused, OBJID_CLIENT, IID_IAccessible, (void**)&pAccessible) == S_OK) {
        VARIANT varRole;
        VARIANT varChild{};
        if (pAccessible->get_accState(varChild, &varRole) == S_OK) {
            if ((varRole.vt == VT_I4) && (varRole.lVal == ROLE_SYSTEM_TEXT)) {
                return true;
            }
        }

        // Release COM resources
        pAccessible->Release();
    }
    return false;
}


KeyboardHandler &GetKeyboardHandler()
{
    static KeyboardHandler handler;
    return handler;
}
