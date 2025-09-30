#include "logWindow.h"

LogWindow::LogWindow(DatabaseManager *dbInit, QWidget *parent)
    : QDialog(parent), dbMan(dbInit)
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

    // Проверка валидности логина и пароля
    if (!isValidInput(login) || !isValidInput(password))
    {
        showErrorMessage("Неверный логин или пароль");
        return;
    }

    // Если валидация прошла успешно, очищаем сообщение об ошибке
    label->setStyleSheet("");
    label->setText("Введите логин и пароль");

    // Здесь можно продолжить логику входа
}

void LogWindow::on_Clear_fields_btn_clicked()
{
    lineEdit_3->clear();
    lineEdit_4->clear();
    // Очищаем сообщение об ошибке
    label->setStyleSheet("");
    label->setText("Введите логин и пароль");
    return;
}

// Функция валидации: проверяет, что строка содержит только английские буквы и цифры
bool LogWindow::isValidInput(const QString &input)
{
    if (input.isEmpty())
    {
        return false;
    }

    for (const QChar &ch : input)
    {
        if (!((ch >= 'a' && ch <= 'z') ||
              (ch >= 'A' && ch <= 'Z') ||
              (ch >= '0' && ch <= '9')))
        {
            return false;
        }
    }

    return true;
}

// Функция вывода сообщения об ошибке красным цветом
void LogWindow::showErrorMessage(const QString &message)
{
    label->setStyleSheet("QLabel { color : red; }");
    label->setText(message);
}
