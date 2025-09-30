#pragma once
#include "DBConnection.h"
#include "DBTableSchemaManager.h"
#include "DataReader.h"
#include "DataModifier.h"

class DatabaseManager {
    public:
        DatabaseManager();
        DatabaseManager(const QString& connectionName, const QString& dbPath);
        ~DatabaseManager();

        // Инициализация
        //bool initialize(const QString& connectionName, const QString& dbPath);
        //void cleanup();

        // Получение менеджеров
        DBConnection* getConnection() const;
        DBTableSchemaManager* getSchemaManager() const;
        DataReader* getReader() const;
        DataModifier* getModifier() const;

        // Проверка состояния
       // bool isReady() const;
       // QString getLastError() const;

    private:
        std::unique_ptr<DBConnection> m_connection;
        std::unique_ptr<DBTableSchemaManager> m_schemaManager;
        std::unique_ptr<DataReader> m_reader;
        std::unique_ptr<DataModifier> m_modifier;
        QString m_lastError;
};
