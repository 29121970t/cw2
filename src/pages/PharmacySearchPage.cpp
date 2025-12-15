#include "PharmacySearchPage.h"

#include <QDate>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QObject>
#include <QVBoxLayout>
#include <algorithm>
#include <optional>
#include <ranges>

#include "../core/ServiceLocator.h"
#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"
#include "../utils/PharmacyUtils.h"

PharmacySearchPage::PharmacySearchPage(QWidget* parent)
    : BaseSearchPage(parent),
      drugRepo(Core::ServiceLocator::get<Models::DrugRepository>()),
      pharmacyRepo(Core::ServiceLocator::get<Models::PharmacyRepository>()),
      stockDlg(new StockDialog(this)),
      pharmacyDlg(new PharmacyDialog(this)) {
    setupUi();
}

void PharmacySearchPage::setupUi() {
    auto* mode = getModeCombo();
    auto& addButton = *getAddButton();
    mode->addItems({tr("По аптеке"), tr("По препарату")});
    auto* search = getSearchEdit();
    search->setPlaceholderText(tr("Фильтр по названию или адресу..."));

    auto* tbl = getTable();
    tbl->horizontalHeader()->setSectionsClickable(true);
    tbl->horizontalHeader()->setSortIndicatorShown(true);

    // periodic update of "open/closed" state
    openUpdateTimer = new QTimer(this);
    openUpdateTimer->setInterval(30000);  // 30s
    connect(openUpdateTimer, &QTimer::timeout, this, &PharmacySearchPage::refresh);
    openUpdateTimer->start();

    auto v = new QVBoxLayout();
    auto top = new QHBoxLayout();
    top->addWidget(mode);
    top->addWidget(&addButton);
    setVisible(getAllowAdd());
    top->addWidget(search, 1);

    v->addLayout(top);
    v->addWidget(tbl, 1);
    setLayout(v);

    connect(tbl, &QTableView::doubleClicked, this, &PharmacySearchPage::openDetails);
    connect(tbl->horizontalHeader(), &QHeaderView::sectionClicked, this, &PharmacySearchPage::onHeaderClicked);
    // connect(tbl->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &PharmacySearchPage::onSortIndicatorChange);
}

void PharmacySearchPage::setDrug(quint32 id) {
    this->drugId = id;
    refresh();
}

void PharmacySearchPage::refresh() {
    fillModel();
    applyActionsDelegateToLastColumn();
    if (sortSection >= 0) {
        getTable()->horizontalHeader()->setSortIndicator(sortSection, sortOrder);
    }
    getTable()->setColumnHidden(0, true);
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

bool matchesFilter(const Models::Pharmacy& pharmacy, const QString& filter) {
    if (filter.isEmpty()) return true;
    return pharmacy.name.contains(filter, Qt::CaseInsensitive) || pharmacy.address.contains(filter, Qt::CaseInsensitive);
}

RowData makeRow(const Models::Pharmacy& pharmacy, double price = qQNaN()) {
    return {pharmacy.id, pharmacy.name, pharmacy.address, Utils::isPharmacyOpenNow(pharmacy), pharmacy.phone, price};
}

int compareStrings(const QString& lhs, const QString& rhs, Qt::SortOrder order) {
    const int cmp = lhs.localeAwareCompare(rhs);
    if (cmp == 0) return 0;
    const int sign = (cmp < 0) ? -1 : 1;
    return order == Qt::AscendingOrder ? sign : -sign;
}

bool compareByName(const RowData& a, const RowData& b, Qt::SortOrder order) {
    const int cmp = compareStrings(a.name, b.name, order);
    if (cmp == 0) return a.id < b.id;
    return cmp < 0;
}

bool compareByAddress(const RowData& a, const RowData& b, Qt::SortOrder order) {
    const int cmp = compareStrings(a.address, b.address, order);
    if (cmp == 0) {
        return compareByName(a, b, Qt::AscendingOrder);
    }
    return cmp < 0;
}

bool compareByPhone(const RowData& a, const RowData& b, Qt::SortOrder order) {
    const int cmp = compareStrings(a.phone, b.phone, order);
    if (cmp == 0) {
        return compareByName(a, b, Qt::AscendingOrder);
    }
    return cmp < 0;
}

std::optional<bool> compareByOpenState(const RowData& a, const RowData& b, Qt::SortOrder order) {
    if (a.openNow == b.openNow) {
        return std::nullopt;
    }
    const bool openFirst = (order == Qt::AscendingOrder) ? a.openNow : !a.openNow;
    return openFirst;
}

std::optional<bool> compareByPrice(const RowData& a, const RowData& b, int section, int priceColumn, Qt::SortOrder order) {
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

bool compareRows(const RowData& a, const RowData& b, int section, Qt::SortOrder order, int priceColumn) {
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
QVector<RowData> collectRows(const Models::PharmacyRepository* pharmacyRepo, const Models::DrugRepository* drugRepo, quint32 drugId,
                             const QString& filter) {
    QVector<RowData> rows;
    if (!pharmacyRepo) return rows;

    if (drugId == 0) {
        const auto& pharmacies = pharmacyRepo->allPharmacies();
        rows.reserve(pharmacies.size());
        for (const auto& pharmacy : pharmacies) {
            if (!matchesFilter(pharmacy, filter)) continue;
            rows.push_back(makeRow(pharmacy));
        }
    } else {
        if (!drugRepo) return rows;
        for (const auto& stock : pharmacyRepo->stocksForDrug(drugId)) {
            const auto* pharmacy = pharmacyRepo->findPharmacyConst(stock.pharmacyId);
            if (!pharmacy || !matchesFilter(*pharmacy, filter)) continue;
            rows.push_back(makeRow(*pharmacy, stock.price));
        }
    }
    return rows;
}

void sortRows(QVector<RowData>& rows, int sortSection, Qt::SortOrder order, int priceColumn) {
    const auto comparator = [sortSection, order, priceColumn](const RowData& lhs, const RowData& rhs) {
        return compareRows(lhs, rhs, sortSection, order, priceColumn);
    };
    std::ranges::stable_sort(rows, comparator);
}

void writeRows(QStandardItemModel* model, const QVector<RowData>& rows, bool includePrice) {
    if (!model) return;
    model->clear();
    model->setHorizontalHeaderLabels({QObject::tr("ID"), QObject::tr("Цена"), QObject::tr("Аптека"), QObject::tr("Открыто сейчас"),
                                      QObject::tr("Адрес"), QObject::tr("Телефон"), QString()});

    for (const auto& row : rows) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(row.id));
        items << new QStandardItem(std::isnan(row.price) ? QString() : QString::number(row.price, 'f', 2));
        items << new QStandardItem(row.name);
        items << new QStandardItem(row.openNow ? QObject::tr("Открыто") : QObject::tr("Закрыто"));
        items << new QStandardItem(row.address);
        items << new QStandardItem(row.phone);

        items << new QStandardItem(QString());
        model->appendRow(items);
    }
}
}  // namespace

void PharmacySearchPage::fillModel() const {
    const QString filter = getSearchEdit()->text().trimmed();
    auto rows = collectRows(pharmacyRepo, drugRepo, drugId, filter);
    const int priceColumn = (drugId ? 1 : -1);
    sortRows(rows, sortSection, sortOrder, priceColumn);

    writeRows(getModel(), rows, priceColumn >= 0);
    if (priceColumn >= 0) {
        getTable()->setColumnHidden(1, false);
    }
	else{
        getTable()->setColumnHidden(1, true);
	}
}

void PharmacySearchPage::onHeaderClicked(int section) {
    if (const auto* modelPtr = getModel(); section > modelPtr->columnCount() - 3) return;  // ignore action column
    if (sortSection == section) {
        sortOrder = (sortOrder == Qt::AscendingOrder) ? Qt::DescendingOrder : Qt::AscendingOrder;
    } else {
        sortSection = section;
        sortOrder = Qt::AscendingOrder;
    }
    getTable()->horizontalHeader()->setSortIndicator(sortSection, sortOrder);
    refresh();
}

void PharmacySearchPage::addElement() { onRowAdd(0); }

void PharmacySearchPage::filterChanged(const QString&) { refresh(); }

quint32 PharmacySearchPage::currentPharmacyId() const {
    const auto sel = getTable()->selectionModel()->selectedRows();
    if (sel.isEmpty()) return 0;
    return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void PharmacySearchPage::openDetails() {
    auto id = currentPharmacyId();
    if (!id) return;

    emit openPharmacyDetails(id, drugId);
}

void PharmacySearchPage::modeChanged(int index) {
    if (index == 1) {
        emit switchToDrugSearch(getSearchEdit()->text());
        getModeCombo()->setCurrentIndex(0);
    }
}

void PharmacySearchPage::onRowAdd(int row) {
    if (!pharmacyRepo) return;
    pharmacyDlg->reset();
    if (pharmacyDlg->exec() == QDialog::Accepted) {
        Models::Pharmacy p = pharmacyDlg->value();
        pharmacyRepo->addPharmacy(p);
        pharmacyRepo->save();
        refresh();
    }
}

void PharmacySearchPage::onRowEdit(int row) {
    if (!pharmacyRepo) return;
    auto id = currentPharmacyId();
    if (!id) return;
    auto* p = pharmacyRepo->findPharmacy(id);
    if (!p) return;
    pharmacyDlg->setValue(*p);
    if (pharmacyDlg->exec() == QDialog::Accepted) {
        *p = pharmacyDlg->value();
        p->id = id;
        pharmacyRepo->updatePharmacy(*p);
        pharmacyRepo->save();
        refresh();
    };
}

void PharmacySearchPage::onRowDelete(int row) {
    if (!pharmacyRepo) return;
    auto id = currentPharmacyId();
    if (!id) return;
    if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранную аптеку?")) == QMessageBox::Yes) {
        pharmacyRepo->removePharmacy(id);
        pharmacyRepo->save();
        refresh();
    }
}
