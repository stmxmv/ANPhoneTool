//
// Created by aojoie on 9/12/2023.
//

#pragma once

#include <QEvent>

class ANPhoneEvent : public QEvent
{
public:
    enum Type {
        DeviceSocket = QEvent::User + 1,
        Control,
    };

    ANPhoneEvent(Type type) : QEvent(QEvent::Type(type)){}
};

// DeviceSocketEvent
class DeviceSocketEvent : public ANPhoneEvent
{
public:
    DeviceSocketEvent() : ANPhoneEvent(DeviceSocket){}
};

