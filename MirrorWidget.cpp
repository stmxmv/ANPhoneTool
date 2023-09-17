#include "MirrorWidget.h"
#include "ui_MirrorWidget.h"

#include <QScreen>

#define MIRROR_PORT 13133

static QRect getScreenRect(QWidget *window)
{
    QRect screenRect;
    QWidget *win = window;
    if (!win) {
        return screenRect;
    }

    QWindow *winHandle = win->windowHandle();
    QScreen *screen = QGuiApplication::primaryScreen();
    if (winHandle) {
        screen = winHandle->screen();
    }
    if (!screen) {
        return screenRect;
    }

    screenRect = screen->availableGeometry();
    return screenRect;
}


MirrorWidget::MirrorWidget(QWidget *parent) :
                                              QWidget(parent),
                                              ui(new Ui::MirrorWidget),
                                              m_VideoWidget(),
                                              m_AndroidDaemon(QDir::currentPath() +  "/adb.exe", this)
{
    ui->setupUi(this);

    m_VideoWidget = new QD3D11VideoWidget(this);
    ui->keepRatioWidget->setWidget(m_VideoWidget);
    ui->keepRatioWidget->setWidthHeightRatio(m_widthHeightRatio);

    ui->keepRatioWidget->setMouseTracking(true);

    TcpServer::connect(&m_Server, &QTcpServer::newConnection, this, [this](){

                         AN_LOG(Info, "mirror socket connected");

                         m_deviceSocket = dynamic_cast<DeviceSocket*>(m_Server.nextPendingConnection());
                         m_Decoder.setDeviceSocket(m_deviceSocket);
                         m_Decoder.startDecode();
                         updateFrameSize(QSize(1079, 2105)); // crop

                         QObject::connect(m_deviceSocket, &DeviceSocket::disconnected, this, [this] ()
                                                {
                                                    AN_LOG(Info, "mirror socket disconnected");
                                                });
                     });

//    connect(&m_AndroidDaemon, &AndroidDaemon::OnStart, [](bool success)
//            {
//                AN_LOG(Info, "Android Daemon Start");
//            });
//
//    connect(&m_AndroidDaemon, &AndroidDaemon::OnConnect, this,
//            [this](bool success, const QString& deviceName, const QSize& size)
//            {
//                AN_LOG(Info, "Android Daemon Connect %d %s", (int) success, deviceName.toStdString().c_str());
//
//                if (success)
//                {
//                    m_Decoder.setDeviceSocket(m_AndroidDaemon.getDeviceSocket());
//                    m_Decoder.startDecode();
//
//                    updateFrameSize(size);
//                }
//            });

    bool success = m_Frames.init();
    Q_ASSERT(success);
    m_Decoder.setFrames(&m_Frames);

    connect(&m_Decoder, &Decoder::onNewFrame, this, [this]()
            {
                m_Frames.lock();
                const AVFrame *frame = m_Frames.consumeRenderedFrame();

                if (frame)
                {
                    // do render
                    m_VideoWidget->setFrameSize(QSize(frame->width, frame->height));
                    m_VideoWidget->updateTextures(frame->data[0], frame->data[1], frame->data[2],
                                                  frame->linesize[0], frame->linesize[1], frame->linesize[2]);
                }

                m_Frames.unLock();
            });
}

MirrorWidget::~MirrorWidget()
{
//    m_AndroidDaemon.stop();
    m_Server.close();
    if (m_deviceSocket) {
        m_deviceSocket->close();
    }
    m_Decoder.stopDecode();
    m_Frames.deinit();
    delete ui;
}

void MirrorWidget::updateFrameSize(const QSize &newSize)
{
    if (m_frameSize != newSize) {
        m_frameSize = newSize;

        m_widthHeightRatio = 1.0f * newSize.width() / newSize.height();
        ui->keepRatioWidget->setWidthHeightRatio(m_widthHeightRatio);

        bool vertical = m_widthHeightRatio < 1.0f;
        QSize showSize = newSize;
        QRect screenRect = getScreenRect(window());
        if (screenRect.isEmpty()) {
            qWarning() << "getScreenRect is empty";
            return;
        }
        if (vertical) {
            showSize.setHeight(qMin(newSize.height(), screenRect.height() - 200));
            showSize.setWidth(showSize.height() * m_widthHeightRatio);
        } else {
            showSize.setWidth(qMin(newSize.width(), screenRect.width() / 2));
            showSize.setHeight(showSize.width() / m_widthHeightRatio);
        }

        if (isMaximized()) {
            showNormal();
        }

        if (showSize != size()) {
            resize(showSize);
            center();
        }
    }
}

void MirrorWidget::center()
{
    QRect screenRect = getScreenRect(window());
    if (screenRect.isEmpty()) {
        qWarning() << "getScreenRect is empty";
        return;
    }
    // 窗口居中
    move(screenRect.center() - QRect(0, 0, size().width(), size().height()).center());
}

void MirrorWidget::startMirror(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate)
{
//    m_AndroidDaemon.start(serial, localPort, maxSize, bitRate);
    m_Server.listen(QHostAddress::Any, MIRROR_PORT);
}
