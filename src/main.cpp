#include "logWindow.h"
#include <QApplication>
#include <QMainWindow>
#include "DBManager.h"


int main( int argc, char *argv[]) {
/*
    int a = 3;
    int b = 99;
    a += b;
    b = a - b;
    a = a-b;
*/
    QApplication app(argc, argv);

    DatabaseManager dbInit;


    LogWindow logWindow(&dbInit);

    logWindow.setWindowTitle("Logging Window");
    logWindow.show();
    //app.exec();
    return app.exec();
}
