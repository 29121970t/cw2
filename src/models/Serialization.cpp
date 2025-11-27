#include "Serialization.h"
#include "Entities.h"

namespace Models {

QDataStream& operator<<(QDataStream& out, const Drug& d)
{
	out << d.id << d.tradeName << d.medicalName << d.manufacturer
	    << d.dosageForm << d.country << d.prescriptionRequired;
	return out;
}

QDataStream& operator>>(QDataStream& in, Drug& d)
{
	in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
	   >> d.dosageForm >> d.country >> d.prescriptionRequired;
	return in;
}

QDataStream& operator<<(QDataStream& out, const Pharmacy& p)
{
	out << p.id << p.name << p.address << p.phone << p.latitude << p.longitude;
	out << quint32(p.hours.size());
	for (const auto& [open, close] : p.hours) {
		out << open << close;
	}
	return out;
}

QDataStream& operator>>(QDataStream& in, Pharmacy& p)
{
	in >> p.id >> p.name >> p.address >> p.phone >> p.latitude >> p.longitude;
	quint32 n = 0;
	in >> n;
	p.hours.clear();
	p.hours.reserve(int(n));
	for (quint32 i=0;i<n;++i) {
		QTime open;
		QTime close;
		in >> open >> close;
		p.hours.push_back({open, close});
	}
	return in;
}

QDataStream& operator<<(QDataStream& out, const Stock& s)
{
	out << s.drugId << s.pharmacyId << s.price;
	return out;
}

QDataStream& operator>>(QDataStream& in, Stock& s)
{
	in >> s.drugId >> s.pharmacyId >> s.price;
	return in;
}

} // namespace Models

