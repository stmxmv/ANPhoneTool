#include "widget.h"
#include "./ui_widget.h"

#include "MirrorWidget.h"
#include "Network/DesktopServer.h"

#include <QDir>
#include <adb/AdbHandler.h>
#include <QFileDialog>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_testButton_clicked()
{
    QString adbPath = QDir::currentPath() +  "/adb.exe";

    AdbHandler *adb = new AdbHandler(adbPath, this);

    connect(adb, &AdbHandler::OnResult, this, [this](AdbHandler::AdbResult result) {
        qDebug() << ">>>>>>>>>>>>" << result;
    });

    QStringList devices = adb->getDeviceSerials();
    if (!devices.empty())
    {
        AN_LOG(Info, "Device %s", adb->getDeviceSerials().first().toStdString().c_str());
    }
    else
    {
        AN_LOG(Info, "No Device attached");
    }
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

