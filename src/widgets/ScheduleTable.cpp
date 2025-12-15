#include "ScheduleTable.h"

#include <QHeaderView>
#include <QTimeEdit>

namespace Widgets {

static QString dayName(int idx) {
    static const QStringList names = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
    return names.value(idx);
}

ScheduleTable::ScheduleTable(QWidget* parent) : QTableWidget(7, 3, parent) {
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalHeaderLabels({tr("День"), tr("Открытие"), tr("Закрытие")});
    verticalHeader()->setVisible(false);
    horizontalHeader()->setStretchLastSection(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    setColumnWidth(0, 120);
    setColumnWidth(1, 95);
    setColumnWidth(2, 95);
    for (int r = 0; r < rowCount(); ++r) setRowHeight(r, 26);
    for (int i = 0; i < 7; ++i) {
        auto dayItem = new QTableWidgetItem(dayName(i));
        dayItem->setFlags(Qt::ItemIsEnabled);
        setItem(i, 0, dayItem);
        setCellWidget(i, 1, new QTimeEdit(QTime(8, 0)));
        setCellWidget(i, 2, new QTimeEdit(QTime(22, 0)));
    }
}

void ScheduleTable::setHours(const QVector<QPair<QTime, QTime>>& h) {
    for (int i = 0; i < 7 && i < h.size(); ++i) {
        auto* const e1 = qobject_cast<QTimeEdit*>(cellWidget(i, 1));
        auto* const e2 = qobject_cast<QTimeEdit*>(cellWidget(i, 2));
        if (e1 && e2) {
            if (h[i].first.isValid()) e1->setTime(h[i].first);
            if (h[i].second.isValid()) e2->setTime(h[i].second);
        }
    }
}

QVector<QPair<QTime, QTime>> ScheduleTable::hours() const {
    QVector<QPair<QTime, QTime>> res(7);
    for (int i = 0; i < 7; ++i) {
        const auto* e1 = qobject_cast<QTimeEdit*>(cellWidget(i, 1));
        const auto* e2 = qobject_cast<QTimeEdit*>(cellWidget(i, 2));
        res[i] = {e1 ? e1->time() : QTime(), e2 ? e2->time() : QTime()};
    }
    return res;
}

}  // namespace Widgets
