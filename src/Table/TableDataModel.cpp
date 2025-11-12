#include "TableDataModel.h"

TableDataModel::TableDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableDataModel::~TableDataModel()
{
}

bool TableDataModel::insertRows(int row, int count){
    if (row < 0 || row > m_rowCount || count <= 0) return false;

    return insertRows(row, count, QModelIndex());
}

bool TableDataModel::insertColumns(int column, int count){
    if (column < 0 || column > m_columnCount || count <= 0) return false;

    return insertColumns(column, count, QModelIndex());
}

bool TableDataModel::removeRows(int row, int count){
    if (row < 0 || row > m_rowCount || count <= 0) return false;

    return removeRows(row, count, QModelIndex());
}
bool TableDataModel::removeColumns(int column, int count){
    if (column < 0 || column > m_columnCount || count <= 0) return false;

    return removeColumns(column, count, QModelIndex());
}

bool TableDataModel::insertRows(int row, int count, const QModelIndex &parent){
    Q_UNUSED(parent);

    // Уведомляем представление о начале вставки
    beginInsertRows(QModelIndex(), row, row + count - 1);

    // Создаем новые строки с пустыми ячейками
    for (int i = 0; i < count; ++i) {
        QVector<QVariant> newRow(m_columnCount); // Строка с m_columnCount пустыми ячейками
        m_data.insert(row + i, newRow); // Вставляем строку в позицию row + i
    }

    m_rowCount += count; // Увеличиваем счетчик строк

    // Уведомляем представление об окончании вставки
    endInsertRows();

    return true;
}

bool TableDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if (row < 0 || row + count > m_rowCount || count <= 0) {
        return false;
    }

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    // Удаляем строки из массива
    for (int i = 0; i < count; ++i) {
        m_data.removeAt(row);
    }

    m_rowCount -= count;

    endRemoveRows();

    return true;
}

bool TableDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if (column < 0 || column > m_columnCount || count <= 0) {
        return false;
    }

    // Уведомляем представление о начале вставки
    beginInsertColumns(QModelIndex(), column, column + count - 1);

    // Добавляем столбцы во все строки
    for (int i = 0; i < m_rowCount; ++i) {
        for (int j = 0; j < count; ++j) {
            m_data[i].insert(column + j, QVariant());
        }
    }

    m_columnCount += count;

    // Уведомляем представление об окончании вставки
    endInsertColumns();

    return true;
}

bool TableDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if (column < 0 || column + count > m_columnCount || count <= 0) {
        return false;
    }

    // Уведомляем представление о начале удаления
    beginRemoveColumns(QModelIndex(), column, column + count - 1);

    // Удаляем столбцы из всех строк
    for (int i = 0; i < m_rowCount; ++i) {
        for (int j = 0; j < count; ++j) {
            m_data[i].removeAt(column);
        }
    }

    m_columnCount -= count;

    // Уведомляем представление об окончании удаления
    endRemoveColumns();

    return true;
}

bool TableDataModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole) {
        return false;
    }

    int row = index.row();
    int column = index.column();

    if (row < 0 || row >= m_rowCount ||
        column < 0 || column >= m_columnCount) {
        return false;
    }

    // Устанавливаем новое значение
    m_data[row][column] = value;

    // Уведомляем представление об изменении данных
    emit dataChanged(index, index, {role});

    return true;
}
