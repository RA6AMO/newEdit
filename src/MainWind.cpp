#include "MainWind.h"
#include "TreeWidget.h"

MainWindow::MainWindow(DatabaseManager *dbInit, QMainWindow *parent)
    : QMainWindow(parent), dbMan(dbInit)
{
    setupUi(this);
    Ltree = std::make_unique<LTreeWidget>(treeWidget);

}

MainWindow::~MainWindow()
{

}
