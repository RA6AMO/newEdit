#pragma once
#include <QWidget>
#include <QMainWindow>
#include "ui_login.h"
#include "DBManager.h"
//#include "DBConnection.h"


class LogWindow : public QDialog, private Ui::Login
{
    Q_OBJECT
public:
    LogWindow(DatabaseManager *dbInit, QWidget *parent = nullptr);
    ~LogWindow();

signals:
    void loginSuccess();

private slots:
    void on_login_btn_clicked();
    void on_Clear_fields_btn_clicked();

private:
    DatabaseManager *dbMan;
    //DBConnection *Connection;

    // Функции валидации
    bool isValidInput(const QString &input);
    void showErrorMessage(const QString &message);
};
