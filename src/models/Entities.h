#pragma once

#include <QString>
#include <QTime>
#include <QVector>
#include <QPair>

class QDataStream;

namespace Models {

struct Drug {
	quint32 id = 0;
	QString tradeName;
	QString medicalName;
	QString manufacturer;
	QString dosageForm;
	QString country;
	bool prescriptionRequired = false;

	friend QDataStream& operator<<(QDataStream &out, const Drug &d);
	friend QDataStream& operator>>(QDataStream &in, Drug &d);
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

	friend QDataStream& operator<<(QDataStream &out, const Pharmacy &p);
	friend QDataStream& operator>>(QDataStream &in, Pharmacy &p);
};

struct Stock {
	quint32 drugId = 0;
	quint32 pharmacyId = 0;
	double price = 0.0;

	friend QDataStream& operator<<(QDataStream &out, const Stock &s);
	friend QDataStream& operator>>(QDataStream &in, Stock &s);
};

} // namespace Models

