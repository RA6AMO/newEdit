#include "Table/Delegates/ButtonDelegate.h"
#include <QPainter>
#include <QApplication>
#include <QStyleOptionButton>
#include <QMouseEvent>

namespace TableSystem {

ButtonDelegate::ButtonDelegate(const QString& buttonText, QObject* parent)
    : BaseCellDelegate(parent)
    , m_buttonText(buttonText)
{
}

void ButtonDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                          const QModelIndex& index) const
{
    // ЗАГОТОВКА - минимальная реализация
    // Вы можете улучшить: отрисовка полноценной кнопки с иконками, стилями и т.д.

    QStyleOptionButton buttonOption;
    buttonOption.rect = option.rect;
    buttonOption.text = m_buttonText;
    buttonOption.state = QStyle::State_Enabled;

    if (option.state & QStyle::State_Selected)
        buttonOption.state |= QStyle::State_Sunken;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter);
}

bool ButtonDelegate::editorEvent(QEvent* event, QAbstractItemModel* model,
                                const QStyleOptionViewItem& option,
                                const QModelIndex& index)
{
    // ЗАГОТОВКА - простая обработка клика
    // Вы можете улучшить: визуальная обратная связь, долгое нажатие и т.д.

    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

        if (option.rect.contains(mouseEvent->pos())) {
            emit buttonClicked(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

} // namespace TableSystem
