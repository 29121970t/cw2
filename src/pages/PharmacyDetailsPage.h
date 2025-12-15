#pragma once

#include <QLabel>
#include <QPushButton>
#include <QTableWidget>

#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"
#include "../models/DrugRepository.h"
#include "../models/PharmacyRepository.h"
#include "../widgets/MapWebView.h"
#include "BaseTablePage.h"

class PharmacyDetailsPage : public BaseTablePage {
    Q_OBJECT
   public:
    explicit PharmacyDetailsPage(QWidget* parent = nullptr);
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

    Models::DrugRepository* drugRepo = nullptr;
    Models::PharmacyRepository* pharmacyRepo = nullptr;
    quint32 pharmacyId = 0;
    quint32 forDrugId = 0;

    QLabel* labelName = nullptr;
    QLabel* labelAddress = nullptr;
    QLabel* labelPhone = nullptr;
    QTableWidget* scheduleView = nullptr;
    Widgets::MapWebView* map = nullptr;
    QPushButton* addStockButton = nullptr;
    QPushButton* btnEditPharmacy = nullptr;
    StockDialog* stockDlg = nullptr;
    PharmacyDialog* pharmacyDlg = nullptr;
};
