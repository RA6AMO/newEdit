#pragma once
#include <QSqlDatabase>
#include <QString>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QList>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

#define DEFAULT_CONNECTION_NAME "default_connection"

class DBConnection{
public:
    DBConnection();
    DBConnection(QString connectionName, QString filePath);
    ~DBConnection();

    DBConnection(const DBConnection& other);
    DBConnection(DBConnection&& other);

    DBConnection& operator =(DBConnection& other);
    DBConnection& operator =(DBConnection&& other);
    bool openDB(QString connectionName, QString filePath);
    //void closeDB(QString connectionName);
    void closeDB();
    void createTable();
private:
    int CountConnections;
    //QSqlDatabase db;
    //QList<QString> connectionNames;
};
