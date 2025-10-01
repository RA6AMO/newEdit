#include "DBManager.h"
//#include "DBTableSchemaManager.h"
//#include "DataReader.h"
#include <memory>

DatabaseManager::DatabaseManager()
{
    m_connection = std::make_unique<DBConnection>() ;
    m_schemaManager = std::make_unique<DBTableSchemaManager>();
    m_reader = std::make_unique<DataReader>();
    m_modifier = std::make_unique<DataModifier>();
}

DatabaseManager::DatabaseManager(const QString& connectionName, const QString& dbPath)
{
    m_connection = std::make_unique<DBConnection>(connectionName, dbPath) ;
    m_schemaManager = std::make_unique<DBTableSchemaManager>(connectionName);
    m_reader = std::make_unique<DataReader>(connectionName);
    m_modifier = std::make_unique<DataModifier>(connectionName);
}

DatabaseManager::~DatabaseManager()
{
    m_connection->closeDB();
}

DBConnection* DatabaseManager::getConnection() const
{
    return m_connection.get();
}

DBTableSchemaManager* DatabaseManager::getSchemaManager() const
{
    return m_schemaManager.get();
}

DataReader* DatabaseManager::getReader() const
{
    return m_reader.get();
}

DataModifier* DatabaseManager::getModifier() const
{
    return m_modifier.get();
}
