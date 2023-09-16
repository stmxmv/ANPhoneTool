//
// Created by aojoie on 9/13/2023.
//

#pragma once

class KeyboardHandler {

public:

    /// up down left right correspond to 0 1 2 3
    void sendArrowKey(int id);

    [[deprecated]]
    bool isUserEditing();

    void sendPaste();
};

KeyboardHandler &GetKeyboardHandler();
