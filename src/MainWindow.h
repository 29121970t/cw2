#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QToolBar>
#include <QAction>
#include "models/Repository.h"
#include "core/ServiceLocator.h"

class DrugSearchPage;
class PharmacySearchPage;
class PharmacyDetailsPage;

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);

private slots:
	void openDrugSearch();
	void openPharmacySearchForDrug(quint32 drugId);
	void openPharmacyDetails(quint32 pharmacyId, quint32 forDrugId = 0);
	void openPharmacySearchWithFilter(const QString &query);
	void openDrugSearchWithFilter(const QString &query);

private:
	void buildUi();
	void connectSignals();
	QStackedWidget *stack = nullptr;
	QToolBar *toolbar = nullptr;
	QAction *actionBack = nullptr;

	// pages
	DrugSearchPage *pageDrugs = nullptr;
	PharmacySearchPage *pagePharmaciesForDrug = nullptr;
	PharmacyDetailsPage *pagePharmacyDetails = nullptr;
};


