# Пример использования Table System

## Быстрый старт

### 1. Базовое использование в MainWindow

```cpp
#include "Table/MainTable.h"
using namespace TableSystem;

// В MainWindow.h
class MainWindow : public QMainWindow {
private:
    MainTable* m_mainTable;
};

// В MainWindow.cpp конструкторе
void MainWindow::setupTable() {
    // Создание таблицы
    m_mainTable = new MainTable(this);
    
    // Определение столбцов
    QList<TableColumn> columns;
    
    // ID столбец (не редактируемый)
    TableColumn idCol("id", "ID", ColumnType::Number);
    idCol.setEditable(false);
    columns << idCol;
    
    // Текстовые столбцы
    columns << TableColumn("name", "Название", ColumnType::Text);
    columns << TableColumn("description", "Описание", ColumnType::Text);
    
    // Числовой столбец
    TableColumn priceCol("price", "Цена", ColumnType::Number);
    priceCol.setDefaultValue(0.0);
    columns << priceCol;
    
    // Столбец с датой
    columns << TableColumn("created_at", "Дата создания", ColumnType::Date);
    
    // Столбец с кнопкой
    TableColumn actionCol("action", "Действие", ColumnType::Button);
    actionCol.setButtonText("Открыть");
    actionCol.setEditable(false);
    columns << actionCol;
    
    m_mainTable->setColumns(columns);
    
    // Загрузка данных из DatabaseManager
    QList<QVariantMap> data = dbMan->getReader()->selectAll("example_table");
    m_mainTable->setData(data);
    
    // Показать toolbar с кнопками управления
    m_mainTable->showToolbar(true);
    
    // Добавить на layout
    centralWidget()->layout()->addWidget(m_mainTable);
}
```

### 2. Регистрация обработчика кнопки

```cpp
// Обработчик для столбца с кнопкой "action"
m_mainTable->registerCellHandler("action", 
    [this](int row, int col, const QVariant& cellData) -> CellActionResult {
        // Получить данные строки
        QString name = m_mainTable->model()->cellData(row, 1).toString();
        int id = m_mainTable->model()->cellData(row, 0).toInt();
        
        qDebug() << "Кнопка нажата для строки:" << row << "ID:" << id << "Имя:" << name;
        
        // Здесь ваша логика - открыть файл, показать диалог и т.д.
        // Пример: открыть детальное окно
        // DetailDialog dialog(id, this);
        // dialog.exec();
        
        return CellActionResult(true, QString("Действие выполнено для %1").arg(name));
    }
);
```

### 3. Добавление столбца динамически

```cpp
// Через ColumnManager (более гибко)
TableColumn statusCol("status", "Статус", ColumnType::ComboBox);
statusCol.setComboBoxItems({"Новый", "В работе", "Завершен", "Отменен"});
statusCol.setDefaultValue("Новый");
m_mainTable->addColumn(statusCol);

// Или проще
m_mainTable->addColumn("Примечание", ColumnType::Text);

// Используя ColumnManager напрямую
m_mainTable->columnManager()->addComboBoxColumn("Приоритет", {"Низкий", "Средний", "Высокий"});
```

### 4. Работа с изменениями

```cpp
// Проверка несохраненных изменений
if (m_mainTable->hasUnsavedChanges()) {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Несохраненные изменения",
        "Сохранить изменения?",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel
    );
    
    if (reply == QMessageBox::Yes) {
        saveTableChanges();
    } else if (reply == QMessageBox::No) {
        m_mainTable->revertChanges();
    }
}

// Получить измененные данные
QList<QVariantMap> modifiedRows = m_mainTable->getModifiedData();
for (const QVariantMap& row : modifiedRows) {
    qDebug() << "Измененная строка:" << row;
}
```

### 5. Отправка изменений на сервер (через TCP)

```cpp
void MainWindow::saveTableChanges() {
    // Получить очередь команд
    QList<DetailedCommand> commands = m_mainTable->getPendingCommands();
    
    if (commands.isEmpty()) {
        qDebug() << "Нет изменений для сохранения";
        return;
    }
    
    // Вариант 1: Детальный формат
    for (const DetailedCommand& cmd : commands) {
        QVariantMap cmdData = cmd.toMap();
        
        // Отправить через TCP
        // tcpClient->sendCommand(cmdData);
        
        qDebug() << "Команда:" << cmd.type << "Строка:" << cmd.rowId << "Столбец:" << cmd.columnId;
    }
    
    // Вариант 2: SQL формат
    QList<SqlCommand> sqlCommands = m_mainTable->syncManager()->getUnsyncedSqlCommands();
    for (const SqlCommand& cmd : sqlCommands) {
        qDebug() << "SQL:" << cmd.sql << "Params:" << cmd.params;
        // tcpClient->sendSQL(cmd.sql, cmd.params);
    }
    
    // Вариант 3: JSON пакет
    QByteArray jsonPacket = m_mainTable->syncManager()->generateSyncPacket("json");
    // tcpClient->send(jsonPacket);
    
    // После успешной отправки
    m_mainTable->markAsSaved();
    m_mainTable->clearPendingCommands();
}

// Обработка ответа от сервера
void MainWindow::onServerResponse(const QByteArray& response) {
    m_mainTable->syncManager()->processSyncResponse(response);
}
```

### 6. Интеграция с LTreeWidget (фильтрация)

```cpp
// Подключить сигнал от дерева
connect(m_treeWidget, &LTreeWidget::itemClicked,
        this, [this](const QString& nodeId, const QString& nodeName) {
    // Фильтровать таблицу по выбранному узлу дерева
    // Например, показывать только записи с parent_id = nodeId
    
    // Способ 1: Перезагрузить данные из БД с фильтром
    QList<QVariantMap> filteredData = dbMan->getReader()->select(
        "example_table",
        QString("parent_id = %1").arg(nodeId)
    );
    m_mainTable->setData(filteredData);
    
    // Способ 2: Использовать QSortFilterProxyModel (добавьте позже)
});
```

### 7. Расширение функционала

#### Добавление валидации через metadata

```cpp
TableColumn emailCol("email", "Email", ColumnType::Text);
emailCol.setMetadata("validation_regex", "^[\\w.-]+@[\\w.-]+\\.\\w+$");
emailCol.setMetadata("error_message", "Неверный формат email");
emailCol.setMetadata("required", true);

m_mainTable->addColumn(emailCol);

// В вашем коде валидации
QString regex = column.metadata("validation_regex").toString();
if (!QRegExp(regex).exactMatch(value)) {
    cell->setValid(false);
    cell->setValidationError(column.metadata("error_message").toString());
}
```

#### Создание собственного делегата

```cpp
// MyCustomDelegate.h
class MyCustomDelegate : public TableSystem::BaseCellDelegate {
public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        // Ваша кастомная отрисовка
        // Например: прогресс-бар, звездочки рейтинга и т.д.
    }
};

// Регистрация
MyCustomDelegate* delegate = new MyCustomDelegate(this);
m_mainTable->interactionManager()->registerDelegate("rating", delegate);
```

#### Обработка изменений в реальном времени

```cpp
connect(m_mainTable, &MainTable::cellActionTriggered,
        this, [](const QString& columnId, const CellActionResult& result) {
    qDebug() << "Действие в столбце:" << columnId;
    qDebug() << "Результат:" << result.success << result.message;
});

connect(m_mainTable, &MainTable::unsavedChangesStateChanged,
        this, [this](bool hasChanges) {
    // Обновить UI - показать индикатор несохраненных изменений
    saveButton->setEnabled(hasChanges);
    statusBar()->showMessage(hasChanges ? "Есть несохраненные изменения" : "Все сохранено");
});
```

## Реализация своих функций

### 1. Реализация convertToSql в DataSyncManager

```cpp
// Переопределите метод для вашей БД структуры
SqlCommand DataSyncManager::convertToSql(const DetailedCommand& cmd) const {
    SqlCommand sqlCmd;
    sqlCmd.id = cmd.id;
    sqlCmd.timestamp = cmd.timestamp;
    
    switch (cmd.type) {
        case ChangeType::Update:
            sqlCmd.sql = QString("UPDATE my_table SET %1 = ? WHERE id = ?").arg(cmd.columnId);
            sqlCmd.params << cmd.newValue << cmd.rowId;
            break;
            
        case ChangeType::Insert: {
            QStringList columns = cmd.fullRowData.keys();
            QStringList placeholders;
            for (int i = 0; i < columns.size(); ++i) {
                placeholders << "?";
            }
            
            sqlCmd.sql = QString("INSERT INTO my_table (%1) VALUES (%2)")
                .arg(columns.join(", "))
                .arg(placeholders.join(", "));
            
            for (const QString& col : columns) {
                sqlCmd.params << cmd.fullRowData[col];
            }
            break;
        }
            
        case ChangeType::Delete:
            sqlCmd.sql = "DELETE FROM my_table WHERE id = ?";
            sqlCmd.params << cmd.rowId;
            break;
            
        default:
            break;
    }
    
    return sqlCmd;
}
```

### 2. Реализация делегата для специфических данных

```cpp
// ProgressBarDelegate.h - отображение прогресса
class ProgressBarDelegate : public TableSystem::BaseCellDelegate {
public:
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        int progress = index.data().toInt();
        
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QString("%1%").arg(progress);
        progressBarOption.textVisible = true;
        
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }
};

// Использование
m_mainTable->interactionManager()->registerDelegate("progress", new ProgressBarDelegate(this));
```

## Советы по использованию

1. **Всегда устанавливайте столбцы перед данными**
   ```cpp
   m_mainTable->setColumns(columns);  // Сначала
   m_mainTable->setData(data);        // Потом
   ```

2. **Используйте metadata для расширяемости**
   ```cpp
   column.setMetadata("custom_property", myValue);
   ```

3. **Не забывайте про сигналы**
   ```cpp
   connect(m_mainTable, &MainTable::dataChanged, this, &MyClass::onDataChanged);
   ```

4. **Используйте уникальные ID для столбцов**
   ```cpp
   TableColumn col("unique_id", "Отображаемое имя", ColumnType::Text);
   ```

5. **Проверяйте изменения перед закрытием окна**
   ```cpp
   void MyWindow::closeEvent(QCloseEvent* event) {
       if (m_mainTable->hasUnsavedChanges()) {
           // Спросить пользователя
       }
   }
   ```
