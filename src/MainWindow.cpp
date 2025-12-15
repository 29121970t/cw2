#include "MainWindow.h"

#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QMouseEvent>
#include <QStandardPaths>
#include <memory>

#include "MainWindow.h"
#include "pages/DrugSearchPage.h"
#include "pages/PharmacyDetailsPage.h"
#include "pages/PharmacySearchPage.h"


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      stack(new QStackedWidget(this)),
      toolbar(addToolBar("MainToolbar")),
      actionBack(new QAction(tr("Назад"), this)),
      pageDrugs(new DrugSearchPage(this)),
      pagePharmaciesForDrug(new PharmacySearchPage(this)),
      pagePharmacyDetails(new PharmacyDetailsPage(this))

{
    setWindowFlags(Qt::Window);
    setMouseTracking(true);
    buildUi();
    connectSignals();
    openDrugSearch();
}

void MainWindow::buildUi() {
    setCentralWidget(stack);
    actionBack->setEnabled(false);
    toolbar->addAction(actionBack);
    toolbar->addSeparator();
    toolbar->setMovable(false);
    stack->addWidget(pageDrugs);
    stack->addWidget(pagePharmaciesForDrug);
    stack->addWidget(pagePharmacyDetails);
}

void MainWindow::connectSignals() const {
    connect(pageDrugs, &DrugSearchPage::openPharmaciesForDrug, this, &MainWindow::openPharmacySearchForDrug);
    connect(pageDrugs, &DrugSearchPage::switchToPharmacySearch, this, &MainWindow::openPharmacySearchWithFilter);
    connect(pagePharmaciesForDrug, &PharmacySearchPage::openPharmacyDetails, this, &MainWindow::openPharmacyDetails);
    connect(pagePharmaciesForDrug, &PharmacySearchPage::switchToDrugSearch, this, &MainWindow::openDrugSearchWithFilter);
    connect(actionBack, &QAction::triggered, this, &MainWindow::selectPreviousPage);
}
void MainWindow::selectPreviousPage() {
    if (stack->currentIndex() > 0) {
        stack->setCurrentIndex(stack->currentIndex() - 1);
    }
    actionBack->setEnabled(stack->currentIndex() > 0);
}
void MainWindow::openDrugSearch() {
    pageDrugs->setAllowAdd(true);
    stack->setCurrentWidget(pageDrugs);
    actionBack->setEnabled(stack->currentIndex() > 0);
}

void MainWindow::openPharmacySearchForDrug(quint32 drugId) {
    pagePharmaciesForDrug->setAllowAdd(false);
    pagePharmaciesForDrug->setDrug(drugId);
    stack->setCurrentWidget(pagePharmaciesForDrug);
    actionBack->setEnabled(true);
}

void MainWindow::openPharmacyDetails(quint32 pharmacyId, quint32 forDrugId) {
    pagePharmacyDetails->setPharmacy(pharmacyId, forDrugId);
    stack->setCurrentWidget(pagePharmacyDetails);
    actionBack->setEnabled(true);
}

void MainWindow::openPharmacySearchWithFilter(const QString& query) {
    pagePharmaciesForDrug->setAllowAdd(true);
    pagePharmaciesForDrug->setDrug(0);
    pagePharmaciesForDrug->setInitialFilter(query);
    stack->setCurrentWidget(pagePharmaciesForDrug);
    actionBack->setEnabled(true);
}

void MainWindow::openDrugSearchWithFilter(const QString& query) {
    pageDrugs->setInitialFilter(query);
    stack->setCurrentWidget(pageDrugs);
    actionBack->setEnabled(stack->currentIndex() > 0);
}
bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::MouseButtonRelease && actionBack->isEnabled()) {
        auto mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::BackButton) {
            mouseEvent->accept();
            selectPreviousPage();
            return true;
        }
    }

    return QObject::eventFilter(obj, event);
}
