#include "TableInteract.h"


TableInteract::TableInteract(MainTable *tableView,QObject *parent)
    : QObject(parent), tableView(tableView)
{
    tableModel = new TableDataModel();

    tableView->setModel(tableModel);

    // Подключаем сигналы от CustomHeaderView к слотам TableDataModel
    CustomHeaderView *header = tableView->getCustomHeader();
    if (header) {
        connect(header, &CustomHeaderView::headerAddRequested,
                tableModel, &TableDataModel::onHeaderAddRequested);
        connect(header, &CustomHeaderView::headerDeleteRequested,
                tableModel, &TableDataModel::onHeaderDeleteRequested);
        connect(header, &CustomHeaderView::headerRenameRequested,
                tableModel, &TableDataModel::onHeaderRenameRequested);
    }

    RowHeaderView *rowHeader = tableView->getRowHeader();
    if (rowHeader) {
        connect(rowHeader, &RowHeaderView::rowAddRequested,
                tableModel, &TableDataModel::onRowAddRequested);
        connect(rowHeader, &RowHeaderView::rowDeleteRequested,
                tableModel, &TableDataModel::onRowDeleteRequested);
    }

    if (tableView) {
        connect(tableView, &MainTable::editCellRequested,
                this, &TableInteract::determineCellType);
    }

    ForTestCommand();

}

TableInteract::~TableInteract()
{
    delete tableModel;
}

void TableInteract::ForTestCommand()
{
    bool success = true;

    // Добавляем 2 столбца используя onHeaderAddRequested
    int currentColCount = tableModel->columnCount(QModelIndex());
    success &= tableModel->onHeaderAddRequested(currentColCount > 0 ? currentColCount - 1 : 0, true);
    success &= tableModel->onHeaderAddRequested(0, true);
    success &= tableModel->onHeaderAddRequested(0, true);
    currentColCount = tableModel->columnCount(QModelIndex());

    success &= tableModel->insertRows(tableModel->rowCount(QModelIndex()), 1, QModelIndex());
    success &= tableModel->insertRows(tableModel->rowCount(QModelIndex()), 2, QModelIndex());

    success &= tableModel->setData(tableModel->index(0, 0), "Test1");
    success &= tableModel->setData(tableModel->index(0, 1), "Test2");
    success &= tableModel->setData(tableModel->index(1, 0), "Test3");
    success &= tableModel->setData(tableModel->index(1, 1), "Test4");
    success &= tableModel->setData(tableModel->index(2, 2), "Test5");

    // Переименовываем заголовки столбцов используя onHeaderRenameRequested
    success &= tableModel->onHeaderRenameRequested(0, "Header1");
    success &= tableModel->onHeaderRenameRequested(1, "Header2");
    success &= tableModel->onHeaderRenameRequested(2, "Header3");


    //tableModel->removeRows(0, 1);
    //tableModel->removeColumns(0, 1);

}

void TableInteract::determineCellType(const QModelIndex &index)
{
    if (!tableModel || !index.isValid()) {
        return;
    }

    const QVariant cellValue = tableModel->data(index, Qt::DisplayRole);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const char *typeName = cellValue.isValid() ? cellValue.metaType().name() : "Invalid";
#else
    const char *typeName = cellValue.isValid() ? cellValue.typeName() : "Invalid";
#endif

    qDebug() << "Тип данных ячейки"
             << QStringLiteral("[%1, %2]").arg(index.row()).arg(index.column())
             << ":" << (cellValue.isNull() ? "Null" : typeName);
}
