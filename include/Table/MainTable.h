#pragma once
#include <QTableView>
#include <QMainWindow>
#include <QMenu>
#include <QContextMenuEvent>
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
    RowHeaderView* getRowHeader() const { return m_rowHeader; }

signals:
    void editCellRequested(QModelIndex index);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    CustomHeaderView *m_customHeader;
    RowHeaderView *m_rowHeader;
};
