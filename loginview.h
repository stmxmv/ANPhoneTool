#ifndef LOGINVIEW_H
#define LOGINVIEW_H

#include <QWidget>
#include "widget.h"
namespace Ui {
class LoginView;
}

class LoginView : public QWidget
{
    Q_OBJECT

    Widget *m_MainWidget;

public:
    explicit LoginView(QWidget *parent = nullptr);
    ~LoginView();

    void SetMainWidget(Widget *widget)
    {
        m_MainWidget = widget;
    }

    void OnLogin();

private slots:
    void on_pushButton_clicked();

private:
    Ui::LoginView *ui;
};

#endif // LOGINVIEW_H
