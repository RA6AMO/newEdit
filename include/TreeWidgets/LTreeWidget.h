#pragma once
#include <QTreeWidget>
#include <QMainWindow>
#include "DBManager.h"

class LTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    LTreeWidget(QString tableName,QTreeWidget *parent = nullptr, DatabaseManager *dbMan = nullptr);
    ~LTreeWidget();

private:
    void iniTree(QString tableName);
    QTreeWidget *parent;
    DatabaseManager *dbMan;
};
