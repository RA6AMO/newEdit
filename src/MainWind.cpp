#include "MainWind.h"

MainWindow::MainWindow(DatabaseManager *dbInit, QWidget *parent)
    : QMainWindow(parent), dbMan(dbInit)
{
    //setupUi(this);


}

MainWindow::~MainWindow()
{
}
