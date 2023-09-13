//
// Created by aojoie on 9/13/2023.
//

#include "KeyboardHandler.h"
#include "Utilities/Log.h"

#include <Windows.h>

void KeyboardHandler::sendErrorKey(int id)
{
    INPUT input;
    ZeroMemory(&input, sizeof(input));

    input.type = INPUT_KEYBOARD;

    switch (id)
    {
        case 0:
            input.ki.wVk = VK_UP;
            break;
        case 1:
            input.ki.wVk = VK_DOWN;
            break;
        case 2:
            input.ki.wVk = VK_LEFT;
            break;
        case 3:
            input.ki.wVk = VK_RIGHT;
            break;
        default:
            return;
    }

    UINT uSent = SendInput(1, &input, sizeof(INPUT));
    if (uSent != 1)
    {
        AN_LOG(Error, "Send Keyboard Fail");
    }
}


KeyboardHandler &GetKeyboardHandler()
{
    static KeyboardHandler handler;
    return handler;
}
