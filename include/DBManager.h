#pragma once
#include "DBConnection.h"
#include "DBTableSchemaManager.h"

class DatabaseManager {
    public:
        DatabaseManager();
        DatabaseManager(const QString& connectionName, const QString& dbPath);
        ~DatabaseManager();

        // Инициализация
        bool initialize(const QString& connectionName, const QString& dbPath);
        void cleanup();

        // Получение менеджеров
        DBConnection* getConnection() const;
        DBTableSchemaManager* getSchemaManager() const;
        //DataReader* getReader() const;
        //DataModifier* getModifier() const;

        // Проверка состояния
        bool isReady() const;
        QString getLastError() const;

    private:
        DBConnection* m_connection;
        DBTableSchemaManager* m_schemaManager;
        //DataReader* m_reader;
        //DataModifier* m_modifier;
        QString m_lastError;
};
