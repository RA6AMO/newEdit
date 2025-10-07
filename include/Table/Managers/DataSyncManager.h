#pragma once
#include <QObject>
#include <QDateTime>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QVariantMap>
#include <QByteArray>
#include "../TableTypes.h"

namespace TableSystem {

// Детальная команда
struct DetailedCommand {
    int id;
    QDateTime timestamp;
    ChangeType type;
    int rowId;
    QString columnId;
    QVariant oldValue;
    QVariant newValue;
    QVariantMap fullRowData;  // Для INSERT

    DetailedCommand() : id(0), rowId(-1), type(ChangeType::Update) {}

    QVariantMap toMap() const;
    static DetailedCommand fromMap(const QVariantMap& map);
};

// SQL-подобная команда
struct SqlCommand {
    int id;
    QDateTime timestamp;
    QString sql;
    QVariantList params;

    SqlCommand() : id(0) {}

    QVariantMap toMap() const;
    static SqlCommand fromMap(const QVariantMap& map);
};

class DataSyncManager : public QObject {
    Q_OBJECT

public:
    explicit DataSyncManager(QObject* parent = nullptr);

    // ========== Отслеживание изменений ==========

    void trackCellUpdate(int rowId, const QString& columnId,
                         const QVariant& oldValue, const QVariant& newValue);

    void trackRowInsert(int rowId, const QVariantMap& rowData);

    void trackRowDelete(int rowId, const QVariantMap& rowData);

    void trackColumnAdd(const QString& columnId);

    void trackColumnRemove(const QString& columnId);

    // ========== Получение команд ==========

    // Детальный формат
    QList<DetailedCommand> getDetailedCommands() const;
    QList<DetailedCommand> getUnsyncedDetailedCommands() const;

    // SQL формат
    QList<SqlCommand> getSqlCommands() const;
    QList<SqlCommand> getUnsyncedSqlCommands() const;

    // Общая информация
    int commandCount() const;
    int unsyncedCommandCount() const;

    // ========== Управление очередью ==========

    void markAsSynced(int commandId);
    void markAllAsSynced();

    void clearSyncedCommands();
    void clearAllCommands();

    // ========== Конвертация (ЗАГОТОВКИ - для вашей реализации) ==========

    // Генерация SQL из детальной команды
    SqlCommand convertToSql(const DetailedCommand& cmd) const;

    // Генерация пакета для отправки на сервер
    QByteArray generateSyncPacket(const QString& format = "json") const;

    // Обработка ответа от сервера
    void processSyncResponse(const QByteArray& response);

signals:
    void commandAdded(int commandId);
    void commandSynced(int commandId);
    void syncStatusChanged(int total, int synced);

private:
    QList<DetailedCommand> m_detailedCommands;
    QList<SqlCommand> m_sqlCommands;
    QMap<int, bool> m_syncStatus;  // commandId -> synced
    int m_nextCommandId;

    int generateCommandId();
};

} // namespace TableSystem
