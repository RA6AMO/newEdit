#pragma once
#include <QWidget>
#include <QMainWindow>
#include "ui_login.h"
#include "DBManager.h"
#include "DBConnection.h"


class LogWindow : public QDialog, private Ui::Login
{
    Q_OBJECT
public:
    LogWindow(DatabaseManager &dbInit, QWidget *parent = nullptr);
    ~LogWindow();

private slots:
    void on_login_btn_clicked();
    void on_Clear_fields_btn_clicked();

private:
    DBConnection *Connection;
};
