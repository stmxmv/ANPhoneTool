#include "widget.h"
#include "./ui_widget.h"

#include <adb/AdbProcess.h>
#include <QDir>

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

    qDebug() << adb->getDeivceIPAdress();
}

