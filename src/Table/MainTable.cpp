#include "MainTable.h"
#include <QMenu>
#include <QContextMenuEvent>
#include <QItemSelectionModel>

MainTable::MainTable(QWidget *parent) : QTableView(parent)
{
    m_customHeader = new CustomHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(m_customHeader);

    m_rowHeader = new RowHeaderView(Qt::Vertical, this);
    setVerticalHeader(m_rowHeader);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    setSelectionBehavior(QAbstractItemView::SelectItems);
}

MainTable::~MainTable()
{

}

void MainTable::contextMenuEvent(QContextMenuEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (!index.isValid())
    {
        QTableView::contextMenuEvent(event);
        return;
    }

    QMenu menu(this);
    QAction *editAction = menu.addAction(tr("Изменить"));

    QAction *selectedAction = menu.exec(event->globalPos());
    if (selectedAction == editAction)
    {
        if (selectionModel())
        {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
        }
        emit editCellRequested(index);
    }
}
