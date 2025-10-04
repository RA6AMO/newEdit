#include "LTreeWidget.h"
#include <qcontainerfwd.h>
#include <QMessageBox>

struct TreeStruct
{
    QString name;
    QString parent_id;
    QString id;
};
LTreeWidget::LTreeWidget(QWidget *parent) : QTreeWidget(parent), dbMan(nullptr)
{
    setupContextMenu();
    // Не инициализируем дерево, так как нет имени таблицы и менеджера БД
}

LTreeWidget::LTreeWidget(QString tableName,QWidget *parent, DatabaseManager *dbMan) : QTreeWidget(parent), dbMan(dbMan)
{
    m_tableName = tableName;
    setupContextMenu();
    iniTree(tableName);

    // Подключаем сигналы
    connect(this, &QTreeWidget::itemClicked, this, &LTreeWidget::onItemClicked);
    connect(this, &QTreeWidget::itemDoubleClicked, this, &LTreeWidget::onItemDoubleClicked);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &LTreeWidget::showContextMenu);

    // Включаем контекстное меню
    setContextMenuPolicy(Qt::CustomContextMenu);
}

LTreeWidget::~LTreeWidget()
{
}

void LTreeWidget::setupContextMenu()
{
    contextMenu = new QMenu(this);

    addChildAction = contextMenu->addAction("Добавить дочерний элемент");
    deleteAction = contextMenu->addAction("Удалить");
    renameAction = contextMenu->addAction("Переименовать");
    addRootAction = contextMenu->addAction("Добавить корневой элемент");

    connect(addChildAction, &QAction::triggered, this, &LTreeWidget::onAddChild);
    connect(deleteAction, &QAction::triggered, this, &LTreeWidget::onDeleteNode);
    connect(renameAction, &QAction::triggered, this, &LTreeWidget::onRenameNode);
    connect(addRootAction, &QAction::triggered, this, &LTreeWidget::addNodeToRoot);
}

void LTreeWidget::iniTree(QString tableName)
{
    QList<QSqlRecord> records = dbMan->getReader()->selectAll(tableName);
    std::vector<TreeStruct> treeNodes;
    for(const auto &record : records) {
        TreeStruct tmp;
        tmp.name = record.value("name").toString();
        tmp.parent_id = record.value("parent_id").toString();
        tmp.id = record.value("id").toString();
        treeNodes.push_back(tmp);
    }

    std::sort(treeNodes.begin(), treeNodes.end(), [](const TreeStruct &a, const TreeStruct &b) {
        return a.parent_id < b.parent_id;
    });
        // Сначала создаем все элементы
        for(const auto &node : treeNodes) {
            std::shared_ptr<QTreeWidgetItem> item = std::make_shared<QTreeWidgetItem>();
            item->setText(0, node.name);
            item->setData(0, Qt::UserRole, node.parent_id); // Сохраняем ID в данных элемента
            itemMap[node.id] = item;
        }

        // Затем строим иерархию
        for(const auto &node : treeNodes) {
            if(node.parent_id.isEmpty() || node.parent_id == "0" || node.parent_id == "NULL"){
                // Корневой элемент
                addTopLevelItem(itemMap[node.id].get());
            }
            else
            {
                // Дочерний элемент
                if(itemMap.contains(node.parent_id)) {
                    itemMap[node.parent_id]->addChild(itemMap[node.id].get());
                }
            }
        }

        // Включаем стрелочки для разворачивания/сворачивания
        setRootIsDecorated(true);
}

void LTreeWidget::onItemClicked(QTreeWidgetItem *item, int column)
{
    if (!item) return;

    // Находим ID узла по элементу
    QString nodeId;
    for (auto it = itemMap.begin(); it != itemMap.end(); ++it) {
        if (it.value().get() == item) {
            nodeId = it.key();
            break;
        }
    }

    if (!nodeId.isEmpty()) {
        currentSelectedNodeId = nodeId;
        emit itemClicked(nodeId, item->text(0));
    }
}

void LTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    if (!item) return;

    // Находим ID узла по элементу
    QString nodeId;
    for (auto it = itemMap.begin(); it != itemMap.end(); ++it) {
        if (it.value().get() == item) {
            nodeId = it.key();
            break;
        }
    }

    if (!nodeId.isEmpty()) {
        emit itemDoubleClicked(nodeId, item->text(0));
    }
}

void LTreeWidget::showContextMenu(const QPoint &pos)
{
    QTreeWidgetItem *item = itemAt(pos);

    if (item) {
        // Клик по элементу - показываем меню для элемента
        currentSelectedNodeId.clear();
        for (auto it = itemMap.begin(); it != itemMap.end(); ++it) {
            if (it.value().get() == item) {
                currentSelectedNodeId = it.key();
                break;
            }
        }

        // Показываем все действия для элемента
        addChildAction->setVisible(true);
        deleteAction->setVisible(true);
        renameAction->setVisible(true);
        addRootAction->setVisible(false);
    } else {
        // Клик по пустому месту - показываем только добавление корневого элемента
        currentSelectedNodeId.clear();
        addChildAction->setVisible(false);
        deleteAction->setVisible(false);
        renameAction->setVisible(false);
        addRootAction->setVisible(true);
        // Добавляем действие для корневого элемента
        //QAction *addRootAction = contextMenu->addAction("Добавить корневой элемент");
        //connect(addRootAction, &QAction::triggered, this, &LTreeWidget::addNodeToRoot);
    }

    contextMenu->exec(mapToGlobal(pos));
}

void LTreeWidget::onAddChild()
{
    if (!currentSelectedNodeId.isEmpty()) {
        addNodeToParent(currentSelectedNodeId);
    }
}

void LTreeWidget::onDeleteNode()
{
    if (!currentSelectedNodeId.isEmpty()) {
        deleteNode(currentSelectedNodeId);
    }
}

void LTreeWidget::onRenameNode()
{
    if (!currentSelectedNodeId.isEmpty()) {
        renameNode(currentSelectedNodeId);
    }
}

void LTreeWidget::addNodeToRoot()
{
    QString name = QInputDialog::getText(this, tr("Новый узел"), tr("Имя:"));
    if (name.isEmpty()) return;

    QVariantMap values;
    values["name"] = name;
    values["parent_id"] = 0;

    qint64 id = dbMan->getModifier()->insertRecordAndReturnId(m_tableName, values, "id");
    if (id < 0) {
        QMessageBox::warning(this, tr("Ошибка"), dbMan->getModifier()->getLastError());
        return;
    }

    auto item = std::make_shared<QTreeWidgetItem>();
    item->setText(0, name);
    item->setData(0, Qt::UserRole, QString::number(0));
    itemMap[QString::number(id)] = item;
    addTopLevelItem(item.get());
}

void LTreeWidget::addNodeToParent(const QString &parentId)
{
    if (!itemMap.contains(parentId)) return;

    QString name = QInputDialog::getText(this, tr("Новый узел"), tr("Имя:"));
    if (name.isEmpty()) return;

    QVariantMap values;
    values["name"] = name;
    values["parent_id"] = parentId;

    qint64 id = dbMan->getModifier()->insertRecordAndReturnId(m_tableName, values, "id");
    if (id < 0) {
        QMessageBox::warning(this, tr("Ошибка"), dbMan->getModifier()->getLastError());
        return;
    }

    auto item = std::make_shared<QTreeWidgetItem>();
    item->setText(0, name);
    item->setData(0, Qt::UserRole, parentId);
    itemMap[QString::number(id)] = item;
    itemMap[parentId]->addChild(item.get());
}

void LTreeWidget::deleteNode(const QString &nodeId)
{
    if (!itemMap.contains(nodeId)) return;

    QString nodeName = itemMap[nodeId]->text(0);
    if (QMessageBox::question(this, tr("Удалить"), tr("Удалить '%1'?").arg(nodeName),
                              QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) {
        return;
    }

    // Получаем родителя удаляемого узла
    QString parentId = itemMap[nodeId]->data(0, Qt::UserRole).toString();

    // В транзакции: перепривязываем детей к родителю удаляемого узла, затем удаляем узел
    bool ok = dbMan->getModifier()->executeInTransaction([&]{
        // Перепривязываем всех детей к родителю удаляемого узла

        int affected = dbMan->getModifier()->updateColumn(m_tableName, "parent_id", parentId,QString("parent_id = %1").arg(nodeId));
        qDebug() << "affected" << affected;
        if (affected < 0) {
            return false;
        }

        // Удаляем сам узел
        affected = dbMan->getModifier()->deleteRecordById(m_tableName, nodeId, "id");
        if (affected < 0) {
            qDebug() << "affected" << affected;
            return false;
        }
        return true;
    });

    if (!ok) {
        QMessageBox::warning(this, tr("Ошибка"), dbMan->getModifier()->getLastError());
        return;
    }

    // Обновляем UI: перепривязываем детей в дереве
    QList<QTreeWidgetItem*> childrenToMove;
    for (auto it = itemMap.begin(); it != itemMap.end(); ++it) {
        if (it.value()->data(0, Qt::UserRole).toString() == nodeId) {
            childrenToMove.append(it.value().get());
        }
    }

    for (QTreeWidgetItem* child : childrenToMove) {
        // Удаляем из текущего родителя
        if (child->parent()) {
            child->parent()->removeChild(child);
        } else {
            takeTopLevelItem(indexOfTopLevelItem(child));
        }

        // Перепривязываем к новому родителю
        if (parentId == "0") {
            addTopLevelItem(child);
        } else if (itemMap.contains(parentId)) {
            itemMap[parentId]->addChild(child);
        }

        // Обновляем данные
        child->setData(0, Qt::UserRole, parentId);
    }

    // Удаляем сам узел из UI
    if (parentId == "0") {
        takeTopLevelItem(indexOfTopLevelItem(itemMap[nodeId].get()));
    } else if (itemMap.contains(parentId)) {
        itemMap[parentId]->removeChild(itemMap[nodeId].get());
    }

    // Удаляем из карты
    itemMap.remove(nodeId);
}

void LTreeWidget::renameNode(const QString &nodeId)
{
    if (!itemMap.contains(nodeId)) return;

    QString current = itemMap[nodeId]->text(0);
    bool ok;
    QString name = QInputDialog::getText(this, tr("Переименовать"), tr("Имя:"),
                                         QLineEdit::Normal, current, &ok);
    if (!ok || name.isEmpty() || name == current) return;

    if (!dbMan->getModifier()->updateRecordById(m_tableName, nodeId, {{"name", name}}, "id")) {
        QMessageBox::warning(this, tr("Ошибка"), dbMan->getModifier()->getLastError());
        return;
    }

    itemMap[nodeId]->setText(0, name);
}
