#include "DBManager.h"

DatabaseManager::DatabaseManager()
{
    DBConnection dbManager;
    m_connection = &dbManager;
}

DatabaseManager::DatabaseManager(const QString& connectionName, const QString& dbPath)
{
    DBConnection dbManager(connectionName, dbPath);
    m_connection = &dbManager;
}

DatabaseManager::~DatabaseManager()
{
    m_connection->closeDB();
}

DBConnection* DatabaseManager::getConnection() const
{
    return m_connection;
}
