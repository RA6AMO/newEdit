#include "TableDataModel.h"

TableDataModel::TableDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    m_columnHeaders.clear();
}

TableDataModel::~TableDataModel()
{
}




bool TableDataModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (row < 0 || row > m_rowCount || count <= 0) return false;

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
    if (column < 0 || column > m_columnCount || count <= 0) return false;

    beginInsertColumns(QModelIndex(), column, column + count - 1);

    // Добавляем столбцы во все строки
    for (int i = 0; i < m_rowCount; ++i) {
        for (int j = 0; j < count; ++j) {
            m_data[i].insert(column + j, QVariant());
        }
    }

    // Вставляем пустые заголовки для новых столбцов
    for (int j = 0; j < count; ++j) {
        m_columnHeaders.insert(column + j, QString());
    }

    m_columnCount += count;

    endInsertColumns();

    return true;
}

bool TableDataModel::insertRows(int row, int count, const QModelIndex &parent){
    Q_UNUSED(parent);
    if (row < 0 || row > m_rowCount || count <= 0) return false;


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

bool TableDataModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (column < 0 || column > m_columnCount || count <= 0) return false;

    beginRemoveColumns(QModelIndex(), column, column + count - 1);

    // Удаляем столбцы из всех строк
    for (int i = 0; i < m_rowCount; ++i) {
        for (int j = 0; j < count; ++j) {
            m_data[i].removeAt(column);
        }
    }

    // Удаляем заголовки
    for (int j = 0; j < count; ++j) {
        if (column < m_columnHeaders.size()) {
            m_columnHeaders.removeAt(column);
        }
    }

    m_columnCount -= count;

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

int TableDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rowCount;  // Возвращаем количество строк из m_data
}

int TableDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_columnCount;  // Возвращаем количество столбцов из m_data
}

QVariant TableDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    if (index.row() >= m_rowCount || index.column() >= m_columnCount)
        return QVariant();

    return m_data[index.row()][index.column()];  // Возвращаем данные из m_data
}

QVariant TableDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    if (orientation == Qt::Horizontal) {
        // Проверяем границы
        if (section < 0 || section >= m_columnCount)
            return QVariant();

        // Если есть сохраненный заголовок, возвращаем его
        if (section < m_columnHeaders.size() && !m_columnHeaders[section].isEmpty())
            return m_columnHeaders[section];

        // Иначе возвращаем заголовок по умолчанию
        return QString("Столбец %1").arg(section + 1);
    }

    if (orientation == Qt::Vertical) {
        // Заголовки для строк (опционально)
        return section + 1;
    }

    return QVariant();
}

bool TableDataModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (orientation == Qt::Horizontal) {
        if (section < 0 || section >= m_columnCount)
            return false;

        // Расширяем вектор заголовков при необходимости
        while (m_columnHeaders.size() <= section) {
            m_columnHeaders.append(QString());
        }

        m_columnHeaders[section] = value.toString();

        // Уведомляем представление об изменении заголовка
        emit headerDataChanged(orientation, section, section);

        return true;
    }

    return false;
}
