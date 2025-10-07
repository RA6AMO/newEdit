#include "Table/Delegates/BaseCellDelegate.h"
#include <QPainter>

namespace TableSystem {

BaseCellDelegate::BaseCellDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

void BaseCellDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    // ЗАГОТОВКА - используем стандартное отображение
    // Вы можете переопределить для кастомной отрисовки
    QStyledItemDelegate::paint(painter, option, index);
}

QWidget* BaseCellDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
    // ЗАГОТОВКА - используем стандартный редактор
    // Вы можете переопределить для кастомного редактора
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void BaseCellDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // ЗАГОТОВКА - используем стандартную установку данных
    QStyledItemDelegate::setEditorData(editor, index);
}

void BaseCellDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                   const QModelIndex& index) const
{
    // ЗАГОТОВКА - используем стандартное сохранение данных
    QStyledItemDelegate::setModelData(editor, model, index);
}

} // namespace TableSystem
