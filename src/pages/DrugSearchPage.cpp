#include "DrugSearchPage.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QVBoxLayout>

#include "../core/ServiceLocator.h"
#include "../dialogs/DrugDialog.h"
#include "../models/PharmacyRepository.h"



DrugSearchPage::DrugSearchPage(QWidget* parent)
    : BaseSearchPage(parent),
      drugRepo(Core::ServiceLocator::get<Models::DrugRepository>()),
      pharmacyRepo(Core::ServiceLocator::get<Models::PharmacyRepository>()),
      dlg(new DrugDialog(this)) {
    setupUi();
    refresh();
}

void DrugSearchPage::setupUi() {
    auto& modeCombo = *getModeCombo();
    auto& searchField = *getSearchEdit();
    auto& addButton = *getAddButton();
    auto& tableView = *getTable();

    modeCombo.addItems({tr("По препарату"), tr("По аптеке")});
    searchField.setPlaceholderText(tr("Поиск по названию препарата (торговому или МНН)..."));

    tableView.horizontalHeader()->setSortIndicatorShown(true);
    tableView.setSortingEnabled(true);

    QSortFilterProxyModel* proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(getModel());
    tableView.setModel(proxyModel);

    auto v = new QVBoxLayout();
    auto top = new QHBoxLayout();

    top->addWidget(&modeCombo);
    top->addWidget(&addButton);
    addButton.setVisible(getAllowAdd());

    top->addWidget(&searchField, 1);
    v->addLayout(top);
    v->addWidget(&tableView, 1);
    setLayout(v);

    connect(&tableView, &QTableView::doubleClicked, this, &DrugSearchPage::openPharmacies);
}

void DrugSearchPage::fillModel(const QVector<Models::Drug>& rows) {
    auto* modelPtr = getModel();
    modelPtr->clear();
    modelPtr->setHorizontalHeaderLabels(
        {tr("ID"), tr("Наименование (торговое)"), tr("МНН"), tr("Производитель"), tr("Форма"), tr("Страна"), tr("Рецепт"), QString()});
    for (const auto& d : rows) {
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(d.id));
        items << new QStandardItem(d.tradeName);
        items << new QStandardItem(d.medicalName);
        items << new QStandardItem(d.manufacturer);
        items << new QStandardItem(d.dosageForm);
        items << new QStandardItem(d.country);
        items << new QStandardItem(d.prescriptionRequired ? tr("Да") : tr("Нет"));
        items << new QStandardItem(QString());
        modelPtr->appendRow(items);
    }
    // shit
    applyActionsDelegateToLastColumn();

    // raw fix
    getTable()->setColumnHidden(0, true);
}
// make more complex
void DrugSearchPage::refresh() {
    if (!drugRepo) return;
    fillModel(drugRepo->allDrugs());
}
// file search?
void DrugSearchPage::filterChanged(const QString& text) {
    if (!drugRepo) return;
    if (text.trimmed().isEmpty()) {
        refresh();
        return;
    }
    QVector<Models::Drug> filtered;
    for (const auto& d : drugRepo->allDrugs()) {
        if (d.tradeName.contains(text, Qt::CaseInsensitive) || d.medicalName.contains(text, Qt::CaseInsensitive)) {
            filtered.push_back(d);
        }
    }
    fillModel(filtered);
}
// store id as class member?
quint32 DrugSearchPage::slectedDrugId() const {
    const auto* tableView = getTable();
    const auto sel = tableView->selectionModel()->selectedRows();
    if (sel.isEmpty()) return 0;
    return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void DrugSearchPage::openPharmacies() {
    if (getModeCombo()->currentIndex() == 1) {
        emit switchToPharmacySearch(getSearchEdit()->text());
    } else {
        auto id = slectedDrugId();
        if (!id) return;

        emit openPharmaciesForDrug(id);
    }
}

void DrugSearchPage::onRowAdd(int row) {
    if (!drugRepo) return;
    dlg->reset();
    if (dlg->exec() == QDialog::Accepted) {
        Models::Drug d = dlg->value();
        drugRepo->addDrug(d);
        drugRepo->save();
        refresh();
    }
}
void DrugSearchPage::addElement() { onRowAdd(0); }

void DrugSearchPage::onRowEdit(int row) {
    if (!drugRepo) return;
    auto id = slectedDrugId();
    if (!id) return;
    auto* d = drugRepo->findDrug(id);
    if (!d) return;
    dlg->setValue(*d);
    if (dlg->exec() == QDialog::Accepted) {
        *d = dlg->value();
        d->id = id;
        drugRepo->updateDrug(*d);
        drugRepo->save();
        refresh();
    }
}

void DrugSearchPage::onRowDelete(int row) {
    if (!drugRepo) return;
    auto id = slectedDrugId();
    if (!id) return;
    if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранный препарат?")) == QMessageBox::Yes) {
        drugRepo->removeDrug(id);
        drugRepo->save();
        pharmacyRepo->removeStocksByDrug(id);
        refresh();
    }
}

void DrugSearchPage::modeChanged(int index) {
    if (index == 1) {
        emit switchToPharmacySearch(getSearchEdit()->text());
        getModeCombo()->setCurrentIndex(0);
    }
}
