#include "DrugRepository.h"
#include "Serialization.h"

#include <algorithm>

namespace {
constexpr quint32 DRUG_DATA_VERSION = 1;
}

namespace Models {

DrugRepository::DrugRepository()
	: BaseRepository("drugs.bin")
{
}

bool DrugRepository::load()
{
	return readFromFile([this](QDataStream &in){
		quint32 version = 0;
		in >> version;
		if (version != DRUG_DATA_VERSION) {
			return false;
		}
		quint32 count = 0;
		in >> count;
		QVector<Drug> loaded;
		loaded.reserve(count);
		for (quint32 i = 0; i < count; ++i) {
			Drug d;
			in >> d;
			loaded.push_back(d);
		}
		drugs = loaded;
		return true;
	});
}

bool DrugRepository::save() const
{
	return writeToFile([this](QDataStream &out){
		out << DRUG_DATA_VERSION;
		out << quint32(drugs.size());
		for (const auto &d : drugs) {
			out << d;
		}
		return true;
	});
}

void DrugRepository::seedSampleData()
{
	drugs.clear();
	Drug d1; d1.tradeName="НО-ШПА"; d1.medicalName="Дротаверин"; d1.manufacturer="Хиноин"; d1.dosageForm="таблетки 40мг N24"; d1.country="Венгрия"; d1.prescriptionRequired=false;
	Drug d2; d2.tradeName="НО-ШПА ФОРТЕ"; d2.medicalName="Дротаверин"; d2.manufacturer="Хиноин"; d2.dosageForm="таблетки 80мг N24"; d2.country="Венгрия"; d2.prescriptionRequired=false;
	Drug d3; d3.tradeName="АНАЛЬГИН"; d3.medicalName="Метамизол натрия"; d3.manufacturer="Борисовский ЗМП"; d3.dosageForm="таблетки 500мг N10"; d3.country="Беларусь"; d3.prescriptionRequired=false;
	addDrug(d1);
	addDrug(d2);
	addDrug(d3);
}

quint32 DrugRepository::addDrug(const Drug &d)
{
	Drug copy = d;
	copy.id = nextDrugId();
	drugs.push_back(copy);
	return copy.id;
}

bool DrugRepository::updateDrug(const Drug &d)
{
	for (auto &x : drugs) {
		if (x.id == d.id) {
			x = d;
			return true;
		}
	}
	return false;
}

bool DrugRepository::removeDrug(quint32 id)
{
	const int before = drugs.size();
	drugs.erase(std::remove_if(drugs.begin(), drugs.end(), [id](const Drug &drug) { return drug.id == id; }), drugs.end());
	return before != drugs.size();
}

Drug* DrugRepository::findDrug(quint32 id)
{
	for (auto &x : drugs)
		if (x.id == id)
			return &x;
	return nullptr;
}

const Drug* DrugRepository::findDrugConst(quint32 id) const
{
	for (const auto &x : drugs)
		if (x.id == id)
			return &x;
	return nullptr;
}

quint32 DrugRepository::nextDrugId() const
{
	quint32 maxId = 0;
	for (const auto &d : drugs) {
		maxId = std::max(maxId, d.id);
	}
	return maxId + 1;
}

} // namespace Models

