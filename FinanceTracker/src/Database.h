#pragma once

#include <QDate>
#include <QMap>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

struct Transaction {
    int id = 0;
    QString type;
    QString category;
    QString description;
    double amount = 0.0;
    QDate date;
};

class Database {
public:
    Database();
    bool initialize();
    bool addTransaction(const Transaction& transaction);
    bool deleteTransaction(int id);
    QVector<Transaction> transactionsForMonth(const QDate& month);
    QMap<QString, double> expenseTotalsForMonth(const QDate& month);
    QString lastError() const;

private:
    QSqlDatabase db_;
    QString lastError_;
};
