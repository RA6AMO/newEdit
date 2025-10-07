#pragma once
#include "BaseCellDelegate.h"
#include <QString>

namespace TableSystem {

// Делегат для отображения кнопки в ячейке - ЗАГОТОВКА
class ButtonDelegate : public BaseCellDelegate {
    Q_OBJECT

public:
    explicit ButtonDelegate(const QString& buttonText, QObject* parent = nullptr);

    // ЗАГОТОВКА - для полной реализации переопределите эти методы
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model,
                    const QStyleOptionViewItem& option,
                    const QModelIndex& index) override;

signals:
    void buttonClicked(const QModelIndex& index);

private:
    QString m_buttonText;
};

} // namespace TableSystem
