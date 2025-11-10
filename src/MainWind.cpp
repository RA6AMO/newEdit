#include "MainWind.h"


MainWindow::MainWindow(DatabaseManager *dbInit, QMainWindow *parent)
    : QMainWindow(parent), dbMan(dbInit)
{
    setupUi(this);



    Ltree = std::make_unique<LTreeWidget>("tree_nodes",treeWidget, dbMan);



}

MainWindow::~MainWindow()
{

}


void MainWindow::onTreeItemClicked(const QString &nodeId, const QString &nodeName)
{
    // Здесь обрабатываем клик по элементу дерева
    qDebug() << "Клик по элементу:" << nodeName << "ID:" << nodeId;
    // Добавьте свою логику обработки клика
}

void MainWindow::onTreeItemDoubleClicked(const QString &nodeId, const QString &nodeName)
{
    // Здесь обрабатываем двойной клик по элементу дерева
    qDebug() << "Двойной клик по элементу:" << nodeName << "ID:" << nodeId;
    // Добавьте свою логику обработки двойного клика
}
