#pragma once
#include <QWidget>
#include "TableTypes.h"
#include "Managers/DataSyncManager.h"

class QTableView;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

namespace TableSystem {

class TableModel;
class ColumnManager;
class CellInteractionManager;
class DataSyncManager;
class TableColumn;

class MainTable : public QWidget {
    Q_OBJECT

public:
    explicit MainTable(QWidget* parent = nullptr);
    ~MainTable();

    // ========== Установка данных ==========

    void setColumns(const QList<TableColumn>& columns);
    void setData(const QList<QVariantMap>& rows);
    void setData(const QList<QVector<QVariant>>& rows);

    // ========== Получение данных ==========

    QList<QVariantMap> getData() const;
    QList<QVariantMap> getModifiedData() const;

    // ========== Управление столбцами ==========

    void addColumn(const TableColumn& column);
    void addColumn(const QString& name, ColumnType type);
    void removeColumn(const QString& columnId);

    QList<TableColumn> columns() const;

    // ========== Интерактивность ==========

    void registerCellHandler(const QString& columnId, CellActionHandler handler);
    void unregisterCellHandler(const QString& columnId);

    // ========== Изменения ==========

    bool hasUnsavedChanges() const;
    void markAsSaved();
    void revertChanges();

    QList<DetailedCommand> getPendingCommands() const;
    void clearPendingCommands();

    // ========== Доступ к компонентам ==========

    TableModel* model() const;
    ColumnManager* columnManager() const;
    CellInteractionManager* interactionManager() const;
    DataSyncManager* syncManager() const;
    QTableView* tableView() const;

    // ========== UI элементы (опционально) ==========

    void showToolbar(bool show);
    void enableEditing(bool enable);

signals:
    void dataChanged();
    void columnAdded(const QString& columnId);
    void columnRemoved(const QString& columnId);
    void cellActionTriggered(const QString& columnId, const CellActionResult& result);
    void unsavedChangesStateChanged(bool hasChanges);

private slots:
    void onCellClicked(const QModelIndex& index);
    void onCellDoubleClicked(const QModelIndex& index);
    void onModelDataChanged();
    void onAddRowClicked();
    void onRemoveRowClicked();
    void onAddColumnClicked();

private:
    void setupUI();
    void setupConnections();
    void initializeManagers();

    // UI компоненты
    QTableView* m_tableView;
    QWidget* m_toolbar;
    QPushButton* m_addRowButton;
    QPushButton* m_removeRowButton;
    QPushButton* m_addColumnButton;

    // Компоненты системы
    TableModel* m_model;
    ColumnManager* m_columnManager;
    CellInteractionManager* m_interactionManager;
    DataSyncManager* m_syncManager;
};

} // namespace TableSystem
