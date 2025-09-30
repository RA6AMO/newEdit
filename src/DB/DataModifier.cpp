#include "DataModifier.h"
#include <QDebug>

// ========================================
// === КОНСТРУКТОР И ДЕСТРУКТОР ===
// ========================================

DataModifier::DataModifier(const QString& connectionName)
    : m_connectionName(connectionName)
    , m_lastInsertId(-1)
    , m_affectedRows(0)
    , m_inTransaction(false)
{}

DataModifier::~DataModifier()
{
    // Откатываем незавершенную транзакцию при уничтожении объекта
    if (m_inTransaction) {
        rollbackTransaction();
    }
}

void DataModifier::setConnectionName(const QString& connectionName)
{
    m_connectionName = connectionName;
}

// ========================================
// === БАЗОВЫЕ ОПЕРАЦИИ INSERT ===
// ========================================

bool DataModifier::insertRecord(const QString& tableName, const QVariantMap& values)
{
    if (tableName.isEmpty() || values.isEmpty()) {
        m_lastError = "Table name or values are empty";
        return false;
    }

    QStringList columns;
    QStringList placeholders;
    QVariantList bindValues;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        columns << it.key();
        placeholders << "?";
        bindValues << it.value();
    }

    QString queryStr = QString("INSERT INTO %1 (%2) VALUES (%3)")
                           .arg(tableName)
                           .arg(columns.join(", "))
                           .arg(placeholders.join(", "));

    QSqlQuery query(getDatabase());
    query.prepare(queryStr);

    for (const QVariant& value : bindValues) {
        query.addBindValue(value);
    }

    return executeAndUpdateStats(query);
}

int DataModifier::insertRecords(const QString& tableName, const QStringList& columns,
                                const QList<QVariantList>& values)
{
    if (tableName.isEmpty() || columns.isEmpty() || values.isEmpty()) {
        m_lastError = "Table name, columns or values are empty";
        return 0;
    }

    QString placeholders = buildPlaceholders(columns.size());
    QString queryStr = QString("INSERT INTO %1 (%2) VALUES (%3)")
                           .arg(tableName)
                           .arg(columns.join(", "))
                           .arg(placeholders);

    QSqlQuery query(getDatabase());
    query.prepare(queryStr);

    int successCount = 0;

    for (const QVariantList& record : values) {
        if (record.size() != columns.size()) {
            continue; // Пропускаем записи с неверным количеством значений
        }

        for (const QVariant& value : record) {
            query.addBindValue(value);
        }

        if (query.exec()) {
            successCount++;
        } else {
            setError(query.lastError());
        }
    }

    return successCount;
}

qint64 DataModifier::insertRecordAndReturnId(const QString& tableName,
                                             const QVariantMap& values,
                                             const QString& /*idColumnName*/)
{
    if (!insertRecord(tableName, values)) {
        return -1;
    }

    return m_lastInsertId;
}

// ========================================
// === БАЗОВЫЕ ОПЕРАЦИИ UPDATE ===
// ========================================

int DataModifier::updateRecords(const QString& tableName, const QVariantMap& values,
                                const QString& whereClause)
{
    if (tableName.isEmpty() || values.isEmpty()) {
        m_lastError = "Table name or values are empty";
        return -1;
    }

    QStringList setClauses;
    QVariantList bindValues;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        setClauses << QString("%1 = ?").arg(it.key());
        bindValues << it.value();
    }

    QString queryStr = QString("UPDATE %1 SET %2")
                           .arg(tableName)
                           .arg(setClauses.join(", "));

    if (!whereClause.isEmpty()) {
        queryStr += " WHERE " + whereClause;
    }

    QSqlQuery query(getDatabase());
    query.prepare(queryStr);

    for (const QVariant& value : bindValues) {
        query.addBindValue(value);
    }

    if (executeAndUpdateStats(query)) {
        return m_affectedRows;
    }

    return -1;
}

bool DataModifier::updateRecordById(const QString& tableName, const QVariant& recordId,
                                   const QVariantMap& values, const QString& idColumn)
{
    QString whereClause = QString("%1 = %2").arg(idColumn).arg(escapeValue(recordId));
    int affected = updateRecords(tableName, values, whereClause);
    return affected > 0;
}

int DataModifier::updateColumn(const QString& tableName, const QString& columnName,
                               const QVariant& value, const QString& whereClause)
{
    QVariantMap values;
    values[columnName] = value;
    return updateRecords(tableName, values, whereClause);
}

// ========================================
// === БАЗОВЫЕ ОПЕРАЦИИ DELETE ===
// ========================================

int DataModifier::deleteRecords(const QString& tableName, const QString& whereClause)
{
    if (tableName.isEmpty()) {
        m_lastError = "Table name is empty";
        return -1;
    }

    QString queryStr = QString("DELETE FROM %1").arg(tableName);

    if (!whereClause.isEmpty()) {
        queryStr += " WHERE " + whereClause;
    }

    QSqlQuery query(getDatabase());
    if (!query.exec(queryStr)) {
        setError(query.lastError());
        return -1;
    }

    m_affectedRows = query.numRowsAffected();
    return m_affectedRows;
}

bool DataModifier::deleteRecordById(const QString& tableName, const QVariant& recordId,
                                   const QString& idColumn)
{
    QString whereClause = QString("%1 = %2").arg(idColumn).arg(escapeValue(recordId));
    int affected = deleteRecords(tableName, whereClause);
    return affected > 0;
}

bool DataModifier::deleteAllRecords(const QString& tableName)
{
    return deleteRecords(tableName, QString()) >= 0;
}

bool DataModifier::truncateTable(const QString& tableName)
{
    if (tableName.isEmpty()) {
        m_lastError = "Table name is empty";
        return false;
    }

    QSqlQuery query(getDatabase());
    QString queryStr;

    // SQLite не поддерживает TRUNCATE, используем DELETE
    QSqlDatabase database = getDatabase();
    if (database.driverName().contains("SQLITE", Qt::CaseInsensitive)) {
        queryStr = QString("DELETE FROM %1").arg(tableName);
        if (!query.exec(queryStr)) {
            setError(query.lastError());
            return false;
        }
        // Сбрасываем автоинкремент для SQLite
        queryStr = QString("DELETE FROM sqlite_sequence WHERE name='%1'").arg(tableName);
        query.exec(queryStr); // Игнорируем ошибку, если таблицы нет
        return true;
    }
    queryStr = QString("TRUNCATE TABLE %1").arg(tableName);
    if (!query.exec(queryStr)) {
        setError(query.lastError());
        return false;
    }
    m_affectedRows = query.numRowsAffected();
    return true;
}

// ========================================
// === ОПЕРАЦИИ UPSERT ===
// ========================================

bool DataModifier::upsertRecord(const QString& tableName, const QVariantMap& values,
                               const QStringList& conflictColumns)
{
    if (tableName.isEmpty() || values.isEmpty() || conflictColumns.isEmpty()) {
        m_lastError = "Table name, values or conflict columns are empty";
        return false;
    }

    QStringList columns;
    QStringList placeholders;
    QVariantList bindValues;

    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        columns << it.key();
        placeholders << "?";
        bindValues << it.value();
    }

    QSqlDatabase database = getDatabase();
    QString queryStr;

    if (database.driverName().contains("SQLITE", Qt::CaseInsensitive)) {
        // SQLite: INSERT ... ON CONFLICT ... DO UPDATE
        QStringList updateClauses;
        for (const QString& col : columns) {
            if (!conflictColumns.contains(col)) {
                updateClauses << QString("%1 = excluded.%1").arg(col);
            }
        }

        queryStr = QString("INSERT INTO %1 (%2) VALUES (%3) ON CONFLICT(%4) DO UPDATE SET %5")
                       .arg(tableName)
                       .arg(columns.join(", "))
                       .arg(placeholders.join(", "))
                       .arg(conflictColumns.join(", "))
                       .arg(updateClauses.join(", "));
    } else {
        // MySQL: REPLACE INTO
        queryStr = QString("REPLACE INTO %1 (%2) VALUES (%3)")
                       .arg(tableName)
                       .arg(columns.join(", "))
                       .arg(placeholders.join(", "));
    }

    QSqlQuery query(getDatabase());
    query.prepare(queryStr);

    for (const QVariant& value : bindValues) {
        query.addBindValue(value);
    }

    return executeAndUpdateStats(query);
}

bool DataModifier::insertIfNotExists(const QString& tableName, const QVariantMap& values,
                                    const QStringList& checkColumns)
{
    if (tableName.isEmpty() || values.isEmpty() || checkColumns.isEmpty()) {
        m_lastError = "Table name, values or check columns are empty";
        return false;
    }

    // Проверяем существование записи
    QStringList whereClauses;
    for (const QString& col : checkColumns) {
        if (values.contains(col)) {
            whereClauses << QString("%1 = %2").arg(col).arg(escapeValue(values[col]));
        }
    }

    QString checkQuery = QString("SELECT COUNT(*) FROM %1 WHERE %2")
                             .arg(tableName)
                             .arg(whereClauses.join(" AND "));

    QSqlQuery query(getDatabase());
    if (!query.exec(checkQuery)) {
        setError(query.lastError());
        return false;
    }

    if (query.next() && query.value(0).toInt() > 0) {
        // Запись уже существует
        return false;
    }

    // Вставляем новую запись
    return insertRecord(tableName, values);
}

// ========================================
// === ТРАНЗАКЦИИ ===
// ========================================

bool DataModifier::beginTransaction()
{
    if (m_inTransaction) {
        m_lastError = "Transaction already in progress";
        return false;
    }

    QSqlDatabase database = getDatabase();
    if (database.transaction()) {
        m_inTransaction = true;
        clearLastError();
        return true;
    }

    setError(database.lastError());
    return false;
}

bool DataModifier::commitTransaction()
{
    if (!m_inTransaction) {
        m_lastError = "No transaction in progress";
        return false;
    }

    QSqlDatabase database = getDatabase();
    if (database.commit()) {
        m_inTransaction = false;
        clearLastError();
        return true;
    }

    setError(database.lastError());
    return false;
}

bool DataModifier::rollbackTransaction()
{
    if (!m_inTransaction) {
        m_lastError = "No transaction in progress";
        return false;
    }

    QSqlDatabase database = getDatabase();
    if (database.rollback()) {
        m_inTransaction = false;
        clearLastError();
        return true;
    }

    setError(database.lastError());
    return false;
}

bool DataModifier::executeInTransaction(const std::function<bool()>& operations)
{
    if (!beginTransaction()) {
        return false;
    }

    bool success = false;
    try {
        success = operations();
    } catch (...) {
        rollbackTransaction();
        throw;
    }

    if (success) {
        return commitTransaction();
    }
    rollbackTransaction();
    return false;
}

bool DataModifier::isInTransaction() const
{
    return m_inTransaction;
}

// ========================================
// === ПАКЕТНЫЕ ОПЕРАЦИИ ===
// ========================================

int DataModifier::batchInsert(const QString& tableName, const QStringList& columns,
                              const QList<QVariantList>& values, int batchSize)
{
    if (tableName.isEmpty() || columns.isEmpty() || values.isEmpty()) {
        m_lastError = "Table name, columns or values are empty";
        return 0;
    }

    QString placeholders = buildPlaceholders(columns.size());
    QString queryStr = QString("INSERT INTO %1 (%2) VALUES (%3)")
                           .arg(tableName)
                           .arg(columns.join(", "))
                           .arg(placeholders);

    QSqlQuery query(getDatabase());
    query.prepare(queryStr);

    int successCount = 0;
    int currentBatch = 0;
    bool inLocalTransaction = false;

    // Если не в транзакции и указан размер пакета, создаем транзакцию
    if (!m_inTransaction && batchSize > 0) {
        beginTransaction();
        inLocalTransaction = true;
    }

    for (const QVariantList& record : values) {
        if (record.size() != columns.size()) {
            continue;
        }

        for (const QVariant& value : record) {
            query.addBindValue(value);
        }

        if (query.exec()) {
            successCount++;
            currentBatch++;

            // Коммитим пакет, если достигли размера
            if (inLocalTransaction && batchSize > 0 && currentBatch >= batchSize) {
                commitTransaction();
                beginTransaction();
                currentBatch = 0;
            }
        } else {
            setError(query.lastError());
        }
    }

    if (inLocalTransaction) {
        commitTransaction();
    }

    return successCount;
}

int DataModifier::batchUpdate(const QString& tableName, const QList<QVariantMap>& updates,
                              const QString& idColumn)
{
    if (tableName.isEmpty() || updates.isEmpty()) {
        m_lastError = "Table name or updates are empty";
        return 0;
    }

    int successCount = 0;
    bool inLocalTransaction = false;

    if (!m_inTransaction) {
        beginTransaction();
        inLocalTransaction = true;
    }

    for (const QVariantMap& update : updates) {
        if (!update.contains(idColumn)) {
            continue;
        }

        QVariant recordId = update[idColumn];
        QVariantMap values = update;
        values.remove(idColumn);

        if (updateRecordById(tableName, recordId, values, idColumn)) {
            successCount++;
        }
    }

    if (inLocalTransaction) {
        commitTransaction();
    }

    return successCount;
}

int DataModifier::batchDelete(const QString& tableName, const QVariantList& ids,
                              const QString& idColumn)
{
    if (tableName.isEmpty() || ids.isEmpty()) {
        m_lastError = "Table name or IDs are empty";
        return 0;
    }

    QStringList idStrings;
    for (const QVariant& recordId : ids) {
        idStrings << escapeValue(recordId);
    }

    QString whereClause = QString("%1 IN (%2)").arg(idColumn).arg(idStrings.join(", "));
    return deleteRecords(tableName, whereClause);
}

// ========================================
// === СПЕЦИАЛИЗИРОВАННЫЕ ОПЕРАЦИИ ===
// ========================================

int DataModifier::incrementValue(const QString& tableName, const QString& columnName,
                                 int increment, const QString& whereClause)
{
    if (tableName.isEmpty() || columnName.isEmpty()) {
        m_lastError = "Table name or column name is empty";
        return -1;
    }

    QString queryStr = QString("UPDATE %1 SET %2 = %2 + %3")
                           .arg(tableName)
                           .arg(columnName)
                           .arg(increment);

    if (!whereClause.isEmpty()) {
        queryStr += " WHERE " + whereClause;
    }

    QSqlQuery query(getDatabase());
    if (!query.exec(queryStr)) {
        setError(query.lastError());
        return -1;
    }

    m_affectedRows = query.numRowsAffected();
    return m_affectedRows;
}

int DataModifier::decrementValue(const QString& tableName, const QString& columnName,
                                 int decrement, const QString& whereClause)
{
    return incrementValue(tableName, columnName, -decrement, whereClause);
}

int DataModifier::replaceNullValues(const QString& tableName, const QString& columnName,
                                   const QVariant& defaultValue)
{
    if (tableName.isEmpty() || columnName.isEmpty()) {
        m_lastError = "Table name or column name is empty";
        return -1;
    }

    QString queryStr = QString("UPDATE %1 SET %2 = %3 WHERE %2 IS NULL")
                           .arg(tableName)
                           .arg(columnName)
                           .arg(escapeValue(defaultValue));

    QSqlQuery query(getDatabase());
    if (!query.exec(queryStr)) {
        setError(query.lastError());
        return -1;
    }

    m_affectedRows = query.numRowsAffected();
    return m_affectedRows;
}

int DataModifier::copyRecords(const QString& tableName, const QString& whereClause,
                             const QVariantMap& modifications)
{
    if (tableName.isEmpty()) {
        m_lastError = "Table name is empty";
        return 0;
    }

    // Получаем список колонок таблицы
    QSqlQuery query(getDatabase());
    QString selectQuery = QString("SELECT * FROM %1 WHERE %2")
                              .arg(tableName)
                              .arg(whereClause.isEmpty() ? "1=1" : whereClause);

    if (!query.exec(selectQuery)) {
        setError(query.lastError());
        return 0;
    }

    QSqlRecord record = query.record();
    QStringList columns;
    for (int i = 0; i < record.count(); ++i) {
        columns << record.fieldName(i);
    }

    int copyCount = 0;
    bool inLocalTransaction = false;

    if (!m_inTransaction) {
        beginTransaction();
        inLocalTransaction = true;
    }

    while (query.next()) {
        QVariantMap values;
        for (const QString& col : columns) {
            values[col] = query.value(col);
        }

        // Применяем модификации
        for (auto it = modifications.constBegin(); it != modifications.constEnd(); ++it) {
            values[it.key()] = it.value();
        }

        // Удаляем автоинкрементный ID, если он есть
        if (values.contains("id")) {
            values.remove("id");
        }

        if (insertRecord(tableName, values)) {
            copyCount++;
        }
    }

    if (inLocalTransaction) {
        commitTransaction();
    }

    return copyCount;
}

// ========================================
// === ВЫПОЛНЕНИЕ ПРОИЗВОЛЬНЫХ ЗАПРОСОВ ===
// ========================================

int DataModifier::executeModifyQuery(const QString& queryStr)
{
    if (queryStr.isEmpty()) {
        m_lastError = "Query string is empty";
        return -1;
    }

    QSqlQuery query(getDatabase());
    if (!query.exec(queryStr)) {
        setError(query.lastError());
        return -1;
    }

    m_affectedRows = query.numRowsAffected();
    m_lastInsertId = query.lastInsertId().toLongLong();
    return m_affectedRows;
}

int DataModifier::executePreparedQuery(const QString& queryStr, const QVariantList& bindValues)
{
    if (queryStr.isEmpty()) {
        m_lastError = "Query string is empty";
        return -1;
    }

    QSqlQuery query(getDatabase());
    query.prepare(queryStr);

    for (const QVariant& value : bindValues) {
        query.addBindValue(value);
    }

    if (executeAndUpdateStats(query)) {
        return m_affectedRows;
    }

    return -1;
}

// ========================================
// === УТИЛИТЫ И ИНФОРМАЦИЯ ===
// ========================================

qint64 DataModifier::getLastInsertId() const
{
    return m_lastInsertId;
}

int DataModifier::getAffectedRows() const
{
    return m_affectedRows;
}

QString DataModifier::getLastError() const
{
    return m_lastError;
}

void DataModifier::clearLastError()
{
    m_lastError.clear();
}

bool DataModifier::wasLastOperationSuccessful() const
{
    return m_lastError.isEmpty();
}

// ========================================
// === ПРИВАТНЫЕ ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ ===
// ========================================

QSqlDatabase DataModifier::getDatabase() const
{
    if (m_connectionName.isEmpty()) {
        return QSqlDatabase::database();
    }
    return QSqlDatabase::database(m_connectionName);
}

void DataModifier::setError(const QSqlError& error)
{
    if (error.isValid()) {
        m_lastError = error.text();
    }
}

QString DataModifier::escapeValue(const QVariant& value) const
{
    if (value.isNull()) {
        return "NULL";
    }

    switch (value.type()) {
    case QVariant::Int:
    case QVariant::LongLong:
    case QVariant::UInt:
    case QVariant::ULongLong:
    case QVariant::Double:
        return value.toString();

    case QVariant::Bool:
        return value.toBool() ? "1" : "0";

    case QVariant::String:
    default: {
        QString str = value.toString();
        str.replace("'", "''"); // Экранируем одинарные кавычки
        return QString("'%1'").arg(str);
    }
    }
}

QString DataModifier::buildPlaceholders(int count) const
{
    QStringList placeholders;
    for (int i = 0; i < count; ++i) {
        placeholders << "?";
    }
    return placeholders.join(", ");
}

bool DataModifier::executeAndUpdateStats(QSqlQuery& query)
{
    clearLastError();

    if (!query.exec()) {
        setError(query.lastError());
        m_affectedRows = 0;
        m_lastInsertId = -1;
        return false;
    }

    m_affectedRows = query.numRowsAffected();
    m_lastInsertId = query.lastInsertId().toLongLong();

    return true;
}
