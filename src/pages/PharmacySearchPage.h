#pragma once

#include <QTimer>

#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"
#include "../models/DrugRepository.h"
#include "../models/PharmacyRepository.h"
#include "BaseSearchPage.h"

class PharmacySearchPage : public BaseSearchPage {
    Q_OBJECT
   public:
    explicit PharmacySearchPage(QWidget* parent = nullptr);

    void setDrug(quint32 drugId);

   signals:
    void openPharmacyDetails(quint32 pharmacyId, quint32 forDrugId);
    void switchToDrugSearch(const QString& initialQuery);

   private slots:
    void refresh();
    void filterChanged(const QString& text) override;
    void openDetails();
    void modeChanged(int index) override;
    void onRowAdd(int row) override;
    void onRowEdit(int row) override;
    void onRowDelete(int row) override;

   private:
    void setupUi();
    void fillModel() const;
    quint32 currentPharmacyId() const;
    void onHeaderClicked(int section);
    // void onSortIndicatorChange(int section, Qt::SortOrder ord);


    void addElement() override;

    Models::DrugRepository* drugRepo = nullptr;
    Models::PharmacyRepository* pharmacyRepo = nullptr;
    quint32 drugId = 0;
    int sortSection = -1;
    Qt::SortOrder sortOrder = Qt::AscendingOrder;
    QTimer* openUpdateTimer = nullptr;
    StockDialog* stockDlg = nullptr;
    PharmacyDialog* pharmacyDlg = nullptr;
};
