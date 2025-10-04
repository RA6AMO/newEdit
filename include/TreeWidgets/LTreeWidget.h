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
    explicit LTreeWidget(QWidget *parent = nullptr);
    LTreeWidget(QString tableName, QWidget *parent = nullptr, DatabaseManager *dbMan = nullptr);
    ~LTreeWidget();

    void configure(const QString &tableName, DatabaseManager *dbMan);

    void addNodeToRoot();
    void addNodeToParent(const QString &parentId);
    void deleteNode(const QString &nodeId);
    void renameNode(const QString &nodeId);

signals:
    void itemClicked(const QString &nodeId, const QString &nodeName);
    void itemDoubleClicked(const QString &nodeId, const QString &nodeName);

private slots:
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onItemDoubleClicked(QTreeWidgetItem *item, int column);
    void showContextMenu(const QPoint &pos);
    void onAddChild();
    void onDeleteNode();
    void onRenameNode();

private:
    void iniTree(QString tableName);
    void setupContextMenu();
    QString m_tableName;
    DatabaseManager *dbMan;

    //std::vector<TreeStruct> treeNodes;
    QMap<QString, std::shared_ptr<QTreeWidgetItem>> itemMap;
    QMenu *contextMenu;
    QAction *addChildAction;
    QAction *deleteAction;
    QAction *renameAction;
    QString currentSelectedNodeId;

    // Helper methods
    bool isRoot(const QString &nodeId);
    void attachToParent(const QString &childId, const QString &parentId);


};
