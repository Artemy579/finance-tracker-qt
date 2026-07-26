#pragma once

#include "Database.h"
#include <QMainWindow>

class ExpenseChartWidget;
class QComboBox;
class QDateEdit;
class QDateTimeEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QTableWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void addTransaction();
    void deleteSelected();
    void reload();
    void exportCsv();

private:
    void buildUi();
    int selectedId() const;

    Database database_;
    QComboBox* typeBox_ = nullptr;
    QLineEdit* categoryEdit_ = nullptr;
    QLineEdit* descriptionEdit_ = nullptr;
    QDoubleSpinBox* amountEdit_ = nullptr;
    QDateEdit* dateEdit_ = nullptr;
    QDateEdit* monthEdit_ = nullptr;
    QTableWidget* table_ = nullptr;
    QLabel* incomeLabel_ = nullptr;
    QLabel* expenseLabel_ = nullptr;
    QLabel* balanceLabel_ = nullptr;
    ExpenseChartWidget* chart_ = nullptr;
    QVector<Transaction> currentTransactions_;
};
