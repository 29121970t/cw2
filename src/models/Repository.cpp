#include "Repository.h"
#include "Serialization.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QtAlgorithms>
#include <ranges>

static const quint32 DATA_VERSION = 2;

using namespace Models;

Repository::Repository() = default;

QString Repository::dataFilePath() const
{
	const auto base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
	QDir().mkpath(base);
	return base + QDir::separator() + "drug_system.bin";
}

bool Repository::load()
{
	QFile f(dataFilePath());
	if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
		return false;
	}
	QDataStream in(&f);
	in.setVersion(QDataStream::Qt_6_0);
	quint32 version = 0;
	in >> version;
	if (version != DATA_VERSION && version != 1) {
		return false;
	}
	drugs.clear();
	pharmacies.clear();
	stocks.clear();

	quint32 drugCount = 0;
	in >> drugCount;
	for (quint32 i = 0; i < drugCount; ++i) {
		Models::Drug d;
		if (version == 1) {
			// read legacy layout with min/max prices and drop them
			in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
			   >> d.dosageForm >> d.country >> d.prescriptionRequired;
			double legacyMin = 0;
			double legacyMax = 0;
			in >> legacyMin;
			in >> legacyMax;
		} else {
			in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
			   >> d.dosageForm >> d.country >> d.prescriptionRequired;
		}
		drugs.push_back(d);
	}
	quint32 pharmacyCount = 0;
	in >> pharmacyCount;
	for (quint32 i = 0; i < pharmacyCount; ++i) {
		Models::Pharmacy p; in >> p; pharmacies.push_back(p);
	}
	quint32 stockCount = 0;
	in >> stockCount;
	for (quint32 i = 0; i < stockCount; ++i) {
		Models::Stock s; in >> s; stocks.push_back(s);
	}
	return true;
}

bool Repository::save() const
{
	QFile f(dataFilePath());
	if (!f.open(QIODevice::WriteOnly)) {
		return false;
	}
	QDataStream out(&f);
	out.setVersion(QDataStream::Qt_6_0);
	out << DATA_VERSION;
	out << quint32(drugs.size());
	for (const auto &d : drugs) out << d;
	out << quint32(pharmacies.size());
	for (const auto &p : pharmacies) out << p;
	out << quint32(stocks.size());
	for (const auto &s : stocks) out << s;
	return true;
}

quint32 Repository::nextDrugId() const
{
	quint32 maxId = 0;
	for (const auto &d : drugs) maxId = std::max(maxId, d.id);
	return maxId + 1;
}

quint32 Repository::nextPharmacyId() const
{
	quint32 maxId = 0;
	for (const auto &p : pharmacies) maxId = std::max(maxId, p.id);
	return maxId + 1;
}

quint32 Repository::addDrug(const Drug &d)
{
	Drug copy = d;
	copy.id = nextDrugId();
	drugs.push_back(copy);
	return copy.id;
}

bool Repository::updateDrug(const Drug &d)
{
	for (auto &x : drugs) {
		if (x.id == d.id) { x = d; return true; }
	}
	return false;
}

bool Repository::removeDrug(quint32 id)
{
	const int before = drugs.size();
	const auto [first, last] = std::ranges::remove_if(drugs, [id](const Drug& d){ return d.id == id; });
	drugs.erase(first, last);
	const auto stockRange = std::ranges::remove_if(stocks, [id](const Stock& s){ return s.drugId == id; });
	stocks.erase(stockRange.begin(), stockRange.end());
	return before != drugs.size();
}

Drug* Repository::findDrug(quint32 id)
{
	for (auto &x : drugs) if (x.id==id) return &x;
	return nullptr;
}

const Drug* Repository::findDrugConst(quint32 id) const
{
	for (const auto &x : drugs) if (x.id==id) return &x;
	return nullptr;
}

quint32 Repository::addPharmacy(const Pharmacy &p)
{
	Pharmacy copy = p;
	copy.id = nextPharmacyId();
	if (copy.hours.size()!=7) copy.hours = QVector<QPair<QTime,QTime>>(7);
	pharmacies.push_back(copy);
	return copy.id;
}

bool Repository::updatePharmacy(const Pharmacy &p)
{
	for (auto &x : pharmacies) {
		if (x.id == p.id) { x = p; return true; }
	}
	return false;
}

bool Repository::removePharmacy(quint32 id)
{
	const int before = pharmacies.size();
	const auto [first, last] = std::ranges::remove_if(pharmacies, [id](const Pharmacy& p){ return p.id == id; });
	pharmacies.erase(first, last);
	const auto stockRange = std::ranges::remove_if(stocks, [id](const Stock& s){ return s.pharmacyId == id; });
	stocks.erase(stockRange.begin(), stockRange.end());
	return before != pharmacies.size();
}

Pharmacy* Repository::findPharmacy(quint32 id)
{
	for (auto &x : pharmacies) if (x.id==id) return &x;
	return nullptr;
}

const Pharmacy* Repository::findPharmacyConst(quint32 id) const
{
	for (const auto &x : pharmacies) if (x.id==id) return &x;
	return nullptr;
}

bool Repository::setStock(quint32 pharmacyId, quint32 drugId, double price)
{
	for (auto &s : stocks) {
		if (s.pharmacyId==pharmacyId && s.drugId==drugId) { s.price = price; return true; }
	}
	Models::Stock s; s.pharmacyId = pharmacyId; s.drugId = drugId; s.price = price;
	stocks.push_back(s);
	return true;
}

bool Repository::removeStock(quint32 pharmacyId, quint32 drugId)
{
	const int before = stocks.size();
	const auto range = std::ranges::remove_if(stocks, [pharmacyId,drugId](const Stock& s){
		return s.pharmacyId == pharmacyId && s.drugId == drugId;
	});
	stocks.erase(range.begin(), range.end());
	return before != stocks.size();
}

QVector<Models::Stock> Repository::stocksForDrug(quint32 drugId) const
{
	QVector<Models::Stock> res;
	for (const auto &s : stocks) if (s.drugId==drugId) res.push_back(s);
	return res;
}

QVector<Models::Stock> Repository::stocksForPharmacy(quint32 pharmacyId) const
{
	QVector<Models::Stock> res;
	for (const auto &s : stocks) if (s.pharmacyId==pharmacyId) res.push_back(s);
	return res;
}

void Repository::seedSampleData()
{
	drugs.clear(); pharmacies.clear(); stocks.clear();
	Models::Drug d1; d1.tradeName="НО-ШПА"; d1.medicalName="Дротаверин"; d1.manufacturer="Хиноин"; d1.dosageForm="таблетки 40мг N24"; d1.country="Венгрия"; d1.prescriptionRequired=false;
	Models::Drug d2; d2.tradeName="НО-ШПА ФОРТЕ"; d2.medicalName="Дротаверин"; d2.manufacturer="Хиноин"; d2.dosageForm="таблетки 80мг N24"; d2.country="Венгрия"; d2.prescriptionRequired=false;
	Models::Drug d3; d3.tradeName="АНАЛЬГИН"; d3.medicalName="Метамизол натрия"; d3.manufacturer="Борисовский ЗМП"; d3.dosageForm="таблетки 500мг N10"; d3.country="Беларусь"; d3.prescriptionRequired=false;
	d1.id = addDrug(d1); d2.id = addDrug(d2); d3.id = addDrug(d3);

	Models::Pharmacy p1; p1.name="Аптека Новая N1 ООО Пролайф"; p1.address="Бобруйск, ул. Пролетарская, 17-116"; p1.phone="(029) 6130830"; p1.latitude=53.135; p1.longitude=29.224;
	Models::Pharmacy p2; p2.name="Могилевское РУП Фармация Аптека N221"; p2.address="Бобруйск, ул. Минская, 47-23"; p2.phone="(0225) 730755"; p2.latitude=53.152; p2.longitude=29.205;
	Models::Pharmacy p3; p3.name="Аптека Новая N7 ООО Пролайф"; p3.address="Могилев, ул. Первомайская, 48-76"; p3.phone="(029) 6205690"; p3.latitude=53.908; p3.longitude=30.336;
	for (auto *pp : {&p1,&p2,&p3}) {
		pp->hours = QVector<QPair<QTime,QTime>>(7);
		for (int i=0;i<7;++i) {
			pp->hours[i] = {QTime(8,0), QTime(22,0)};
		}
	}
	p1.id = addPharmacy(p1);
	p2.id = addPharmacy(p2);
	p3.id = addPharmacy(p3);

	setStock(p1.id, d1.id, 9.57);
	setStock(p1.id, d2.id, 10.50);
	setStock(p2.id, d1.id, 9.58);
	setStock(p2.id, d3.id, 1.20);
	setStock(p3.id, d1.id, 9.57);
}


