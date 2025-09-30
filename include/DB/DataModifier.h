#pragma once
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QList>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <functional>

/**
 * @brief Класс для модификации данных в базе данных
 *
 * DataModifier предоставляет универсальный интерфейс для изменения данных в БД.
 * Поддерживает базовые операции INSERT, UPDATE, DELETE, а также транзакции и пакетные операции.
 *
 * Основные возможности:
 * - Вставка одной или нескольких записей
 * - Обновление записей по условию
 * - Удаление записей по условию
 * - Транзакции для атомарных операций
 * - Пакетные операции для повышения производительности
 * - UPSERT (вставка или обновление)
 * - Валидация и обработка ошибок
 */
class DataModifier {
public:
    /**
     * @brief Конструктор
     * @param connectionName Имя подключения к БД (должно быть зарегистрировано в QSqlDatabase)
     */
    explicit DataModifier(const QString& connectionName = QString());


    /**
     * @brief Деструктор - откатывает незавершенные транзакции
     */
    ~DataModifier();

    /**
     * @brief Установить имя подключения
     * @param connectionName Имя подключения
     */
    void setConnectionName(const QString& connectionName);

    // ========================================
    // === БАЗОВЫЕ ОПЕРАЦИИ INSERT ===
    // ========================================

    /**
     * @brief Вставить одну запись
     * @param tableName Имя таблицы
     * @param values Карта "имя_колонки -> значение"
     * @return true если операция успешна
     */
    bool insertRecord(const QString& tableName, const QVariantMap& values);

    /**
     * @brief Вставить несколько записей
     * @param tableName Имя таблицы
     * @param columns Список имен колонок
     * @param values Список записей (каждая запись - список значений)
     * @return Количество успешно вставленных записей
     */
    int insertRecords(const QString& tableName, const QStringList& columns,
                     const QList<QVariantList>& values);

    /**
     * @brief Вставить запись с возвратом ID
     * @param tableName Имя таблицы
     * @param values Карта "имя_колонки -> значение"
     * @param idColumnName Имя колонки с автоинкрементным ID
     * @return ID вставленной записи (-1 при ошибке)
     */
    qint64 insertRecordAndReturnId(const QString& tableName, const QVariantMap& values,
                                   const QString& idColumnName = "id");

    // ========================================
    // === БАЗОВЫЕ ОПЕРАЦИИ UPDATE ===
    // ========================================

    /**
     * @brief Обновить записи по условию
     * @param tableName Имя таблицы
     * @param values Карта "имя_колонки -> новое_значение"
     * @param whereClause Условие WHERE (без ключевого слова WHERE)
     * @return Количество обновленных записей (-1 при ошибке)
     */
    int updateRecords(const QString& tableName, const QVariantMap& values,
                     const QString& whereClause);

    /**
     * @brief Обновить запись по ID
     * @param tableName Имя таблицы
     * @param recordId Значение ID
     * @param values Карта "имя_колонки -> новое_значение"
     * @param idColumn Имя колонки с ID
     * @return true если запись обновлена
     */
    bool updateRecordById(const QString& tableName, const QVariant& recordId,
                         const QVariantMap& values, const QString& idColumn = "id");

    /**
     * @brief Обновить одну колонку по условию
     * @param tableName Имя таблицы
     * @param columnName Имя колонки для обновления
     * @param value Новое значение
     * @param whereClause Условие WHERE
     * @return Количество обновленных записей
     */
    int updateColumn(const QString& tableName, const QString& columnName,
                    const QVariant& value, const QString& whereClause);

    // ========================================
    // === БАЗОВЫЕ ОПЕРАЦИИ DELETE ===
    // ========================================

    /**
     * @brief Удалить записи по условию
     * @param tableName Имя таблицы
     * @param whereClause Условие WHERE (без ключевого слова WHERE)
     * @return Количество удаленных записей (-1 при ошибке)
     */
    int deleteRecords(const QString& tableName, const QString& whereClause);

    /**
     * @brief Удалить запись по ID
     * @param tableName Имя таблицы
     * @param recordId Значение ID
     * @param idColumn Имя колонки с ID
     * @return true если запись удалена
     */
    bool deleteRecordById(const QString& tableName, const QVariant& recordId,
                         const QString& idColumn = "id");

    /**
     * @brief Удалить все записи из таблицы (очистка)
     * @param tableName Имя таблицы
     * @return true если операция успешна
     */
    bool deleteAllRecords(const QString& tableName);

    /**
     * @brief Усечь таблицу (быстрое удаление всех записей с сбросом автоинкремента)
     * @param tableName Имя таблицы
     * @return true если операция успешна
     */
    bool truncateTable(const QString& tableName);

    // ========================================
    // === ОПЕРАЦИИ UPSERT ===
    // ========================================

    /**
     * @brief Вставить или обновить запись (SQLite REPLACE или INSERT ... ON CONFLICT)
     * @param tableName Имя таблицы
     * @param values Карта "имя_колонки -> значение"
     * @param conflictColumns Колонки для проверки конфликта (обычно первичный ключ)
     * @return true если операция успешна
     */
    bool upsertRecord(const QString& tableName, const QVariantMap& values,
                     const QStringList& conflictColumns);

    /**
     * @brief Вставить запись, если она не существует
     * @param tableName Имя таблицы
     * @param values Карта "имя_колонки -> значение"
     * @param checkColumns Колонки для проверки существования
     * @return true если запись вставлена, false если уже существует
     */
    bool insertIfNotExists(const QString& tableName, const QVariantMap& values,
                          const QStringList& checkColumns);

    // ========================================
    // === ТРАНЗАКЦИИ ===
    // ========================================

    /**
     * @brief Начать транзакцию
     * @return true если транзакция начата
     */
    bool beginTransaction();

    /**
     * @brief Подтвердить транзакцию
     * @return true если транзакция подтверждена
     */
    bool commitTransaction();

    /**
     * @brief Откатить транзакцию
     * @return true если транзакция откачена
     */
    bool rollbackTransaction();

    /**
     * @brief Выполнить набор операций в транзакции
     * @param operations Функция с набором операций (возвращает true если все успешно)
     * @return true если все операции выполнены успешно
     */
    bool executeInTransaction(const std::function<bool()>& operations);

    /**
     * @brief Проверить, находится ли класс в транзакции
     * @return true если транзакция активна
     */
    bool isInTransaction() const;

    // ========================================
    // === ПАКЕТНЫЕ ОПЕРАЦИИ ===
    // ========================================

    /**
     * @brief Пакетная вставка с использованием prepared statement
     * @param tableName Имя таблицы
     * @param columns Список имен колонок
     * @param values Список записей для вставки
     * @param batchSize Размер пакета (0 = все за раз)
     * @return Количество успешно вставленных записей
     */
    int batchInsert(const QString& tableName, const QStringList& columns,
                   const QList<QVariantList>& values, int batchSize = 100);

    /**
     * @brief Пакетное обновление нескольких записей
     * @param tableName Имя таблицы
     * @param updates Список карт с обновлениями (каждая должна содержать условие)
     * @param idColumn Имя колонки ID для условия WHERE
     * @return Количество обновленных записей
     */
    int batchUpdate(const QString& tableName, const QList<QVariantMap>& updates,
                   const QString& idColumn = "id");

    /**
     * @brief Пакетное удаление записей по списку ID
     * @param tableName Имя таблицы
     * @param ids Список ID для удаления
     * @param idColumn Имя колонки ID
     * @return Количество удаленных записей
     */
    int batchDelete(const QString& tableName, const QVariantList& ids,
                   const QString& idColumn = "id");

    // ========================================
    // === СПЕЦИАЛИЗИРОВАННЫЕ ОПЕРАЦИИ ===
    // ========================================

    /**
     * @brief Инкрементировать числовое значение
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @param increment Значение для инкремента (по умолчанию 1)
     * @param whereClause Условие WHERE
     * @return Количество обновленных записей
     */
    int incrementValue(const QString& tableName, const QString& columnName,
                      int increment, const QString& whereClause);

    /**
     * @brief Декрементировать числовое значение
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @param decrement Значение для декремента (по умолчанию 1)
     * @param whereClause Условие WHERE
     * @return Количество обновленных записей
     */
    int decrementValue(const QString& tableName, const QString& columnName,
                      int decrement, const QString& whereClause);

    /**
     * @brief Заменить NULL значения на значение по умолчанию
     * @param tableName Имя таблицы
     * @param columnName Имя колонки
     * @param defaultValue Значение по умолчанию
     * @return Количество обновленных записей
     */
    int replaceNullValues(const QString& tableName, const QString& columnName,
                         const QVariant& defaultValue);

    /**
     * @brief Копировать записи внутри таблицы
     * @param tableName Имя таблицы
     * @param whereClause Условие для выбора записей для копирования
     * @param modifications Изменения для копий (опционально)
     * @return Количество скопированных записей
     */
    int copyRecords(const QString& tableName, const QString& whereClause,
                   const QVariantMap& modifications = QVariantMap());

    // ========================================
    // === ВЫПОЛНЕНИЕ ПРОИЗВОЛЬНЫХ ЗАПРОСОВ ===
    // ========================================

    /**
     * @brief Выполнить произвольный SQL-запрос на модификацию
     * @param query SQL-запрос (INSERT, UPDATE, DELETE)
     * @return Количество затронутых записей (-1 при ошибке)
     */
    int executeModifyQuery(const QString& query);

    /**
     * @brief Выполнить произвольный SQL-запрос с параметрами
     * @param query SQL-запрос с плейсхолдерами (?)
     * @param bindValues Список значений для подстановки
     * @return Количество затронутых записей (-1 при ошибке)
     */
    int executePreparedQuery(const QString& query, const QVariantList& bindValues);

    // ========================================
    // === УТИЛИТЫ И ИНФОРМАЦИЯ ===
    // ========================================

    /**
     * @brief Получить ID последней вставленной записи
     * @return ID последней вставленной записи
     */
    qint64 getLastInsertId() const;

    /**
     * @brief Получить количество записей, затронутых последней операцией
     * @return Количество затронутых записей
     */
    int getAffectedRows() const;

    /**
     * @brief Получить последнюю ошибку
     * @return Текст последней ошибки
     */
    QString getLastError() const;

    /**
     * @brief Очистить последнюю ошибку
     */
    void clearLastError();

    /**
     * @brief Проверить успешность последней операции
     * @return true если последняя операция была успешной
     */
    bool wasLastOperationSuccessful() const;

private:
    QString m_connectionName;        ///< Имя подключения к БД
    mutable QString m_lastError;     ///< Текст последней ошибки
    mutable qint64 m_lastInsertId;   ///< ID последней вставленной записи
    mutable int m_affectedRows;      ///< Количество затронутых записей
    bool m_inTransaction;            ///< Флаг активной транзакции

    /**
     * @brief Получить подключение к БД
     * @return Объект подключения
     */
    QSqlDatabase getDatabase() const;

    /**
     * @brief Установить информацию об ошибке
     * @param error Объект ошибки
     */
    void setError(const QSqlError& error);

    /**
     * @brief Экранировать значение для SQL-запроса
     * @param value Значение для экранирования
     * @return Экранированная строка
     */
    QString escapeValue(const QVariant& value) const;

    /**
     * @brief Построить список плейсхолдеров для prepared statement
     * @param count Количество плейсхолдеров
     * @return Строка вида "?, ?, ?"
     */
    QString buildPlaceholders(int count) const;

    /**
     * @brief Выполнить запрос и обновить статистику
     * @param query Подготовленный запрос
     * @return true если запрос выполнен успешно
     */
    bool executeAndUpdateStats(QSqlQuery& query);
};
