#include "TableColumnCreater.h"


TableColumnCreater::TableColumnCreater(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TableColumnCreater)
{
    ui->setupUi(this);
    setupUI();

    // Подключение сигналов кнопок
    connect(ui->pushButtonAceept, &QPushButton::clicked,
            this, &TableColumnCreater::onAcceptClicked);
    connect(ui->pushButtonCancel, &QPushButton::clicked,
            this, &TableColumnCreater::onCancelClicked);

    // Валидация при изменении текста
    connect(ui->lineEditColumn, &QLineEdit::textChanged,
            this, &TableColumnCreater::validateInput);
}

TableColumnCreater::~TableColumnCreater()
{
    delete ui;
}

void TableColumnCreater::setupUI()
{
    using namespace TableSystem;

    // Заполняем ComboBox типами колонок
    ui->comboBoxColumn->addItem("Текст", static_cast<int>(ColumnType::Text));
    ui->comboBoxColumn->addItem("Число", static_cast<int>(ColumnType::Number));
    ui->comboBoxColumn->addItem("Дата", static_cast<int>(ColumnType::Date));
    ui->comboBoxColumn->addItem("Изображение", static_cast<int>(ColumnType::Image));
    ui->comboBoxColumn->addItem("Булево", static_cast<int>(ColumnType::Boolean));
    ui->comboBoxColumn->addItem("Выпадающий список", static_cast<int>(ColumnType::ComboBox));
    ui->comboBoxColumn->addItem("Кнопка", static_cast<int>(ColumnType::Button));
    ui->comboBoxColumn->addItem("Диалог файла", static_cast<int>(ColumnType::FileDialog));
    ui->comboBoxColumn->addItem("Пользовательский", static_cast<int>(ColumnType::Custom));

    // По умолчанию кнопка "Создать" неактивна
    ui->pushButtonAceept->setEnabled(false);

    // Плейсхолдер для подсказки
    ui->lineEditColumn->setPlaceholderText("Введите имя колонки");
}

void TableColumnCreater::onAcceptClicked()
{
    if (isInputValid()) {
        accept(); // Закрывает диалог с результатом QDialog::Accepted
    } else {
        QMessageBox::warning(this, "Ошибка",
            "Имя колонки должно содержать только буквы, цифры и подчеркивания, "
            "начинаться с буквы и быть длиной от 1 до 50 символов");
    }
}

void TableColumnCreater::onCancelClicked()
{
    reject(); // Закрывает диалог с результатом QDialog::Rejected
}

void TableColumnCreater::validateInput()
{
    // Активируем кнопку "Создать" только если ввод валиден
    ui->pushButtonAceept->setEnabled(isInputValid());
}

bool TableColumnCreater::isInputValid() const
{
    QString name = ui->lineEditColumn->text().trimmed();

    // Проверка длины
    if (name.length() < 1 || name.length() > 50) {
        return false;
    }

    // Проверка формата: начинается с буквы, содержит буквы, цифры, подчеркивания
    QRegularExpression regex("^[a-zA-Zа-яА-ЯёЁ][a-zA-Zа-яА-ЯёЁ0-9_]*$");
    return regex.match(name).hasMatch();
}

QString TableColumnCreater::getColumnName() const
{
    return ui->lineEditColumn->text().trimmed();
}

TableSystem::ColumnType TableColumnCreater::getColumnType() const
{
    int currentData = ui->comboBoxColumn->currentData().toInt();
    return static_cast<TableSystem::ColumnType>(currentData);
}
