#ifndef MIRRORWIDGET_H
#define MIRRORWIDGET_H


#include <Render/QD3D11VideoWidget.h>
#include <QWidget>

#include <Render/Decoder.h>
#include <Render/Frames.h>
#include <Network/AndroidDaemon.h>

namespace Ui {
class MirrorWidget;
}

class MirrorWidget : public QWidget
{
    Q_OBJECT

    float m_widthHeightRatio;
    QSize m_frameSize;
    QD3D11VideoWidget *m_VideoWidget;

    AndroidDaemon m_AndroidDaemon;
    Decoder m_Decoder;
    Frames m_Frames;

    TcpServer m_Server;
    DeviceSocket *m_deviceSocket;

public:

    explicit MirrorWidget(QWidget *parent = nullptr);

    ~MirrorWidget();

    void startMirror(const QString &serial, quint16 localPort, quint16 maxSize, quint32 bitRate);

    void updateFrameSize(const QSize &newSize);

    void center();

private:
    Ui::MirrorWidget *ui;
};

#endif // MIRRORWIDGET_H
