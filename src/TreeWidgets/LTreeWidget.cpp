#include "LTreeWidget.h"
#include <qcontainerfwd.h>
#include <QMessageBox>

struct TreeStruct
{
    QString name;
    QString parent_id;
    QString id;
};

LTreeWidget::LTreeWidget(QString tableName,QTreeWidget *parent, DatabaseManager *dbMan) : QTreeWidget(parent), dbMan(dbMan)
{
    m_tableName = tableName;
    iniTree(tableName);
}

LTreeWidget::~LTreeWidget()
{
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

        // Разворачиваем все элементы для демонстрации
        //expandAll();
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
        if (!dbMan->getModifier()->updateColumn(m_tableName, "parent_id", parentId,
                                               QString("parent_id = %1").arg(nodeId))) {
            return false;
        }

        // Удаляем сам узел
        return dbMan->getModifier()->deleteRecordById(m_tableName, nodeId, "id");
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
