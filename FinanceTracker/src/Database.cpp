#include "Database.h"

#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>

Database::Database()
    : db_(QSqlDatabase::addDatabase("QSQLITE", "finance_tracker_connection")) {}

bool Database::initialize() {
    const QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (!QDir().mkpath(dataDir)) {
        lastError_ = "Не удалось создать каталог данных";
        return false;
    }
    db_.setDatabaseName(dataDir + "/finance_tracker.db");
    if (!db_.open()) {
        lastError_ = db_.lastError().text();
        return false;
    }
    QSqlQuery query(db_);
    const bool ok = query.exec(
        "CREATE TABLE IF NOT EXISTS transactions ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "type TEXT NOT NULL CHECK(type IN ('Доход', 'Расход')),"
        "category TEXT NOT NULL,"
        "description TEXT NOT NULL DEFAULT '',"
        "amount REAL NOT NULL CHECK(amount > 0),"
        "operation_date TEXT NOT NULL)"
    );
    if (!ok) lastError_ = query.lastError().text();
    return ok;
}

bool Database::addTransaction(const Transaction& transaction) {
    QSqlQuery query(db_);
    query.prepare(
        "INSERT INTO transactions(type, category, description, amount, operation_date) "
        "VALUES(?, ?, ?, ?, ?)"
    );
    query.addBindValue(transaction.type);
    query.addBindValue(transaction.category.trimmed());
    query.addBindValue(transaction.description.trimmed());
    query.addBindValue(transaction.amount);
    query.addBindValue(transaction.date.toString(Qt::ISODate));
    const bool ok = query.exec();
    if (!ok) lastError_ = query.lastError().text();
    return ok;
}

bool Database::deleteTransaction(int id) {
    QSqlQuery query(db_);
    query.prepare("DELETE FROM transactions WHERE id = ?");
    query.addBindValue(id);
    const bool ok = query.exec();
    if (!ok) lastError_ = query.lastError().text();
    return ok;
}

QVector<Transaction> Database::transactionsForMonth(const QDate& month) {
    QVector<Transaction> result;
    const QDate first(month.year(), month.month(), 1);
    const QDate next = first.addMonths(1);
    QSqlQuery query(db_);
    query.prepare(
        "SELECT id, type, category, description, amount, operation_date "
        "FROM transactions WHERE operation_date >= ? AND operation_date < ? "
        "ORDER BY operation_date DESC, id DESC"
    );
    query.addBindValue(first.toString(Qt::ISODate));
    query.addBindValue(next.toString(Qt::ISODate));
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return result;
    }
    while (query.next()) {
        result.push_back({
            query.value(0).toInt(), query.value(1).toString(), query.value(2).toString(),
            query.value(3).toString(), query.value(4).toDouble(),
            QDate::fromString(query.value(5).toString(), Qt::ISODate)
        });
    }
    return result;
}

QMap<QString, double> Database::expenseTotalsForMonth(const QDate& month) {
    QMap<QString, double> totals;
    const QDate first(month.year(), month.month(), 1);
    const QDate next = first.addMonths(1);
    QSqlQuery query(db_);
    query.prepare(
        "SELECT category, SUM(amount) FROM transactions "
        "WHERE type = 'Расход' AND operation_date >= ? AND operation_date < ? GROUP BY category"
    );
    query.addBindValue(first.toString(Qt::ISODate));
    query.addBindValue(next.toString(Qt::ISODate));
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return totals;
    }
    while (query.next()) totals.insert(query.value(0).toString(), query.value(1).toDouble());
    return totals;
}

QString Database::lastError() const { return lastError_; }
