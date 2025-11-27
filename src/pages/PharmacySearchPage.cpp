#include "PharmacySearchPage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDate>
#include <QMouseEvent>
#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"
#include "../utils/PharmacyUtils.h"
#include "../utils/QtHelpers.h"

PharmacySearchPage::PharmacySearchPage(QWidget *parent)
	: BaseSearchPage(parent),
	  stockDlg(Utils::QtHelpers::makeOwned<StockDialog>(this)),
	  pharmacyDlg(Utils::QtHelpers::makeOwned<PharmacyDialog>(this))
{
	setupUi();
}

void PharmacySearchPage::setupUi()
{
	modeCombo->addItems({tr("По аптеке"), tr("По препарату")});
	searchEdit->setPlaceholderText(tr("Фильтр по названию или адресу..."));

	setupTable();
	table->horizontalHeader()->setSectionsClickable(true);
	table->horizontalHeader()->setSortIndicatorShown(true);
	setupActionsDelegate();

	// periodic update of "open/closed" state
	openUpdateTimer = Utils::QtHelpers::makeOwned<QTimer>(this);
	openUpdateTimer->setInterval(30000); // 30s
	connect(openUpdateTimer, &QTimer::timeout, this, &PharmacySearchPage::refresh);
	openUpdateTimer->start();

	auto v = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	auto top = Utils::QtHelpers::makeOwned<QHBoxLayout>();
	top->addWidget(modeCombo);
	top->addWidget(searchEdit, 1);
	v->addLayout(top);
	v->addWidget(table, 1);
	setLayout(v);

	setupSearch();
	connect(table, &QTableView::doubleClicked, this, &PharmacySearchPage::openDetails);
	connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &PharmacySearchPage::onHeaderClicked);
}

void PharmacySearchPage::setDrug(quint32 drugId)
{
	this->drugId = drugId;
	refresh();
}

void PharmacySearchPage::setInitialFilter(const QString &text)
{
	BaseSearchPage::setInitialFilter(text);
}

void PharmacySearchPage::refresh()
{
	model->clear();
	if (drugId == 0) {
		model->setHorizontalHeaderLabels({tr("ID"), tr("Аптека"), tr("Адрес"), tr("Открыто сейчас"), tr("Телефон"), QString()});
	} else {
		model->setHorizontalHeaderLabels({tr("ID"), tr("Аптека"), tr("Адрес"), tr("Открыто сейчас"), tr("Телефон"), tr("Цена"), QString()});
	}
	fillModel();
	applyActionsDelegateToLastColumn();
	if (sortSection >= 0) {
		table->horizontalHeader()->setSortIndicator(sortSection, sortOrder);
	}
}

// Use shared utility for determining whether the pharmacy is open
static inline bool isOpenNow(const Models::Pharmacy &p) { return Utils::isPharmacyOpenNow(p); }

void PharmacySearchPage::fillModel()
{
	struct Row { quint32 id; QString name; QString address; bool open; QString phone; double price; };
	QVector<Row> rowsOut;
	const QString filter = searchEdit->text().trimmed();
	if (drugId == 0) {
		for (const auto &p : repo->allPharmacies()) {
			if (!filter.isEmpty()) {
				if (!p.name.contains(filter, Qt::CaseInsensitive) &&
				    !p.address.contains(filter, Qt::CaseInsensitive)) continue;
			}
			rowsOut.push_back({p.id, p.name, p.address, isOpenNow(p), p.phone, qQNaN()});
		}
	} else {
		for (const auto &s : repo->stocksForDrug(drugId)) {
			const auto *p = repo->findPharmacyConst(s.pharmacyId);
			if (!p) continue;
			if (!filter.isEmpty()) {
				if (!p->name.contains(filter, Qt::CaseInsensitive) &&
				    !p->address.contains(filter, Qt::CaseInsensitive)) continue;
			}
			rowsOut.push_back({p->id, p->name, p->address, isOpenNow(*p), p->phone, s.price});
		}
	}
	// Sorting (header-driven)
	const int priceCol = (drugId == 0 ? -1 : 5);
	std::sort(rowsOut.begin(), rowsOut.end(), [this, priceCol](const Row &a, const Row &b){
		if (sortSection == priceCol) {
			if (std::isnan(a.price) && !std::isnan(b.price)) return false;
			if (!std::isnan(a.price) && std::isnan(b.price)) return true;
			if (!qFuzzyCompare(a.price+1, b.price+1))
				return sortOrder == Qt::AscendingOrder ? (a.price < b.price) : (a.price > b.price);
			return a.name.localeAwareCompare(b.name) < 0;
		}
		if (sortSection < 0) {
			if (a.open != b.open) return a.open && !b.open;
			return a.name.localeAwareCompare(b.name) < 0;
		}
		switch (sortSection) {
			case 1: return sortOrder==Qt::AscendingOrder ? (a.name.localeAwareCompare(b.name) < 0)
			                                             : (a.name.localeAwareCompare(b.name) > 0);
			case 2: return sortOrder==Qt::AscendingOrder ? (a.address.localeAwareCompare(b.address) < 0)
			                                             : (a.address.localeAwareCompare(b.address) > 0);
			case 3: if (a.open != b.open) return sortOrder==Qt::AscendingOrder ? (a.open && !b.open) : (!a.open && b.open);
			        return a.name.localeAwareCompare(b.name) < 0;
			case 4: return sortOrder==Qt::AscendingOrder ? (a.phone.localeAwareCompare(b.phone) < 0)
			                                             : (a.phone.localeAwareCompare(b.phone) > 0);
			default: return a.name.localeAwareCompare(b.name) < 0;
		}
	});
	// Fill model
	for (const auto &r : rowsOut) {
		QList<QStandardItem*> items;
		items << Utils::QtHelpers::makeOwned<QStandardItem>(QString::number(r.id));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(r.name);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(r.address);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(r.open ? tr("Открыто") : tr("Закрыто"));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(r.phone);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(std::isnan(r.price) ? QString() : QString::number(r.price, 'f', 2));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(QString());
		model->appendRow(items);
	}
}

void PharmacySearchPage::onHeaderClicked(int section)
{
	if (section == model->columnCount()-1) return; // ignore action column
	if (sortSection == section) {
		sortOrder = (sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
	} else {
		sortSection = section;
		sortOrder = Qt::AscendingOrder;
	}
	table->horizontalHeader()->setSortIndicator(sortSection, sortOrder);
	refresh();
}

void PharmacySearchPage::filterChanged(const QString &)
{
	refresh();
}

quint32 PharmacySearchPage::currentPharmacyId() const
{
	const auto sel = table->selectionModel()->selectedRows();
	if (sel.isEmpty()) return 0;
	return model->item(sel.first().row(), 0)->text().toUInt();
}

void PharmacySearchPage::addPharmacy()
{
	pharmacyDlg->reset();
	if (pharmacyDlg->exec() == QDialog::Accepted) {
		Models::Pharmacy p = pharmacyDlg->value();
		repo->addPharmacy(p);
		repo->save();
		refresh();
	}
}

void PharmacySearchPage::editPharmacy()
{
	auto id = currentPharmacyId();
	if (!id) return;
	auto *p = repo->findPharmacy(id);
	if (!p) return;
	pharmacyDlg->setValue(*p);
	if (pharmacyDlg->exec() == QDialog::Accepted) {
		*p = pharmacyDlg->value(); p->id = id;
		repo->updatePharmacy(*p);
		repo->save();
		refresh();
	}
}

void PharmacySearchPage::deletePharmacy()
{
	auto id = currentPharmacyId();
	if (!id) return;
	if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранную аптеку?")) == QMessageBox::Yes) {
		repo->removePharmacy(id);
		repo->save();
		refresh();
	}
}

void PharmacySearchPage::editPrice()
{
	auto id = currentPharmacyId();
	if (!id) return;
	stockDlg->setInitial(drugId, id, 0.0, drugId == 0);
	if (stockDlg->exec() == QDialog::Accepted) {
		auto v = stockDlg->value();
		repo->setStock(v.pharmacyId, v.drugId, v.price);
		repo->save();
		refresh();
	}
}

void PharmacySearchPage::openDetails()
{
	auto id = currentPharmacyId();
	if (!id) return;
	emit openPharmacyDetails(id, drugId);
}

void PharmacySearchPage::modeChanged(int index)
{
	if (index == 1) {
		emit switchToDrugSearch(searchEdit->text());
		modeCombo->setCurrentIndex(0);
	}
}

void PharmacySearchPage::onRowAdd(int row)
{
	addPharmacy();
}

void PharmacySearchPage::onRowEdit(int row)
{
	editPharmacy();
}

void PharmacySearchPage::onRowDelete(int row)
{
	deletePharmacy();
}

 


