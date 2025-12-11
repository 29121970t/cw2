#pragma once

#include <QString>
#include <QTime>
#include <QVector>
#include <QPair>
#include <QDataStream>


namespace Models {

struct Drug {
	quint32 id = 0;
	QString tradeName;
	QString medicalName;
	QString manufacturer;
	QString dosageForm;
	QString country;
	bool prescriptionRequired = false;

	friend QDataStream& operator<<(QDataStream &out, const Drug &d)
	{
		out << d.id << d.tradeName << d.medicalName << d.manufacturer
		    << d.dosageForm << d.country << d.prescriptionRequired;
		return out;
	}

	friend QDataStream& operator>>(QDataStream &in, Drug &d)
	{
		in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
		   >> d.dosageForm >> d.country >> d.prescriptionRequired;
		return in;
	}
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

	friend QDataStream& operator<<(QDataStream &out, const Pharmacy &p)
	{
		out << p.id << p.name << p.address << p.phone << p.latitude << p.longitude;
		out << quint32(p.hours.size());
		for (const auto &[open, close] : p.hours) {
			out << open << close;
		}
		return out;
	}

	friend QDataStream& operator>>(QDataStream &in, Pharmacy &p)
	{
		in >> p.id >> p.name >> p.address >> p.phone >> p.latitude >> p.longitude;
		quint32 n = 0;
		in >> n;
		p.hours.clear();
		p.hours.reserve(int(n));
		for (quint32 i = 0; i < n; ++i) {
			QTime open;
			QTime close;
			in >> open >> close;
			p.hours.push_back({open, close});
		}
		return in;
	}
};
//НАХУЙ pharmacyId НУЖЕН?
struct Stock {
	quint32 drugId = 0;
	quint32 pharmacyId = 0;
	double price = 0.0;

	friend QDataStream& operator<<(QDataStream &out, const Stock &s)
	{
		out << s.drugId << s.pharmacyId << s.price;
		return out;
	}

	friend QDataStream& operator>>(QDataStream &in, Stock &s)
	{
		in >> s.drugId >> s.pharmacyId >> s.price;
		return in;
	}
};

} // namespace Models

