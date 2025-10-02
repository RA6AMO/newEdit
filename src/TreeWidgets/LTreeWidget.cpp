#include "LTreeWidget.h"
#include <qcontainerfwd.h>

struct TreeStruct
{
    QString name;
    QString parent_id;
    QString id;
};

LTreeWidget::LTreeWidget(QString tableName,QTreeWidget *parent, DatabaseManager *dbMan) : QTreeWidget(parent), dbMan(dbMan)
{
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
        // Создаем карту для быстрого поиска элементов по ID
        QMap<QString, std::shared_ptr<QTreeWidgetItem>> itemMap;

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

void LTreeWidget::addNodeToRoot(const QString &node)
{
    TreeStruct tmp;
    tmp.name = node;
    tmp.parent_id = "0";
    tmp.id = QString::number(itemMap.size() + 1);
    itemMap[tmp.id] = std::make_shared<QTreeWidgetItem>();
    itemMap[tmp.id]->setText(0, tmp.name);
    itemMap[tmp.id]->setData(0, Qt::UserRole, tmp.parent_id);
    addTopLevelItem(itemMap[tmp.id].get());
    //expandAll();
}

void LTreeWidget::addNodeToParent(const QString &node, const QString &parentId)
{
    TreeStruct tmp;
    tmp.name = node;
    tmp.parent_id = parentId;
    tmp.id = QString::number(itemMap.size() + 1);
    itemMap[tmp.id] = std::make_shared<QTreeWidgetItem>();
    itemMap[tmp.id]->setText(0, tmp.name);
    itemMap[tmp.id]->setData(0, Qt::UserRole, tmp.parent_id);
    itemMap[parentId]->addChild(itemMap[tmp.id].get());
    //expandAll();
}

void LTreeWidget::deleteNode(const QString &nodeId)
{
    QMap<QString, std::shared_ptr<QTreeWidgetItem>> rebaseItemMap;
    for(const auto &item : itemMap) {
        if(item.second->data(0, Qt::UserRole).toString() == nodeId) {
            rebaseItemMap[item.first] = std::move(item.second);
        }
    }
    if(itemMap[nodeId]->data(0, Qt::UserRole).toString() == "0") {
        removeItem(itemMap[nodeId].get());
    }
    else {
        itemMap[itemMap[nodeId]->data(0, Qt::UserRole).toString()]->removeChild(itemMap[nodeId].get());
    }
    itemMap.remove(nodeId);
    //expandAll();
}
