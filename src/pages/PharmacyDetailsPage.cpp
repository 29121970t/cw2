#include "PharmacyDetailsPage.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QMessageBox>

#include "../core/ServiceLocator.h"
#include "../dialogs/PharmacyDialog.h"
#include "../dialogs/StockDialog.h"

PharmacyDetailsPage::PharmacyDetailsPage(QWidget* parent)
    : BaseTablePage(parent),
      drugRepo(Core::ServiceLocator::get<Models::DrugRepository>()),
      pharmacyRepo(Core::ServiceLocator::get<Models::PharmacyRepository>()),
      labelName(new QLabel(this)),
      labelAddress(new QLabel(this)),
      labelPhone(new QLabel(this)),
      scheduleView(new QTableWidget(7, 2, this)),
      map(new Widgets::MapWebView(this)),
      addStockButton(new QPushButton(tr("Добавить препарат"), this)),
      btnEditPharmacy(new QPushButton(tr("Редактировать аптеку"), this)),
      stockDlg(new StockDialog(this)),
      pharmacyDlg(new PharmacyDialog(this)) {
    setupUi();
}

void PharmacyDetailsPage::setupUi() {
    auto* tableView = getTable();
    tableView->horizontalHeader()->setSectionsClickable(true);
    tableView->horizontalHeader()->setSortIndicatorShown(true);
    tableView->setSortingEnabled(true);
    tableView->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    labelName->setStyleSheet("font-weight: bold; font-size: 18px;");

    // schedule view (read-only)
    scheduleView->setHorizontalHeaderLabels({tr("День"), tr("Время")});
    scheduleView->verticalHeader()->setVisible(false);
    scheduleView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    scheduleView->setSelectionMode(QAbstractItemView::NoSelection);
    scheduleView->horizontalHeader()->setStretchLastSection(true);
    static const QStringList days = {"Понедельник", "Вторник", "Среда", "Четверг", "Пятница", "Суббота", "Воскресенье"};
    for (int i = 0; i < 7; ++i) {
        scheduleView->setItem(i, 0, new QTableWidgetItem(days.value(i)));
        scheduleView->item(i, 0)->setFlags(Qt::ItemIsEnabled);
        scheduleView->setItem(i, 1, new QTableWidgetItem(""));
        scheduleView->item(i, 1)->setFlags(Qt::ItemIsEnabled);
    }

    auto top = new QVBoxLayout();
    top->addWidget(labelName);
    top->addWidget(labelAddress);
    top->addWidget(labelPhone);

    auto left = new QVBoxLayout();
    left->addLayout(top);
    left->addWidget(scheduleView, Qt::AlignLeft);
    left->addWidget(btnEditPharmacy);
    left->addWidget(addStockButton);

    auto right = new QVBoxLayout();
    right->addWidget(map, 1);

    auto grid = new QHBoxLayout();
    grid->addLayout(left, 1);
    grid->addLayout(right, 1);

    auto v = new QVBoxLayout();
    v->addLayout(grid, 1);
    v->addWidget(tableView, 1);
    setLayout(v);

    connect(btnEditPharmacy, &QPushButton::clicked, this, &PharmacyDetailsPage::editPharmacy);
    connect(tableView, &QTableView::doubleClicked, this, &PharmacyDetailsPage::editAssortmentRow);
    connect(addStockButton, &QPushButton::clicked, this, &PharmacyDetailsPage::onRowAdd);
}

void PharmacyDetailsPage::setPharmacy(quint32 newPharmacyId, quint32 contextDrugId) {
    this->pharmacyId = newPharmacyId;
    this->forDrugId = contextDrugId;
    refresh();
}

void PharmacyDetailsPage::refresh() {
    if (!pharmacyRepo) return;
    const auto* p = pharmacyRepo->findPharmacyConst(pharmacyId);
    if (!p) return;
    labelName->setText(p->name);
    labelAddress->setText(tr("Адрес: %1").arg(p->address));
    labelPhone->setText(tr("Телефон: %1").arg(p->phone));
    // fill scheduleView with "HH:mm-HH:mm"
    for (int i = 0; i < scheduleView->rowCount() && i < p->hours.size(); ++i) {
        const auto& [openTime, closeTime] = p->hours[i];
        QString text;
        if (openTime.isValid() && closeTime.isValid()) {
            text = openTime.toString("HH:mm") + "-" + closeTime.toString("HH:mm");
        } else {
            text = QString();
        }
        auto* it = scheduleView->item(i, 1);
        if (!it) {
            it = new QTableWidgetItem();
            scheduleView->setItem(i, 1, it);
        }
        it->setText(text);
    }
    map->setLocation(p->latitude, p->longitude);
    fillAssortment();
}

void PharmacyDetailsPage::fillAssortment() {
    if (!pharmacyRepo || !drugRepo) return;
    auto* modelPtr = getModel();
    modelPtr->clear();
    modelPtr->setHorizontalHeaderLabels({tr("ID преп."), tr("Наименование"), tr("МНН"), tr("Цена"), tr("Производитель"), tr("Форма"),
	                                  tr("Страна"), QString()});
    for (const auto& s : pharmacyRepo->stocksForPharmacy(pharmacyId)) {
        const auto* d = drugRepo->findDrugConst(s.drugId);
        if (!d) continue;
        QList<QStandardItem*> row;
        row << new QStandardItem(QString::number(d->id));
        row << new QStandardItem(d->tradeName);
        row << new QStandardItem(d->medicalName);
        row << new QStandardItem(QString::number(s.price, 'f', 2));
        row << new QStandardItem(d->manufacturer);
        row << new QStandardItem(d->dosageForm);
        row << new QStandardItem(d->country);
        row << new QStandardItem(QString());
        modelPtr->appendRow(row);
    }
    applyActionsDelegateToLastColumn();
    getTable()->hideColumn(0);
}

void PharmacyDetailsPage::editPharmacy() {
    if (!pharmacyRepo) return;
    auto* p = pharmacyRepo->findPharmacy(pharmacyId);
    if (!p) return;
    pharmacyDlg->setValue(*p);
    if (pharmacyDlg->exec() == QDialog::Accepted) {
        *p = pharmacyDlg->value();
        p->id = pharmacyId;
        pharmacyRepo->updatePharmacy(*p);
        pharmacyRepo->save();
        refresh();
    }
}

void PharmacyDetailsPage::editAssortmentRow() { onRowEdit(getTable()->currentIndex().row()); }

quint32 PharmacyDetailsPage::currentSelectedDrugId() const {
    const auto sel = getTable()->selectionModel()->selectedRows();
    if (sel.isEmpty()) return 0;
    return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void PharmacyDetailsPage::onRowAdd(int) {
    if (!pharmacyRepo) return;
    // reuse dialog, allow choosing any drug
    stockDlg->setInitial(-1, pharmacyId, 0.0, true);
    if (stockDlg->exec() != QDialog::Accepted) return;
    const auto v = stockDlg->value();
    pharmacyRepo->setStock(v.pharmacyId, v.drugId, v.price);
    pharmacyRepo->save();
    fillAssortment();
}

void PharmacyDetailsPage::onRowEdit(int row) {
    if (!pharmacyRepo) return;
    if (row < 0) return;
    selectRow(row);
    const quint32 drugId = currentSelectedDrugId();
    if (!drugId) return;
    const QString priceStr = getModel()->item(row, 3)->text();
    double price = priceStr.toDouble();
    stockDlg->setInitial(drugId, pharmacyId, price, false);
    if (stockDlg->exec() != QDialog::Accepted) return;
    const auto v = stockDlg->value();
    pharmacyRepo->setStock(v.pharmacyId, v.drugId, v.price);
    pharmacyRepo->save();
    fillAssortment();
}

void PharmacyDetailsPage::onRowDelete(int row) {
    if (!pharmacyRepo) return;
    if (row < 0) return;
    selectRow(row);
    const quint32 drugId = currentSelectedDrugId();
    if (!drugId) return;
    if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранный препарат?")) == QMessageBox::Yes) {
        pharmacyRepo->removeStock(pharmacyId, drugId);
        pharmacyRepo->save();
    }

    fillAssortment();
}
