#pragma once
#include <QTableWidget>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include "DBManager.h"
#include <vector>
#include <algorithm>
#include <QMap>
#include <memory>


class MainTable : public QTableView
{
    Q_OBJECT
public:
    MainTable(QWidget *parent = nullptr);
    ~MainTable();
};
