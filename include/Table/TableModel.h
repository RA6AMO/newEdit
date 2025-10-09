#pragma once
#include <QAbstractTableModel>
#include "TableTypes.h"
#include "TableColumn.h"
#include "TableCell.h"
#include <QList>
#include <QVector>
#include <QMap>

namespace TableSystem {

class ColumnManager;
class DataSyncManager;

class TableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    explicit TableModel(QObject* parent = nullptr);
    ~TableModel();

    // ========== QAbstractTableModel интерфейс ==========
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // ========== Установка данных (без БД) ==========

    // Вариант 1: список словарей {columnId: value}
    void setTableData(const QList<QVariantMap>& rows);

    // Вариант 2: двумерный массив значений
    void setTableData(const QList<QVector<QVariant>>& rows);

    // Получение данных
    QList<QVariantMap> getTableData() const;
    QList<QVariantMap> getModifiedRows() const;  // Только измененные строки

    // ========== Управление столбцами ==========

    void setColumns(const QList<TableColumn>& columns);
    QList<TableColumn> columns() const;

    void addColumn(const TableColumn& column);
    void removeColumn(const QString& columnId);
    void removeColumn(int columnIndex);

    TableColumn column(int index) const;
    TableColumn column(const QString& columnId) const;
    int columnIndex(const QString& columnId) const;

    // ========== Управление строками ==========


    bool insertRow(int row, const QModelIndex& parent = QModelIndex());
    bool removeRow(int row, const QModelIndex& parent = QModelIndex());

    void addRow(const QVariantMap& rowData = QVariantMap());
    void deleteRow(int row);

    // ========== Работа с ячейками ==========

    TableCell* cell(int row, int col);
    const TableCell* cell(int row, int col) const;

    QVariant cellData(int row, int col) const;
    bool setCellData(int row, int col, const QVariant& value);

    // ========== Отслеживание изменений ==========

    bool hasUnsavedChanges() const;
    void markAllAsSaved();
    void revertAllChanges();
    void revertRow(int row);

    QList<int> modifiedRows() const;
    QList<QPair<int, int>> modifiedCells() const;  // {row, col}

    // ========== Связь с менеджерами ==========

    void setColumnManager(ColumnManager* manager);
    void setDataSyncManager(DataSyncManager* manager);

    ColumnManager* columnManager() const;
    DataSyncManager* dataSyncManager() const;

    // ========== Вспомогательные методы ==========

    void clear();
    int findRow(const QString& columnId, const QVariant& value) const;

signals:
    void cellDataChanged(int row, int col, const QVariant& oldValue, const QVariant& newValue);
    void rowInserted(int row);
    void rowDeleted(int row);
    void columnAdded(const QString& columnId);
    void columnRemoved(const QString& columnId);
    void modelModified(bool hasChanges);

private:
    QList<TableColumn> m_columns;
    QList<QList<TableCell>> m_data;
    QMap<QString, int> m_columnIndexMap;

    ColumnManager* m_columnManager;
    DataSyncManager* m_dataSyncManager;

    void updateColumnIndexMap();
    bool isValidIndex(int row, int col) const;
    void emitDataChanged(int row, int col);
};

} // namespace TableSystem
