#pragma once

#include "BaseTablePage.h"
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include "../models/Repository.h"
#include "../widgets/MapWebView.h"
#include "../dialogs/StockDialog.h"
#include "../dialogs/PharmacyDialog.h"

class PharmacyDetailsPage : public BaseTablePage {
	Q_OBJECT
public:
	explicit PharmacyDetailsPage(QWidget *parent = nullptr);
	void setPharmacy(quint32 pharmacyId, quint32 forDrugId = 0);

private slots:
	void refresh();
	void editPharmacy();
	void editAssortmentRow();

protected slots:
	void onRowAdd(int row) override;
	void onRowEdit(int row) override;
	void onRowDelete(int row) override;

private:
	void setupUi();
	void fillAssortment();
	quint32 currentSelectedDrugId() const;

	Models::Repository *repo = nullptr;
	quint32 pharmacyId = 0;
	quint32 forDrugId = 0;

	QLabel *labelName = nullptr;
	QLabel *labelAddress = nullptr;
	QLabel *labelPhone = nullptr;
	QTableWidget *scheduleView = nullptr;
	Widgets::MapWebView *map = nullptr;
	QPushButton *btnEditPharmacy = nullptr;
	StockDialog *stockDlg = nullptr;
	PharmacyDialog *pharmacyDlg = nullptr;
};


