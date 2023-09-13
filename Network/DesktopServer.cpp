//
// Created by aojoie on 9/13/2023.
//

#include "Utilities/Log.h"
#include "Core/KeyboardHandler.h"
#include "DesktopServer.h"
#include "DeviceSocket.h"

#include <QLabel>
#include <QTimer>
#include <QCursor>

static uint16_t CONTROL_PORT = 13131;
static uint16_t DATA_PORT = 13132;


void DesktopControlServer::incomingConnection(qintptr handle)
{
    DeviceControlSocket *controlSocket = new DeviceControlSocket(this);
    controlSocket->setSocketDescriptor(handle);
    addPendingConnection(controlSocket);
}

void DesktopDataServer::incomingConnection(qintptr handle)
{
    DeviceDataSocket *dataSocket = new DeviceDataSocket(this);
    dataSocket->setSocketDescriptor(handle);
    addPendingConnection(dataSocket);
}

DesktopServer::DesktopServer(QObject *parent) : QObject(parent) {
    if (!m_ControlServer.listen(QHostAddress::Any, CONTROL_PORT))
    {
        qCritical(QString("Could not listen on port %1").arg(CONTROL_PORT).toStdString().c_str());
    }

    AN_LOG(Info, "Server start listen at port %hd", CONTROL_PORT);

    if (!m_DataServer.listen(QHostAddress::Any, DATA_PORT))
    {
        qCritical(QString("Could not listen on port %1").arg(DATA_PORT).toStdString().c_str());
    }

    AN_LOG(Info, "Server start listen at port %hd", DATA_PORT);

    connect(&m_ControlServer, &QTcpServer::newConnection, this, [this]()
            {
                QTcpSocket *socket = m_ControlServer.nextPendingConnection();
                if (socket == nullptr) return;

                AN_LOG(Info, "Desktop Control socket connect to %s", socket->peerAddress().toString().toStdString().c_str());
                DeviceControlSocket *controlSocket = (DeviceControlSocket *)socket;
                QObject::connect(controlSocket, &DeviceControlSocket::OnReadMessage, this, &DesktopServer::OnReadMessage);

                QObject::connect(socket, &QTcpSocket::disconnected, this, [socket]()
                                {
                                     AN_LOG(Info, "Desktop Control socket disconnected to %s", socket->peerAddress().toString().toStdString().c_str());
                                    socket->deleteLater();
                                });

                QObject::connect(socket, &QTcpSocket::aboutToClose, this, [socket]()
                                {
                                     AN_LOG(Info, "Desktop Control socket about to close to %s", socket->peerAddress().toString().toStdString().c_str());
                                    socket->deleteLater();
                                });

            });

    connect(&m_DataServer, &QTcpServer::newConnection, this, [this]()
            {
                QTcpSocket *socket = m_DataServer.nextPendingConnection();
                if (socket == nullptr) return;

                AN_LOG(Info, "Desktop Data socket connect to %s", socket->peerAddress().toString().toStdString().c_str());
                DeviceDataSocket *controlSocket = (DeviceDataSocket *)socket;
//                QObject::connect(controlSocket, &DeviceDataSocket::OnReadMessage, this, &DesktopServer::OnReadMessage);

                QObject::connect(socket, &QTcpSocket::disconnected, this, [socket]()
                                 {
                                     AN_LOG(Info, "Desktop Data socket disconnected to %s", socket->peerAddress().toString().toStdString().c_str());
                                     socket->deleteLater();
                                 });

                QObject::connect(socket, &QTcpSocket::aboutToClose, this, [socket]()
                                 {
                                     AN_LOG(Info, "Desktop Data socket about to close to %s", socket->peerAddress().toString().toStdString().c_str());
                                     socket->deleteLater();
                                 });

            });

}

void DesktopServer::OnReadMessage(AN::DesktopMessage message)
{
    AN_LOG(Info, "Receive message type %s", AN::DesktopMessageType_Name(message.type()).c_str());
//    AN_LOG(Info, "Receive message data %s", message.data().c_str());

    switch (message.type())
    {
        case AN::kDesktopMessageNone:
            break;
        case AN::kDesktopMessageArrowKey:
        {
            AN::ArrowKey *key = (AN::ArrowKey *) message.data().data();
            GetKeyboardHandler().sendErrorKey((int) *key);
        }
        break;

        case AN::kDesktopMessageSendText:
        {
            // data should be text string
            QLabel* label = new QLabel();
            label->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen | Qt::FramelessWindowHint);
            label->setAttribute(Qt::WA_ShowWithoutActivating);
            label->setAttribute(Qt::WA_TranslucentBackground);
            //    label->setGeometry(320, 200, 750, 360);
            label->setAlignment(Qt::AlignCenter);
            label->setStyleSheet("font-size: 25pt; border: none;  background-color: rgba(0, 0, 0, 0); color:rgb(255,255,0)");
            label->setText(message.data().c_str());

            QPoint mousePos = QCursor::pos();
            int widgetWidth = label->width();
            int widgetHeight = label->height();
            int x = mousePos.x() - widgetWidth / 2;
            int y = mousePos.y() - widgetHeight / 2;

            // Set the widget's position
            label->setGeometry(x, y, widgetWidth, widgetHeight);

            label->show();

            QTimer *timer = new QTimer();
            QObject::connect(timer, &QTimer::timeout, [=]()
                             {
                                 timer->deleteLater();
                                 label->hide();
                                 label->deleteLater();
                             });
            timer->setInterval(5000);
            timer->start();

            break;
        }

        default:
        {
            AN_LOG(Error, "Unknown client message type %d", message.type());
        }
    }
}


DesktopServer &GetDesktopServer()
{
    static DesktopServer desktopServer;
    return desktopServer;
}
