#pragma once
#include <QTreeWidget>
#include <QMainWindow>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include "DBManager.h"
#include <vector>
#include <algorithm>
#include <QMap>
#include <memory>


struct TreeStruct;

class LTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    LTreeWidget(QString tableName,QTreeWidget *parent = nullptr, DatabaseManager *dbMan = nullptr);
    ~LTreeWidget();

    void addNodeToRoot(const QString &node);
    void addNodeToParent(const QString &node, const QString &parentId);

private:
    void iniTree(QString tableName);
    QTreeWidget *parent;
    DatabaseManager *dbMan;

    //std::vector<TreeStruct> treeNodes;
    QMap<QString, std::shared_ptr<QTreeWidgetItem>> itemMap;


};
