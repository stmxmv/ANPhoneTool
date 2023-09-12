#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <Render/QD3D11VideoWidget.h>
#include <Render/Decoder.h>
#include <Render/Frames.h>
#include <Network/AndroidDaemon.h>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:

    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_testButton_clicked();

    void on_videoButton_clicked();

    void on_startDaemonButton_clicked();

    void on_stopDaemonButton_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
