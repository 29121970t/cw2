#pragma once

#include <QString>
#include <QTime>
#include <QVector>
#include <QPair>

namespace Models {

struct Drug {
	quint32 id = 0;
	QString tradeName;
	QString medicalName;
	QString manufacturer;
	QString dosageForm;
	QString country;
	bool prescriptionRequired = false;
};

struct Pharmacy {
	quint32 id = 0;
	QString name;
	QString address;
	QString phone;
	double latitude = 0.0;
	double longitude = 0.0;
	// 7 entries, Mon..Sun (Qt: Monday=1)
	QVector<QPair<QTime,QTime>> hours; // invalid times -> closed
};

struct Stock {
	quint32 drugId = 0;
	quint32 pharmacyId = 0;
	double price = 0.0;
};

} // namespace Models

