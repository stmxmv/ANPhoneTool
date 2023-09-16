//
// Created by aojoie on 9/16/2023.
//

#pragma once

#include "wintoastlib.h"

#include <QEventLoop>
#include <QObject>

#include <thread>



class UserNotificationCenter : public QObject {

    Q_OBJECT

    QEventLoop *volatile  m_EventLoop;
    std::thread m_Thread;

public:
    [[deprecated("this class need to be reimplement using current queue")]]
    explicit UserNotificationCenter(QObject *parent = Q_NULLPTR);

    ~UserNotificationCenter() override;

    void showToast(_In_ WinToastLib::WinToastTemplate const& toast, _In_ WinToastLib::IWinToastHandler* eventHandler,
                    _Out_opt_ WinToastLib::WinToast::WinToastError* error = nullptr);
};

UserNotificationCenter &GetUserNotificationCenter();