#include "ExpenseChartWidget.h"

#include <QPainter>
#include <algorithm>

ExpenseChartWidget::ExpenseChartWidget(QWidget* parent) : QWidget(parent) {
    setMinimumHeight(220);
}

void ExpenseChartWidget::setData(const QMap<QString, double>& data) {
    data_ = data;
    update();
}

QSize ExpenseChartWidget::minimumSizeHint() const { return {420, 220}; }

void ExpenseChartWidget::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(rect(), palette().base());
    painter.setPen(palette().text().color());
    painter.drawText(QRect(10, 5, width() - 20, 25), Qt::AlignCenter, "Расходы по категориям");

    if (data_.isEmpty()) {
        painter.drawText(rect().adjusted(20, 40, -20, -20), Qt::AlignCenter, "За выбранный месяц расходов нет");
        return;
    }

    double maxValue = 0.0;
    for (double value : data_) maxValue = std::max(maxValue, value);
    const int left = 130;
    const int top = 45;
    const int itemCount = static_cast<int>(data_.size());
    const int barHeight = std::max(18, (height() - top - 20) / itemCount - 8);
    int y = top;
    int colorIndex = 0;
    const QColor colors[] = {QColor(80, 150, 240), QColor(80, 190, 140), QColor(245, 170, 70),
                             QColor(180, 110, 220), QColor(230, 100, 120)};
    for (auto it = data_.cbegin(); it != data_.cend(); ++it) {
        painter.setPen(palette().text().color());
        painter.drawText(QRect(5, y, left - 15, barHeight), Qt::AlignRight | Qt::AlignVCenter, it.key());
        const int maxBarWidth = width() - left - 90;
        const int barWidth = static_cast<int>((it.value() / maxValue) * maxBarWidth);
        painter.setBrush(colors[colorIndex++ % 5]);
        painter.setPen(Qt::NoPen);
        painter.drawRoundedRect(QRect(left, y + 2, barWidth, barHeight - 4), 4, 4);
        painter.setPen(palette().text().color());
        painter.drawText(QRect(left + barWidth + 8, y, 80, barHeight), Qt::AlignVCenter,
                         QString::number(it.value(), 'f', 2) + " ₽");
        y += barHeight + 8;
    }
}
