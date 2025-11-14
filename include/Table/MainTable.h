#pragma once
#include <QTableView>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <vector>
#include <algorithm>
#include <QMap>
#include <memory>
#include "HeaderTable.h"


class MainTable : public QTableView
{
    Q_OBJECT
public:
    MainTable(QWidget *parent = nullptr);
    ~MainTable();

    CustomHeaderView* getCustomHeader() const { return m_customHeader; }

private:
    CustomHeaderView *m_customHeader;
};
