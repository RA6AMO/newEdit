#pragma once
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QSqlRecord>
#include <QList>
#include <QMap>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlField>
#include <QSqlDriver>
#include <QSqlIndex>


/**
 * @brief Класс для чтения данных и метаданных из базы данных
 *
 * DataReader предоставляет универсальный интерфейс для работы с различными типами БД.
 * Поддерживает SQLite (через PRAGMA) и другие БД через стандартные SQL-запросы.
 *
 * Основные возможности:
 * - Базовые операции чтения данных (SELECT, WHERE, ORDER BY, LIMIT)
 * - Агрегатные функции (COUNT, MIN, MAX, SUM, AVG)
 * - Получение метаданных о таблицах, колонках, индексах
 * - Анализ структуры БД и статистика данных
 * - Поиск и фильтрация с поддержкой паттернов
 */
class DataReader {
public:
    /**
     * @brief Конструктор
     * @param connectionName Имя подключения к БД (должно быть зарегистрировано в QSqlDatabase)
     */
    DataReader(const QString& connectionName);

    DataReader();

    void setConnectionName(const QString& connectionName);

    // === БАЗОВЫЕ ОПЕРАЦИИ ЧТЕНИЯ ===

    /**
     * @brief Выбрать все записи из таблицы
     * @param tableName Имя таблицы
     * @return Список записей (QSqlRecord)
     */
    QList<QSqlRecord> selectAll(const QString& tableName) const;

    /**
     * @brief Выбрать записи с условием WHERE
     * @param tableName Имя таблицы
     * @param whereClause Условие WHERE (без ключевого слова WHERE)
     * @return Список записей, соответствующих условию
     */
    QList<QSqlRecord> selectWhere(const QString& tableName, const QString& whereClause) const;

    /**
     * @brief Выполнить произвольный SELECT-запрос
     * @param query SQL-запрос
     * @return Результат выполнения запроса
     */
    QList<QSqlRecord> selectCustom(const QString& query) const;

    // === ПОДСЧЕТ ЗАПИСЕЙ ===

    /**
     * @brief Подсчитать общее количество записей в таблице
     * @param tableName Имя таблицы
     * @return Количество записей (-1 при ошибке)
     */
    int countRecords(const QString& tableName) const;

    /**
     * @brief Подсчитать записи с условием WHERE
     * @param tableName Имя таблицы
     * @param whereClause Условие WHERE
     * @return Количество записей (-1 при ошибке)
     */
    int countRecordsWhere(const QString& tableName, const QString& whereClause) const;

    // === ПОИСК КОНКРЕТНЫХ ЗАПИСЕЙ ===

    /**
     * @brief Найти запись по ID
     * @param tableName Имя таблицы
     * @param idColumn Имя колонки с ID
     * @param id Значение ID
     * @return Найденная запись (пустая при отсутствии)
     */
    QSqlRecord findById(const QString& tableName, const QString& idColumn, const QVariant& id) const;

    /**
     * @brief Найти записи по значению колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @param value Значение для поиска
     * @return Список найденных записей
     */
    QList<QSqlRecord> findByColumn(const QString& tableName, const QString& columnName, const QVariant& value) const;

    // === СОРТИРОВКА И ОГРАНИЧЕНИЯ ===

    /**
     * @brief Выбрать записи с сортировкой
     * @param tableName Имя таблицы
     * @param orderBy Колонка для сортировки
     * @param ascending Направление сортировки (true = по возрастанию)
     * @return Отсортированный список записей
     */
    QList<QSqlRecord> selectOrdered(const QString& tableName, const QString& orderBy, bool ascending = true) const;

    /**
     * @brief Выбрать ограниченное количество записей
     * @param tableName Имя таблицы
     * @param limit Максимальное количество записей
     * @param offset Смещение (количество пропускаемых записей)
     * @return Список записей с ограничением
     */
    QList<QSqlRecord> selectLimited(const QString& tableName, int limit, int offset = 0) const;

    // === АГРЕГАТНЫЕ ФУНКЦИИ ===

    /**
     * @brief Получить минимальное значение колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Минимальное значение
     */
    QVariant getMinValue(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Получить максимальное значение колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Максимальное значение
     */
    QVariant getMaxValue(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Получить сумму значений колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Сумма значений
     */
    QVariant getSumValue(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Получить среднее значение колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Среднее значение
     */
    QVariant getAvgValue(const QString& tableName, const QString& columnName) const;

    // === ПРОВЕРКА СУЩЕСТВОВАНИЯ ===

    /**
     * @brief Проверить существование записи по условию
     * @param tableName Имя таблицы
     * @param whereClause Условие WHERE
     * @return true если запись существует
     */
    bool recordExists(const QString& tableName, const QString& whereClause) const;

    // === МЕТАДАННЫЕ О ТАБЛИЦАХ ===

    // === ИНФОРМАЦИЯ О ТАБЛИЦАХ В БАЗЕ ДАННЫХ ===

    /**
     * @brief Получить список всех таблиц в БД
     * @return Список имен таблиц
     */
    QStringList getTableNames() const;

    /**
     * @brief Получить список всех представлений (VIEW) в БД
     * @return Список имен представлений
     */
    QStringList getViewNames() const;

    /**
     * @brief Проверить существование таблицы
     * @param tableName Имя таблицы
     * @return true если таблица существует
     */
    bool tableExists(const QString& tableName) const;

    /**
     * @brief Проверить существование представления
     * @param viewName Имя представления
     * @return true если представление существует
     */
    bool viewExists(const QString& viewName) const;

    // === СТРУКТУРА ТАБЛИЦ ===

    /**
     * @brief Получить список колонок таблицы
     * @param tableName Имя таблицы
     * @return Список имен колонок
     */
    QStringList getColumnNames(const QString& tableName) const;

    /**
     * @brief Получить полную структуру таблицы
     * @param tableName Имя таблицы
     * @return Список записей с информацией о колонках (тип, nullable, default и т.д.)
     */
    QList<QSqlRecord> getTableStructure(const QString& tableName) const;

    /**
     * @brief Получить список колонок первичного ключа
     * @param tableName Имя таблицы
     * @return Список имен колонок первичного ключа
     */
    QStringList getPrimaryKeyColumns(const QString& tableName) const;

    /**
     * @brief Получить список колонок внешних ключей
     * @param tableName Имя таблицы
     * @return Список имен колонок внешних ключей
     */
    QStringList getForeignKeyColumns(const QString& tableName) const;

    /**
     * @brief Получить список индексов таблицы
     * @param tableName Имя таблицы
     * @return Список имен индексов
     */
    QStringList getIndexNames(const QString& tableName) const;

    // === ИНФОРМАЦИЯ О КОЛОНКАХ ===

    /**
     * @brief Получить тип данных колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Тип данных колонки
     */
    QString getColumnType(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Проверить, может ли колонка содержать NULL
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return true если колонка может быть NULL
     */
    bool isColumnNullable(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Проверить, является ли колонка частью первичного ключа
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return true если колонка входит в первичный ключ
     */
    bool isColumnPrimaryKey(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Проверить, является ли колонка автоинкрементной
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return true если колонка автоинкрементная
     */
    bool isColumnAutoIncrement(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Получить значение по умолчанию для колонки
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Значение по умолчанию
     */
    QString getColumnDefaultValue(const QString& tableName, const QString& columnName) const;

    // === РАСШИРЕННЫЕ МЕТОДЫ ДЛЯ РАБОТЫ С ДАННЫМИ ===

    // === ГРУППИРОВКА И АГРЕГАЦИЯ ===

    /**
     * @brief Выбрать данные с группировкой
     * @param tableName Имя таблицы
     * @param groupByColumns Колонки для группировки
     * @param aggregateColumns Колонки для агрегации (опционально)
     * @return Результат с группировкой
     */
    QList<QSqlRecord> selectGrouped(const QString& tableName, const QStringList& groupByColumns,
                                   const QStringList& aggregateColumns = QStringList()) const;

    /**
     * @brief Выбрать данные с группировкой и условием HAVING
     * @param tableName Имя таблицы
     * @param groupBy Колонка для группировки
     * @param havingClause Условие HAVING
     * @return Результат с группировкой и фильтрацией
     */
    QList<QSqlRecord> selectWithHaving(const QString& tableName, const QString& groupBy,
                                      const QString& havingClause) const;

    // === ПАГИНАЦИЯ И ПРОИЗВОДИТЕЛЬНОСТЬ ===

    /**
     * @brief Выбрать данные с пагинацией
     * @param tableName Имя таблицы
     * @param page Номер страницы (начиная с 1)
     * @param pageSize Размер страницы
     * @param orderBy Колонка для сортировки (опционально)
     * @return Записи для указанной страницы
     */
    QList<QSqlRecord> selectWithPagination(const QString& tableName, int page, int pageSize,
                                          const QString& orderBy = QString()) const;

    /**
     * @brief Выбрать уникальные значения колонок
     * @param tableName Имя таблицы
     * @param columns Колонки для выборки (пустой список = все колонки)
     * @return Уникальные комбинации значений
     */
    QList<QSqlRecord> selectDistinct(const QString& tableName, const QStringList& columns) const;

    // === АНАЛИЗ И СТАТИСТИКА ДАННЫХ ===

    // === СТАТИСТИКА ПО ТАБЛИЦАМ ===

    /**
     * @brief Получить количество записей во всех таблицах
     * @return Карта "имя_таблицы -> количество_записей"
     */
    QMap<QString, int> getTableRowCounts() const;

    /**
     * @brief Получить информацию о размере таблицы
     * @param tableName Имя таблицы
     * @return Карта с информацией о размере (row_count, page_size и т.д.)
     */
    QMap<QString, QVariant> getTableSizeInfo(const QString& tableName) const;

    /**
     * @brief Получить ограничения таблицы (внешние ключи, проверки и т.д.)
     * @param tableName Имя таблицы
     * @return Список ограничений
     */
    QList<QSqlRecord> getTableConstraints(const QString& tableName) const;

    // === АНАЛИЗ ДАННЫХ ===

    /**
     * @brief Получить статистику по колонке (min, max, avg, count, null_count, distinct_count)
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Статистическая информация
     */
    QList<QSqlRecord> getColumnStatistics(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Получить распределение значений в колонке
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @return Список "значение -> частота" отсортированный по убыванию частоты
     */
    QList<QSqlRecord> getDataDistribution(const QString& tableName, const QString& columnName) const;

    /**
     * @brief Найти дублирующиеся записи
     * @param tableName Имя таблицы
     * @param columns Колонки для проверки дубликатов
     * @return Дублирующиеся комбинации с количеством повторений
     */
    QList<QSqlRecord> findDuplicateRecords(const QString& tableName, const QStringList& columns) const;

    // === ПОИСК И ФИЛЬТРАЦИЯ ===

    /**
     * @brief Поиск по тексту в колонке (LIKE с %)
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @param searchTerm Термин для поиска
     * @return Записи, содержащие термин
     */
    QList<QSqlRecord> searchInText(const QString& tableName, const QString& columnName,
                                  const QString& searchTerm) const;

    /**
     * @brief Выбрать записи по паттерну (LIKE)
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @param pattern Паттерн для поиска (с поддержкой % и _)
     * @return Записи, соответствующие паттерну
     */
    QList<QSqlRecord> selectByPattern(const QString& tableName, const QString& columnName,
                                     const QString& pattern) const;

    // === УТИЛИТЫ ===

    /**
     * @brief Получить последнюю ошибку
     * @return Текст последней ошибки
     */
    QString getLastError() const;

private:
    QString m_connectionName;  ///< Имя подключения к БД
    mutable QString m_lastError;       ///< Текст последней ошибки

    /**
     * @brief Внутренний метод для выполнения SELECT-запросов
     * @param query SQL-запрос
     * @return Результат выполнения запроса
     */
    QList<QSqlRecord> executeSelectQuery(const QString& query) const;
};
