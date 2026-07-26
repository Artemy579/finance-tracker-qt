#include "MainWindow.h"
#include "ExpenseChartWidget.h"

#include <QComboBox>
#include <QDate>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QStringConverter>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    buildUi();
    if (!database_.initialize()) {
        QMessageBox::critical(this, "Ошибка базы данных", database_.lastError());
    }
    reload();
}

void MainWindow::buildUi() {
    setWindowTitle("Finance Tracker");
    resize(1050, 720);
    auto* central = new QWidget(this);
    auto* root = new QVBoxLayout(central);

    auto* form = new QFormLayout;
    typeBox_ = new QComboBox;
    typeBox_->addItems({"Расход", "Доход"});
    categoryEdit_ = new QLineEdit;
    categoryEdit_->setPlaceholderText("Продукты, транспорт, зарплата...");
    descriptionEdit_ = new QLineEdit;
    amountEdit_ = new QDoubleSpinBox;
    amountEdit_->setRange(0.01, 1000000000.0);
    amountEdit_->setDecimals(2);
    amountEdit_->setSuffix(" ₽");
    dateEdit_ = new QDateEdit(QDate::currentDate());
    dateEdit_->setCalendarPopup(true);
    form->addRow("Тип", typeBox_);
    form->addRow("Категория", categoryEdit_);
    form->addRow("Описание", descriptionEdit_);
    form->addRow("Сумма", amountEdit_);
    form->addRow("Дата", dateEdit_);
    root->addLayout(form);

    auto* addButton = new QPushButton("Добавить операцию");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTransaction);
    root->addWidget(addButton);

    auto* monthRow = new QHBoxLayout;
    monthRow->addWidget(new QLabel("Месяц:"));
    monthEdit_ = new QDateEdit(QDate(QDate::currentDate().year(), QDate::currentDate().month(), 1));
    monthEdit_->setDisplayFormat("MMMM yyyy");
    monthEdit_->setCalendarPopup(true);
    monthRow->addWidget(monthEdit_);
    monthRow->addStretch();
    root->addLayout(monthRow);
    connect(monthEdit_, &QDateEdit::dateChanged, this, &MainWindow::reload);

    auto* summary = new QHBoxLayout;
    incomeLabel_ = new QLabel;
    expenseLabel_ = new QLabel;
    balanceLabel_ = new QLabel;
    summary->addWidget(incomeLabel_);
    summary->addWidget(expenseLabel_);
    summary->addWidget(balanceLabel_);
    summary->addStretch();
    root->addLayout(summary);

    table_ = new QTableWidget(0, 6);
    table_->setHorizontalHeaderLabels({"ID", "Тип", "Категория", "Описание", "Сумма", "Дата"});
    table_->setColumnHidden(0, true);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    root->addWidget(table_, 1);

    chart_ = new ExpenseChartWidget;
    root->addWidget(chart_);

    auto* actions = new QHBoxLayout;
    auto* deleteButton = new QPushButton("Удалить выбранную");
    auto* exportButton = new QPushButton("Экспорт CSV");
    actions->addWidget(deleteButton);
    actions->addStretch();
    actions->addWidget(exportButton);
    root->addLayout(actions);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteSelected);
    connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportCsv);

    setCentralWidget(central);
}

void MainWindow::addTransaction() {
    if (categoryEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Проверка", "Введите категорию.");
        return;
    }
    Transaction transaction;
    transaction.type = typeBox_->currentText();
    transaction.category = categoryEdit_->text();
    transaction.description = descriptionEdit_->text();
    transaction.amount = amountEdit_->value();
    transaction.date = dateEdit_->date();
    if (!database_.addTransaction(transaction)) {
        QMessageBox::critical(this, "Ошибка", database_.lastError());
        return;
    }
    categoryEdit_->clear();
    descriptionEdit_->clear();
    amountEdit_->setValue(0.01);
    reload();
}

int MainWindow::selectedId() const {
    const int row = table_->currentRow();
    return row < 0 ? -1 : table_->item(row, 0)->text().toInt();
}

void MainWindow::deleteSelected() {
    const int id = selectedId();
    if (id < 0) {
        QMessageBox::information(this, "Выбор", "Выберите операцию.");
        return;
    }
    if (QMessageBox::question(this, "Удаление", "Удалить выбранную операцию?") != QMessageBox::Yes) return;
    if (!database_.deleteTransaction(id)) QMessageBox::critical(this, "Ошибка", database_.lastError());
    reload();
}

void MainWindow::reload() {
    currentTransactions_ = database_.transactionsForMonth(monthEdit_->date());
    table_->setRowCount(0);
    double income = 0.0;
    double expense = 0.0;
    for (const auto& transaction : currentTransactions_) {
        const int row = table_->rowCount();
        table_->insertRow(row);
        const QString values[] = {
            QString::number(transaction.id), transaction.type, transaction.category,
            transaction.description, QString::number(transaction.amount, 'f', 2) + " ₽",
            transaction.date.toString("dd.MM.yyyy")
        };
        for (int column = 0; column < 6; ++column) {
            table_->setItem(row, column, new QTableWidgetItem(values[column]));
        }
        if (transaction.type == "Доход") income += transaction.amount;
        else expense += transaction.amount;
    }
    incomeLabel_->setText("Доходы: " + QString::number(income, 'f', 2) + " ₽");
    expenseLabel_->setText("Расходы: " + QString::number(expense, 'f', 2) + " ₽");
    balanceLabel_->setText("Баланс: " + QString::number(income - expense, 'f', 2) + " ₽");
    chart_->setData(database_.expenseTotalsForMonth(monthEdit_->date()));
}

void MainWindow::exportCsv() {
    const QString filename = QFileDialog::getSaveFileName(this, "Сохранить отчёт", "finance.csv", "CSV (*.csv)");
    if (filename.isEmpty()) return;
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл.");
        return;
    }
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "ID,Тип,Категория,Описание,Сумма,Дата\n";
    for (const auto& transaction : currentTransactions_) {
        QString description = transaction.description;
        description.replace('"', "\"\"");
        out << transaction.id << ",\"" << transaction.type << "\",\"" << transaction.category
            << "\",\"" << description << "\"," << QString::number(transaction.amount, 'f', 2)
            << ',' << transaction.date.toString(Qt::ISODate) << '\n';
    }
}
