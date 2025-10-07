#include "Table/Managers/DataSyncManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace TableSystem {

// ========== DetailedCommand ==========

QVariantMap DetailedCommand::toMap() const
{
    QVariantMap map;
    map["id"] = id;
    map["timestamp"] = timestamp.toString(Qt::ISODate);
    map["type"] = static_cast<int>(type);
    map["rowId"] = rowId;
    map["columnId"] = columnId;
    map["oldValue"] = oldValue;
    map["newValue"] = newValue;
    map["fullRowData"] = fullRowData;
    return map;
}

DetailedCommand DetailedCommand::fromMap(const QVariantMap& map)
{
    DetailedCommand cmd;
    cmd.id = map.value("id").toInt();
    cmd.timestamp = QDateTime::fromString(map.value("timestamp").toString(), Qt::ISODate);
    cmd.type = static_cast<ChangeType>(map.value("type").toInt());
    cmd.rowId = map.value("rowId").toInt();
    cmd.columnId = map.value("columnId").toString();
    cmd.oldValue = map.value("oldValue");
    cmd.newValue = map.value("newValue");
    cmd.fullRowData = map.value("fullRowData").toMap();
    return cmd;
}

// ========== SqlCommand ==========

QVariantMap SqlCommand::toMap() const
{
    QVariantMap map;
    map["id"] = id;
    map["timestamp"] = timestamp.toString(Qt::ISODate);
    map["sql"] = sql;
    map["params"] = params;
    return map;
}

SqlCommand SqlCommand::fromMap(const QVariantMap& map)
{
    SqlCommand cmd;
    cmd.id = map.value("id").toInt();
    cmd.timestamp = QDateTime::fromString(map.value("timestamp").toString(), Qt::ISODate);
    cmd.sql = map.value("sql").toString();
    cmd.params = map.value("params").toList();
    return cmd;
}

// ========== DataSyncManager ==========

DataSyncManager::DataSyncManager(QObject* parent)
    : QObject(parent)
    , m_nextCommandId(1)
{
}

// ========== Отслеживание изменений ==========

void DataSyncManager::trackCellUpdate(int rowId, const QString& columnId,
                                     const QVariant& oldValue, const QVariant& newValue)
{
    DetailedCommand cmd;
    cmd.id = generateCommandId();
    cmd.timestamp = QDateTime::currentDateTime();
    cmd.type = ChangeType::Update;
    cmd.rowId = rowId;
    cmd.columnId = columnId;
    cmd.oldValue = oldValue;
    cmd.newValue = newValue;

    m_detailedCommands.append(cmd);
    m_syncStatus[cmd.id] = false;

    // Создаем SQL команду (ЗАГОТОВКА - можете переопределить convertToSql)
    SqlCommand sqlCmd = convertToSql(cmd);
    m_sqlCommands.append(sqlCmd);

    emit commandAdded(cmd.id);
    emit syncStatusChanged(commandCount(), unsyncedCommandCount());
}

void DataSyncManager::trackRowInsert(int rowId, const QVariantMap& rowData)
{
    DetailedCommand cmd;
    cmd.id = generateCommandId();
    cmd.timestamp = QDateTime::currentDateTime();
    cmd.type = ChangeType::Insert;
    cmd.rowId = rowId;
    cmd.fullRowData = rowData;

    m_detailedCommands.append(cmd);
    m_syncStatus[cmd.id] = false;

    SqlCommand sqlCmd = convertToSql(cmd);
    m_sqlCommands.append(sqlCmd);

    emit commandAdded(cmd.id);
    emit syncStatusChanged(commandCount(), unsyncedCommandCount());
}

void DataSyncManager::trackRowDelete(int rowId, const QVariantMap& rowData)
{
    DetailedCommand cmd;
    cmd.id = generateCommandId();
    cmd.timestamp = QDateTime::currentDateTime();
    cmd.type = ChangeType::Delete;
    cmd.rowId = rowId;
    cmd.fullRowData = rowData;

    m_detailedCommands.append(cmd);
    m_syncStatus[cmd.id] = false;

    SqlCommand sqlCmd = convertToSql(cmd);
    m_sqlCommands.append(sqlCmd);

    emit commandAdded(cmd.id);
    emit syncStatusChanged(commandCount(), unsyncedCommandCount());
}

void DataSyncManager::trackColumnAdd(const QString& columnId)
{
    DetailedCommand cmd;
    cmd.id = generateCommandId();
    cmd.timestamp = QDateTime::currentDateTime();
    cmd.type = ChangeType::ColumnAdd;
    cmd.columnId = columnId;

    m_detailedCommands.append(cmd);
    m_syncStatus[cmd.id] = false;

    emit commandAdded(cmd.id);
    emit syncStatusChanged(commandCount(), unsyncedCommandCount());
}

void DataSyncManager::trackColumnRemove(const QString& columnId)
{
    DetailedCommand cmd;
    cmd.id = generateCommandId();
    cmd.timestamp = QDateTime::currentDateTime();
    cmd.type = ChangeType::ColumnRemove;
    cmd.columnId = columnId;

    m_detailedCommands.append(cmd);
    m_syncStatus[cmd.id] = false;

    emit commandAdded(cmd.id);
    emit syncStatusChanged(commandCount(), unsyncedCommandCount());
}

// ========== Получение команд ==========

QList<DetailedCommand> DataSyncManager::getDetailedCommands() const
{
    return m_detailedCommands;
}

QList<DetailedCommand> DataSyncManager::getUnsyncedDetailedCommands() const
{
    QList<DetailedCommand> result;
    for (const DetailedCommand& cmd : m_detailedCommands) {
        if (!m_syncStatus.value(cmd.id, false)) {
            result.append(cmd);
        }
    }
    return result;
}

QList<SqlCommand> DataSyncManager::getSqlCommands() const
{
    return m_sqlCommands;
}

QList<SqlCommand> DataSyncManager::getUnsyncedSqlCommands() const
{
    QList<SqlCommand> result;
    for (const SqlCommand& cmd : m_sqlCommands) {
        if (!m_syncStatus.value(cmd.id, false)) {
            result.append(cmd);
        }
    }
    return result;
}

int DataSyncManager::commandCount() const
{
    return m_detailedCommands.size();
}

int DataSyncManager::unsyncedCommandCount() const
{
    int count = 0;
    for (bool synced : m_syncStatus.values()) {
        if (!synced)
            count++;
    }
    return count;
}

// ========== Управление очередью ==========

void DataSyncManager::markAsSynced(int commandId)
{
    if (m_syncStatus.contains(commandId)) {
        m_syncStatus[commandId] = true;
        emit commandSynced(commandId);
        emit syncStatusChanged(commandCount(), unsyncedCommandCount());
    }
}

void DataSyncManager::markAllAsSynced()
{
    for (int id : m_syncStatus.keys()) {
        m_syncStatus[id] = true;
    }
    emit syncStatusChanged(commandCount(), 0);
}

void DataSyncManager::clearSyncedCommands()
{
    // Удаляем синхронизированные команды
    QList<DetailedCommand> newDetailedCommands;
    QList<SqlCommand> newSqlCommands;

    for (const DetailedCommand& cmd : m_detailedCommands) {
        if (!m_syncStatus.value(cmd.id, false)) {
            newDetailedCommands.append(cmd);
        } else {
            m_syncStatus.remove(cmd.id);
        }
    }

    for (const SqlCommand& cmd : m_sqlCommands) {
        if (!m_syncStatus.value(cmd.id, false)) {
            newSqlCommands.append(cmd);
        }
    }

    m_detailedCommands = newDetailedCommands;
    m_sqlCommands = newSqlCommands;

    emit syncStatusChanged(commandCount(), unsyncedCommandCount());
}

void DataSyncManager::clearAllCommands()
{
    m_detailedCommands.clear();
    m_sqlCommands.clear();
    m_syncStatus.clear();
    emit syncStatusChanged(0, 0);
}

// ========== Конвертация (ЗАГОТОВКИ) ==========

SqlCommand DataSyncManager::convertToSql(const DetailedCommand& cmd) const
{
    // ЗАГОТОВКА - вы можете реализовать свою логику преобразования
    SqlCommand sqlCmd;
    sqlCmd.id = cmd.id;
    sqlCmd.timestamp = cmd.timestamp;

    switch (cmd.type) {
        case ChangeType::Update:
            // Пример: UPDATE table SET columnId = ? WHERE rowId = ?
            sqlCmd.sql = QString("UPDATE table SET %1 = ? WHERE id = ?").arg(cmd.columnId);
            sqlCmd.params << cmd.newValue << cmd.rowId;
            break;

        case ChangeType::Insert:
            // Пример: INSERT INTO table (col1, col2) VALUES (?, ?)
            sqlCmd.sql = "INSERT INTO table (...) VALUES (...)";
            // Здесь вы можете построить INSERT на основе fullRowData
            break;

        case ChangeType::Delete:
            // Пример: DELETE FROM table WHERE id = ?
            sqlCmd.sql = "DELETE FROM table WHERE id = ?";
            sqlCmd.params << cmd.rowId;
            break;

        default:
            sqlCmd.sql = "-- Unknown command type";
            break;
    }

    return sqlCmd;
}

QByteArray DataSyncManager::generateSyncPacket(const QString& format) const
{
    // ЗАГОТОВКА - вы можете реализовать свой формат

    if (format == "json") {
        QJsonArray commandsArray;

        for (const DetailedCommand& cmd : getUnsyncedDetailedCommands()) {
            QJsonObject cmdObj;
            cmdObj["id"] = cmd.id;
            cmdObj["timestamp"] = cmd.timestamp.toString(Qt::ISODate);
            cmdObj["type"] = static_cast<int>(cmd.type);
            cmdObj["rowId"] = cmd.rowId;
            cmdObj["columnId"] = cmd.columnId;
            cmdObj["oldValue"] = QJsonValue::fromVariant(cmd.oldValue);
            cmdObj["newValue"] = QJsonValue::fromVariant(cmd.newValue);

            commandsArray.append(cmdObj);
        }

        QJsonObject packet;
        packet["commands"] = commandsArray;
        packet["count"] = commandsArray.size();

        QJsonDocument doc(packet);
        return doc.toJson(QJsonDocument::Compact);
    }

    // Для других форматов - ваша реализация
    return QByteArray();
}

void DataSyncManager::processSyncResponse(const QByteArray& response)
{
    // ЗАГОТОВКА - вы можете реализовать обработку ответа от сервера

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        qWarning() << "DataSyncManager: Invalid JSON response";
        return;
    }

    QJsonObject obj = doc.object();

    // Пример: сервер возвращает список ID синхронизированных команд
    if (obj.contains("syncedIds")) {
        QJsonArray syncedIds = obj["syncedIds"].toArray();

        for (const QJsonValue& val : syncedIds) {
            markAsSynced(val.toInt());
        }
    }
}

int DataSyncManager::generateCommandId()
{
    return m_nextCommandId++;
}

} // namespace TableSystem
