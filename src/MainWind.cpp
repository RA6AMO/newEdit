#include "MainWind.h"
#include "LTreeWidget.h"

MainWindow::MainWindow(DatabaseManager *dbInit, QMainWindow *parent)
    : QMainWindow(parent), dbMan(dbInit)
{
    setupUi(this);
    Ltree = std::make_unique<LTreeWidget>("tree_nodes",treeWidget, dbMan);

    connect(pushButton_createTree, &QPushButton::clicked, this, &MainWindow::tableCreater);

    // Подключаем сигналы от дерева к слотам главного окна
    connect(Ltree.get(), &LTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
    connect(Ltree.get(), &LTreeWidget::itemDoubleClicked, this, &MainWindow::onTreeItemDoubleClicked);
}

MainWindow::~MainWindow()
{

}

bool MainWindow::tableCreater()
{
	QList<ColumnDefinition> columns;

	columns << ColumnDefinition("id", "INTEGER PRIMARY KEY AUTOINCREMENT", false, false, true, false);
	columns << ColumnDefinition("name", "TEXT", false, false, true, false);
	columns << ColumnDefinition("parent_id", "INTEGER", false, false, false, false);

	DBTableSchemaManager schema;
	if (!schema.createTable("tree_nodes", columns)) {
		return false;
	}
	return true;
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
