#include "Table/Managers/CellInteractionManager.h"
#include "Table/TableModel.h"
#include "TableColumn.h"
#include <QTableView>
#include <QStyledItemDelegate>
#include <QDebug>

namespace TableSystem {

CellInteractionManager::CellInteractionManager(QTableView* tableView, TableModel* model,
                                               QObject* parent)
    : QObject(parent)
    , m_tableView(tableView)
    , m_model(model)
{
}

// ========== Регистрация обработчиков ==========

void CellInteractionManager::registerButtonHandler(const QString& columnId, CellActionHandler handler)
{
    registerHandler(columnId, ColumnType::Button, handler);
}

void CellInteractionManager::registerComboBoxHandler(const QString& columnId, CellActionHandler handler)
{
    registerHandler(columnId, ColumnType::ComboBox, handler);
}

void CellInteractionManager::registerFileDialogHandler(const QString& columnId, CellActionHandler handler)
{
    registerHandler(columnId, ColumnType::FileDialog, handler);
}

void CellInteractionManager::registerHandler(const QString& columnId, ColumnType type, CellActionHandler handler)
{
    if (!handler) {
        qWarning() << "CellInteractionManager: Attempting to register null handler for column" << columnId;
        return;
    }

    m_handlers[columnId] = handler;
    emit handlerRegistered(columnId);

    qDebug() << "CellInteractionManager: Handler registered for column" << columnId;
}

void CellInteractionManager::unregisterHandler(const QString& columnId)
{
    if (m_handlers.remove(columnId) > 0) {
        emit handlerUnregistered(columnId);
    }
}

void CellInteractionManager::unregisterAllHandlers()
{
    QStringList columnIds = m_handlers.keys();
    m_handlers.clear();

    for (const QString& columnId : columnIds) {
        emit handlerUnregistered(columnId);
    }
}

// ========== Регистрация делегатов (ЗАГОТОВКА) ==========

void CellInteractionManager::registerDelegate(const QString& columnId, QStyledItemDelegate* delegate)
{
    if (!delegate) {
        qWarning() << "CellInteractionManager: Attempting to register null delegate for column" << columnId;
        return;
    }

    int colIndex = m_model->columnIndex(columnId);
    if (colIndex == -1) {
        qWarning() << "CellInteractionManager: Column" << columnId << "not found";
        return;
    }

    m_delegates[columnId] = delegate;
    m_tableView->setItemDelegateForColumn(colIndex, delegate);

    qDebug() << "CellInteractionManager: Delegate registered for column" << columnId;
}

void CellInteractionManager::unregisterDelegate(const QString& columnId)
{
    int colIndex = m_model->columnIndex(columnId);
    if (colIndex != -1) {
        m_tableView->setItemDelegateForColumn(colIndex, nullptr);
    }

    m_delegates.remove(columnId);
}

// ========== Обработка событий ==========

bool CellInteractionManager::handleCellClick(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    QString columnId = getColumnId(index.column());
    if (columnId.isEmpty())
        return false;

    if (!hasHandler(columnId))
        return false;

    CellActionResult result = executeHandler(columnId, index.row(), index.column());
    emit actionPerformed(columnId, result);

    return result.success;
}

bool CellInteractionManager::handleCellDoubleClick(const QModelIndex& index)
{
    if (!index.isValid())
        return false;

    TableColumn col = m_model->column(index.column());
    switch (col.type()) {
    case ColumnType::Text:       {
        // двойной клик по тексту
        return handleCellClick(index);
    }
    case ColumnType::Number:     {
        // двойной клик по числу
        return handleCellClick(index);
    }
    case ColumnType::Date:       {
        // двойной клик по дате
        return handleCellClick(index);
    }
    case ColumnType::Image:      {
        // двойной клик по изображению
        return handleCellClick(index);
    }
    case ColumnType::Boolean:    {
        // двойной клик по чекбоксу
        return handleCellClick(index);
    }
    case ColumnType::ComboBox:   {
        // двойной клик по комбобоксу
        return handleCellClick(index);
    }
    case ColumnType::Button:     {
        // двойной клик по кнопке
        return handleCellClick(index);
    }
    case ColumnType::FileDialog: {
        // двойной клик для открытия диалога файла
        return handleCellClick(index);
    }
    case ColumnType::Custom:     {
        // пользовательский тип
        return handleCellClick(index);
    }
    }

    // Если появятся новые значения enum и вы забудете обработать:
    return false;
}

bool CellInteractionManager::handleCellValueChanged(const QModelIndex& index, const QVariant& newValue)
{
    if (!index.isValid())
        return false;

    QString columnId = getColumnId(index.column());
    if (columnId.isEmpty())
        return false;

    if (!hasHandler(columnId))
        return false;

    CellActionHandler handler = m_handlers[columnId];
    CellActionResult result = handler(index.row(), index.column(), newValue);

    emit actionPerformed(columnId, result);
    return result.success;
}

// ========== Проверка ==========

bool CellInteractionManager::hasHandler(const QString& columnId) const
{
    return m_handlers.contains(columnId);
}

CellActionHandler CellInteractionManager::getHandler(const QString& columnId) const
{
    return m_handlers.value(columnId);
}

// ========== Приватные методы ==========

QString CellInteractionManager::getColumnId(int columnIndex) const
{
    TableColumn col = m_model->column(columnIndex);
    return col.id();
}

CellActionResult CellInteractionManager::executeHandler(const QString& columnId, int row, int col)
{
    if (!hasHandler(columnId)) {
        return CellActionResult(false, "No handler registered for column: " + columnId);
    }

    CellActionHandler handler = m_handlers[columnId];
    QVariant cellData = m_model->cellData(row, col);

    try {
        return handler(row, col, cellData);
    } catch (const std::exception& e) {
        return CellActionResult(false, QString("Handler exception: %1").arg(e.what()));
    } catch (...) {
        return CellActionResult(false, "Unknown handler exception");
    }
}

} // namespace TableSystem
