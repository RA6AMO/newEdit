#pragma once
#include <QObject>
#include <QVariant>
#include <QList>
#include <QMap>
#include <QVector>
#include <QPair>
#include <QHash>
#include <QSet>
#include <QStack>
#include "TableDataModel.h"
#include "MainTable.h"

class TableInteract : public QObject
{
    Q_OBJECT
public:

    TableInteract(QObject *parent = nullptr);
    ~TableInteract();

private:
    TableDataModel *tableModel;
    MainTable *tableView;

};
