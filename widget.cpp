#include "widget.h"
#include "./ui_widget.h"
#include "loginview.h"
#include "MirrorWidget.h"
#include "Network/DesktopServer.h"
#include <adb/AdbHandler.h>


#include <QDir>
#include <QFileDialog>
#include <QNetworkInterface>
#include <QPainter>
#include <QLabel>

#include "qrencode.h"

QList<QHostAddress> GetAllLocalAddresses()
{
    const QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
    QList<QHostAddress> result;
    for (const auto& p : interfaces)
    {
        for (const QNetworkAddressEntry& entry : p.addressEntries())
        {
            if (entry.ip().protocol() == QAbstractSocket::IPv6Protocol ||
                entry.ip().isLoopback())
            {
                continue;
            }
            result += entry.ip();
        }
    }


    return result;
}

std::string GetAllLocalAddressesString()
{
    QList<QHostAddress> addresses = GetAllLocalAddresses();

    std::string result;
    for (const QHostAddress &addr : addresses)
    {
        result.append(addr.toString().toStdString());
        result.push_back(';');
    }

    return result;
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    setFixedSize(size());

#ifndef AN_TEST_UI
    QWidget *testButton = findChild<QWidget *>("testButton_2");
    testButton->deleteLater();

    testButton = findChild<QWidget *>("testButton");
    testButton->deleteLater();
#endif
}

Widget::~Widget()
{
    delete ui;
}

void Widget::OnLogout()
{
    m_LoginView->show();
    m_LoginView->focusWidget();
    hide();
}

void Widget::on_testButton_clicked()
{
//    QString adbPath = QDir::currentPath() +  "/adb.exe";
//
//    AdbHandler *adb = new AdbHandler(adbPath, this);
//
//    connect(adb, &AdbHandler::OnResult, this, [this](AdbHandler::AdbResult result) {
//        qDebug() << ">>>>>>>>>>>>" << result;
//    });
//
//    QStringList devices = adb->getDeviceSerials();
//    if (!devices.empty())
//    {
//        AN_LOG(Info, "Device %s", adb->getDeviceSerials().first().toStdString().c_str());
//    }
//    else
//    {
//        AN_LOG(Info, "No Device attached");
//    }

    QRcode *qrCode = nullptr;

    std::string qrCodeString = GetAllLocalAddressesString();
    AN_LOG(Info, "%s", qrCodeString.c_str());

    //这里二维码版本传入参数是2,实际上二维码生成后，它的版本是根据二维码内容来决定的
    qrCode = QRcode_encodeString(qrCodeString.c_str(), 2,
                                 QR_ECLEVEL_Q, QR_MODE_8, 1);

    if (nullptr == qrCode)
    {
        return;
    }

    static int scale = 2;

    int qrCode_Width = qrCode->width > 0 ? qrCode->width : 1;
    int width = scale * qrCode_Width;
    int height = scale * qrCode_Width;

    QImage image(width, height, QImage::Format_ARGB32_Premultiplied);

    QPainter painter(&image);
    QColor background(Qt::white);
    painter.setBrush(background);
    painter.setPen(Qt::NoPen);
    painter.drawRect(0, 0, width, height);
    QColor foreground(Qt::black);
    painter.setBrush(foreground);
    for (int y = 0; y < qrCode_Width; ++y)
    {
        for (int x = 0; x < qrCode_Width; ++x)
        {
            unsigned char character = qrCode->data[y * qrCode_Width + x];
            if (character & 0x01)
            {
                QRect rect(x * scale, y * scale, scale, scale);
                painter.drawRects(&rect, 1);
            }
        }
    }

    QPixmap qrPixmap = QPixmap::fromImage(image);
    QLabel *label = new QLabel();
    label->setAttribute(Qt::WA_DeleteOnClose);
    label->setAlignment(Qt::AlignCenter);
    label->setPixmap(qrPixmap);
    label->show();

    QRcode_free(qrCode);
}


void Widget::on_videoButton_clicked()
{
    /// "", 27183, 720, 8000000
    MirrorWidget *mirrorWidget = new MirrorWidget();
    mirrorWidget->startMirror("", 27183, 720, 8000000);
    mirrorWidget->setAttribute(Qt::WA_DeleteOnClose, true);
    mirrorWidget->center();
    mirrorWidget->show();
}


void Widget::on_startDaemonButton_clicked()
{

}


void Widget::on_stopDaemonButton_clicked()
{

}


void Widget::on_ringPhoneButton_clicked()
{
    AN::DeviceMessage deviceMessage;
    deviceMessage.set_type(AN::kDeviceMessageRing);
    deviceMessage.set_data("");

    /// must be send in the server thread
    QMetaObject::invokeMethod(&GetDesktopServer(), [=]()
                              {
                                  GetDesktopServer().sendMessage(deviceMessage);
                              });

}


void Widget::on_sendFileButton_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this,
                                                    "Send File",
                                                    {},
                                                    "All Files (*)");

    if (filePath.isEmpty())
    {
        return;
    }

    AN_LOG(Info, "will send file %s", filePath.toStdString().c_str());

    QMetaObject::invokeMethod(&GetDesktopServer(), [=]()
                              {
                                  GetDesktopServer().sendFile(filePath);
                              });
}


void Widget::on_testButton_2_clicked()
{
    m_LoginView->show();
    m_LoginView->focusWidget();
    hide();
}

