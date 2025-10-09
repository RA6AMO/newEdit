#include "Table/MainTable.h"
#include "Table/TableModel.h"
#include "Table/TableColumn.h"
#include "Table/Managers/ColumnManager.h"
#include "Table/Managers/CellInteractionManager.h"
#include "Table/Managers/DataSyncManager.h"
#include "TableColumnCreater.h"
#include <QTableView>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

namespace TableSystem {

MainTable::MainTable(QTableView* tableView, QWidget* toolbar, QWidget* parent)
    : QWidget(parent)
    , m_tableView(tableView)
    , m_toolbar(toolbar)
    , m_addRowButton(nullptr)
    , m_removeRowButton(nullptr)
    , m_addColumnButton(nullptr)
    , m_model(nullptr)
    , m_columnManager(nullptr)
    , m_interactionManager(nullptr)
    , m_syncManager(nullptr)

{
    setupUI();
    initializeManagers();
    setupConnections();
}

MainTable::~MainTable()
{
    // Менеджеры удалятся автоматически через parent
}

// ========== Установка данных ==========

void MainTable::setColumns(const QList<TableColumn>& columns)
{
    m_model->setColumns(columns);
    m_tableView->resizeColumnsToContents();
}

void MainTable::setData(const QList<QVariantMap>& rows)
{
    m_model->setTableData(rows);
    m_tableView->resizeColumnsToContents();
}

void MainTable::setData(const QList<QVector<QVariant>>& rows)
{
    m_model->setTableData(rows);
    m_tableView->resizeColumnsToContents();
}

// ========== Получение данных ==========

QList<QVariantMap> MainTable::getData() const
{
    return m_model->getTableData();
}

QList<QVariantMap> MainTable::getModifiedData() const
{
    return m_model->getModifiedRows();
}

// ========== Управление столбцами ==========

void MainTable::addColumn(const TableColumn& column)
{
    m_columnManager->addColumn(column);
    m_tableView->resizeColumnsToContents();
}

void MainTable::addColumn(const QString& name, ColumnType type)
{
    m_columnManager->addColumn(name, type);
    m_tableView->resizeColumnsToContents();
}

void MainTable::removeColumn(const QString& columnId)
{
    m_columnManager->removeColumn(columnId);
}

QList<TableColumn> MainTable::columns() const
{
    return m_model->columns();
}

// ========== Интерактивность ==========

void MainTable::registerCellHandler(const QString& columnId, CellActionHandler handler)
{
    m_interactionManager->registerHandler(columnId, ColumnType::Custom, handler);
}

void MainTable::unregisterCellHandler(const QString& columnId)
{
    m_interactionManager->unregisterHandler(columnId);
}

// ========== Изменения ==========

bool MainTable::hasUnsavedChanges() const
{
    return m_model->hasUnsavedChanges();
}

void MainTable::markAsSaved()
{
    m_model->markAllAsSaved();
    m_syncManager->markAllAsSynced();
}

void MainTable::revertChanges()
{
    m_model->revertAllChanges();
}

QList<DetailedCommand> MainTable::getPendingCommands() const
{
    return m_syncManager->getUnsyncedDetailedCommands();
}

void MainTable::clearPendingCommands()
{
    m_syncManager->clearAllCommands();
}

// ========== Доступ к компонентам ==========

TableModel* MainTable::model() const
{
    return m_model;
}

ColumnManager* MainTable::columnManager() const
{
    return m_columnManager;
}

CellInteractionManager* MainTable::interactionManager() const
{
    return m_interactionManager;
}

DataSyncManager* MainTable::syncManager() const
{
    return m_syncManager;
}

QTableView* MainTable::tableView() const
{
    return m_tableView;
}

// ========== UI элементы ==========

void MainTable::showToolbar(bool show)
{
    if (m_toolbar != nullptr)
        m_toolbar->setVisible(show);
}

void MainTable::enableEditing(bool enable)
{
    if (enable) {
        m_tableView->setEditTriggers(QAbstractItemView::DoubleClicked |
                                     QAbstractItemView::EditKeyPressed);
    } else {
        m_tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
}

QWidget* MainTable::toolbar() const
{
    return m_toolbar;
}

// ========== Слоты ==========

void MainTable::onCellClicked(const QModelIndex& index)
{
    // Обработка клика через CellInteractionManager
    m_interactionManager->handleCellClick(index);
}

void MainTable::onCellDoubleClicked(const QModelIndex& index)
{
    m_interactionManager->handleCellDoubleClick(index);
}

void MainTable::onModelDataChanged()
{
    emit dataChanged();
    emit unsavedChangesStateChanged(hasUnsavedChanges());
}

void MainTable::onAddRowClicked()
{
    m_model->addRow();
}

void MainTable::onRemoveRowClicked()
{
    QModelIndex current = m_tableView->currentIndex();
    if (current.isValid()) {
        int row = current.row();

        QMessageBox::StandardButton reply = QMessageBox::question(
            this,
            tr("Удаление строки"),
            tr("Вы уверены, что хотите удалить строку %1?").arg(row + 1),
            QMessageBox::Yes | QMessageBox::No
        );

        if (reply == QMessageBox::Yes) {
            m_model->deleteRow(row);
        }
    } else {
        QMessageBox::information(this, tr("Удаление строки"),
                                tr("Выберите строку для удаления"));
    }
}

void MainTable::onAddColumnClicked()
{
    TableColumnCreater dialog(this);

    // Открываем модальный диалог и ждем результата
    if (dialog.exec() == QDialog::Accepted) {
        QString columnName = dialog.getColumnName();
        ColumnType columnType = dialog.getColumnType();


        addColumn(columnName, columnType);
    }
    // Если Rejected - ничего не делаем, пользователь отменил
}

// ========== Приватные методы ==========

void MainTable::setupUI()
{
    // Настройка переданного tableView
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->verticalHeader()->setVisible(true);

    // Ищем кнопки в toolbar по objectName (установленным в Qt Designer)
    m_addRowButton = m_toolbar->findChild<QPushButton*>("addRowButton");
    m_removeRowButton = m_toolbar->findChild<QPushButton*>("removeRowButton");
    m_addColumnButton = m_toolbar->findChild<QPushButton*>("addColumnButton");
}

void MainTable::setupConnections()
{
    // Кнопки toolbar - подключаем только если они найдены
    if (m_addRowButton) {
        connect(m_addRowButton, &QPushButton::clicked, this, &MainTable::onAddRowClicked);
    }
    if (m_removeRowButton) {
        connect(m_removeRowButton, &QPushButton::clicked, this, &MainTable::onRemoveRowClicked);
    }
    if (m_addColumnButton) {
        connect(m_addColumnButton, &QPushButton::clicked, this, &MainTable::onAddColumnClicked);
    }

    // События таблицы
    connect(m_tableView, &QTableView::clicked, this, &MainTable::onCellClicked);
    connect(m_tableView, &QTableView::doubleClicked, this, &MainTable::onCellDoubleClicked);

    // Сигналы модели
    connect(m_model, &TableModel::cellDataChanged, this, &MainTable::onModelDataChanged);
    connect(m_model, &TableModel::rowInserted, this, &MainTable::onModelDataChanged);
    connect(m_model, &TableModel::rowDeleted, this, &MainTable::onModelDataChanged);
    connect(m_model, &TableModel::modelModified, this, &MainTable::unsavedChangesStateChanged);

    // Сигналы менеджеров
    connect(m_model, &TableModel::columnAdded, this, &MainTable::columnAdded);
    connect(m_model, &TableModel::columnRemoved, this, &MainTable::columnRemoved);

    connect(m_interactionManager, &CellInteractionManager::actionPerformed,
            this, &MainTable::cellActionTriggered);

    // Отслеживание изменений для DataSyncManager
    connect(m_model, &TableModel::cellDataChanged,
            [this](int row, int col, const QVariant& oldValue, const QVariant& newValue) {
        QString columnId = m_model->column(col).id();
        m_syncManager->trackCellUpdate(row, columnId, oldValue, newValue);
    });

    connect(m_model, &TableModel::rowInserted,
            [this](int row) {
        QVariantMap rowData;
        QList<TableColumn> cols = m_model->columns();
        for (int col = 0; col < cols.size(); ++col) {
            rowData[cols[col].id()] = m_model->cellData(row, col);
        }
        m_syncManager->trackRowInsert(row, rowData);
    });

    connect(m_model, &TableModel::rowDeleted,
            [this](int row) {
        // TODO: Нужно сохранить данные строки ДО удаления
        m_syncManager->trackRowDelete(row, QVariantMap());
    });
}

void MainTable::initializeManagers()
{
    // Создаем модель
    m_model = new TableModel(this);
    m_tableView->setModel(m_model);

    // Создаем менеджеры
    m_columnManager = new ColumnManager(m_model, this);
    m_interactionManager = new CellInteractionManager(m_tableView, m_model, this);
    m_syncManager = new DataSyncManager(this);

    // Связываем менеджеры с моделью
    m_model->setColumnManager(m_columnManager);
    m_model->setDataSyncManager(m_syncManager);
}

} // namespace TableSystem
