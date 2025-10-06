#include "DBTableSchemaManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>
#include <QMetaType>

DBTableSchemaManager::DBTableSchemaManager()
    : m_connectionName(DEFAULT_CONNECTION_NAME)
{
}

DBTableSchemaManager::DBTableSchemaManager(const QString& connectionName)
    : m_connectionName(connectionName)
{
}

bool DBTableSchemaManager::createTable(const QString& tableName, const QList<ColumnDefinition>& columns)
{
    if (!validateTableName(tableName)) {
        m_lastError = QString("Неверное имя таблицы: %1").arg(tableName);
        return false;
    }

    if (columns.isEmpty()) {
        m_lastError = "Список колонок не может быть пустым";
        return false;
    }

    if (tableExists(tableName)) {
        m_lastError = QString("Таблица %1 уже существует").arg(tableName);
        return false;
    }

    QString query = buildCreateTableQuery(tableName, columns);
    return executeQuery(query);
}

bool DBTableSchemaManager::dropTable(const QString& tableName)
{
    if (!validateTableName(tableName)) {
        m_lastError = QString("Неверное имя таблицы: %1").arg(tableName);
        return false;
    }

    if (!tableExists(tableName)) {
        m_lastError = QString("Таблица %1 не существует").arg(tableName);
        return false;
    }

    QString query = QString("DROP TABLE %1").arg(tableName);
    return executeQuery(query);
}

bool DBTableSchemaManager::tableExists(const QString& tableName) const
{
    if (!validateTableName(tableName)) {
        return false;
    }

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        return false;
    }

    QStringList tables = db.tables();
    return tables.contains(tableName, Qt::CaseInsensitive);
}

bool DBTableSchemaManager::renameTable(const QString& oldName, const QString& newName)
{
    if (!validateTableName(oldName) || !validateTableName(newName)) {
        m_lastError = "Неверное имя таблицы";
        return false;
    }

    if (!tableExists(oldName)) {
        m_lastError = QString("Таблица %1 не существует").arg(oldName);
        return false;
    }

    if (tableExists(newName)) {
        m_lastError = QString("Таблица %1 уже существует").arg(newName);
        return false;
    }

    QString query = QString("ALTER TABLE %1 RENAME TO %2").arg(oldName, newName);
    return executeQuery(query);
}

bool DBTableSchemaManager::addColumn(const QString& tableName, const ColumnDefinition& column)
{
    if (!validateTableName(tableName) || !validateColumnName(column.name)) {
        m_lastError = "Неверное имя таблицы или колонки";
        return false;
    }

    if (!tableExists(tableName)) {
        m_lastError = QString("Таблица %1 не существует").arg(tableName);
        return false;
    }

    QStringList existingColumns = getColumnNames(tableName);
    if (existingColumns.contains(column.name, Qt::CaseInsensitive)) {
        m_lastError = QString("Колонка %1 уже существует в таблице %2").arg(column.name, tableName);
        return false;
    }

    QString query = QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(tableName, column.name, column.type);

    if (column.isNotNull && !column.defaultValue.isEmpty()) {
        query += QString(" NOT NULL DEFAULT '%1'").arg(column.defaultValue);
    } else if (column.isNotNull) {
        query += " NOT NULL";
    } else if (!column.defaultValue.isEmpty()) {
        query += QString(" DEFAULT '%1'").arg(column.defaultValue);
    }

    if (column.isUnique) {
        query += " UNIQUE";
    }

    return executeQuery(query);
}

bool DBTableSchemaManager::dropColumn(const QString& tableName, const QString& columnName)
{
    if (!validateTableName(tableName) || !validateColumnName(columnName)) {
        m_lastError = "Неверное имя таблицы или колонки";
        return false;
    }

    if (!tableExists(tableName)) {
        m_lastError = QString("Таблица %1 не существует").arg(tableName);
        return false;
    }

    QStringList existingColumns = getColumnNames(tableName);
    if (!existingColumns.contains(columnName, Qt::CaseInsensitive)) {
        m_lastError = QString("Колонка %1 не существует в таблице %2").arg(columnName, tableName);
        return false;
    }

    QString query = QString("ALTER TABLE %1 DROP COLUMN %2").arg(tableName, columnName);
    return executeQuery(query);
}

bool DBTableSchemaManager::renameColumn(const QString& tableName, const QString& oldName, const QString& newName)
{
    if (!validateTableName(tableName) || !validateColumnName(oldName) || !validateColumnName(newName)) {
        m_lastError = "Неверное имя таблицы или колонки";
        return false;
    }

    if (!tableExists(tableName)) {
        m_lastError = QString("Таблица %1 не существует").arg(tableName);
        return false;
    }

    QStringList existingColumns = getColumnNames(tableName);
    if (!existingColumns.contains(oldName, Qt::CaseInsensitive)) {
        m_lastError = QString("Колонка %1 не существует в таблице %2").arg(oldName, tableName);
        return false;
    }

    if (existingColumns.contains(newName, Qt::CaseInsensitive)) {
        m_lastError = QString("Колонка %1 уже существует в таблице %2").arg(newName, tableName);
        return false;
    }

    QString query = QString("ALTER TABLE %1 RENAME COLUMN %2 TO %3").arg(tableName, oldName, newName);
    return executeQuery(query);
}

QStringList DBTableSchemaManager::getTableNames() const
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        return QStringList();
    }

    return db.tables();
}

QStringList DBTableSchemaManager::getColumnNames(const QString& tableName) const
{
    if (!validateTableName(tableName)) {
        return QStringList();
    }

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        return QStringList();
    }

    QStringList columns;
    QSqlRecord record = db.record(tableName);

    for (int i = 0; i < record.count(); ++i) {
        columns.append(record.fieldName(i));
    }

    return columns;
}

QList<ColumnDefinition> DBTableSchemaManager::getTableStructure(const QString& tableName) const
{
    QList<ColumnDefinition> structure;

    if (!validateTableName(tableName)) {
        return structure;
    }

    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        return structure;
    }

    QSqlRecord record = db.record(tableName);

    for (int i = 0; i < record.count(); ++i) {
        QSqlField field = record.field(i);

        QString typeName;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        typeName = QString::fromLatin1(field.metaType().name());
#else
        typeName = QString::fromLatin1(QMetaType::typeName(static_cast<int>(field.type())));
#endif
        ColumnDefinition column(field.name(), typeName);
        column.isNotNull = !field.isNull();
        column.isAutoIncrement = field.isAutoValue();
        column.defaultValue = field.defaultValue().toString();

        // Для определения первичного ключа нужно выполнить дополнительный запрос
        // В SQLite это можно сделать через PRAGMA table_info

        structure.append(column);
    }

    return structure;
}

bool DBTableSchemaManager::createIndex(const QString& indexName, const QString& tableName, const QStringList& columns)
{
    if (!validateTableName(tableName) || indexName.isEmpty()) {
        m_lastError = "Неверное имя таблицы или индекса";
        return false;
    }

    if (!tableExists(tableName)) {
        m_lastError = QString("Таблица %1 не существует").arg(tableName);
        return false;
    }

    if (columns.isEmpty()) {
        m_lastError = "Список колонок для индекса не может быть пустым";
        return false;
    }

    QString columnsStr = columns.join(", ");
    QString query = QString("CREATE INDEX %1 ON %2 (%3)").arg(indexName, tableName, columnsStr);

    return executeQuery(query);
}

bool DBTableSchemaManager::dropIndex(const QString& indexName)
{
    if (indexName.isEmpty()) {
        m_lastError = "Имя индекса не может быть пустым";
        return false;
    }

    QString query = QString("DROP INDEX %1").arg(indexName);
    return executeQuery(query);
}

QString DBTableSchemaManager::getLastError() const
{
    return m_lastError;
}

void DBTableSchemaManager::clearError()
{
    m_lastError.clear();
}

bool DBTableSchemaManager::executeQuery(const QString& query)
{
    QSqlDatabase db = getDatabase();
    if (!db.isOpen()) {
        m_lastError = "База данных не открыта";
        return false;
    }

    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(query)) {
        m_lastError = sqlQuery.lastError().text();
        return false;
    }

    return true;
}

QString DBTableSchemaManager::buildCreateTableQuery(const QString& tableName, const QList<ColumnDefinition>& columns) const
{
    QStringList columnDefinitions;
    QStringList primaryKeys;

    // Сначала собираем список PRIMARY KEY колонок
    for (const auto& column : columns) {
        if (column.isPrimaryKey) {
            primaryKeys.append(column.name);
        }
    }

    for (const auto& column : columns) {
        QString columnDef = QString("%1 %2").arg(column.name, column.type);

        // Если это одиночный PRIMARY KEY - добавляем inline
        if (column.isPrimaryKey && primaryKeys.size() == 1) {
            columnDef += " PRIMARY KEY";

            // AUTOINCREMENT можно использовать только с INTEGER PRIMARY KEY
            if (column.isAutoIncrement) {
                columnDef += " AUTOINCREMENT";
            }
        }

        if (column.isNotNull) {
            columnDef += " NOT NULL";
        }

        if (column.isUnique) {
            columnDef += " UNIQUE";
        }

        if (!column.defaultValue.isEmpty()) {
            columnDef += QString(" DEFAULT '%1'").arg(column.defaultValue);
        }

        columnDefinitions.append(columnDef);
    }

    // Для составного PRIMARY KEY добавляем отдельно
    if (primaryKeys.size() > 1) {
        columnDefinitions.append(QString("PRIMARY KEY (%1)").arg(primaryKeys.join(", ")));
    }

    return QString("CREATE TABLE %1 (%2)").arg(tableName, columnDefinitions.join(", "));
}

QSqlDatabase DBTableSchemaManager::getDatabase() const
{
    return QSqlDatabase::database(m_connectionName);
}

bool DBTableSchemaManager::validateTableName(const QString& tableName) const
{
    if (tableName.isEmpty()) {
        return false;
    }

    // Проверяем, что имя таблицы содержит только допустимые символы
    for (const QChar& ch : tableName) {
        if (!ch.isLetterOrNumber() && ch != '_' && ch != '-') {
            return false;
        }
    }

    return true;
}

bool DBTableSchemaManager::validateColumnName(const QString& columnName) const
{
    if (columnName.isEmpty()) {
        return false;
    }

    // Проверяем, что имя колонки содержит только допустимые символы
    for (const QChar& ch : columnName) {
        if (!ch.isLetterOrNumber() && ch != '_' && ch != '-') {
            return false;
        }
    }

    return true;
}

bool DBTableSchemaManager::setConnectionName(const QString& connectionName)
{
    m_connectionName = connectionName;
    return true;
}

QString DBTableSchemaManager::getConnectionName() const
{
    return m_connectionName;
}
