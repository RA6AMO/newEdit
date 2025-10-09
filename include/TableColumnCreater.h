#pragma once
#include <QDialog>
#include <QWidget>
#include "Table/TableTypes.h"
#include <QMessageBox>
#include <QRegularExpression>
#include <QPushButton>
#include <QLineEdit>
#include "ui_TableColumnCreater.h"

namespace Ui {
    class TableColumnCreater;
}

class TableColumnCreater : public QDialog
{
    Q_OBJECT
public:
    explicit TableColumnCreater(QWidget *parent = nullptr);
    ~TableColumnCreater();

    // Методы для получения данных после закрытия диалога
    QString getColumnName() const;
    TableSystem::ColumnType getColumnType() const;

private slots:
    void onAcceptClicked();
    void onCancelClicked();
    void validateInput();

private:
    void setupUI();
    bool isInputValid() const;

    Ui::TableColumnCreater *ui;
};
