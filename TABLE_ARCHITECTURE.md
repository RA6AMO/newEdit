# Архитектура системы управления таблицей

## Обзор

Реализована гибкая и расширяемая система управления таблицами с поддержкой динамических столбцов, интерактивных ячеек и синхронизации с сервером. Архитектура следует принципам SOLID и позволяет легко расширять функционал.

## Структура проекта

```
include/Table/
├── TableTypes.h              # Базовые типы (enum, struct)
├── TableCell.h               # Класс ячейки с метаданными
├── TableColumn.h             # Класс столбца
├── TableModel.h              # Модель данных (QAbstractTableModel)
├── MainTable.h               # Главный виджет
├── Managers/
│   ├── ColumnManager.h       # Управление столбцами
│   ├── CellInteractionManager.h  # Интерактивные ячейки
│   └── DataSyncManager.h     # Синхронизация с сервером
└── Delegates/
    ├── BaseCellDelegate.h    # Базовый делегат (заготовка)
    ├── ButtonDelegate.h      # Делегат кнопки (заготовка)
    └── ComboBoxDelegate.h    # Делегат комбобокса (заготовка)

src/Table/
├── TableModel.cpp
├── TableCell.cpp
├── TableColumn.cpp
├── MainTable.cpp
├── Managers/
│   ├── ColumnManager.cpp
│   ├── CellInteractionManager.cpp
│   └── DataSyncManager.cpp
└── Delegates/
    ├── BaseCellDelegate.cpp
    ├── ButtonDelegate.cpp
    └── ComboBoxDelegate.cpp
```

## Компоненты системы

### 1. TableTypes.h - Базовые типы

**Назначение:** Центральный файл с общими типами данных.

**Содержит:**
- `ColumnType` - типы столбцов (Text, Number, Date, Image, Boolean, ComboBox, Button, FileDialog, Custom)
- `ChangeType` - типы операций (Insert, Update, Delete, ColumnAdd, ColumnRemove)
- `CellActionResult` - результат действия с ячейкой
- `CellActionHandler` - тип функции-обработчика

**Точки расширения:**
- Добавляйте свои типы столбцов в `ColumnType::Custom`
- Используйте `CellActionResult.additionalData` для передачи дополнительных данных

### 2. TableCell - Ячейка с метаданными

**Назначение:** Обертка над QVariant с информацией об изменениях и валидации.

**Ключевые возможности:**
- Хранение данных (`QVariant data`)
- Отслеживание изменений (`bool modified`)
- Валидация (`bool valid`, `QString validationError`)
- Откат изменений (`originalData`, `restoreOriginal()`)
- Метаданные (`QVariantMap metadata`)

**Пример:**
```cpp
TableCell cell("Значение");
cell.setMetadata("background_color", QColor(Qt::yellow));
cell.setData("Новое значение"); // modified = true
cell.restoreOriginal();          // Откат к "Значение"
```

### 3. TableColumn - Описание столбца

**Назначение:** Содержит всю информацию о столбце.

**Свойства:**
- `id` - уникальный идентификатор
- `name` - отображаемое имя
- `type` - тип данных (ColumnType)
- `editable` - можно ли редактировать
- `visible` - отображается ли столбец
- `defaultValue` - значение по умолчанию
- Специфичные для типа: `comboBoxItems`, `buttonText`, `fileFilter`
- `metadata` - дополнительные настройки

**Пример:**
```cpp
TableColumn col("price", "Цена", ColumnType::Number);
col.setDefaultValue(0.0);
col.setMetadata("min_value", 0);
col.setMetadata("max_value", 999999);
col.setMetadata("precision", 2); // Количество знаков после запятой
```

### 4. TableModel - Модель данных

**Назначение:** Хранит данные таблицы и предоставляет интерфейс для QTableView.

**Ключевые возможности:**
- Наследуется от `QAbstractTableModel`
- Два варианта установки данных: `setTableData(QList<QVariantMap>)` и `setTableData(QList<QVector<QVariant>>)`
- Динамическое добавление/удаление столбцов и строк
- Отслеживание изменений (`hasUnsavedChanges()`, `modifiedCells()`)
- Валидация данных
- Интеграция с менеджерами

**Внутренняя структура:**
```cpp
QList<TableColumn> m_columns;           // Список столбцов
QList<QList<TableCell>> m_data;         // Данные [строка][столбец]
QMap<QString, int> m_columnIndexMap;    // columnId -> индекс
```

**Сигналы:**
- `cellDataChanged(int row, int col, old, new)` - изменение ячейки
- `rowInserted(int row)` - добавление строки
- `rowDeleted(int row)` - удаление строки
- `columnAdded(QString id)` - добавление столбца
- `modelModified(bool hasChanges)` - изменение состояния

### 5. ColumnManager - Управление столбцами

**Назначение:** Упрощает работу со столбцами таблицы.

**Методы:**
- `addColumn(name, type)` - простое добавление
- `addTextColumn()`, `addNumberColumn()`, `addComboBoxColumn()` - специализированные (ЗАГОТОВКИ)
- `removeColumn(id)` - удаление
- `setColumnVisible()`, `setColumnEditable()`, `renameColumn()` - изменение
- `generateUniqueColumnId()` - генерация уникального ID

**Пример:**
```cpp
// Через MainTable
mainTable->addColumn("Примечание", ColumnType::Text);

// Через ColumnManager
mainTable->columnManager()->addComboBoxColumn(
    "Статус",
    {"Новый", "В работе", "Завершен"}
);
```

### 6. DataSyncManager - Синхронизация изменений

**Назначение:** Отслеживает изменения для отправки на сервер в ДВУХ форматах.

**Форматы команд:**

**DetailedCommand (детальный):**
```cpp
struct DetailedCommand {
    int id;
    QDateTime timestamp;
    ChangeType type;
    int rowId;
    QString columnId;
    QVariant oldValue;
    QVariant newValue;
    QVariantMap fullRowData;  // Для INSERT
};
```

**SqlCommand (SQL-подобный):**
```cpp
struct SqlCommand {
    int id;
    QDateTime timestamp;
    QString sql;
    QVariantList params;
};
```

**Методы:**
- `trackCellUpdate()`, `trackRowInsert()`, `trackRowDelete()` - отслеживание
- `getDetailedCommands()`, `getSqlCommands()` - получение команд
- `markAsSynced()`, `clearSyncedCommands()` - управление очередью
- `convertToSql()` - конвертация (ЗАГОТОВКА - для вашей реализации)
- `generateSyncPacket()` - формирование пакета (ЗАГОТОВКА)
- `processSyncResponse()` - обработка ответа (ЗАГОТОВКА)

**Пример использования:**
```cpp
// Получить команды
QList<DetailedCommand> commands = mainTable->getPendingCommands();

// Отправить на сервер (ваша реализация)
for (const DetailedCommand& cmd : commands) {
    tcpClient->send(cmd.toMap());
}

// Пометить как синхронизированные
mainTable->markAsSaved();
```

### 7. CellInteractionManager - Интерактивные ячейки

**Назначение:** Регистрация и вызов обработчиков для интерактивных столбцов.

**Методы:**
- `registerButtonHandler()` - регистрация обработчика кнопки
- `registerComboBoxHandler()` - обработчик комбобокса
- `registerFileDialogHandler()` - обработчик выбора файла
- `registerDelegate()` - регистрация кастомного делегата (ЗАГОТОВКА)
- `handleCellClick()` - обработка клика

**Пример:**
```cpp
mainTable->registerCellHandler("action", 
    [this](int row, int col, const QVariant& data) -> CellActionResult {
        QString name = mainTable->model()->cellData(row, 1).toString();
        
        // Ваша логика
        openDocument(name);
        
        return CellActionResult(true, "Документ открыт");
    }
);
```

### 8. Делегаты - Кастомная отрисовка

**Назначение:** Отображение специфичных типов данных в ячейках.

**Доступные делегаты (ЗАГОТОВКИ):**
- `BaseCellDelegate` - базовый класс
- `ButtonDelegate` - кнопка в ячейке
- `ComboBoxDelegate` - выпадающий список

**Создание своего делегата:**
```cpp
class MyDelegate : public TableSystem::BaseCellDelegate {
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override {
        // Ваша отрисовка
    }
};

// Регистрация
mainTable->interactionManager()->registerDelegate("myColumn", new MyDelegate(this));
```

### 9. MainTable - Главный виджет

**Назначение:** Фасад, объединяющий все компоненты.

**Основные методы:**
- `setColumns()`, `setData()` - установка данных
- `getData()`, `getModifiedData()` - получение данных
- `addColumn()`, `removeColumn()` - управление столбцами
- `registerCellHandler()` - регистрация обработчиков
- `hasUnsavedChanges()`, `markAsSaved()`, `revertChanges()` - изменения
- `getPendingCommands()` - получение команд для сервера
- `showToolbar()`, `enableEditing()` - UI

**Сигналы:**
- `dataChanged()` - данные изменились
- `columnAdded(id)`, `columnRemoved(id)` - изменение столбцов
- `cellActionTriggered(id, result)` - действие с ячейкой
- `unsavedChangesStateChanged(bool)` - изменение статуса

## Принципы SOLID в архитектуре

### Single Responsibility (Единственная ответственность)
- **TableModel** - только хранение и предоставление данных
- **ColumnManager** - только управление столбцами
- **CellInteractionManager** - только обработка интерактивности
- **DataSyncManager** - только отслеживание изменений

### Open/Closed (Открыт для расширения, закрыт для модификации)
- Добавление новых типов столбцов через `ColumnType::Custom`
- Регистрация обработчиков через `std::function` (лямбды)
- Метаданные в `TableColumn` и `TableCell` для расширения

### Liskov Substitution (Принцип подстановки Барбары Лисков)
- Делегаты наследуются от `BaseCellDelegate`
- Можно использовать любой делегат через базовый интерфейс

### Interface Segregation (Разделение интерфейса)
- Менеджеры имеют узкие, специализированные интерфейсы
- Не нужно реализовывать всё сразу

### Dependency Inversion (Инверсия зависимостей)
- Менеджеры работают с `TableModel*`, а не с конкретной реализацией
- Возможность подмены реализации

## Точки расширения для вас

### 1. Реализация convertToSql()
```cpp
// В DataSyncManager.cpp
SqlCommand DataSyncManager::convertToSql(const DetailedCommand& cmd) const {
    // ВАША ЛОГИКА
    // Преобразование DetailedCommand в SQL запрос для вашей БД
}
```

### 2. Реализация делегатов
```cpp
// ButtonDelegate::paint() - отрисовка кнопки
// ComboBoxDelegate::createEditor() - создание редактора
```

### 3. Валидация через metadata
```cpp
TableColumn col;
col.setMetadata("validator", myValidatorFunction);
col.setMetadata("min", 0);
col.setMetadata("max", 100);
```

### 4. Фильтрация (добавьте FilterManager при необходимости)
```cpp
class FilterManager {
    void setFilter(QString columnId, QVariant value);
    QList<int> getFilteredRows();
};
```

### 5. Сериализация для сервера
```cpp
QByteArray DataSyncManager::generateSyncPacket(const QString& format) {
    // JSON, XML, Protocol Buffers - ваша реализация
}
```

## Рекомендации по использованию

1. **Порядок инициализации:**
   ```cpp
   mainTable->setColumns(columns);  // Сначала столбцы
   mainTable->setData(data);        // Потом данные
   ```

2. **Используйте уникальные ID:**
   ```cpp
   TableColumn("unique_id", "Название", ColumnType::Text);
   ```

3. **Метаданные для расширяемости:**
   ```cpp
   column.setMetadata("your_property", value);
   ```

4. **Проверяйте изменения:**
   ```cpp
   if (mainTable->hasUnsavedChanges()) {
       // Спросить пользователя
   }
   ```

5. **Используйте сигналы:**
   ```cpp
   connect(mainTable, &MainTable::dataChanged, this, &MyClass::onDataChanged);
   ```

## Дальнейшее развитие

### Возможные улучшения:

1. **Добавить FilterManager** для фильтрации строк
2. **Реализовать ValidationEngine** для сложной валидации
3. **Добавить Undo/Redo** через Command паттерн
4. **Реализовать QSortFilterProxyModel** для сортировки и фильтрации
5. **Добавить экспорт/импорт** (CSV, Excel, JSON)
6. **Реализовать виртуализацию** для больших таблиц
7. **Добавить поддержку групп столбцов**
8. **Реализовать drag & drop** для перестановки столбцов

## Примеры использования

См. файл `USAGE_EXAMPLE.md` для подробных примеров кода.

## Лицензия

Этот код создан для вашего проекта и может быть свободно использован и модифицирован.
