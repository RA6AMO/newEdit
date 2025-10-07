#pragma once
#include <QVariant>
#include <QString>
#include <QMap>

namespace TableSystem {

class TableCell {
public:
    TableCell();
    explicit TableCell(const QVariant& data);

    // Данные
    QVariant data() const;
    void setData(const QVariant& value);

    // Состояние изменений
    bool isModified() const;
    void setModified(bool modified);

    QVariant originalData() const;  // Для отката изменений
    void saveOriginal();            // Сохранить текущее значение как оригинальное
    void restoreOriginal();         // Восстановить из оригинала

    // Валидация
    bool isValid() const;
    void setValid(bool valid);

    QString validationError() const;
    void setValidationError(const QString& error);

    // Метаданные ячейки
    QVariant metadata(const QString& key) const;
    void setMetadata(const QString& key, const QVariant& value);
    QVariantMap allMetadata() const;

private:
    QVariant m_data;
    QVariant m_originalData;
    bool m_modified;
    bool m_valid;
    QString m_validationError;
    QVariantMap m_metadata;
};

} // namespace TableSystem
