#pragma once
#include <QDebug>
#include <QtGlobal>
#include <QObject>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QHash>
#include <QSet>
#include <QStack>
#include <QModelIndex>
#include "TableDataModel.h"
#include "MainTable.h"

class TableInteract : public QObject
{
    Q_OBJECT
public:

    TableInteract(MainTable *tableView,QObject *parent = nullptr);
    ~TableInteract();


    void ForTestCommand();

private slots:
    void determineCellType(const QModelIndex &index);

private:
    TableDataModel *tableModel;
    MainTable *tableView;




};
