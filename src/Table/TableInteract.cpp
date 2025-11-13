#include "TableInteract.h"

TableInteract::TableInteract(MainTable *tableView,QObject *parent)
    : QObject(parent), tableView(tableView)
{
    tableModel = new TableDataModel();

    tableView->setModel(tableModel);

    ForTestCommand();

}

TableInteract::~TableInteract()
{
    delete tableModel;
}

void TableInteract::ForTestCommand()
{
    bool success = true;
    success &= tableModel->insertRows(tableModel->rowCount(QModelIndex()), 2, QModelIndex());
    success &= tableModel->insertColumns(tableModel->columnCount(QModelIndex()), 2, QModelIndex());
    success &= tableModel->setData(tableModel->index(0, 0), "Test1");
    success &= tableModel->setData(tableModel->index(0, 1), "Test2");
    success &= tableModel->setData(tableModel->index(1, 0), "Test3");
    success &= tableModel->setData(tableModel->index(1, 1), "Test4");

    success &= tableModel->insertRows(tableModel->rowCount(QModelIndex()), 1, QModelIndex());
    success &= tableModel->insertColumns(tableModel->columnCount(QModelIndex()), 1, QModelIndex());

    success &= tableModel->setData(tableModel->index(2, 2), "Test5");
    success &= tableModel->setHeaderData(0, Qt::Horizontal, "Header1");
    success &= tableModel->setHeaderData(1, Qt::Horizontal, "Header2");
    success &= tableModel->setHeaderData(2, Qt::Horizontal, "Header3");


    //tableModel->removeRows(0, 1);
    //tableModel->removeColumns(0, 1);

}
