#pragma once
#include <QVariant>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QDateTime>
#include <functional>

namespace TableSystem {

// Типы столбцов
enum class ColumnType {
    Text,        // QString
    Number,      // int, double
    Date,        // QDateTime
    Image,       // QImage, путь к файлу
    Boolean,     // bool
    ComboBox,    // Выпадающий список
    Button,      // Кнопка в ячейке
    FileDialog,  // Диалог выбора файла
    Custom       // Пользовательский тип
};

// Операции изменения данных
enum class ChangeType {
    Insert,
    Update,
    Delete,
    ColumnAdd,
    ColumnRemove
};

// Результат действия с ячейкой
struct CellActionResult {
    bool success;
    QString message;
    QVariant data;
    QVariantMap additionalData;

    CellActionResult() : success(false) {}
    CellActionResult(bool s, const QString& msg = QString(), const QVariant& d = QVariant())
        : success(s), message(msg), data(d) {}
};

// Тип функции-обработчика для интерактивных ячеек
using CellActionHandler = std::function<CellActionResult(int row, int col, const QVariant& cellData)>;

} // namespace TableSystem
