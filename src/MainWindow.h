#pragma once

#include <QAction>
#include <QMainWindow>
#include <QMouseEvent>
#include <QStackedWidget>
#include <QToolBar>
#include <memory>

#include "core/ServiceLocator.h"
#include "pages/DrugSearchPage.h"
#include "pages/PharmacyDetailsPage.h"
#include "pages/PharmacySearchPage.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
   public:
    explicit MainWindow(QWidget* parent = nullptr);
    bool eventFilter(QObject* obj, QEvent* event) override;
    void selectPreviousPage();

   private slots:
    void openDrugSearch();
    void openPharmacySearchForDrug(quint32 drugId);
    void openPharmacyDetails(quint32 pharmacyId, quint32 forDrugId = 0);
    void openPharmacySearchWithFilter(const QString& query);
    void openDrugSearchWithFilter(const QString& query);

   private:
    void buildUi();
    void connectSignals() const;
    QStackedWidget* stack = nullptr;
    QToolBar* toolbar = nullptr;
    QAction* actionBack = nullptr;


    DrugSearchPage* pageDrugs = nullptr;
    PharmacySearchPage* pagePharmaciesForDrug = nullptr;
    PharmacyDetailsPage* pagePharmacyDetails = nullptr;
};
