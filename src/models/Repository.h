#pragma once

#include "Entities.h"
#include <QVector>

namespace Models {

class Repository {
public:
	Repository();

	bool load();
	bool save();
	void seedSampleData();

	// Accessors
	const QVector<Drug>& allDrugs() const { return drugs; }
	const QVector<Pharmacy>& allPharmacies() const { return pharmacies; }
	const QVector<Stock>& allStocks() const { return stocks; }

	// CRUD Drug
	quint32 addDrug(const Drug &d);
	bool updateDrug(const Drug &d);
	bool removeDrug(quint32 id);
	Drug* findDrug(quint32 id);
	const Drug* findDrugConst(quint32 id) const;

	// CRUD Pharmacy
	quint32 addPharmacy(const Pharmacy &p);
	bool updatePharmacy(const Pharmacy &p);
	bool removePharmacy(quint32 id);
	Pharmacy* findPharmacy(quint32 id);
	const Pharmacy* findPharmacyConst(quint32 id) const;

	// Stock operations
	bool setStock(quint32 pharmacyId, quint32 drugId, double price);
	bool removeStock(quint32 pharmacyId, quint32 drugId);
	QVector<Models::Stock> stocksForDrug(quint32 drugId) const;
	QVector<Models::Stock> stocksForPharmacy(quint32 pharmacyId) const;

	// Helpers
	QString dataFilePath() const;

private:
	quint32 nextDrugId();
	quint32 nextPharmacyId();

	QVector<Drug> drugs;
	QVector<Pharmacy> pharmacies;
	QVector<Stock> stocks;
};

} // namespace Models


