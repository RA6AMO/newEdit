#pragma once
#include "TableTypes.h"
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

namespace TableSystem {

class TableColumn {
public:
    // Конструкторы
    TableColumn();
    TableColumn(const QString& id, const QString& name, ColumnType type);

    // Основные свойства
    QString id() const;
    void setId(const QString& id);

    QString name() const;
    void setName(const QString& name);

    ColumnType type() const;
    void setType(ColumnType type);

    bool isEditable() const;
    void setEditable(bool editable);

    bool isVisible() const;
    void setVisible(bool visible);

    QVariant defaultValue() const;
    void setDefaultValue(const QVariant& value);

    // Для ComboBox
    QStringList comboBoxItems() const;
    void setComboBoxItems(const QStringList& items);

    // Для Button
    QString buttonText() const;
    void setButtonText(const QString& text);

    // Для FileDialog
    QString fileFilter() const;
    void setFileFilter(const QString& filter);

    // Метаданные (валидация, форматирование и т.д.)
    QVariant metadata(const QString& key) const;
    void setMetadata(const QString& key, const QVariant& value);
    QVariantMap allMetadata() const;

    // Сериализация
    QVariantMap toMap() const;
    static TableColumn fromMap(const QVariantMap& map);

private:
    QString m_id;
    QString m_name;
    ColumnType m_type;
    bool m_editable;
    bool m_visible;
    QVariant m_defaultValue;
    QStringList m_comboBoxItems;
    QString m_buttonText;
    QString m_fileFilter;
    QVariantMap m_metadata;
};

} // namespace TableSystem
