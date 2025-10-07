#pragma once
#include "BaseCellDelegate.h"
#include <QStringList>

namespace TableSystem {

// Делегат для выпадающего списка - ЗАГОТОВКА
class ComboBoxDelegate : public BaseCellDelegate {
    Q_OBJECT

public:
    explicit ComboBoxDelegate(const QStringList& items, QObject* parent = nullptr);

    // ЗАГОТОВКА - для полной реализации переопределите эти методы
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                     const QModelIndex& index) const override;

private:
    QStringList m_items;
};

} // namespace TableSystem
