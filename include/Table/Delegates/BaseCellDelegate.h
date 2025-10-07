#pragma once
#include <QStyledItemDelegate>

namespace TableSystem {

// Базовый делегат - ЗАГОТОВКА для вашей реализации
class BaseCellDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    explicit BaseCellDelegate(QObject* parent = nullptr);
    virtual ~BaseCellDelegate() = default;

    // QStyledItemDelegate interface - можете переопределить для кастомизации
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                         const QModelIndex& index) const override;

    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model,
                     const QModelIndex& index) const override;

signals:
    void editorClosed();
};

} // namespace TableSystem
