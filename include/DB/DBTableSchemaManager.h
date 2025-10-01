#pragma once
#include <QString>
#include <QStringList>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

// Имя соединения по умолчанию. Предполагается, что соединение уже создано и открыто.
#define DEFAULT_CONNECTION_NAME "default_connection"

// Описание одной колонки таблицы (используется для операций со схемой — DDL)
struct ColumnDefinition {
    // Имя колонки. Должно соответствовать правилам идентификаторов конкретной СУБД
    QString name;
    // Тип данных колонки (например: "INTEGER", "TEXT", "REAL", "BLOB")
    QString type;
    // Признак участия колонки в первичном ключе (PRIMARY KEY). Может быть составным
    bool isPrimaryKey = false;
    // Автоинкремент (зависит от СУБД; в SQLite работает для INTEGER PRIMARY KEY)
    bool isAutoIncrement = false;
    // Ограничение NOT NULL
    bool isNotNull = true;
    // Ограничение UNIQUE для одиночной колонки
    bool isUnique = false;
    // Значение по умолчанию. Будет подставлено как DEFAULT '...'
    QString defaultValue;

    // Минимальный конструктор: имя и тип
    ColumnDefinition(const QString& colName, const QString& colType)
        : name(colName), type(colType) {}

    // Полный конструктор для удобной инициализации всех признаков
    ColumnDefinition(const QString& colName, const QString& colType, bool primaryKey, bool autoIncrement, bool notNull, bool unique, const QString& defaultVal = QString())
        : name(colName), type(colType), isPrimaryKey(primaryKey), isAutoIncrement(autoIncrement),
          isNotNull(notNull), isUnique(unique), defaultValue(defaultVal) {}
};

// Класс для управления СХЕМОЙ таблиц (DDL): создание/изменение/удаление таблиц и индексов.
// Важно: не изменяет данные и не управляет подключением. Использует уже открытое соединение QSqlDatabase.
class DBTableSchemaManager {
public:
    // Использует соединение с именем DEFAULT_CONNECTION_NAME
    explicit DBTableSchemaManager();
    // Использовать указанное имя соединения QSqlDatabase (соединение должно быть создано и открыто снаружи)
    explicit DBTableSchemaManager(const QString& connectionName);

    // Создание и удаление таблиц
    // Создаёт таблицу с заданными колонками. Вернёт false, если таблица уже существует или входные данные некорректны
    bool createTable(const QString& tableName, const QList<ColumnDefinition>& columns);
    // Удаляет таблицу, если она существует
    bool dropTable(const QString& tableName);
    // Проверяет существование таблицы в текущей базе
    bool tableExists(const QString& tableName) const;
    // Переименовывает таблицу. Вернёт false, если новое имя занято или старое не найдено
    bool renameTable(const QString& oldName, const QString& newName);

    // Работа с колонками
    // Добавляет новую колонку. В некоторых СУБД добавление NOT NULL без DEFAULT запрещено
    bool addColumn(const QString& tableName, const ColumnDefinition& column);
    // Удаляет колонку. Поддержка ALTER TABLE ... DROP COLUMN зависит от СУБД/версии
    bool dropColumn(const QString& tableName, const QString& columnName);
    // Переименовывает колонку. Поддержка зависит от СУБД/версии
    bool renameColumn(const QString& tableName, const QString& oldName, const QString& newName);

    // Информация о структуре
    // Список имён таблиц в текущей базе
    QStringList getTableNames() const;
    // Список имён колонок таблицы
    QStringList getColumnNames(const QString& tableName) const;
    // Упрощённое описание колонок. Некоторые признаки (PK, ограничения) могут требовать доп. запросов
    QList<ColumnDefinition> getTableStructure(const QString& tableName) const;

    // Индексы
    // Создаёт обычный (неуникальный) индекс по перечисленным колонкам
    bool createIndex(const QString& indexName, const QString& tableName, const QStringList& columns);
    // Удаляет индекс по имени
    bool dropIndex(const QString& indexName);

    // Утилиты
    // Текст последней ошибки
    QString getLastError() const;
    // Очистить текст последней ошибки
    void clearError();

    bool setConnectionName(const QString& connectionName);

    QString getConnectionName() const;

private:
    // Последнее сообщение об ошибке операций со схемой
    QString m_lastError;
    // Имя используемого соединения QSqlDatabase
    QString m_connectionName;

    // Унифицированное выполнение DDL-запроса с сохранением текста ошибки
    bool executeQuery(const QString& query);
    // Построение SQL для CREATE TABLE на основе списка колонок
    QString buildCreateTableQuery(const QString& tableName, const QList<ColumnDefinition>& columns) const;
    // Получить объект базы по имени соединения (соединение должно быть открыто заранее)
    QSqlDatabase getDatabase() const;
    // Простая валидация имени таблицы
    bool validateTableName(const QString& tableName) const;
    // Простая валидация имени колонки
    bool validateColumnName(const QString& columnName) const;
};
