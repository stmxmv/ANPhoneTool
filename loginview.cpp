#include "loginview.h"
#include "ui_loginview.h"
#include "widget.h"

LoginView::LoginView(QWidget *parent) :
                                        QWidget(parent),
                                        ui(new Ui::LoginView)
{
    ui->setupUi(this);
    setFixedSize(size());

#ifndef AN_TEST_UI
    QWidget *testButton = findChild<QWidget *>("pushButton");
    testButton->deleteLater();
#endif
}

LoginView::~LoginView()
{
    delete ui;
}

void LoginView::OnLogin()
{
    m_MainWidget->show();
    m_MainWidget->focusWidget();
    hide();
}

void LoginView::on_pushButton_clicked()
{
    m_MainWidget->show();
    m_MainWidget->focusWidget();
    hide();
}

