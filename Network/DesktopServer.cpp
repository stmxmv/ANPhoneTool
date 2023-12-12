//
// Created by aojoie on 9/13/2023.
//

#include "Utilities/Log.h"
#include "Core/KeyboardHandler.h"
#include "DesktopServer.h"
#include "DeviceSocket.h"
#include "wintoastlib.h"
#include "Core/UserNotificationCenter.h"

#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QApplication>
#include <QCursor>
#include <QLabel>
#include <QTimer>
#include <QFileInfo>
#include <QClipboard>
#include <QMessageBox>

#include <format>


#ifdef _WIN32
#include <Windows.h>
static void SetClipboardText(const char* text)
{
    if (!::OpenClipboard(NULL))
        return;
    const int wbuf_length = ::MultiByteToWideChar(CP_UTF8, 0, text, -1, NULL, 0);
    HGLOBAL wbuf_handle = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)wbuf_length * sizeof(WCHAR));
    if (wbuf_handle == NULL)
    {
        ::CloseClipboard();
        return;
    }
    WCHAR* wbuf_global = (WCHAR*)::GlobalLock(wbuf_handle);
    ::MultiByteToWideChar(CP_UTF8, 0, text, -1, wbuf_global, wbuf_length);
    ::GlobalUnlock(wbuf_handle);
    ::EmptyClipboard();
    if (::SetClipboardData(CF_UNICODETEXT, wbuf_handle) == NULL)
        ::GlobalFree(wbuf_handle);
    ::CloseClipboard();
}

#endif

constexpr static uint16_t CONTROL_PORT = 13131;
constexpr static uint16_t DATA_PORT = 13132;

constexpr static uint64_t BLOCK_SIZE = 1024 * 1024; /// 1MB

static int computeRawVarint32Size(int value) {
    if ((value & (0xffffffff <<  7)) == 0) {
        return 1;
    }
    if ((value & (0xffffffff << 14)) == 0) {
        return 2;
    }
    if ((value & (0xffffffff << 21)) == 0) {
        return 3;
    }
    if ((value & (0xffffffff << 28)) == 0) {
        return 4;
    }
    return 5;
}

static uint8_t *writeRawVarint32(uint8_t *out, uint32_t value) {
    while (true) {
        if ((value & ~0x7F) == 0) {
            *out++ = value;
            return out;
        } else {
            *out++ = (value & 0x7F) | 0x80;
            value >>= 7;
        }
    }
}

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

DesktopServer::DesktopServer(QObject *parent)
    : QObject(parent), m_ControlServer(), m_DataServer(), m_ControlSocket(), m_DataSocket()
{
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

                QObject::connect(socket, &QTcpSocket::disconnected, this, [socket, this]()
                                {
                                     AN_LOG(Info, "Desktop Control socket disconnected to %s", socket->peerAddress().toString().toStdString().c_str());
                                    socket->deleteLater();
                                    DoOnDisConnected();
                                });

                QObject::connect(socket, &QTcpSocket::aboutToClose, this, [socket, this]()
                                {
                                     AN_LOG(Info, "Desktop Control socket about to close to %s", socket->peerAddress().toString().toStdString().c_str());
                                    socket->deleteLater();
                                    DoOnDisConnected();
                                });

                m_ControlSocket = controlSocket;

                DoOnConnected();
            });

    connect(&m_DataServer, &QTcpServer::newConnection, this, [this]()
            {
                QTcpSocket *socket = m_DataServer.nextPendingConnection();
                if (socket == nullptr) return;

                AN_LOG(Info, "Desktop Data socket connect to %s", socket->peerAddress().toString().toStdString().c_str());
                DeviceDataSocket *dataSocket = (DeviceDataSocket *)socket;
                QObject::connect(dataSocket, &DeviceDataSocket::OnReadData, this, &DesktopServer::OnReadData);

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

                m_DataSocket = dataSocket;

                DoOnConnected();
            });

}

void DesktopServer::DoOnConnected()
{
    if (m_ControlSocket && m_DataSocket)
    {
        emit OnConnected();
    }
}

void DesktopServer::DoOnDisConnected()
{
    if (m_ControlSocket == nullptr || m_DataSocket == nullptr) return;
    m_ControlSocket = nullptr;
    m_DataSocket = nullptr;
    emit OnDisConnected();
}

void DesktopServer::sendMessage(QTcpSocket *socket, const google::protobuf::Message &message)
{
    std::string bytes = message.SerializeAsString();

    int size = bytes.size();
    int headerLen = computeRawVarint32Size(size);
    uint8_t header[10];
    writeRawVarint32(header, size);
    socket->write((const char *)header, headerLen);
    socket->write(bytes.data(), bytes.size());
}

void DesktopServer::OnReadMessage(DeviceControlSocket *socket, AN::DesktopMessage message)
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
            GetKeyboardHandler().sendArrowKey((int) *key);
        }
        break;

        case AN::kDesktopMessageSendText:
        {
//            if (GetKeyboardHandler().isUserEditing()) {
//                AN_LOG(Debug, "user is editing");
//            }

            SetClipboardText(message.data().c_str());
            GetKeyboardHandler().sendPaste();

            /// GUI must be created in ui thread
            QMetaObject::invokeMethod(
                    qApp, [=]()
                    {
                        // data should be text string
                        QLabel *label = new QLabel();
                        label->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::SplashScreen | Qt::FramelessWindowHint);
                        label->setAttribute(Qt::WA_ShowWithoutActivating);
                        label->setAttribute(Qt::WA_TranslucentBackground);
                        //    label->setGeometry(320, 200, 750, 360);
                        label->setAlignment(Qt::AlignCenter);
                        label->setStyleSheet("font-size: 25pt; border: none;  background-color: rgba(0, 0, 0, 0); color:rgb(255,255,0)");
                        label->setText(message.data().c_str());

                        QPoint mousePos     = QCursor::pos();
                        int    widgetWidth  = label->width();
                        int    widgetHeight = label->height();
                        int    x            = mousePos.x() - widgetWidth / 2;
                        int    y            = mousePos.y() - widgetHeight / 2;

                        // Set the widget's position
                        label->setGeometry(x, y, widgetWidth, widgetHeight);

                        label->show();

                        QTimer *timer = new QTimer();
                        QObject::connect(timer, &QTimer::timeout, [=]()
                                         {
                                 timer->deleteLater();
                                 label->hide();
                                 label->deleteLater(); });
                        timer->setInterval(5000);
                        timer->start(); },
                    Qt::QueuedConnection);

            break;
        }

        case AN::kDesktopMessageSendFile:
        {
            AN::SendFileInfo info;
            if (info.ParseFromArray(message.data().data(), message.data().size()))
            {
                AN_LOG(Info, "client send file name %s size %llu", info.filename().c_str(), info.filesize());
                willReceiveFile(info, socket);

                /// ack send file
                AN::DeviceMessage deviceMessage;
                deviceMessage.set_type(AN::kDeviceMessageAckSendFile);

                AN::AckSendFileInfo ack;
                ack.set_uuid(info.uuid().data(), info.uuid().size());
                ack.set_pos(0);// start position

                deviceMessage.set_data(ack.SerializeAsString());

                sendMessage(socket, deviceMessage);
            }
        }
        break;
        case AN::kDesktopMessageAckSendFile:
        {
            AN::AckSendFileInfo info;
            if (info.ParseFromArray(message.data().data(), message.data().size()))
            {
                AN::UUID uuid;
                memcpy(&uuid, info.uuid().data(), sizeof uuid);
                AN_LOG(Info, "client ack send file %s pos %llu", uuid.string().c_str(), info.pos());

                if (m_SendFileMap.contains(uuid.string()))
                {
                    const QString &filePath = m_SendFileMap[uuid.string()];
                    QFile file(filePath);

                    if (file.open(QIODevice::ReadOnly) && file.seek(info.pos()))
                    {
                        QByteArray buffer(BLOCK_SIZE + sizeof(DataHeader), 0);
                        qint64 fileRead = file.read(buffer.data() + sizeof(DataHeader), BLOCK_SIZE);

                        DataHeader *header = (DataHeader *)buffer.data();
                        header->uuid = uuid;
                        header->blockSize = min(fileRead, BLOCK_SIZE);

                        buffer.resize(header->blockSize + sizeof(DataHeader));

                        if (m_DataSocket)
                        {
                            qint64 written = m_DataSocket->write(buffer);

                            /// FIXME
                            assert(written == header->blockSize + sizeof(DataHeader));
                        }
                    }
                    else
                    {
                        /// TODO send error message
                    }
                }
            }
        }
        break;

        case AN::kDesktopMessageAckSendComplete:
        {
            AN::AckSendFileInfo info;
            if (info.ParseFromArray(message.data().data(), message.data().size()))
            {
                AN::UUID uuid;
                memcpy(&uuid, info.uuid().data(), sizeof uuid);

                if (m_SendFileMap.contains(uuid.string()))
                {
                    const QString &filePath = m_SendFileMap[uuid.string()];

                    AN_LOG(Info, "send file %s complete", filePath.toStdString().c_str());
                }
            }
        }
        break;

        default:
        {
            AN_LOG(Error, "Unknown client message type %d", message.type());
        }
    }
}

void DesktopServer::OnReadData(const QByteArray &bytes)
{
    DataHeader header = *(DataHeader *)bytes.constData();
    if (!m_ReceiveFileMap.contains(header.uuid.string()))
    {
        return;
    }

    ReceiveFileContext &context = m_ReceiveFileMap[header.uuid.string()];

    context.receivedSize += header.blockSize;
    context.file->write(bytes.constData() + sizeof header, header.blockSize);

    if (context.receivedSize >= context.info.filesize())
    {
        context.file->close();
        context.file->deleteLater();
        context.complete = true;

        /// notify we received complete
        AN::DeviceMessage deviceMessage;
        deviceMessage.set_type(AN::kDeviceMessageAckSendComplete);

        AN::AckSendFileInfo ack;
        ack.set_uuid(context.info.uuid().data(), context.info.uuid().size());
        ack.set_pos(context.receivedSize); // start position

        deviceMessage.set_data(ack.SerializeAsString());

        sendMessage(context.controlSocket, deviceMessage);

//        using namespace WinToastLib;
//        WinToastTemplate templ = WinToastTemplate(WinToastTemplate::Text01);
//        templ.setTextField(L"File Download Complete", WinToastTemplate::FirstLine);
//
//        class WinToastHandlerNone : public WinToastLib::IWinToastHandler {
//        public:
//            WinToastHandlerNone() {}
//            // Public interfaces
//            void toastActivated() const override {}
//            virtual void toastActivated(int actionIndex) const override {}
//            void toastDismissed(WinToastDismissalReason state) const override {}
//            void toastFailed() const override {}
//        } *handler = new WinToastHandlerNone();
//
////        WinToast::instance()->showToast(templ, handler);
//
//        GetUserNotificationCenter().showToast(templ, handler);


        QMetaObject::invokeMethod(qApp, []
                                  {
                                      QMessageBox messageBox;

                                      // Set the message text
                                      messageBox.setText("File Download Complete");

                                      // Set the window title (optional)
                                      messageBox.setWindowTitle("ANPhoneTool");

                                      // Add a standard OK button
                                      messageBox.addButton(QMessageBox::Ok);

                                      // Show the message box
                                      messageBox.exec();
                                  });

    }
    else
    {
        /// ack send file
        AN::DeviceMessage deviceMessage;
        deviceMessage.set_type(AN::kDeviceMessageAckSendFile);

        AN::AckSendFileInfo ack;
        ack.set_uuid(context.info.uuid().data(), context.info.uuid().size());
        ack.set_pos(context.receivedSize); // start position

        deviceMessage.set_data(ack.SerializeAsString());

        sendMessage(context.controlSocket, deviceMessage);
    }
}

void DesktopServer::willReceiveFile(const AN::SendFileInfo &info, DeviceControlSocket *controlSocket)
{
    ReceiveFileContext context{};
    context.info = info;

    QStringList downloadPaths = QStandardPaths::standardLocations(QStandardPaths::DownloadLocation);
    QString downloadFolder = downloadPaths.first();
    downloadFolder = downloadFolder + "/" + info.filename().c_str();

    context.file = new QFile(downloadFolder, this);
    context.controlSocket = controlSocket;

    if (!context.file->open(QIODeviceBase::WriteOnly))
    {
        AN_LOG(Error, "cannot create download file at path %s", downloadFolder.toStdString().c_str());
        context.file->deleteLater();
        context.file = nullptr;
        return;
    }
    else
    {
        AN_LOG(Info, "File will download at path %s", downloadFolder.toStdString().c_str());
    }

    AN::UUID uuid;
    memcpy(&uuid, info.uuid().data(), sizeof uuid);
    m_ReceiveFileMap[uuid.string()] = context;
}

void DesktopServer::sendFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    if (!fileInfo.exists())
    {
        return;
    }

    qint64 fileSize = fileInfo.size();

    AN::SendFileInfo info;

    AN::UUID uuid;
    uuid.init();

    m_SendFileMap[uuid.string()] = filePath;

    info.set_uuid(&uuid, sizeof uuid);
    info.set_filename(QFileInfo(filePath).fileName().toStdString().c_str());
    info.set_filesize(fileSize);

    AN_LOG(Info, "will send file with uuid %s size %llu", uuid.string().c_str(), fileSize);

    AN::DeviceMessage message;
    message.set_type(AN::kDeviceMessageSendFile);
    message.set_data(info.SerializeAsString());
    sendMessage(message);
}

static DesktopServerThread s_DesktopServerThread;

DesktopServer &GetDesktopServer()
{
    return *s_DesktopServerThread.getServer();
}

DesktopServerThread &GetDesktopServerThread()
{
    return s_DesktopServerThread;
}

void DesktopServerThread::run()
{
    m_EventLoop = new QEventLoop(this);
    m_Server = new DesktopServer(this);
    m_EventLoop->exec();
}

void DesktopServerThread::stopServer()
{
    if (m_EventLoop == nullptr) return;
    m_EventLoop->quit();
}
