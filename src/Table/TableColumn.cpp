#include "Table/TableColumn.h"

namespace TableSystem {

TableColumn::TableColumn()
    : m_type(ColumnType::Text)
    , m_editable(true)
    , m_visible(true)
{
}

TableColumn::TableColumn(const QString& id, const QString& name, ColumnType type)
    : m_id(id)
    , m_name(name)
    , m_type(type)
    , m_editable(true)
    , m_visible(true)
{
}

QString TableColumn::id() const
{
    return m_id;
}

void TableColumn::setId(const QString& id)
{
    m_id = id;
}

QString TableColumn::name() const
{
    return m_name;
}

void TableColumn::setName(const QString& name)
{
    m_name = name;
}

ColumnType TableColumn::type() const
{
    return m_type;
}

void TableColumn::setType(ColumnType type)
{
    m_type = type;
}

bool TableColumn::isEditable() const
{
    return m_editable;
}

void TableColumn::setEditable(bool editable)
{
    m_editable = editable;
}

bool TableColumn::isVisible() const
{
    return m_visible;
}

void TableColumn::setVisible(bool visible)
{
    m_visible = visible;
}

QVariant TableColumn::defaultValue() const
{
    return m_defaultValue;
}

void TableColumn::setDefaultValue(const QVariant& value)
{
    m_defaultValue = value;
}

QStringList TableColumn::comboBoxItems() const
{
    return m_comboBoxItems;
}

void TableColumn::setComboBoxItems(const QStringList& items)
{
    m_comboBoxItems = items;
}

QString TableColumn::buttonText() const
{
    return m_buttonText;
}

void TableColumn::setButtonText(const QString& text)
{
    m_buttonText = text;
}

QString TableColumn::fileFilter() const
{
    return m_fileFilter;
}

void TableColumn::setFileFilter(const QString& filter)
{
    m_fileFilter = filter;
}

QVariant TableColumn::metadata(const QString& key) const
{
    return m_metadata.value(key);
}

void TableColumn::setMetadata(const QString& key, const QVariant& value)
{
    m_metadata[key] = value;
}

QVariantMap TableColumn::allMetadata() const
{
    return m_metadata;
}

QVariantMap TableColumn::toMap() const
{
    QVariantMap map;
    map["id"] = m_id;
    map["name"] = m_name;
    map["type"] = static_cast<int>(m_type);
    map["editable"] = m_editable;
    map["visible"] = m_visible;
    map["defaultValue"] = m_defaultValue;
    map["comboBoxItems"] = m_comboBoxItems;
    map["buttonText"] = m_buttonText;
    map["fileFilter"] = m_fileFilter;
    map["metadata"] = m_metadata;
    return map;
}

TableColumn TableColumn::fromMap(const QVariantMap& map)
{
    TableColumn column;
    column.setId(map.value("id").toString());
    column.setName(map.value("name").toString());
    column.setType(static_cast<ColumnType>(map.value("type").toInt()));
    column.setEditable(map.value("editable", true).toBool());
    column.setVisible(map.value("visible", true).toBool());
    column.setDefaultValue(map.value("defaultValue"));
    column.setComboBoxItems(map.value("comboBoxItems").toStringList());
    column.setButtonText(map.value("buttonText").toString());
    column.setFileFilter(map.value("fileFilter").toString());

    QVariantMap metadata = map.value("metadata").toMap();
    for (auto it = metadata.begin(); it != metadata.end(); ++it) {
        column.setMetadata(it.key(), it.value());
    }

    return column;
}

} // namespace TableSystem
