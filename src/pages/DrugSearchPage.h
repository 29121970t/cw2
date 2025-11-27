#pragma once

#include "BaseSearchPage.h"
#include "../dialogs/DrugDialog.h"

class DrugSearchPage : public BaseSearchPage {
	Q_OBJECT
public:
	explicit DrugSearchPage(QWidget *parent = nullptr);
	void setInitialFilter(const QString &text) override;

signals:
	void openPharmaciesForDrug(quint32 drugId);
	void switchToPharmacySearch(const QString &initialQuery);

private slots:
	void refresh();
	void filterChanged(const QString &text) override;
	void addDrug();
	void editDrug();
	void deleteDrug();
	void openPharmacies();
	void modeChanged(int index) override;
	void onRowAdd(int row) override;
	void onRowEdit(int row) override;
	void onRowDelete(int row) override;

private:
	void setupUi();
	void fillModel(const QVector<Models::Drug> &rows);
	quint32 currentDrugId() const;

	DrugDialog *dlg = nullptr;
};


