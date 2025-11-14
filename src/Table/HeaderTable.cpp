// src/Table/HeaderTable.cpp
#include "HeaderTable.h"
#include <QDebug>

CustomHeaderView::CustomHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent), m_contextMenuIndex(-1), m_addToRight(false)
{
    // Можно настроить поведение заголовка
    setSectionsClickable(true);  // Разрешить клики
    setSectionsMovable(false);    // Запретить перемещение столбцов
    setStretchLastSection(true);  // Растянуть последний столбец

    // Создаем контекстное меню
    m_contextMenu = new QMenu(this);

    m_addAction = m_contextMenu->addAction("Добавить");
    m_deleteAction = m_contextMenu->addAction("Удалить");
    m_renameAction = m_contextMenu->addAction("Переименовать");

    // Подключаем слоты
    connect(m_addAction, &QAction::triggered, this, &CustomHeaderView::onAdd);
    connect(m_deleteAction, &QAction::triggered, this, &CustomHeaderView::onDelete);
    connect(m_renameAction, &QAction::triggered, this, &CustomHeaderView::onRename);
}

CustomHeaderView::~CustomHeaderView()
{
}

void CustomHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // logicalIndexAt() возвращает индекс столбца по позиции клика
        int logicalIndex = logicalIndexAt(event->pos());
        if (logicalIndex >= 0) {
            qDebug() << "Клик по заголовку столбца:" << logicalIndex;

            // Можно получить текст заголовка из модели
            QAbstractItemModel *model = this->model();
            if (model) {
                QVariant headerText = model->headerData(logicalIndex, orientation(), Qt::DisplayRole);
                qDebug() << "Текст заголовка:" << headerText.toString();
            }

            emit headerColumnClicked(logicalIndex);
        }
    }

    // Важно вызвать базовую реализацию для стандартного поведения
    QHeaderView::mousePressEvent(event);
}

void CustomHeaderView::contextMenuEvent(QContextMenuEvent *event)
{
    // Определяем индекс столбца, по которому кликнули
    int logicalIndex = logicalIndexAt(event->pos());

    if (logicalIndex >= 0) {
        m_contextMenuIndex = logicalIndex;

        // Определяем, в какой части столбца кликнули (левая или правая половина)
        int sectionPos = sectionPosition(logicalIndex);
        int size = sectionSize(logicalIndex);  // Переименовали переменную
        int clickX = event->pos().x();
        m_addToRight = (clickX - sectionPos) > (size / 2);  // Используем новое имя

        // Показываем контекстное меню
        m_contextMenu->exec(event->globalPos());
    }
}

void CustomHeaderView::onAdd()
{
    if (m_contextMenuIndex >= 0) {
        emit headerAddRequested(m_contextMenuIndex, m_addToRight);
    }
}

void CustomHeaderView::onDelete()
{
    if (m_contextMenuIndex >= 0) {
        emit headerDeleteRequested(m_contextMenuIndex);
    }
}

void CustomHeaderView::onRename()
{
    if (m_contextMenuIndex >= 0) {
        QAbstractItemModel *model = this->model();
        QString currentName;

        // Получаем текущее имя заголовка
        if (model) {
            QVariant headerText = model->headerData(m_contextMenuIndex, orientation(), Qt::DisplayRole);
            currentName = headerText.toString();
        }

        // Показываем диалог ввода
        bool ok;
        QString newName = QInputDialog::getText(
            this,
            "Переименовать заголовок",
            "Введите новое имя:",
            QLineEdit::Normal,
            currentName,
            &ok
        );

        if (ok && !newName.isEmpty()) {
            emit headerRenameRequested(m_contextMenuIndex, newName);
        }
    }
}
