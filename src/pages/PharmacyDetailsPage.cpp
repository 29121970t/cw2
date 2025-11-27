#include "PharmacyDetailsPage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"
#include "../core/ServiceLocator.h"
#include "../utils/QtHelpers.h"
#include <QMouseEvent>

PharmacyDetailsPage::PharmacyDetailsPage(QWidget *parent)
	: BaseTablePage(parent),
	  repo(Core::ServiceLocator::get<Models::Repository>()),
	  labelName(Utils::QtHelpers::makeOwned<QLabel>(this)),
	  labelAddress(Utils::QtHelpers::makeOwned<QLabel>(this)),
	  labelPhone(Utils::QtHelpers::makeOwned<QLabel>(this)),
	  scheduleView(Utils::QtHelpers::makeOwned<QTableWidget>(7, 2, this)),
	  map(Utils::QtHelpers::makeOwned<Widgets::MapWebView>(this)),
	  btnEditPharmacy(Utils::QtHelpers::makeOwned<QPushButton>(tr("Редактировать аптеку"), this)),
	  stockDlg(Utils::QtHelpers::makeOwned<StockDialog>(this)),
	  pharmacyDlg(Utils::QtHelpers::makeOwned<PharmacyDialog>(this))
{
	setupUi();
}

void PharmacyDetailsPage::setupUi()
{
	setupTable();
	auto *tableView = getTable();
	tableView->horizontalHeader()->setSectionsClickable(true);
	tableView->horizontalHeader()->setSortIndicatorShown(true);
	setupActionsDelegate();

	labelName->setStyleSheet("font-weight: bold; font-size: 18px;");

	// schedule view (read-only)
	scheduleView->setHorizontalHeaderLabels({tr("День"), tr("Время")});
	scheduleView->verticalHeader()->setVisible(false);
	scheduleView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	scheduleView->setSelectionMode(QAbstractItemView::NoSelection);
	scheduleView->horizontalHeader()->setStretchLastSection(true);
	static const QStringList days = {"Понедельник","Вторник","Среда","Четверг","Пятница","Суббота","Воскресенье"};
	for (int i=0;i<7;++i) {
		scheduleView->setItem(i, 0, Utils::QtHelpers::makeOwned<QTableWidgetItem>(days.value(i)));
		scheduleView->item(i,0)->setFlags(Qt::ItemIsEnabled);
		scheduleView->setItem(i, 1, Utils::QtHelpers::makeOwned<QTableWidgetItem>(""));
		scheduleView->item(i,1)->setFlags(Qt::ItemIsEnabled);
	}

	auto top = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	top->addWidget(labelName);
	top->addWidget(labelAddress);
	top->addWidget(labelPhone);

	auto left = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	left->addLayout(top);
	left->addWidget(scheduleView, 1);
	left->addWidget(btnEditPharmacy);

	auto right = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	right->addWidget(map, 1);

	auto grid = Utils::QtHelpers::makeOwned<QHBoxLayout>();
	grid->addLayout(left, 1);
	grid->addLayout(right, 1);

	auto v = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	v->addLayout(grid, 1);
	v->addWidget(tableView, 1);
	setLayout(v);

	connect(btnEditPharmacy, &QPushButton::clicked, this, &PharmacyDetailsPage::editPharmacy);
	connect(tableView, &QTableView::doubleClicked, this, &PharmacyDetailsPage::editAssortmentRow);
}

void PharmacyDetailsPage::setPharmacy(quint32 newPharmacyId, quint32 contextDrugId)
{
	this->pharmacyId = newPharmacyId;
	this->forDrugId = contextDrugId;
	refresh();
}

void PharmacyDetailsPage::refresh()
{
	const auto *p = repo->findPharmacyConst(pharmacyId);
	if (!p) return;
	labelName->setText(p->name);
	labelAddress->setText(tr("Адрес: %1").arg(p->address));
	labelPhone->setText(tr("Телефон: %1").arg(p->phone));
	// fill scheduleView with "HH:mm-HH:mm"
	for (int i=0;i<scheduleView->rowCount() && i<p->hours.size(); ++i) {
		const auto &[openTime, closeTime] = p->hours[i];
		QString text;
		if (openTime.isValid() && closeTime.isValid()) {
			text = openTime.toString("HH:mm") + "-" + closeTime.toString("HH:mm");
		} else {
			text = QString();
		}
		auto *it = scheduleView->item(i,1);
		if (!it) {
			it = Utils::QtHelpers::makeOwned<QTableWidgetItem>();
			scheduleView->setItem(i,1,it);
		}
		it->setText(text);
	}
	map->setLocation(p->latitude, p->longitude);
	fillAssortment();
}

void PharmacyDetailsPage::fillAssortment()
{
	auto *modelPtr = getModel();
	modelPtr->clear();
	modelPtr->setHorizontalHeaderLabels({tr("ID преп."), tr("Наименование"), tr("МНН"), tr("Цена"), QString()});
	for (const auto &s : repo->stocksForPharmacy(pharmacyId)) {
		const auto *d = repo->findDrugConst(s.drugId);
		if (!d) continue;
		QList<QStandardItem*> row;
		row << Utils::QtHelpers::makeOwned<QStandardItem>(QString::number(d->id));
		row << Utils::QtHelpers::makeOwned<QStandardItem>(d->tradeName);
		row << Utils::QtHelpers::makeOwned<QStandardItem>(d->medicalName);
		row << Utils::QtHelpers::makeOwned<QStandardItem>(QString::number(s.price, 'f', 2));
		row << Utils::QtHelpers::makeOwned<QStandardItem>(QString());
		modelPtr->appendRow(row);
	}
	applyActionsDelegateToLastColumn();
}

void PharmacyDetailsPage::editPharmacy()
{
	auto *p = repo->findPharmacy(pharmacyId);
	if (!p) return;
	pharmacyDlg->setValue(*p);
	if (pharmacyDlg->exec() == QDialog::Accepted) {
		*p = pharmacyDlg->value(); p->id = pharmacyId;
		repo->updatePharmacy(*p);
		repo->save();
		refresh();
	}
}

void PharmacyDetailsPage::editAssortmentRow()
{
	onRowEdit(getTable()->currentIndex().row());
}

quint32 PharmacyDetailsPage::currentSelectedDrugId() const
{
	const auto sel = getTable()->selectionModel()->selectedRows();
	if (sel.isEmpty()) return 0;
	return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void PharmacyDetailsPage::onRowAdd(int)
{
	// reuse dialog, allow choosing any drug
	stockDlg->setInitial(0, pharmacyId, 0.0, true);
	if (stockDlg->exec() != QDialog::Accepted) return;
	const auto v = stockDlg->value();
	repo->setStock(v.pharmacyId, v.drugId, v.price);
	repo->save();
	fillAssortment();
}

void PharmacyDetailsPage::onRowEdit(int row)
{
	if (row < 0) return;
	selectRow(row);
	const quint32 drugId = currentSelectedDrugId();
	if (!drugId) return;
	const QString priceStr = getModel()->item(row, 3)->text();
	double price = priceStr.toDouble();
	stockDlg->setInitial(drugId, pharmacyId, price, false);
	if (stockDlg->exec() != QDialog::Accepted) return;
	const auto v = stockDlg->value();
	repo->setStock(v.pharmacyId, v.drugId, v.price);
	repo->save();
	fillAssortment();
}

void PharmacyDetailsPage::onRowDelete(int row)
{
	if (row < 0) return;
	selectRow(row);
	const quint32 drugId = currentSelectedDrugId();
	if (!drugId) return;
	repo->removeStock(pharmacyId, drugId);
	repo->save();
	fillAssortment();
}


