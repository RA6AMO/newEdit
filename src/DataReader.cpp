#include "../include/DataReader.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlDriver>
#include <QVariant>

namespace {
    /**
     * @brief Объединить идентификаторы в строку через запятую
     * @param identifiers Список идентификаторов
     * @return Строка с идентификаторами, разделенными запятыми
     */
    static QString joinIdentifiers(const QStringList& identifiers) {
        return identifiers.join(", ");
    }

    /**
     * @brief Создать QSqlRecord из текущей позиции QSqlQuery
     * @param query Запрос с позиционированным курсором
     * @return Запись с данными из текущей строки
     */
    static QSqlRecord makeRowRecord(QSqlQuery& query) {
        QSqlRecord row = query.record();
        for (int i = 0; i < row.count(); ++i) {
            row.setValue(i, query.value(i));
        }
        return row;
    }

    /**
     * @brief Проверить, является ли БД SQLite
     * @param db Подключение к БД
     * @return true если это SQLite
     */
    static bool isSQLite(const QSqlDatabase& db) {
        return db.driverName().toLower().contains("sqlite");
    }
}

DataReader::DataReader(const QString& connectionName)
    : m_connectionName(connectionName) {
}

void DataReader::setConnectionName(const QString& connectionName) {
    m_connectionName = connectionName;
}

/**
 * @brief Внутренний метод для выполнения SELECT-запросов
 *
 * Универсальный метод для выполнения любых SELECT-запросов.
 * Обрабатывает ошибки подключения и выполнения запроса.
 *
 * @param queryStr SQL-запрос для выполнения
 * @return Список записей-результатов
 */
QList<QSqlRecord> DataReader::executeSelectQuery(const QString& queryStr) const {
    QList<QSqlRecord> results;
    m_lastError.clear();

    // Получаем подключение к БД по имени
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return results;
    }

    // Выполняем запрос
    QSqlQuery query(db);
    if (!query.exec(queryStr)) {
        m_lastError = query.lastError().text();
        return results;
    }

    // Собираем все результаты
    while (query.next()) {
        results.append(makeRowRecord(query));
    }
    return results;
}

QList<QSqlRecord> DataReader::selectAll(const QString& tableName) const {
    return executeSelectQuery(QString("SELECT * FROM %1").arg(tableName));
}

QList<QSqlRecord> DataReader::selectWhere(const QString& tableName, const QString& whereClause) const {
    QString q = QString("SELECT * FROM %1 WHERE %2").arg(tableName, whereClause);
    return executeSelectQuery(q);
}

QList<QSqlRecord> DataReader::selectCustom(const QString& query) const {
    return executeSelectQuery(query);
}

int DataReader::countRecords(const QString& tableName) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return -1;
    }
    QSqlQuery query(db);
    if (!query.exec(QString("SELECT COUNT(*) FROM %1").arg(tableName))) {
        m_lastError = query.lastError().text();
        return -1;
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

int DataReader::countRecordsWhere(const QString& tableName, const QString& whereClause) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return -1;
    }
    QSqlQuery query(db);
    QString q = QString("SELECT COUNT(*) FROM %1 WHERE %2").arg(tableName, whereClause);
    if (!query.exec(q)) {
        m_lastError = query.lastError().text();
        return -1;
    }
    if (query.next()) {
        return query.value(0).toInt();
    }
    return 0;
}

QSqlRecord DataReader::findById(const QString& tableName, const QString& idColumn, const QVariant& id) const {
    m_lastError.clear();
    QSqlRecord row;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return row;
    }
    QSqlQuery query(db);
    QString q = QString("SELECT * FROM %1 WHERE %2 = ? LIMIT 1").arg(tableName, idColumn);
    if (!query.prepare(q)) {
        m_lastError = query.lastError().text();
        return row;
    }
    query.addBindValue(id);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return row;
    }
    if (query.next()) {
        row = makeRowRecord(query);
    }
    return row;
}

QList<QSqlRecord> DataReader::findByColumn(const QString& tableName, const QString& columnName, const QVariant& value) const {
    QList<QSqlRecord> results;
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return results;
    }
    QSqlQuery query(db);
    QString q = QString("SELECT * FROM %1 WHERE %2 = ?").arg(tableName, columnName);
    if (!query.prepare(q)) {
        m_lastError = query.lastError().text();
        return results;
    }
    query.addBindValue(value);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return results;
    }
    while (query.next()) {
        results.append(makeRowRecord(query));
    }
    return results;
}

QList<QSqlRecord> DataReader::selectOrdered(const QString& tableName, const QString& orderBy, bool ascending) const {
    QString q = QString("SELECT * FROM %1 ORDER BY %2 %3")
        .arg(tableName, orderBy, ascending ? "ASC" : "DESC");
    return executeSelectQuery(q);
}

QList<QSqlRecord> DataReader::selectLimited(const QString& tableName, int limit, int offset) const {
    QString q = QString("SELECT * FROM %1 LIMIT %2 OFFSET %3")
        .arg(tableName)
        .arg(limit)
        .arg(offset);
    return executeSelectQuery(q);
}

QVariant DataReader::getMinValue(const QString& tableName, const QString& columnName) const {
    QList<QSqlRecord> r = executeSelectQuery(QString("SELECT MIN(%1) AS min_value FROM %2").arg(columnName, tableName));
    if (!r.isEmpty()) return r.first().value(0);
    return QVariant();
}

QVariant DataReader::getMaxValue(const QString& tableName, const QString& columnName) const {
    QList<QSqlRecord> r = executeSelectQuery(QString("SELECT MAX(%1) AS max_value FROM %2").arg(columnName, tableName));
    if (!r.isEmpty()) return r.first().value(0);
    return QVariant();
}

QVariant DataReader::getSumValue(const QString& tableName, const QString& columnName) const {
    QList<QSqlRecord> r = executeSelectQuery(QString("SELECT SUM(%1) AS sum_value FROM %2").arg(columnName, tableName));
    if (!r.isEmpty()) return r.first().value(0);
    return QVariant();
}

QVariant DataReader::getAvgValue(const QString& tableName, const QString& columnName) const {
    QList<QSqlRecord> r = executeSelectQuery(QString("SELECT AVG(%1) AS avg_value FROM %2").arg(columnName, tableName));
    if (!r.isEmpty()) return r.first().value(0);
    return QVariant();
}

bool DataReader::recordExists(const QString& tableName, const QString& whereClause) const {
    QList<QSqlRecord> r = executeSelectQuery(QString("SELECT 1 FROM %1 WHERE %2 LIMIT 1").arg(tableName, whereClause));
    return !r.isEmpty();
}

/**
 * @brief Получить список всех таблиц в БД
 *
 * Для SQLite использует системную таблицу sqlite_master,
 * для других БД - стандартный метод QSqlDatabase::tables().
 * Исключает системные таблицы SQLite (начинающиеся с 'sqlite_').
 *
 * @return Список имен таблиц
 */
QStringList DataReader::getTableNames() const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return {};
    }

    if (isSQLite(db)) {
        // Для SQLite используем системную таблицу sqlite_master
        QStringList names;
        QSqlQuery q(db);
        if (!q.exec("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%' ORDER BY name")) {
            m_lastError = q.lastError().text();
            return names;
        }
        while (q.next()) names << q.value(0).toString();
        return names;
    }

    // Для других БД используем стандартный метод Qt
    return db.tables(QSql::Tables);
}

QStringList DataReader::getViewNames() const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return {};
    }
    if (isSQLite(db)) {
        QStringList names;
        QSqlQuery q(db);
        if (!q.exec("SELECT name FROM sqlite_master WHERE type='view' ORDER BY name")) {
            m_lastError = q.lastError().text();
            return names;
        }
        while (q.next()) names << q.value(0).toString();
        return names;
    }
    return db.tables(QSql::Views);
}

bool DataReader::tableExists(const QString& tableName) const {
    return getTableNames().contains(tableName);
}

bool DataReader::viewExists(const QString& viewName) const {
    return getViewNames().contains(viewName);
}

/**
 * @brief Получить список колонок таблицы
 *
 * Для SQLite использует PRAGMA table_info для получения точной информации о колонках.
 * Для других БД использует универсальный подход: выполнение запроса с WHERE 1=0
 * для получения метаданных без фактических данных.
 *
 * @param tableName Имя таблицы
 * @return Список имен колонок
 */
QStringList DataReader::getColumnNames(const QString& tableName) const {
    m_lastError.clear();
    QStringList columns;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return columns;
    }

    if (isSQLite(db)) {
        // Для SQLite используем PRAGMA table_info - самый надежный способ
        QSqlQuery q(db);
        if (!q.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
            m_lastError = q.lastError().text();
            return columns;
        }
        while (q.next()) {
            // В PRAGMA table_info колонка с именем находится под индексом 1
            columns << q.value(1).toString();
        }
        return columns;
    }

    // Универсальный подход для других БД: запрос с WHERE 1=0 возвращает метаданные без данных
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT * FROM %1 WHERE 1=0").arg(tableName))) {
        m_lastError = q.lastError().text();
        return columns;
    }
    QSqlRecord rec = q.record();
    for (int i = 0; i < rec.count(); ++i) columns << rec.fieldName(i);
    return columns;
}

/**
 * @brief Получить полную структуру таблицы
 *
 * Возвращает детальную информацию о каждой колонке таблицы:
 * - имя колонки
 * - тип данных
 * - возможность NULL
 * - значение по умолчанию
 * - порядковый номер
 *
 * Для SQLite использует PRAGMA table_info, для других БД - information_schema.
 *
 * @param tableName Имя таблицы
 * @return Список записей с информацией о колонках
 */
QList<QSqlRecord> DataReader::getTableStructure(const QString& tableName) const {
    m_lastError.clear();
    if (tableName.isEmpty()) return {};

    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return {};
    }

    if (isSQLite(db)) {
        // Для SQLite используем PRAGMA table_info - возвращает полную структуру
        return executeSelectQuery(QString("PRAGMA table_info(%1)").arg(tableName));
    }

    // Для других БД используем information_schema (работает в MySQL, PostgreSQL)
    QString q = QString(
        "SELECT column_name AS name, data_type AS type, is_nullable, column_default, ordinal_position "
        "FROM information_schema.columns WHERE table_name = '%1' ORDER BY ordinal_position").arg(tableName);
    return executeSelectQuery(q);
}

/**
 * @brief Получить список колонок первичного ключа
 *
 * Для SQLite анализирует результат PRAGMA table_info, где колонка pk (индекс 5)
 * содержит 1 для колонок первичного ключа.
 * Для других БД использует QSqlDatabase::primaryIndex().
 *
 * @param tableName Имя таблицы
 * @return Список имен колонок первичного ключа
 */
QStringList DataReader::getPrimaryKeyColumns(const QString& tableName) const {
    m_lastError.clear();
    QStringList cols;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return cols;
    }

    if (isSQLite(db)) {
        // Для SQLite анализируем PRAGMA table_info
        QSqlQuery q(db);
        if (!q.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
            m_lastError = q.lastError().text();
            return cols;
        }
        while (q.next()) {
            // В PRAGMA table_info колонка pk (индекс 5) содержит 1 для PK колонок
            if (q.value(5).toInt() != 0) cols << q.value(1).toString();
        }
        return cols;
    }

    // Для других БД используем встроенный метод Qt
    QSqlIndex pk = db.primaryIndex(tableName);
    for (int i = 0; i < pk.count(); ++i) cols << pk.fieldName(i);
    return cols;
}

/**
 * @brief Получить список колонок внешних ключей
 *
 * Для SQLite использует PRAGMA foreign_key_list, который возвращает информацию
 * о внешних ключах, включая колонку "from" с именем локальной колонки.
 * Для других БД использует information_schema с JOIN между таблицами ограничений.
 *
 * @param tableName Имя таблицы
 * @return Список имен колонок внешних ключей (без дубликатов)
 */
QStringList DataReader::getForeignKeyColumns(const QString& tableName) const {
    m_lastError.clear();
    QStringList cols;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return cols;
    }

    if (isSQLite(db)) {
        // Для SQLite используем PRAGMA foreign_key_list
        QSqlQuery q(db);
        if (!q.exec(QString("PRAGMA foreign_key_list(%1)").arg(tableName))) {
            m_lastError = q.lastError().text();
            return cols;
        }
        while (q.next()) {
            // В PRAGMA foreign_key_list колонка "from" содержит имя локальной колонки
            cols << q.value("from").toString();
        }
        cols.removeDuplicates();
        return cols;
    }

    // Для других БД используем information_schema
    QString sql = QString(
        "SELECT kcu.column_name FROM information_schema.table_constraints tc "
        "JOIN information_schema.key_column_usage kcu ON tc.constraint_name = kcu.constraint_name "
        "WHERE tc.table_name = '%1' AND tc.constraint_type = 'FOREIGN KEY'").arg(tableName);
    QList<QSqlRecord> rows = executeSelectQuery(sql);
    for (const QSqlRecord& r : rows) cols << r.value(0).toString();
    cols.removeDuplicates();
    return cols;
}

QStringList DataReader::getIndexNames(const QString& tableName) const {
    m_lastError.clear();
    QStringList names;
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return names;
    }
    if (isSQLite(db)) {
        QSqlQuery q(db);
        if (!q.exec(QString("PRAGMA index_list(%1)").arg(tableName))) {
            m_lastError = q.lastError().text();
            return names;
        }
        while (q.next()) names << q.value("name").toString();
        return names;
    }
    // Fallback: information_schema (may vary by db)
    QString sql = QString(
        "SELECT indexname FROM pg_indexes WHERE tablename = '%1'"
    ).arg(tableName);
    QList<QSqlRecord> rows = executeSelectQuery(sql);
    for (const QSqlRecord& r : rows) names << r.value(0).toString();
    return names;
}

QString DataReader::getColumnType(const QString& tableName, const QString& columnName) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return QString();
    }
    if (isSQLite(db)) {
        QSqlQuery q(db);
        if (q.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
            while (q.next()) {
                if (q.value(1).toString().compare(columnName, Qt::CaseInsensitive) == 0) {
                    return q.value(2).toString();
                }
            }
        } else {
            m_lastError = q.lastError().text();
        }
        return QString();
    }
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT %1 FROM %2 WHERE 1=0").arg(columnName, tableName))) {
        m_lastError = q.lastError().text();
        return QString();
    }
    QSqlRecord rec = q.record();
    int idx = rec.indexOf(columnName);
    if (idx >= 0) {
        return rec.field(idx).typeID() >= 0 ? rec.field(idx).typeName() : QString();
    }
    return QString();
}

bool DataReader::isColumnNullable(const QString& tableName, const QString& columnName) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) return false;
    if (isSQLite(db)) {
        QSqlQuery q(db);
        if (q.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
            while (q.next()) {
                if (q.value(1).toString().compare(columnName, Qt::CaseInsensitive) == 0) {
                    return q.value(3).toInt() == 0; // notnull == 0 => nullable
                }
            }
        } else {
            m_lastError = q.lastError().text();
        }
        return false;
    }
    // Generic: attempt to inspect field
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT %1 FROM %2 WHERE 1=0").arg(columnName, tableName))) {
        m_lastError = q.lastError().text();
        return false;
    }
    QSqlRecord rec = q.record();
    int idx = rec.indexOf(columnName);
    if (idx >= 0) return rec.field(idx).isNull() || !rec.field(idx).isReadOnly();
    return false;
}

bool DataReader::isColumnPrimaryKey(const QString& tableName, const QString& columnName) const {
    return getPrimaryKeyColumns(tableName).contains(columnName);
}

/**
 * @brief Проверить, является ли колонка автоинкрементной
 *
 * Для SQLite анализирует DDL таблицы из sqlite_master, ища определение колонки
 * как "INTEGER PRIMARY KEY" (в SQLite это автоматически автоинкрементная rowid).
 * Для других БД использует QSqlField::isAutoValue().
 *
 * @param tableName Имя таблицы
 * @param columnName Имя колонки
 * @return true если колонка автоинкрементная
 */
bool DataReader::isColumnAutoIncrement(const QString& tableName, const QString& columnName) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) return false;

    if (isSQLite(db)) {
        // В SQLite AUTOINCREMENT применяется только к INTEGER PRIMARY KEY колонкам
        QSqlQuery q(db);
        if (!q.exec(QString("SELECT sql FROM sqlite_master WHERE type='table' AND name='%1'").arg(tableName))) {
            m_lastError = q.lastError().text();
            return false;
        }
        if (q.next()) {
            QString ddl = q.value(0).toString().toLower();
            // Простая проверка: определение колонки содержит "<name> integer primary key"
            // В SQLite INTEGER PRIMARY KEY автоматически автоинкрементная (rowid)
            if (ddl.contains(QString("%1 integer primary key").arg(columnName.toLower()))) {
                return true; // INTEGER PRIMARY KEY в SQLite = автоинкрементная rowid
            }
        }
        return false;
    }

    // Для других БД используем метаданные QSqlField
    QSqlQuery q(db);
    if (!q.exec(QString("SELECT %1 FROM %2 WHERE 1=0").arg(columnName, tableName))) {
        m_lastError = q.lastError().text();
        return false;
    }
    QSqlRecord rec = q.record();
    int idx = rec.indexOf(columnName);
    if (idx >= 0) return rec.field(idx).isAutoValue();
    return false;
}

QString DataReader::getColumnDefaultValue(const QString& tableName, const QString& columnName) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) return QString();
    if (isSQLite(db)) {
        QSqlQuery q(db);
        if (!q.exec(QString("PRAGMA table_info(%1)").arg(tableName))) {
            m_lastError = q.lastError().text();
            return QString();
        }
        while (q.next()) {
            if (q.value(1).toString().compare(columnName, Qt::CaseInsensitive) == 0) {
                return q.value(4).toString();
            }
        }
        return QString();
    }
    // Generic: use information_schema if available
    QString sql = QString(
        "SELECT column_default FROM information_schema.columns WHERE table_name='%1' AND column_name='%2'"
    ).arg(tableName, columnName);
    QList<QSqlRecord> r = executeSelectQuery(sql);
    if (!r.isEmpty()) return r.first().value(0).toString();
    return QString();
}

QList<QSqlRecord> DataReader::selectGrouped(const QString& tableName, const QStringList& groupByColumns, const QStringList& aggregateColumns) const {
    QString selectParts;
    QStringList parts;
    if (!groupByColumns.isEmpty()) parts << joinIdentifiers(groupByColumns);
    if (!aggregateColumns.isEmpty()) parts << joinIdentifiers(aggregateColumns);
    selectParts = parts.join(", ");
    if (selectParts.isEmpty()) selectParts = "*";
    QString q = QString("SELECT %1 FROM %2").arg(selectParts, tableName);
    if (!groupByColumns.isEmpty()) {
        q += QString(" GROUP BY %1").arg(joinIdentifiers(groupByColumns));
    }
    return executeSelectQuery(q);
}

QList<QSqlRecord> DataReader::selectWithHaving(const QString& tableName, const QString& groupBy, const QString& havingClause) const {
    QString q = QString("SELECT %1, COUNT(*) AS cnt FROM %2 GROUP BY %1 HAVING %3")
        .arg(groupBy, tableName, havingClause);
    return executeSelectQuery(q);
}

QList<QSqlRecord> DataReader::selectWithPagination(const QString& tableName, int page, int pageSize, const QString& orderBy) const {
    int offset = qMax(0, (page - 1) * pageSize);
    QString q = QString("SELECT * FROM %1").arg(tableName);
    if (!orderBy.isEmpty()) q += QString(" ORDER BY %1").arg(orderBy);
    q += QString(" LIMIT %1 OFFSET %2").arg(pageSize).arg(offset);
    return executeSelectQuery(q);
}

QList<QSqlRecord> DataReader::selectDistinct(const QString& tableName, const QStringList& columns) const {
    QString cols = columns.isEmpty() ? "*" : joinIdentifiers(columns);
    QString q = QString("SELECT DISTINCT %1 FROM %2").arg(cols, tableName);
    return executeSelectQuery(q);
}

QMap<QString, int> DataReader::getTableRowCounts() const {
    QMap<QString, int> counts;
    for (const QString& t : getTableNames()) {
        counts[t] = countRecords(t);
    }
    return counts;
}

/**
 * @brief Получить информацию о размере таблицы
 *
 * Для SQLite использует PRAGMA page_size и page_count для получения
 * приблизительного размера БД в байтах, а также количество записей в таблице.
 * Для других БД возвращает только количество записей.
 *
 * @param tableName Имя таблицы
 * @return Карта с информацией о размере (page_size, page_count, database_size_bytes, row_count)
 */
QMap<QString, QVariant> DataReader::getTableSizeInfo(const QString& tableName) const {
    QMap<QString, QVariant> info;
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return info;
    }

    if (isSQLite(db)) {
        // Для SQLite получаем приблизительный размер БД через статистику страниц
        QSqlQuery q(db);
        if (q.exec("PRAGMA page_size")) {
            if (q.next()) info["page_size"] = q.value(0).toInt();
        }
        if (q.exec("PRAGMA page_count")) {
            if (q.next()) info["page_count"] = q.value(0).toInt();
        }
        // Вычисляем общий размер БД в байтах
        if (info.contains("page_size") && info.contains("page_count")) {
            info["database_size_bytes"] = info["page_size"].toInt() * info["page_count"].toInt();
        }
        // Размер конкретной таблицы в SQLite сложно получить точно, поэтому показываем количество записей
        info["row_count"] = countRecords(tableName);
        return info;
    }

    // Для других БД доступна только информация о количестве записей
    info["row_count"] = countRecords(tableName);
    return info;
}

QList<QSqlRecord> DataReader::getTableConstraints(const QString& tableName) const {
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) return {};
    if (isSQLite(db)) {
        // Return foreign keys as constraints
        QString sql = QString("PRAGMA foreign_key_list(%1)").arg(tableName);
        return executeSelectQuery(sql);
    }
    // Fallback: information_schema table constraints
    QString sql = QString(
        "SELECT constraint_name, constraint_type FROM information_schema.table_constraints WHERE table_name='%1'"
    ).arg(tableName);
    return executeSelectQuery(sql);
}

/**
 * @brief Получить статистику по колонке
 *
 * Вычисляет комплексную статистику для указанной колонки:
 * - min_value: минимальное значение
 * - max_value: максимальное значение
 * - avg_value: среднее значение
 * - total_count: общее количество записей
 * - null_count: количество NULL значений
 * - distinct_count: количество уникальных значений
 *
 * @param tableName Имя таблицы
 * @param columnName Имя колонки
 * @return Статистическая информация в виде одной записи
 */
QList<QSqlRecord> DataReader::getColumnStatistics(const QString& tableName, const QString& columnName) const {
    QString sql = QString(
        "SELECT MIN(%1) AS min_value, MAX(%1) AS max_value, AVG(%1) AS avg_value, "
        "COUNT(*) AS total_count, SUM(CASE WHEN %1 IS NULL THEN 1 ELSE 0 END) AS null_count, "
        "COUNT(DISTINCT %1) AS distinct_count FROM %2").arg(columnName, tableName);
    return executeSelectQuery(sql);
}

QList<QSqlRecord> DataReader::getDataDistribution(const QString& tableName, const QString& columnName) const {
    QString sql = QString(
        "SELECT %1 AS value, COUNT(*) AS freq FROM %2 GROUP BY %1 ORDER BY freq DESC"
    ).arg(columnName, tableName);
    return executeSelectQuery(sql);
}

/**
 * @brief Найти дублирующиеся записи
 *
 * Группирует записи по указанным колонкам и находит группы с количеством > 1.
 * Использует GROUP BY с HAVING для фильтрации только дубликатов.
 *
 * @param tableName Имя таблицы
 * @param columns Колонки для проверки дубликатов
 * @return Дублирующиеся комбинации с количеством повторений (cnt)
 */
QList<QSqlRecord> DataReader::findDuplicateRecords(const QString& tableName, const QStringList& columns) const {
    if (columns.isEmpty()) return {};
    QString cols = joinIdentifiers(columns);
    QString sql = QString("SELECT %1, COUNT(*) AS cnt FROM %2 GROUP BY %1 HAVING cnt > 1")
        .arg(cols, tableName);
    return executeSelectQuery(sql);
}

/**
 * @brief Поиск по тексту в колонке
 *
 * Выполняет поиск с использованием LIKE и символов подстановки (%).
 * Автоматически добавляет % в начало и конец поискового термина для поиска подстроки.
 * Использует подготовленные запросы для безопасности.
 *
 * @param tableName Имя таблицы
 * @param columnName Имя колонки для поиска
 * @param searchTerm Термин для поиска
 * @return Записи, содержащие искомый термин
 */
QList<QSqlRecord> DataReader::searchInText(const QString& tableName, const QString& columnName, const QString& searchTerm) const {
    QList<QSqlRecord> results;
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return results;
    }

    QSqlQuery query(db);
    QString q = QString("SELECT * FROM %1 WHERE %2 LIKE ?").arg(tableName, columnName);
    if (!query.prepare(q)) {
        m_lastError = query.lastError().text();
        return results;
    }

    // Добавляем % для поиска подстроки в любом месте текста
    query.addBindValue(QString("%%" + searchTerm + "%%"));
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return results;
    }

    while (query.next()) results.append(makeRowRecord(query));
    return results;
}

/**
 * @brief Выбрать записи по паттерну
 *
 * Выполняет поиск с использованием пользовательского паттерна LIKE.
 * Паттерн может содержать символы подстановки % (любое количество символов)
 * и _ (один символ). Использует подготовленные запросы для безопасности.
 *
 * @param tableName Имя таблицы
 * @param columnName Имя колонки для поиска
 * @param pattern Паттерн для поиска (с поддержкой % и _)
 * @return Записи, соответствующие паттерну
 */
QList<QSqlRecord> DataReader::selectByPattern(const QString& tableName, const QString& columnName, const QString& pattern) const {
    QList<QSqlRecord> results;
    m_lastError.clear();
    QSqlDatabase db = QSqlDatabase::database(m_connectionName);
    if (!db.isValid() || !db.isOpen()) {
        m_lastError = QString("Database is not open for connection '%1'").arg(m_connectionName);
        return results;
    }

    QSqlQuery query(db);
    QString q = QString("SELECT * FROM %1 WHERE %2 LIKE ?").arg(tableName, columnName);
    if (!query.prepare(q)) {
        m_lastError = query.lastError().text();
        return results;
    }

    // Используем паттерн как есть (пользователь сам добавляет % и _)
    query.addBindValue(pattern);
    if (!query.exec()) {
        m_lastError = query.lastError().text();
        return results;
    }

    while (query.next()) results.append(makeRowRecord(query));
    return results;
}

QString DataReader::getLastError() const {
    return m_lastError;
}
