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


    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole);

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex());
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex());


private:




    QVector<QVector<QVariant>> m_data;
    QVector<QString> m_columnHeaders;

    int m_rowCount = 0;
    int m_columnCount = 0;

signals:
    //void rowsInserted(int start, int end);
};
