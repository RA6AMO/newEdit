#include "MainWind.h"
#include "LTreeWidget.h"

MainWindow::MainWindow(DatabaseManager *dbInit, QMainWindow *parent)
    : QMainWindow(parent), dbMan(dbInit)
{
    setupUi(this);
    Ltree = std::make_unique<LTreeWidget>("tree_nodes",treeWidget,dbMan);

    connect(pushButton_createTree, &QPushButton::clicked, this, &MainWindow::tableCreater);
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
