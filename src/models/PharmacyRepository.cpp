#include "PharmacyRepository.h"
#include "Serialization.h"

#include <algorithm>

namespace {
constexpr quint32 PHARMACY_DATA_VERSION = 1;
}

namespace Models {

PharmacyRepository::PharmacyRepository()
	: BaseRepository("pharmacies.bin")
{
}

bool PharmacyRepository::load()
{
	return readFromFile([this](QDataStream &in){
		quint32 version = 0;
		in >> version;
		if (version != PHARMACY_DATA_VERSION) {
			return false;
		}
		quint32 pharmaciesCount = 0;
		in >> pharmaciesCount;
		QVector<Pharmacy> loadedPharmacies;
		loadedPharmacies.reserve(pharmaciesCount);
		for (quint32 i = 0; i < pharmaciesCount; ++i) {
			Pharmacy p;
			in >> p;
			loadedPharmacies.push_back(p);
		}

		quint32 stocksCount = 0;
		in >> stocksCount;
		QVector<Stock> loadedStocks;
		loadedStocks.reserve(stocksCount);
		for (quint32 i = 0; i < stocksCount; ++i) {
			Stock s;
			in >> s;
			loadedStocks.push_back(s);
		}

		pharmacies = loadedPharmacies;
		stocks = loadedStocks;
		return true;
	});
}

bool PharmacyRepository::save() const
{
	return writeToFile([this](QDataStream &out){
		out << PHARMACY_DATA_VERSION;
		out << quint32(pharmacies.size());
		for (const auto &p : pharmacies) {
			out << p;
		}
		out << quint32(stocks.size());
		for (const auto &s : stocks) {
			out << s;
		}
		return true;
	});
}

void PharmacyRepository::seedSampleData(const QVector<Drug> &drugsSeed)
{
	auto findDrugId = [&drugsSeed](const QString &tradeName) -> quint32 {
		for (const auto &d : drugsSeed) {
			if (d.tradeName.compare(tradeName, Qt::CaseInsensitive) == 0) {
				return d.id;
			}
		}
		return 0;
	};

	pharmacies.clear();
	stocks.clear();

	Pharmacy p1; p1.name="Аптека Новая N1 ООО Пролайф"; p1.address="Бобруйск, ул. Пролетарская, 17-116"; p1.phone="(029) 6130830"; p1.latitude=53.135; p1.longitude=29.224;
	Pharmacy p2; p2.name="Могилевское РУП Фармация Аптека N221"; p2.address="Бобруйск, ул. Минская, 47-23"; p2.phone="(0225) 730755"; p2.latitude=53.152; p2.longitude=29.205;
	Pharmacy p3; p3.name="Аптека Новая N7 ООО Пролайф"; p3.address="Могилев, ул. Первомайская, 48-76"; p3.phone="(029) 6205690"; p3.latitude=53.908; p3.longitude=30.336;

	for (auto *pp : {&p1,&p2,&p3}) {
		pp->hours = QVector<QPair<QTime,QTime>>(7);
		for (int i=0;i<7;++i) {
			pp->hours[i] = {QTime(8,0), QTime(22,0)};
		}
	}
	const auto id1 = findDrugId("НО-ШПА");
	const auto id2 = findDrugId("НО-ШПА ФОРТЕ");
	const auto id3 = findDrugId("АНАЛЬГИН");

	p1.id = addPharmacy(p1);
	p2.id = addPharmacy(p2);
	p3.id = addPharmacy(p3);

	if (id1) {
		setStock(p1.id, id1, 9.57);
		setStock(p2.id, id1, 9.58);
		setStock(p3.id, id1, 9.57);
	}
	if (id2) {
		setStock(p1.id, id2, 10.50);
	}
	if (id3) {
		setStock(p2.id, id3, 1.20);
	}
}

quint32 PharmacyRepository::addPharmacy(const Pharmacy &p)
{
	Pharmacy copy = p;
	copy.id = nextPharmacyId();
	if (copy.hours.size() != 7) {
		copy.hours = QVector<QPair<QTime, QTime>>(7);
	}
	pharmacies.push_back(copy);
	return copy.id;
}

bool PharmacyRepository::updatePharmacy(const Pharmacy &p)
{
	for (auto &x : pharmacies) {
		if (x.id == p.id) {
			x = p;
			return true;
		}
	}
	return false;
}

bool PharmacyRepository::removePharmacy(quint32 id)
{
	const int before = pharmacies.size();
	pharmacies.erase(std::remove_if(pharmacies.begin(), pharmacies.end(), [id](const Pharmacy &pharmacy) {
		return pharmacy.id == id;
	}), pharmacies.end());
	stocks.erase(std::remove_if(stocks.begin(), stocks.end(), [id](const Stock &s) { return s.pharmacyId == id; }), stocks.end());
	return before != pharmacies.size();
}

Pharmacy* PharmacyRepository::findPharmacy(quint32 id)
{
	for (auto &x : pharmacies)
		if (x.id == id)
			return &x;
	return nullptr;
}

const Pharmacy* PharmacyRepository::findPharmacyConst(quint32 id) const
{
	for (const auto &x : pharmacies)
		if (x.id == id)
			return &x;
	return nullptr;
}

bool PharmacyRepository::setStock(quint32 pharmacyId, quint32 drugId, double price)
{
	for (auto &s : stocks) {
		if (s.pharmacyId == pharmacyId && s.drugId == drugId) {
			s.price = price;
			return true;
		}
	}
	Models::Stock s;
	s.pharmacyId = pharmacyId;
	s.drugId = drugId;
	s.price = price;
	stocks.push_back(s);
	return true;
}

bool PharmacyRepository::removeStock(quint32 pharmacyId, quint32 drugId)
{
	const int before = stocks.size();
	stocks.erase(std::remove_if(stocks.begin(), stocks.end(), [pharmacyId, drugId](const Stock &s) {
		return s.pharmacyId == pharmacyId && s.drugId == drugId;
	}), stocks.end());
	return before != stocks.size();
}

QVector<Stock> PharmacyRepository::stocksForDrug(quint32 drugId) const
{
	QVector<Stock> res;
	for (const auto &s : stocks)
		if (s.drugId == drugId)
			res.push_back(s);
	return res;
}

QVector<Stock> PharmacyRepository::stocksForPharmacy(quint32 pharmacyId) const
{
	QVector<Stock> res;
	for (const auto &s : stocks)
		if (s.pharmacyId == pharmacyId)
			res.push_back(s);
	return res;
}

void PharmacyRepository::removeStocksByDrug(quint32 drugId)
{
	stocks.erase(std::remove_if(stocks.begin(), stocks.end(), [drugId](const Stock &s){
		return s.drugId == drugId;
	}), stocks.end());
}

quint32 PharmacyRepository::nextPharmacyId() const
{
	quint32 maxId = 0;
	for (const auto &p : pharmacies) {
		maxId = std::max(maxId, p.id);
	}
	return maxId + 1;
}

} // namespace Models
