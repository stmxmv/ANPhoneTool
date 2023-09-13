//
// Created by aojoie on 9/13/2023.
//

#pragma once

#include "DesktopMessage.pb.h"

#include <QTcpServer>


class DesktopControlServer : public QTcpServer
{
    Q_OBJECT

public:
    using QTcpServer::QTcpServer;
    virtual void incomingConnection(qintptr handle);
};

class DesktopDataServer : public QTcpServer
{
    Q_OBJECT

public:
    using QTcpServer::QTcpServer;
    virtual void incomingConnection(qintptr handle);
};

class DesktopServer : public QObject
{
    Q_OBJECT

    DesktopControlServer m_ControlServer;
    DesktopDataServer m_DataServer;

    void OnReadMessage(AN::DesktopMessage message);

public:

    explicit DesktopServer(QObject *parent = Q_NULLPTR);

};

/// get global instance
DesktopServer &GetDesktopServer();
