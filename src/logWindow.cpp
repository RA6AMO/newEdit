#include "logWindow.h"

LogWindow::LogWindow(DatabaseManager &dbInit, QWidget *parent)
    : QDialog(parent), Connection(dbInit.getConnection())
{
    setupUi(this);
    //this->Manager = dbManager;

    connect(login_btn, &QPushButton::clicked, this, &LogWindow::on_login_btn_clicked);
    connect(Clear_fields_btn, &QPushButton::clicked, this, &LogWindow::on_Clear_fields_btn_clicked);
}

LogWindow::~LogWindow()
{

}

void LogWindow::on_login_btn_clicked()
{
    QString login = lineEdit_3->text();
    QString password = lineEdit_4->text();
}

void LogWindow::on_Clear_fields_btn_clicked()
{
    lineEdit_3->clear();
    lineEdit_4->clear();
    return;
}
