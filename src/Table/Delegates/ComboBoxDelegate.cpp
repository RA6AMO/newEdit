#include "Table/Delegates/ComboBoxDelegate.h"
#include <QComboBox>

namespace TableSystem {

ComboBoxDelegate::ComboBoxDelegate(const QStringList& items, QObject* parent)
    : BaseCellDelegate(parent)
    , m_items(items)
{
}

QWidget* ComboBoxDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                                       const QModelIndex& index) const
{
    // ЗАГОТОВКА - базовая реализация QComboBox
    // Вы можете улучшить: автодополнение, фильтрация, иконки и т.д.

    Q_UNUSED(option)
    Q_UNUSED(index)

    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItems(m_items);

    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    // ЗАГОТОВКА - устанавливаем текущее значение

    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;

    QString currentText = index.data(Qt::EditRole).toString();
    int currentIndex = comboBox->findText(currentText);

    if (currentIndex >= 0)
        comboBox->setCurrentIndex(currentIndex);
}

void ComboBoxDelegate::setModelData(QWidget* editor, QAbstractItemModel* model,
                                   const QModelIndex& index) const
{
    // ЗАГОТОВКА - сохраняем выбранное значение

    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;

    QString value = comboBox->currentText();
    model->setData(index, value, Qt::EditRole);
}

} // namespace TableSystem
