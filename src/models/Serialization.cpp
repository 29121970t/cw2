#include "Serialization.h"
#include "Entities.h"

namespace Models {

QDataStream& operator<<(QDataStream& out, const Models::Drug& d)
{
	out << d.id << d.tradeName << d.medicalName << d.manufacturer
	    << d.dosageForm << d.country << d.prescriptionRequired;
	return out;
}

QDataStream& operator>>(QDataStream& in, Models::Drug& d)
{
	in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
	   >> d.dosageForm >> d.country >> d.prescriptionRequired;
	return in;
}

QDataStream& operator<<(QDataStream& out, const Models::Pharmacy& p)
{
	out << p.id << p.name << p.address << p.phone << p.latitude << p.longitude;
	out << quint32(p.hours.size());
	for (const auto& pr : p.hours) {
		out << pr.first << pr.second;
	}
	return out;
}

QDataStream& operator>>(QDataStream& in, Models::Pharmacy& p)
{
	in >> p.id >> p.name >> p.address >> p.phone >> p.latitude >> p.longitude;
	quint32 n = 0;
	in >> n;
	p.hours.clear();
	p.hours.reserve(int(n));
	for (quint32 i=0;i<n;++i) {
		QTime a,b;
		in >> a >> b;
		p.hours.push_back({a,b});
	}
	return in;
}

QDataStream& operator<<(QDataStream& out, const Models::Stock& s)
{
	out << s.drugId << s.pharmacyId << s.price;
	return out;
}

QDataStream& operator>>(QDataStream& in, Models::Stock& s)
{
	in >> s.drugId >> s.pharmacyId >> s.price;
	return in;
}

} // namespace Models

