#include "DBManager.h"

DatabaseManager::DatabaseManager()
{
    DBConnection dbManager;
    m_connection = &dbManager;
    DBTableSchemaManager schemaManager;
    m_schemaManager = &schemaManager;
}

DatabaseManager::DatabaseManager(const QString& connectionName, const QString& dbPath)
{
    DBConnection dbManager(connectionName, dbPath);
    m_connection = &dbManager;
    DBTableSchemaManager schemaManager(connectionName);
    m_schemaManager = &schemaManager;
}

DatabaseManager::~DatabaseManager()
{
    m_connection->closeDB();
}

DBConnection* DatabaseManager::getConnection() const
{
    return m_connection;
}

DBTableSchemaManager* DatabaseManager::getSchemaManager() const
{
    return m_schemaManager;
}
