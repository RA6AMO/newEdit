#include "MainWind.h"
#include "LTreeWidget.h"
#include <QDateTime>
#include <QVariantMap>
#include "DataModifier.h"

MainWindow::MainWindow(DatabaseManager *dbInit, QMainWindow *parent)
    : QMainWindow(parent), dbMan(dbInit)
{
    setupUi(this);
    Ltree = std::make_unique<LTreeWidget>("tree_nodes",treeWidget, dbMan);

    connect(pushButton_createTree, &QPushButton::clicked, this, &MainWindow::tableCreater);

    // Подключаем сигналы от дерева к слотам главного окна
    connect(Ltree.get(), &LTreeWidget::itemClicked, this, &MainWindow::onTreeItemClicked);
    connect(Ltree.get(), &LTreeWidget::itemDoubleClicked, this, &MainWindow::onTreeItemDoubleClicked);

    tableFiller("example_table");
}

MainWindow::~MainWindow()
{

}

bool MainWindow::tableCreater()
{
	QList<ColumnDefinition> columns;

	// INTEGER PRIMARY KEY AUTOINCREMENT - автоинкрементный первичный ключ
	columns << ColumnDefinition("id", "INTEGER", true, true, true, false);

	// TEXT - текстовые данные (NOT NULL)
	columns << ColumnDefinition("name", "TEXT", false, false, true, false);

	// REAL - числа с плавающей точкой
	columns << ColumnDefinition("price", "REAL", false, false, false, false);

	// INTEGER - целые числа
	columns << ColumnDefinition("quantity", "INTEGER", false, false, false, false);

	// TEXT - текстовое поле для даты/времени
	columns << ColumnDefinition("created_at", "TEXT", false, false, false, false);

	// BLOB - бинарные данные (например, для изображений)
	columns << ColumnDefinition("binary_data", "BLOB", false, false, false, false);

	if (!dbMan->getSchemaManager()->createTable("example_table", columns)) {
		qDebug() << "error creating table:" << dbMan->getSchemaManager()->getLastError();
		return false;
	}

	qDebug() << "table example_table successfully created";
	return true;
}

bool MainWindow::tableFiller(const QString& tableName)
{
	// Проверяем, что таблица существует
	if (!dbMan->getSchemaManager()->tableExists(tableName)) {
		qDebug() << "Таблица" << tableName << "не существует";
		return false;
	}

	DataModifier* modifier = dbMan->getModifier();

	// Начинаем транзакцию для быстрой вставки
	if (!modifier->beginTransaction()) {
		qDebug() << "Ошибка начала транзакции:" << modifier->getLastError();
		return false;
	}

	// Массив тестовых данных
	QList<QVariantMap> testData = {
		{
			{"name", "Ноутбук Lenovo"},
			{"price", 45999.99},
			{"quantity", 15},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		},
		{
			{"name", "Клавиатура механическая"},
			{"price", 3500.50},
			{"quantity", 42},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		},
		{
			{"name", "Мышь беспроводная"},
			{"price", 1250.00},
			{"quantity", 67},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		},
		{
			{"name", "Монитор 27 дюймов"},
			{"price", 18750.75},
			{"quantity", 23},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		},
		{
			{"name", "USB-кабель Type-C"},
			{"price", 299.99},
			{"quantity", 150},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		},
		{
			{"name", "Внешний SSD 1TB"},
			{"price", 8990.00},
			{"quantity", 34},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		},
		{
			{"name", "Веб-камера HD"},
			{"price", 2150.00},
			{"quantity", 28},
			{"created_at", QDateTime::currentDateTime().toString(Qt::ISODate)},
			{"binary_data", QByteArray()}
		}
	};

	// Вставляем все записи
	int successCount = 0;
	for (const auto& record : testData) {
		if (modifier->insertRecord(tableName, record)) {
			successCount++;
			qDebug() << "Вставлена запись:" << record["name"].toString();
		} else {
			qDebug() << "Ошибка вставки записи:" << record["name"].toString()
			         << "-" << modifier->getLastError();
		}
	}

	// Подтверждаем транзакцию
	if (!modifier->commitTransaction()) {
		qDebug() << "Ошибка подтверждения транзакции:" << modifier->getLastError();
		modifier->rollbackTransaction();
		return false;
	}

	qDebug() << "Успешно вставлено записей:" << successCount << "из" << testData.size();
	return successCount > 0;
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
