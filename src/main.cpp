#include "logWindow.h"
#include <QApplication>
#include <QMainWindow>
#include "DBManager.h"


int main( int argc, char *argv[]) {

    QApplication app(argc, argv);

    DatabaseManager dbInit;


    LogWindow logWindow(dbInit);

    logWindow.setWindowTitle("Logging Window");
    logWindow.show();
    //app.exec();
    return app.exec();
}
