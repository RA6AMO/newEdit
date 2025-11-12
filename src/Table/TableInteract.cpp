#include "TableInteract.h"

TableInteract::TableInteract(QObject *parent)
    : QObject(parent)
{
    tableModel = new TableDataModel();
    tableView = new MainTable;
}

TableInteract::~TableInteract()
{
    delete tableModel;
    delete tableView;
}
