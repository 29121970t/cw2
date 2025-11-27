#include "MainWindow.h"
#include <QMessageBox>
#include <QStandardPaths>
#include <QFileInfo>
#include <QIcon>
#include "pages/DrugSearchPage.h"
#include "pages/PharmacySearchPage.h"
#include "pages/PharmacyDetailsPage.h"
#include "utils/QtHelpers.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent),
	  stack(Utils::QtHelpers::makeOwned<QStackedWidget>(this)),
	  toolbar(addToolBar("MainToolbar")),
	  actionBack(Utils::QtHelpers::makeOwned<QAction>(tr("Назад"), this))
{
	buildUi();
	connectSignals();
	openDrugSearch();
}

void MainWindow::buildUi()
{
	setCentralWidget(stack);
	actionBack->setEnabled(false);
	toolbar->addAction(actionBack);
	toolbar->addSeparator();
}

void MainWindow::connectSignals()
{
	connect(actionBack, &QAction::triggered, this, [this](){
		if (stack->currentIndex() > 0) {
			stack->setCurrentIndex(stack->currentIndex() - 1);
		}
		actionBack->setEnabled(stack->currentIndex() > 0);
	});
}

void MainWindow::openDrugSearch()
{
	if (!pageDrugs) {
		pageDrugs = Utils::QtHelpers::makeOwned<DrugSearchPage>(this);
		connect(pageDrugs, &DrugSearchPage::openPharmaciesForDrug,
		        this, &MainWindow::openPharmacySearchForDrug);
		connect(pageDrugs, &DrugSearchPage::switchToPharmacySearch,
		        this, &MainWindow::openPharmacySearchWithFilter);
		stack->addWidget(pageDrugs);
	}
	stack->setCurrentWidget(pageDrugs);
	actionBack->setEnabled(stack->currentIndex() > 0);
}

void MainWindow::openPharmacySearchForDrug(quint32 drugId)
{
	if (!pagePharmaciesForDrug) {
		pagePharmaciesForDrug = Utils::QtHelpers::makeOwned<PharmacySearchPage>(this);
		connect(pagePharmaciesForDrug, &PharmacySearchPage::openPharmacyDetails,
		        this, &MainWindow::openPharmacyDetails);
		connect(pagePharmaciesForDrug, &PharmacySearchPage::switchToDrugSearch,
		        this, &MainWindow::openDrugSearchWithFilter);
		stack->addWidget(pagePharmaciesForDrug);
	}
	pagePharmaciesForDrug->setDrug(drugId);
	stack->setCurrentWidget(pagePharmaciesForDrug);
	actionBack->setEnabled(true);
}

void MainWindow::openPharmacyDetails(quint32 pharmacyId, quint32 forDrugId)
{
	const bool wasFull = isFullScreen();
	const bool wasMax = isMaximized();
	if (!pagePharmacyDetails) {
		pagePharmacyDetails = Utils::QtHelpers::makeOwned<PharmacyDetailsPage>(this);
		stack->addWidget(pagePharmacyDetails);
	}
	pagePharmacyDetails->setPharmacy(pharmacyId, forDrugId);
	stack->setCurrentWidget(pagePharmacyDetails);
	actionBack->setEnabled(true);
	// Preserve window state (fullscreen/maximized) when switching pages
	if (wasFull) {
		setWindowState(Qt::WindowFullScreen);
	} else if (wasMax) {
		setWindowState(Qt::WindowMaximized);
	}
}

void MainWindow::openPharmacySearchWithFilter(const QString &query)
{
	if (!pagePharmaciesForDrug) {
		pagePharmaciesForDrug = Utils::QtHelpers::makeOwned<PharmacySearchPage>(this);
		connect(pagePharmaciesForDrug, &PharmacySearchPage::openPharmacyDetails,
		        this, &MainWindow::openPharmacyDetails);
		connect(pagePharmaciesForDrug, &PharmacySearchPage::switchToDrugSearch,
		        this, &MainWindow::openDrugSearchWithFilter);
		stack->addWidget(pagePharmaciesForDrug);
	}
	pagePharmaciesForDrug->setDrug(0);
	pagePharmaciesForDrug->setInitialFilter(query);
	stack->setCurrentWidget(pagePharmaciesForDrug);
	actionBack->setEnabled(true);
}

void MainWindow::openDrugSearchWithFilter(const QString &query)
{
	if (!pageDrugs) {
		pageDrugs = Utils::QtHelpers::makeOwned<DrugSearchPage>(this);
		connect(pageDrugs, &DrugSearchPage::openPharmaciesForDrug,
		        this, &MainWindow::openPharmacySearchForDrug);
		connect(pageDrugs, &DrugSearchPage::switchToPharmacySearch,
		        this, &MainWindow::openPharmacySearchWithFilter);
		stack->addWidget(pageDrugs);
	}
	pageDrugs->setInitialFilter(query);
	stack->setCurrentWidget(pageDrugs);
	actionBack->setEnabled(stack->currentIndex() > 0);
}


