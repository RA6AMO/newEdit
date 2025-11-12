#pragma once
#include <QAbstractTableModel>
#include <QObject>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QHash>
#include <QSet>
#include <QStack>

class TableDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TableDataModel(QObject *parent = nullptr);
    ~TableDataModel();

    bool insertRows(int row, int count);
    bool insertColumns(int column, int count);
    bool removeRows(int row, int count);
    bool removeColumns(int column, int count);


private:

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());


    QVector<QVector<QVariant>> m_data;
    int m_rowCount = 0;
    int m_columnCount = 0;

signals:
    //void rowsInserted(int start, int end);
};


/*
//////////////////////////////////////////////////////////
// В TableDataModel.h добавить:
class TableDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TableDataModel(QObject *parent = nullptr);
    ~TableDataModel();

    // Обязательные методы модели
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // Методы для редактирования
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // Методы для добавления строк/столбцов
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
    QVector<QVector<QVariant>> m_data; // Хранилище данных
    int m_rowCount = 0;
    int m_columnCount = 0;
};


// В TableDataModel.cpp:
#include "TableDataModel.h"

TableDataModel::TableDataModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

TableDataModel::~TableDataModel()
{
}

int TableDataModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_rowCount;
}

int TableDataModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_columnCount;
}

QVariant TableDataModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    if (index.row() >= m_rowCount || index.column() >= m_columnCount)
        return QVariant();

    return m_data[index.row()][index.column()];
}

bool TableDataModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if (row < 0 || row > m_rowCount)
        return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);

    // Добавляем новые строки
    for (int i = 0; i < count; ++i) {
        QVector<QVariant> newRow(m_columnCount);
        m_data.insert(row + i, newRow);
    }
    m_rowCount += count;

    endInsertRows();
    return true;
}

bool TableDataModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    if (column < 0 || column > m_columnCount)
        return false;

    beginInsertColumns(QModelIndex(), column, column + count - 1);

    // Добавляем столбцы во все строки
    for (int i = 0; i < m_rowCount; ++i) {
        for (int j = 0; j < count; ++j) {
            m_data[i].insert(column + j, QVariant());
        }
    }
    m_columnCount += count;

    endInsertColumns();
    return true;
}
