#include "logWindow.h"
#include <QApplication>
#include <QMainWindow>
#include "DBManager.h"
#include "MainWind.h"


int main( int argc, char *argv[]) {
/*
    int a = 3;
    int b = 99;
    a += b;
    b = a - b;
    a = a-b;
*/
    QApplication app(argc, argv);

    DatabaseManager dbInit("default_connection","my_database.db");
/*
    QVariantMap userData;
    userData["login"] = "NOTADMIN";
    userData["password"] = "NOTADMIN123";
    dbInit.getModifier()->insertRecord("users", userData);
*/

    MainWindow mainWindow(&dbInit);
    LogWindow logWindow(&dbInit);
    QObject::connect(&logWindow, &LogWindow::loginSuccess, [&mainWindow](){
        mainWindow.show();
    });

    logWindow.setWindowTitle("Logging Window");
    logWindow.show();
    //app.exec();
    return app.exec();
}
