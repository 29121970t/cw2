#include "PharmacySearchPage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QDate>
#include <QMouseEvent>
#include <QObject>
#include <algorithm>
#include <optional>
#include <ranges>
#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"
#include "../core/ServiceLocator.h"
#include "../utils/PharmacyUtils.h"
#include "../utils/QtHelpers.h"
#include <cmath>

PharmacySearchPage::PharmacySearchPage(QWidget *parent)
	: BaseSearchPage(parent),
	  drugRepo(Core::ServiceLocator::get<Models::DrugRepository>()),
	  pharmacyRepo(Core::ServiceLocator::get<Models::PharmacyRepository>()),
	  stockDlg(Utils::QtHelpers::makeOwned<StockDialog>(this)),
	  pharmacyDlg(Utils::QtHelpers::makeOwned<PharmacyDialog>(this))
{
	setupUi();
}

void PharmacySearchPage::setupUi()
{
	auto *mode = getModeCombo();
	mode->addItems({tr("По аптеке"), tr("По препарату")});
	auto *search = getSearchEdit();
	search->setPlaceholderText(tr("Фильтр по названию или адресу..."));

	setupTable();
	auto *tbl = getTable();
	tbl->horizontalHeader()->setSectionsClickable(true);
	tbl->horizontalHeader()->setSortIndicatorShown(true);
	setupActionsDelegate();

	// periodic update of "open/closed" state
	openUpdateTimer = Utils::QtHelpers::makeOwned<QTimer>(this);
	openUpdateTimer->setInterval(30000); // 30s
	connect(openUpdateTimer, &QTimer::timeout, this, &PharmacySearchPage::refresh);
	openUpdateTimer->start();

	auto v = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	auto top = Utils::QtHelpers::makeOwned<QHBoxLayout>();
	top->addWidget(mode);
	top->addWidget(search, 1);
	v->addLayout(top);
	v->addWidget(tbl, 1);
	setLayout(v);

	setupSearch();
	connect(tbl, &QTableView::doubleClicked, this, &PharmacySearchPage::openDetails);
	connect(tbl->horizontalHeader(), &QHeaderView::sectionClicked, this, &PharmacySearchPage::onHeaderClicked);
}

void PharmacySearchPage::setDrug(quint32 id)
{
	this->drugId = id;
	refresh();
}

void PharmacySearchPage::refresh()
{
	fillModel();
	applyActionsDelegateToLastColumn();
	if (sortSection >= 0) {
		getTable()->horizontalHeader()->setSortIndicator(sortSection, sortOrder);
	}
}

namespace {
struct RowData {
	quint32 id = 0;
	QString name;
	QString address;
	bool openNow = false;
	QString phone;
	double price = qQNaN();
};

bool matchesFilter(const Models::Pharmacy &pharmacy, const QString &filter)
{
	if (filter.isEmpty()) return true;
	return pharmacy.name.contains(filter, Qt::CaseInsensitive)
	    || pharmacy.address.contains(filter, Qt::CaseInsensitive);
}

RowData makeRow(const Models::Pharmacy &pharmacy, double price = qQNaN())
{
	return {
		pharmacy.id,
		pharmacy.name,
		pharmacy.address,
		Utils::isPharmacyOpenNow(pharmacy),
		pharmacy.phone,
		price
	};
}

int compareStrings(const QString &lhs, const QString &rhs, Qt::SortOrder order)
{
	const int cmp = lhs.localeAwareCompare(rhs);
	if (cmp == 0) return 0;
	const int sign = (cmp < 0) ? -1 : 1;
	return order == Qt::AscendingOrder ? sign : -sign;
}

bool compareByName(const RowData &a, const RowData &b, Qt::SortOrder order)
{
	const int cmp = compareStrings(a.name, b.name, order);
	if (cmp == 0) return a.id < b.id;
	return cmp < 0;
}

bool compareByAddress(const RowData &a, const RowData &b, Qt::SortOrder order)
{
	const int cmp = compareStrings(a.address, b.address, order);
	if (cmp == 0) {
		return compareByName(a, b, Qt::AscendingOrder);
	}
	return cmp < 0;
}

bool compareByPhone(const RowData &a, const RowData &b, Qt::SortOrder order)
{
	const int cmp = compareStrings(a.phone, b.phone, order);
	if (cmp == 0) {
		return compareByName(a, b, Qt::AscendingOrder);
	}
	return cmp < 0;
}

std::optional<bool> compareByOpenState(const RowData &a, const RowData &b, Qt::SortOrder order)
{
	if (a.openNow == b.openNow) {
		return std::nullopt;
	}
	const bool openFirst = (order == Qt::AscendingOrder) ? a.openNow : !a.openNow;
	return openFirst;
}

std::optional<bool> compareByPrice(const RowData &a, const RowData &b, int section, int priceColumn, Qt::SortOrder order)
{
	if (section != priceColumn || priceColumn < 0) {
		return std::nullopt;
	}
	const bool aNaN = std::isnan(a.price);
	const bool bNaN = std::isnan(b.price);
	if (aNaN != bNaN) {
		return !aNaN && bNaN;
	}
	if (!aNaN && !bNaN && !qFuzzyCompare(a.price + 1, b.price + 1)) {
		return order == Qt::AscendingOrder ? (a.price < b.price) : (a.price > b.price);
	}
	return std::nullopt;
}

bool compareRows(const RowData &a, const RowData &b, int section, Qt::SortOrder order, int priceColumn)
{
	if (const auto priceResult = compareByPrice(a, b, section, priceColumn, order); priceResult.has_value()) {
		return *priceResult;
	}

	if (section < 0) {
		if (const auto openResult = compareByOpenState(a, b, Qt::AscendingOrder); openResult.has_value()) {
			return *openResult;
		}
		return compareByName(a, b, Qt::AscendingOrder);
	}

	switch (section) {
		case 1:
			return compareByName(a, b, order);
		case 2:
			return compareByAddress(a, b, order);
		case 3: {
			if (const auto openResult = compareByOpenState(a, b, order); openResult.has_value()) {
				return *openResult;
			}
			return compareByName(a, b, order);
		}
		case 4:
			return compareByPhone(a, b, order);
		default:
			return compareByName(a, b, order);
	}
}
QVector<RowData> collectRows(const Models::PharmacyRepository *pharmacyRepo, const Models::DrugRepository *drugRepo, quint32 drugId, const QString &filter)
{
	QVector<RowData> rows;
	if (!pharmacyRepo) return rows;

	if (drugId == 0) {
		const auto &pharmacies = pharmacyRepo->allPharmacies();
		rows.reserve(pharmacies.size());
		for (const auto &pharmacy : pharmacies) {
			if (!matchesFilter(pharmacy, filter)) continue;
			rows.push_back(makeRow(pharmacy));
		}
	} else {
		if (!drugRepo) return rows;
		for (const auto &stock : pharmacyRepo->stocksForDrug(drugId)) {
			const auto *pharmacy = pharmacyRepo->findPharmacyConst(stock.pharmacyId);
			if (!pharmacy || !matchesFilter(*pharmacy, filter)) continue;
			rows.push_back(makeRow(*pharmacy, stock.price));
		}
	}
	return rows;
}

void sortRows(QVector<RowData> &rows, int sortSection, Qt::SortOrder order, int priceColumn)
{
	const auto comparator = [sortSection, order, priceColumn](const RowData &lhs, const RowData &rhs) {
		return compareRows(lhs, rhs, sortSection, order, priceColumn);
	};
	std::ranges::stable_sort(rows, comparator);
}

void writeRows(QStandardItemModel *model, const QVector<RowData> &rows, bool includePrice)
{
	if (!model) return;
	model->clear();
	if (includePrice) {
		model->setHorizontalHeaderLabels({QObject::tr("ID"), QObject::tr("Аптека"), QObject::tr("Адрес"),
		                                  QObject::tr("Открыто сейчас"), QObject::tr("Телефон"),
		                                  QObject::tr("Цена"), QString()});
	} else {
		model->setHorizontalHeaderLabels({QObject::tr("ID"), QObject::tr("Аптека"), QObject::tr("Адрес"),
		                                  QObject::tr("Открыто сейчас"), QObject::tr("Телефон"), QString()});
	}

	for (const auto &row : rows) {
		QList<QStandardItem*> items;
		items << Utils::QtHelpers::makeOwned<QStandardItem>(QString::number(row.id));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(row.name);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(row.address);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(row.openNow ? QObject::tr("Открыто") : QObject::tr("Закрыто"));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(row.phone);
		if (includePrice) {
			items << Utils::QtHelpers::makeOwned<QStandardItem>(std::isnan(row.price) ? QString()
				: QString::number(row.price, 'f', 2));
		}
		items << Utils::QtHelpers::makeOwned<QStandardItem>(QString());
		model->appendRow(items);
	}
}
} // namespace

void PharmacySearchPage::fillModel() const
{
	const QString filter = getSearchEdit()->text().trimmed();
	auto rows = collectRows(pharmacyRepo, drugRepo, drugId, filter);
	const int priceColumn = (drugId == 0 ? -1 : 5);
	sortRows(rows, sortSection, sortOrder, priceColumn);

	writeRows(getModel(), rows, priceColumn >= 0);
}

void PharmacySearchPage::onHeaderClicked(int section)
{
	if (const auto *modelPtr = getModel(); section == modelPtr->columnCount()-1) return; // ignore action column
	if (sortSection == section) {
		sortOrder = (sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
	} else {
		sortSection = section;
		sortOrder = Qt::AscendingOrder;
	}
	getTable()->horizontalHeader()->setSortIndicator(sortSection, sortOrder);
	refresh();
}

void PharmacySearchPage::filterChanged(const QString &)
{
	refresh();
}

quint32 PharmacySearchPage::currentPharmacyId() const
{
	const auto sel = getTable()->selectionModel()->selectedRows();
	if (sel.isEmpty()) return 0;
	return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void PharmacySearchPage::addPharmacy()
{
	if (!pharmacyRepo) return;
	pharmacyDlg->reset();
	if (pharmacyDlg->exec() == QDialog::Accepted) {
		Models::Pharmacy p = pharmacyDlg->value();
		pharmacyRepo->addPharmacy(p);
		pharmacyRepo->save();
		refresh();
	}
}

void PharmacySearchPage::editPharmacy()
{
	if (!pharmacyRepo) return;
	auto id = currentPharmacyId();
	if (!id) return;
	auto *p = pharmacyRepo->findPharmacy(id);
	if (!p) return;
	pharmacyDlg->setValue(*p);
	if (pharmacyDlg->exec() == QDialog::Accepted) {
		*p = pharmacyDlg->value(); p->id = id;
		pharmacyRepo->updatePharmacy(*p);
		pharmacyRepo->save();
		refresh();
	}
}

void PharmacySearchPage::deletePharmacy()
{
	if (!pharmacyRepo) return;
	auto id = currentPharmacyId();
	if (!id) return;
	if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранную аптеку?")) == QMessageBox::Yes) {
		pharmacyRepo->removePharmacy(id);
		pharmacyRepo->save();
		refresh();
	}
}

void PharmacySearchPage::editPrice()
{
	if (!pharmacyRepo) return;
	auto id = currentPharmacyId();
	if (!id) return;
	stockDlg->setInitial(drugId, id, 0.0, drugId == 0);
	if (stockDlg->exec() == QDialog::Accepted) {
		auto v = stockDlg->value();
		pharmacyRepo->setStock(v.pharmacyId, v.drugId, v.price);
		pharmacyRepo->save();
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
		emit switchToDrugSearch(getSearchEdit()->text());
		getModeCombo()->setCurrentIndex(0);
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

 


