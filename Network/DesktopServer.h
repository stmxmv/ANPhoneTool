//
// Created by aojoie on 9/13/2023.
//

#pragma once

#include "DesktopMessage.pb.h"
#include "DeviceMessage.pb.h"

#include <Network/DeviceSocket.h>

#include <QEventLoop>
#include <QTcpServer>
#include <QThread>
#include <QByteArray>
#include <QFile>

#include <list>
#include <unordered_map>

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

    DeviceControlSocket *m_ControlSocket;
    DeviceDataSocket *m_DataSocket;

    struct ReceiveFileContext {
        AN::SendFileInfo info;
        QFile *file;
        size_t receivedSize;
        DeviceControlSocket *controlSocket;
        bool complete;
    };

    std::unordered_map<std::string, ReceiveFileContext> m_ReceiveFileMap;
    std::unordered_map<std::string, QString> m_SendFileMap;

    void OnReadMessage(DeviceControlSocket *socket, AN::DesktopMessage message);

    void OnReadData(const QByteArray &array);

public:

    explicit DesktopServer(QObject *parent = Q_NULLPTR);

    void sendMessage(QTcpSocket *socket, const google::protobuf::Message &message);

    void sendMessage(const google::protobuf::Message &message)
    {
        if (m_ControlSocket)
        {
            sendMessage(m_ControlSocket, message);
        }
    }

    void willReceiveFile(const AN::SendFileInfo &info, DeviceControlSocket *controlSocket);

    void sendFile(const QString &filePath);
};

class DesktopServerThread : public QThread
{
    Q_OBJECT

    QEventLoop    *m_EventLoop;
    DesktopServer *m_Server;

public:

    using QThread::QThread;

protected:

    virtual void run() override;

public:

    DesktopServer *getServer() { return m_Server; }
    void stopServer();

};

/// get global instance
DesktopServer &GetDesktopServer(); // need to start the thread first, otherwise null pointer dereference
DesktopServerThread &GetDesktopServerThread();
