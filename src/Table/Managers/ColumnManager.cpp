#include "Table/Managers/ColumnManager.h"
#include "Table/TableModel.h"
#include <QRegularExpression>
#include <QDebug>

namespace TableSystem {

ColumnManager::ColumnManager(TableModel* model, QObject* parent)
    : QObject(parent)
    , m_model(model)
{
}

// ========== Добавление столбцов ==========

bool ColumnManager::addColumn(const QString& name, ColumnType type, bool editable)
{
    if (name.isEmpty()) {
        qWarning() << "ColumnManager: Column name cannot be empty";
        return false;
    }

    TableColumn column;
    column.setId(generateUniqueColumnId(name));
    column.setName(name);
    column.setType(type);
    column.setEditable(editable);
    column.setVisible(true);

    return addColumn(column);
}

bool ColumnManager::addColumn(const TableColumn& column)
{
    if (!validateColumnId(column.id())) {
        qWarning() << "ColumnManager: Invalid column ID:" << column.id();
        return false;
    }

    if (columnExists(column.id())) {
        qWarning() << "ColumnManager: Column with ID" << column.id() << "already exists";
        return false;
    }

    m_model->addColumn(column);
    emit columnAdded(column);

    return true;
}

// Специализированные методы - ЗАГОТОВКИ
bool ColumnManager::addTextColumn(const QString& name, const QVariant& defaultValue)
{
    TableColumn column;
    column.setId(generateUniqueColumnId(name));
    column.setName(name);
    column.setType(ColumnType::Text);
    column.setDefaultValue(defaultValue.isValid() ? defaultValue : QString());
    column.setEditable(true);

    return addColumn(column);
}

bool ColumnManager::addNumberColumn(const QString& name, double defaultValue)
{
    TableColumn column;
    column.setId(generateUniqueColumnId(name));
    column.setName(name);
    column.setType(ColumnType::Number);
    column.setDefaultValue(defaultValue);
    column.setEditable(true);

    return addColumn(column);
}

bool ColumnManager::addComboBoxColumn(const QString& name, const QStringList& items)
{
    TableColumn column;
    column.setId(generateUniqueColumnId(name));
    column.setName(name);
    column.setType(ColumnType::ComboBox);
    column.setComboBoxItems(items);
    column.setDefaultValue(items.isEmpty() ? QString() : items.first());
    column.setEditable(true);

    return addColumn(column);
}

bool ColumnManager::addButtonColumn(const QString& name, const QString& buttonText)
{
    TableColumn column;
    column.setId(generateUniqueColumnId(name));
    column.setName(name);
    column.setType(ColumnType::Button);
    column.setButtonText(buttonText);
    column.setEditable(false); // Кнопки не редактируются

    return addColumn(column);
}

bool ColumnManager::addFileDialogColumn(const QString& name, const QString& filter)
{
    TableColumn column;
    column.setId(generateUniqueColumnId(name));
    column.setName(name);
    column.setType(ColumnType::FileDialog);
    column.setFileFilter(filter);
    column.setEditable(true);

    return addColumn(column);
}

// ========== Удаление столбцов ==========

bool ColumnManager::removeColumn(const QString& columnId)
{
    if (!columnExists(columnId)) {
        qWarning() << "ColumnManager: Column" << columnId << "does not exist";
        return false;
    }

    m_model->removeColumn(columnId);
    emit columnRemoved(columnId);

    return true;
}

bool ColumnManager::removeColumn(int columnIndex)
{
    QList<TableColumn> cols = m_model->columns();

    if (columnIndex < 0 || columnIndex >= cols.size()) {
        qWarning() << "ColumnManager: Invalid column index:" << columnIndex;
        return false;
    }

    QString columnId = cols[columnIndex].id();
    m_model->removeColumn(columnIndex);
    emit columnRemoved(columnId);

    return true;
}

// ========== Изменение столбцов ==========

bool ColumnManager::setColumnVisible(const QString& columnId, bool visible)
{
    TableColumn col = m_model->column(columnId);
    if (col.id().isEmpty())
        return false;

    col.setVisible(visible);
    return updateColumn(columnId, col);
}

bool ColumnManager::setColumnEditable(const QString& columnId, bool editable)
{
    TableColumn col = m_model->column(columnId);
    if (col.id().isEmpty())
        return false;

    col.setEditable(editable);
    return updateColumn(columnId, col);
}

bool ColumnManager::renameColumn(const QString& columnId, const QString& newName)
{
    TableColumn col = m_model->column(columnId);
    if (col.id().isEmpty())
        return false;

    col.setName(newName);
    return updateColumn(columnId, col);
}

bool ColumnManager::updateColumn(const QString& columnId, const TableColumn& newColumn)
{
    int idx = m_model->columnIndex(columnId);
    if (idx == -1) {
        qWarning() << "ColumnManager: Column" << columnId << "not found";
        return false;
    }

    // Удаляем старый и добавляем новый
    // TODO: Оптимизировать - создать метод updateColumn в TableModel
    m_model->removeColumn(idx);
    m_model->addColumn(newColumn);

    emit columnModified(columnId);
    return true;
}

// ========== Получение информации ==========

QList<TableColumn> ColumnManager::allColumns() const
{
    return m_model->columns();
}

TableColumn ColumnManager::column(const QString& columnId) const
{
    return m_model->column(columnId);
}

int ColumnManager::columnCount() const
{
    return m_model->columns().size();
}

bool ColumnManager::columnExists(const QString& columnId) const
{
    return m_model->columnIndex(columnId) != -1;
}

// ========== Генерация ID ==========

QString ColumnManager::generateUniqueColumnId(const QString& baseName) const
{
    QString sanitized = sanitizeColumnId(baseName);
    QString id = sanitized;
    int counter = 1;

    while (columnExists(id)) {
        id = QString("%1_%2").arg(sanitized).arg(counter++);
    }

    return id;
}

QString ColumnManager::sanitizeColumnId(const QString& name) const
{
    QString result = name.toLower();

    // Заменяем пробелы на подчеркивания
    result.replace(' ', '_');

    // Удаляем все, кроме букв, цифр и подчеркиваний
    QRegularExpression regex("[^a-z0-9_]");
    result.remove(regex);

    // Если пустая строка, используем "column"
    if (result.isEmpty())
        result = "column";

    return result;
}

bool ColumnManager::validateColumnId(const QString& id) const
{
    if (id.isEmpty())
        return false;

    // ID должен содержать только буквы, цифры и подчеркивания
    QRegularExpression regex("^[a-z0-9_]+$");
    return regex.match(id).hasMatch();
}

} // namespace TableSystem
