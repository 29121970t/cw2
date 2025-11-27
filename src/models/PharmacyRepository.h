#pragma once

#include "BaseRepository.h"
#include "Entities.h"
#include <QVector>

namespace Models {

class PharmacyRepository : public BaseRepository {
public:
	PharmacyRepository();

	bool load();
	bool save() const;
	void seedSampleData(const QVector<Drug> &drugs);

	const QVector<Pharmacy>& allPharmacies() const { return pharmacies; }
	const QVector<Stock>& allStocks() const { return stocks; }

	quint32 addPharmacy(const Pharmacy &p);
	bool updatePharmacy(const Pharmacy &p);
	bool removePharmacy(quint32 id);
	Pharmacy* findPharmacy(quint32 id);
	const Pharmacy* findPharmacyConst(quint32 id) const;

	bool setStock(quint32 pharmacyId, quint32 drugId, double price);
	bool removeStock(quint32 pharmacyId, quint32 drugId);
	QVector<Stock> stocksForDrug(quint32 drugId) const;
	QVector<Stock> stocksForPharmacy(quint32 pharmacyId) const;
	void removeStocksByDrug(quint32 drugId);

private:
	quint32 nextPharmacyId() const;

	QVector<Pharmacy> pharmacies;
	QVector<Stock> stocks;
};

} // namespace Models


