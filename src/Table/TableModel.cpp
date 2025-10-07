#include "Table/TableModel.h"
#include <QDebug>
#include <QColor>

namespace TableSystem {

TableModel::TableModel(QObject* parent)
    : QAbstractTableModel(parent)
    , m_columnManager(nullptr)
    , m_dataSyncManager(nullptr)
{
}

TableModel::~TableModel()
{
}

// ========== QAbstractTableModel интерфейс ==========

int TableModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_data.size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;
    return m_columns.size();
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !isValidIndex(index.row(), index.column()))
        return QVariant();

    const TableCell& cell = m_data[index.row()][index.column()];

    switch (role) {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return cell.data();

        case Qt::BackgroundRole:
            // Подсветка измененных ячеек
            if (cell.isModified())
                return QColor(255, 255, 200); // Светло-желтый
            break;

        case Qt::ForegroundRole:
            // Красный цвет для невалидных ячеек
            if (!cell.isValid())
                return QColor(Qt::red);
            break;

        case Qt::ToolTipRole:
            // Показываем ошибку валидации
            if (!cell.isValid())
                return cell.validationError();
            break;
    }

    return QVariant();
}

bool TableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !isValidIndex(index.row(), index.column()))
        return false;

    if (role != Qt::EditRole)
        return false;

    return setCellData(index.row(), index.column(), value);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        if (section >= 0 && section < m_columns.size())
            return m_columns[section].name();
    } else {
        return section + 1; // Номера строк
    }

    return QVariant();
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() >= 0 && index.column() < m_columns.size()) {
        if (m_columns[index.column()].isEditable())
            flags |= Qt::ItemIsEditable;
    }

    return flags;
}

// ========== Установка данных ==========

void TableModel::setTableData(const QList<QVariantMap>& rows)
{
    beginResetModel();

    m_data.clear();

    for (const QVariantMap& rowMap : rows) {
        QList<TableCell> rowCells;

        for (const TableColumn& col : m_columns) {
            QVariant value = rowMap.value(col.id(), col.defaultValue());
            TableCell cell(value);
            cell.saveOriginal(); // Сохраняем как оригинал
            rowCells.append(cell);
        }

        m_data.append(rowCells);
    }

    endResetModel();
}

void TableModel::setTableData(const QList<QVector<QVariant>>& rows)
{
    beginResetModel();

    m_data.clear();

    for (const QVector<QVariant>& rowVector : rows) {
        QList<TableCell> rowCells;

        for (int col = 0; col < m_columns.size(); ++col) {
            QVariant value = (col < rowVector.size()) ? rowVector[col] : m_columns[col].defaultValue();
            TableCell cell(value);
            cell.saveOriginal();
            rowCells.append(cell);
        }

        m_data.append(rowCells);
    }

    endResetModel();
}

QList<QVariantMap> TableModel::getTableData() const
{
    QList<QVariantMap> result;

    for (const QList<TableCell>& row : m_data) {
        QVariantMap rowMap;

        for (int col = 0; col < m_columns.size() && col < row.size(); ++col) {
            rowMap[m_columns[col].id()] = row[col].data();
        }

        result.append(rowMap);
    }

    return result;
}

QList<QVariantMap> TableModel::getModifiedRows() const
{
    QList<QVariantMap> result;

    for (int rowIdx = 0; rowIdx < m_data.size(); ++rowIdx) {
        const QList<TableCell>& row = m_data[rowIdx];

        bool isModified = false;
        for (const TableCell& cell : row) {
            if (cell.isModified()) {
                isModified = true;
                break;
            }
        }

        if (isModified) {
            QVariantMap rowMap;
            for (int col = 0; col < m_columns.size() && col < row.size(); ++col) {
                rowMap[m_columns[col].id()] = row[col].data();
            }
            result.append(rowMap);
        }
    }

    return result;
}

// ========== Управление столбцами ==========

void TableModel::setColumns(const QList<TableColumn>& columns)
{
    beginResetModel();
    m_columns = columns;
    updateColumnIndexMap();
    endResetModel();
}

QList<TableColumn> TableModel::columns() const
{
    return m_columns;
}

void TableModel::addColumn(const TableColumn& column)
{
    int newColIndex = m_columns.size();

    beginInsertColumns(QModelIndex(), newColIndex, newColIndex);

    m_columns.append(column);

    // Добавить пустые ячейки во все строки
    for (QList<TableCell>& row : m_data) {
        TableCell cell(column.defaultValue());
        cell.saveOriginal();
        row.append(cell);
    }

    updateColumnIndexMap();
    endInsertColumns();

    emit columnAdded(column.id());
}

void TableModel::removeColumn(const QString& columnId)
{
    int idx = columnIndex(columnId);
    if (idx != -1)
        removeColumn(idx);
}

void TableModel::removeColumn(int columnIndex)
{
    if (columnIndex < 0 || columnIndex >= m_columns.size())
        return;

    QString columnId = m_columns[columnIndex].id();

    beginRemoveColumns(QModelIndex(), columnIndex, columnIndex);

    m_columns.removeAt(columnIndex);

    // Удалить ячейки из всех строк
    for (QList<TableCell>& row : m_data) {
        if (columnIndex < row.size())
            row.removeAt(columnIndex);
    }

    updateColumnIndexMap();
    endRemoveColumns();

    emit columnRemoved(columnId);
}

TableColumn TableModel::column(int index) const
{
    if (index >= 0 && index < m_columns.size())
        return m_columns[index];
    return TableColumn();
}

TableColumn TableModel::column(const QString& columnId) const
{
    int idx = columnIndex(columnId);
    if (idx != -1)
        return m_columns[idx];
    return TableColumn();
}

int TableModel::columnIndex(const QString& columnId) const
{
    return m_columnIndexMap.value(columnId, -1);
}

// ========== Управление строками ==========

bool TableModel::insertRow(int row, const QModelIndex& parent)
{
    if (parent.isValid())
        return false;

    if (row < 0 || row > m_data.size())
        row = m_data.size();

    beginInsertRows(QModelIndex(), row, row);

    QList<TableCell> newRow;
    for (const TableColumn& col : m_columns) {
        TableCell cell(col.defaultValue());
        cell.saveOriginal();
        newRow.append(cell);
    }

    m_data.insert(row, newRow);
    endInsertRows();

    emit rowInserted(row);
    return true;
}

bool TableModel::removeRow(int row, const QModelIndex& parent)
{
    if (parent.isValid() || row < 0 || row >= m_data.size())
        return false;

    beginRemoveRows(QModelIndex(), row, row);
    m_data.removeAt(row);
    endRemoveRows();

    emit rowDeleted(row);
    return true;
}

void TableModel::addRow(const QVariantMap& rowData)
{
    int newRow = m_data.size();

    beginInsertRows(QModelIndex(), newRow, newRow);

    QList<TableCell> row;
    for (const TableColumn& col : m_columns) {
        QVariant value = rowData.value(col.id(), col.defaultValue());
        TableCell cell(value);
        cell.saveOriginal();
        row.append(cell);
    }

    m_data.append(row);
    endInsertRows();

    emit rowInserted(newRow);
}

void TableModel::deleteRow(int row)
{
    removeRow(row);
}

// ========== Работа с ячейками ==========

TableCell* TableModel::cell(int row, int col)
{
    if (isValidIndex(row, col))
        return &m_data[row][col];
    return nullptr;
}

const TableCell* TableModel::cell(int row, int col) const
{
    if (isValidIndex(row, col))
        return &m_data[row][col];
    return nullptr;
}

QVariant TableModel::cellData(int row, int col) const
{
    if (isValidIndex(row, col))
        return m_data[row][col].data();
    return QVariant();
}

bool TableModel::setCellData(int row, int col, const QVariant& value)
{
    if (!isValidIndex(row, col))
        return false;

    TableCell& cell = m_data[row][col];
    QVariant oldValue = cell.data();

    if (oldValue == value)
        return false;

    cell.setData(value);
    emitDataChanged(row, col);

    emit cellDataChanged(row, col, oldValue, value);
    emit modelModified(hasUnsavedChanges());

    return true;
}

// ========== Отслеживание изменений ==========

bool TableModel::hasUnsavedChanges() const
{
    for (const QList<TableCell>& row : m_data) {
        for (const TableCell& cell : row) {
            if (cell.isModified())
                return true;
        }
    }
    return false;
}

void TableModel::markAllAsSaved()
{
    for (QList<TableCell>& row : m_data) {
        for (TableCell& cell : row) {
            cell.saveOriginal();
        }
    }

    emit modelModified(false);
}

void TableModel::revertAllChanges()
{
    for (QList<TableCell>& row : m_data) {
        for (TableCell& cell : row) {
            cell.restoreOriginal();
        }
    }

    emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
    emit modelModified(false);
}

void TableModel::revertRow(int row)
{
    if (row < 0 || row >= m_data.size())
        return;

    for (TableCell& cell : m_data[row]) {
        cell.restoreOriginal();
    }

    emit dataChanged(index(row, 0), index(row, columnCount() - 1));
    emit modelModified(hasUnsavedChanges());
}

QList<int> TableModel::modifiedRows() const
{
    QList<int> result;

    for (int rowIdx = 0; rowIdx < m_data.size(); ++rowIdx) {
        for (const TableCell& cell : m_data[rowIdx]) {
            if (cell.isModified()) {
                result.append(rowIdx);
                break;
            }
        }
    }

    return result;
}

QList<QPair<int, int>> TableModel::modifiedCells() const
{
    QList<QPair<int, int>> result;

    for (int row = 0; row < m_data.size(); ++row) {
        for (int col = 0; col < m_data[row].size(); ++col) {
            if (m_data[row][col].isModified()) {
                result.append(qMakePair(row, col));
            }
        }
    }

    return result;
}

// ========== Связь с менеджерами ==========

void TableModel::setColumnManager(ColumnManager* manager)
{
    m_columnManager = manager;
}

void TableModel::setDataSyncManager(DataSyncManager* manager)
{
    m_dataSyncManager = manager;
}

ColumnManager* TableModel::columnManager() const
{
    return m_columnManager;
}

DataSyncManager* TableModel::dataSyncManager() const
{
    return m_dataSyncManager;
}

// ========== Вспомогательные методы ==========

void TableModel::clear()
{
    beginResetModel();
    m_data.clear();
    m_columns.clear();
    m_columnIndexMap.clear();
    endResetModel();
}

int TableModel::findRow(const QString& columnId, const QVariant& value) const
{
    int colIdx = columnIndex(columnId);
    if (colIdx == -1)
        return -1;

    for (int row = 0; row < m_data.size(); ++row) {
        if (m_data[row][colIdx].data() == value)
            return row;
    }

    return -1;
}

void TableModel::updateColumnIndexMap()
{
    m_columnIndexMap.clear();
    for (int i = 0; i < m_columns.size(); ++i) {
        m_columnIndexMap[m_columns[i].id()] = i;
    }
}

bool TableModel::isValidIndex(int row, int col) const
{
    return row >= 0 && row < m_data.size() &&
           col >= 0 && col < m_columns.size() &&
           col < m_data[row].size();
}

void TableModel::emitDataChanged(int row, int col)
{
    QModelIndex idx = index(row, col);
    emit dataChanged(idx, idx);
}

} // namespace TableSystem
