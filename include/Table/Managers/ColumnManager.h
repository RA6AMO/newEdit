#pragma once
#include <QObject>
#include "../TableTypes.h"
#include "../TableColumn.h"
#include <QList>

namespace TableSystem {

class TableModel;

class ColumnManager : public QObject {
    Q_OBJECT

public:
    explicit ColumnManager(TableModel* model, QObject* parent = nullptr);

    // ========== Добавление столбцов ==========

    // Простое добавление
    bool addColumn(const QString& name, ColumnType type, bool editable = true);

    // Полное описание
    bool addColumn(const TableColumn& column);

    // Специализированные методы (ЗАГОТОВКИ - для вашей реализации)
    bool addTextColumn(const QString& name, const QVariant& defaultValue = QString());
    bool addNumberColumn(const QString& name, double defaultValue = 0.0);
    bool addComboBoxColumn(const QString& name, const QStringList& items);
    bool addButtonColumn(const QString& name, const QString& buttonText);
    bool addFileDialogColumn(const QString& name, const QString& filter = "All Files (*)");

    // ========== Удаление столбцов ==========

    bool removeColumn(const QString& columnId);
    bool removeColumn(int columnIndex);

    // ========== Изменение столбцов ==========

    bool setColumnVisible(const QString& columnId, bool visible);
    bool setColumnEditable(const QString& columnId, bool editable);
    bool renameColumn(const QString& columnId, const QString& newName);
    bool updateColumn(const QString& columnId, const TableColumn& newColumn);

    // ========== Получение информации ==========

    QList<TableColumn> allColumns() const;
    TableColumn column(const QString& columnId) const;
    int columnCount() const;
    bool columnExists(const QString& columnId) const;

    // ========== Генерация ID ==========

    QString generateUniqueColumnId(const QString& baseName) const;

signals:
    void columnAdded(const TableColumn& column);
    void columnRemoved(const QString& columnId);
    void columnModified(const QString& columnId);

private:
    TableModel* m_model;

    QString sanitizeColumnId(const QString& name) const;
    bool validateColumnId(const QString& id) const;
};

} // namespace TableSystem
