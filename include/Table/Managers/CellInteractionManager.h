#pragma once
#include <QObject>
#include <QMap>
#include "../TableTypes.h"

class QTableView;
class QModelIndex;
class QStyledItemDelegate;

namespace TableSystem {

class TableModel;

class CellInteractionManager : public QObject {
    Q_OBJECT

public:
    explicit CellInteractionManager(QTableView* tableView, TableModel* model,
                                    QObject* parent = nullptr);

    // ========== Регистрация обработчиков ==========

    // Для кнопок
    void registerButtonHandler(const QString& columnId, CellActionHandler handler);

    // Для комбобоксов
    void registerComboBoxHandler(const QString& columnId, CellActionHandler handler);

    // Для файловых диалогов
    void registerFileDialogHandler(const QString& columnId, CellActionHandler handler);

    // Универсальный обработчик
    void registerHandler(const QString& columnId, ColumnType type, CellActionHandler handler);

    // Удаление обработчиков
    void unregisterHandler(const QString& columnId);
    void unregisterAllHandlers();

    // ========== Регистрация делегатов (ЗАГОТОВКА) ==========

    void registerDelegate(const QString& columnId, QStyledItemDelegate* delegate);
    void unregisterDelegate(const QString& columnId);

    // ========== Обработка событий ==========

    bool handleCellClick(const QModelIndex& index);
    bool handleCellDoubleClick(const QModelIndex& index);
    bool handleCellValueChanged(const QModelIndex& index, const QVariant& newValue);

    // ========== Проверка ==========

    bool hasHandler(const QString& columnId) const;
    CellActionHandler getHandler(const QString& columnId) const;

signals:
    void actionPerformed(const QString& columnId, const CellActionResult& result);
    void handlerRegistered(const QString& columnId);
    void handlerUnregistered(const QString& columnId);

private:
    QTableView* m_tableView;
    TableModel* m_model;

    QMap<QString, CellActionHandler> m_handlers;
    QMap<QString, QStyledItemDelegate*> m_delegates;

    QString getColumnId(int columnIndex) const;
    CellActionResult executeHandler(const QString& columnId, int row, int col);
};

} // namespace TableSystem
