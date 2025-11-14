#include "MainTable.h"

MainTable::MainTable(QWidget *parent) : QTableView(parent)
{
    m_customHeader = new CustomHeaderView(Qt::Horizontal, this);
    setHorizontalHeader(m_customHeader);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
}

MainTable::~MainTable()
{

}
