#pragma once

#include <QMap>
#include <QWidget>

class ExpenseChartWidget : public QWidget {
    Q_OBJECT
public:
    explicit ExpenseChartWidget(QWidget* parent = nullptr);
    void setData(const QMap<QString, double>& data);
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QMap<QString, double> data_;
};
