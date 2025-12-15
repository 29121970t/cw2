#pragma once

#include "../dialogs/DrugDialog.h"
#include "../models/DrugRepository.h"
#include "../models/PharmacyRepository.h"

#include "BaseSearchPage.h"

class DrugSearchPage : public BaseSearchPage {
    Q_OBJECT
   public:
    explicit DrugSearchPage(QWidget* parent = nullptr);

   signals:
    void openPharmaciesForDrug(quint32 drugId);
    void switchToPharmacySearch(const QString& initialQuery);

   private slots:
    void refresh();
    void filterChanged(const QString& text) override;
    void openPharmacies();
    void addElement() override;
    void modeChanged(int index) override;
    void onRowAdd(int row) override;
    void onRowEdit(int row) override;
    void onRowDelete(int row) override;

   private:
    void setupUi();
    void fillModel(const QVector<Models::Drug>& rows);
    quint32 slectedDrugId() const;

    Models::DrugRepository* drugRepo = nullptr;
    Models::PharmacyRepository* pharmacyRepo = nullptr;
    DrugDialog* dlg = nullptr;
};
