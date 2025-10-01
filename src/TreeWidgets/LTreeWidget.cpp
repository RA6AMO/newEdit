#include "LTreeWidget.h"


LTreeWidget::LTreeWidget(QString tableName,QTreeWidget *parent, DatabaseManager *dbMan) : QTreeWidget(parent), dbMan(dbMan)
{
    iniTree(tableName);
}

LTreeWidget::~LTreeWidget()
{
}

void LTreeWidget::iniTree(QString tableName)
{
    QList<QSqlRecord> records = dbMan->getDataReader()->selectAll(tableName);
    for(const auto &record : records)
    {
        QString name = record.value("name").toString();
        QString parent_id = record.value("parent_id").toString();

    }


}
