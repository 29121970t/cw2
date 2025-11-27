#pragma once

#include <QTableWidget>
#include <QTime>
#include <QVector>
#include <QPair>

namespace Widgets {

class ScheduleTable : public QTableWidget {
	Q_OBJECT
public:
	explicit ScheduleTable(QWidget *parent = nullptr);

	void setHours(const QVector<QPair<QTime,QTime>> &hours);
	QVector<QPair<QTime,QTime>> hours() const;
};

} // namespace Widgets


