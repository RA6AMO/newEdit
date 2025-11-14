#pragma once
#include <QHeaderView>
#include <QMouseEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QInputDialog>

class CustomHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    CustomHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);
    ~CustomHeaderView();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void headerColumnClicked(int logicalIndex);
    // Сигналы для контекстного меню
    void headerAddRequested(int logicalIndex, bool addToRight);  // индекс и направление (false=слева, true=справа)
    void headerDeleteRequested(int logicalIndex);  // индекс столбца
    void headerRenameRequested(int logicalIndex, const QString &newName);  // индекс и новое имя

private slots:
    void onAdd();
    void onDelete();
    void onRename();

private:
    int m_contextMenuIndex;  // индекс столбца, по которому вызвали меню
    bool m_addToRight;  // направление добавления (false=слева, true=справа)
    QMenu *m_contextMenu;
    QAction *m_addAction;
    QAction *m_deleteAction;
    QAction *m_renameAction;
};
