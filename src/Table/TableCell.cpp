#include "Table/TableCell.h"

namespace TableSystem {

TableCell::TableCell()
    : m_modified(false)
    , m_valid(true)
{
}

TableCell::TableCell(const QVariant& data)
    : m_data(data)
    , m_originalData(data)
    , m_modified(false)
    , m_valid(true)
{
}

QVariant TableCell::data() const
{
    return m_data;
}

void TableCell::setData(const QVariant& value)
{
    if (m_data != value) {
        m_data = value;
        m_modified = true;
    }
}

bool TableCell::isModified() const
{
    return m_modified;
}

void TableCell::setModified(bool modified)
{
    m_modified = modified;
}

QVariant TableCell::originalData() const
{
    return m_originalData;
}

void TableCell::saveOriginal()
{
    m_originalData = m_data;
    m_modified = false;
}

void TableCell::restoreOriginal()
{
    m_data = m_originalData;
    m_modified = false;
    m_valid = true;
    m_validationError.clear();
}

bool TableCell::isValid() const
{
    return m_valid;
}

void TableCell::setValid(bool valid)
{
    m_valid = valid;
}

QString TableCell::validationError() const
{
    return m_validationError;
}

void TableCell::setValidationError(const QString& error)
{
    m_validationError = error;
    m_valid = error.isEmpty();
}

QVariant TableCell::metadata(const QString& key) const
{
    return m_metadata.value(key);
}

void TableCell::setMetadata(const QString& key, const QVariant& value)
{
    m_metadata[key] = value;
}

QVariantMap TableCell::allMetadata() const
{
    return m_metadata;
}

} // namespace TableSystem
