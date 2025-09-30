#include "DBConnection.h"
#include <QCoreApplication>

DBConnection::DBConnection()
{
    CountConnections = 0;
    // Получаем путь к корню проекта (где находится исполняемый файл)
    QString defaultPath = QCoreApplication::applicationDirPath();
    if (!QDir().mkpath(defaultPath)) { // Создаем папку если её нет, обработка ошибки
        qWarning() << "folder not opening" << defaultPath;
        // Можно выбросить исключение или обработать ошибку по-другому
        return;
    }
    QString dbPath = defaultPath + "/my_database.db";
    QString connectionName = "default_connection";
    if (!openDB(connectionName, dbPath)) {
        qWarning() << "not opening database" << dbPath;
        // Можно выбросить исключение или обработать ошибку по-другому
        return;
    }
}


DBConnection::DBConnection(QString connectionName, QString filePath)
{
    CountConnections = 0;
    openDB(connectionName, filePath);
}

DBConnection::~DBConnection()
{
    closeDB();
}

DBConnection::DBConnection(const DBConnection& other){
    CountConnections = other.CountConnections;
}
DBConnection::DBConnection(DBConnection&& other){
    CountConnections = other.CountConnections;
    other.CountConnections = 0;
}

DBConnection& DBConnection::operator =(DBConnection& other){
    if (this == &other) {
        return *this;
    }
    CountConnections = other.CountConnections;
    return *this;
}
DBConnection& DBConnection::operator =(DBConnection&& other){
    if (this == &other) {
        return *this;
    }
    CountConnections = other.CountConnections;
    other.CountConnections = 0;
    return *this;
}

bool DBConnection::openDB(QString connectionName, QString filePath)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    db.setDatabaseName(filePath);

    if (!db.open()){
        return false;
    }
    if(connectionName == DEFAULT_CONNECTION_NAME){
        createTable();
        CountConnections++;
    }
    //createTable();

    return true;
}

void DBConnection::closeDB()
{
    QList<QString> connectionList = QSqlDatabase::connectionNames();

    for (const QString &connectionName : connectionList){
        QSqlDatabase db = QSqlDatabase::database(connectionName);
        if (db.isOpen()){
            db.close();
        }
        QSqlDatabase::removeDatabase(connectionName);
    }
    //QSqlDatabase::removeDatabase(connectionName);
}

void DBConnection::createTable()
{
    QList<QString> connectionList = QSqlDatabase::connectionNames();

    if (connectionList.isEmpty()) {
        qWarning() << "no active connections with database";
        return;
    }
    if (connectionList.indexOf(DEFAULT_CONNECTION_NAME) == -1) {
        qWarning() << "no active connection with database by name:" << DEFAULT_CONNECTION_NAME;
        return;
    }
    QSqlDatabase db = QSqlDatabase::database(connectionList.at(connectionList.indexOf(DEFAULT_CONNECTION_NAME)));

    if (!db.isOpen()) {
        qWarning() << "database not opened";
        return;
    }

    QSqlQuery query(db);

    // Подготавливаем SQL-запрос для проверки существования таблицы 'users' в системной таблице sqlite_master
    query.prepare("SELECT name FROM sqlite_master WHERE type='table' AND name='users'");

    // Выполняем запрос и проверяем на ошибки
    if (!query.exec()) {
        qWarning() << "error checking existence of table:" << query.lastError().text();
        return;
    }
    // Если таблица не существует (query.next() возвращает false), создаем её
    if (!query.next()) {
        // Формируем SQL-запрос для создания таблицы пользователей
        QString createTableQuery =
            "CREATE TABLE users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT," // Автоинкрементный первичный ключ
            "login TEXT UNIQUE NOT NULL," // Уникальное поле логина (не может быть пустым)
            "password TEXT NOT NULL," // Поле пароля (не может быть пустым)
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP" // Время создания записи (по умолчанию текущее время)
            ")";

        // Выполняем запрос создания таблицы и проверяем на ошибки
        if (!query.exec(createTableQuery)) {
            qWarning() << "error creating table users:" << query.lastError().text();
        } else {
            qDebug() << "table users successfully created";
        }
    } else {
        qDebug() << "table users already exists";
    }
}
