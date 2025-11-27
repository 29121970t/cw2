#include "DrugSearchPage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include "../dialogs/DrugDialog.h"
#include "../utils/QtHelpers.h"

DrugSearchPage::DrugSearchPage(QWidget *parent)
	: BaseSearchPage(parent),
	  dlg(Utils::QtHelpers::makeOwned<DrugDialog>(this))
{
	setupUi();
	refresh();
}

void DrugSearchPage::setupUi()
{
	auto *modeComboCtrl = getModeCombo();
	auto *searchCtrl = getSearchEdit();
	auto *tableView = getTable();

	modeComboCtrl->addItems({tr("По препарату"), tr("По аптеке")});
	searchCtrl->setPlaceholderText(tr("Поиск по названию препарата (торговому или МНН)..."));

	setupTable();
	tableView->horizontalHeader()->setSortIndicatorShown(true);
	tableView->setSortingEnabled(true);
	setupActionsDelegate();

	auto v = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	auto top = Utils::QtHelpers::makeOwned<QHBoxLayout>();
	top->addWidget(modeComboCtrl);
	top->addWidget(searchCtrl, 1);
	v->addLayout(top);
	v->addWidget(tableView, 1);
	setLayout(v);

	setupSearch();
	connect(tableView, &QTableView::doubleClicked, this, &DrugSearchPage::openPharmacies);
}

void DrugSearchPage::fillModel(const QVector<Models::Drug> &rows)
{
	auto *modelPtr = getModel();
	modelPtr->clear();
	modelPtr->setHorizontalHeaderLabels({tr("ID"), tr("Наименование (торговое)"), tr("МНН"), tr("Производитель"), tr("Форма"),
	                                  tr("Страна"), tr("Рецепт"), QString()});
	for (const auto &d : rows) {
		QList<QStandardItem*> items;
		items << Utils::QtHelpers::makeOwned<QStandardItem>(QString::number(d.id));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(d.tradeName);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(d.medicalName);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(d.manufacturer);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(d.dosageForm);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(d.country);
		items << Utils::QtHelpers::makeOwned<QStandardItem>(d.prescriptionRequired ? tr("Да") : tr("Нет"));
		items << Utils::QtHelpers::makeOwned<QStandardItem>(QString());
		modelPtr->appendRow(items);
	}
	applyActionsDelegateToLastColumn();
}

void DrugSearchPage::refresh()
{
	fillModel(getRepository()->allDrugs());
}

void DrugSearchPage::filterChanged(const QString &text)
{
	if (text.trimmed().isEmpty()) {
		refresh();
		return;
	}
	QVector<Models::Drug> filtered;
	for (const auto &d : getRepository()->allDrugs()) {
		if (d.tradeName.contains(text, Qt::CaseInsensitive) ||
		    d.medicalName.contains(text, Qt::CaseInsensitive)) {
			filtered.push_back(d);
		}
	}
	fillModel(filtered);
}

quint32 DrugSearchPage::currentDrugId() const
{
	const auto *tableView = getTable();
	const auto sel = tableView->selectionModel()->selectedRows();
	if (sel.isEmpty()) return 0;
	return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void DrugSearchPage::addDrug()
{
	dlg->reset();
	if (dlg->exec() == QDialog::Accepted) {
		Models::Drug d = dlg->value();
		auto *repo = getRepository();
		repo->addDrug(d);
		repo->save();
		refresh();
	}
}

void DrugSearchPage::editDrug()
{
	auto id = currentDrugId();
	if (!id) return;
	auto *repo = getRepository();
	auto *d = repo->findDrug(id);
	if (!d) return;
	dlg->setValue(*d);
	if (dlg->exec() == QDialog::Accepted) {
		*d = dlg->value(); d->id = id;
		repo->updateDrug(*d);
		repo->save();
		refresh();
	}
}

void DrugSearchPage::deleteDrug()
{
	auto id = currentDrugId();
	if (!id) return;
	if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранный препарат?")) == QMessageBox::Yes) {
		auto *repo = getRepository();
		repo->removeDrug(id);
		repo->save();
		refresh();
	}
}

void DrugSearchPage::openPharmacies()
{
	auto *modeComboCtrl = getModeCombo();
	auto *searchCtrl = getSearchEdit();
	if (modeComboCtrl->currentIndex() == 1) {
		emit switchToPharmacySearch(searchCtrl->text());
	} else {
		auto id = currentDrugId();
		if (!id) return;
		emit openPharmaciesForDrug(id);
	}
}

void DrugSearchPage::onRowAdd(int row)
{
	addDrug();
}

void DrugSearchPage::onRowEdit(int row)
{
	editDrug();
}

void DrugSearchPage::onRowDelete(int row)
{
	deleteDrug();
}

void DrugSearchPage::modeChanged(int index)
{
	if (index == 1) {
		emit switchToPharmacySearch(getSearchEdit()->text());
		// вернуть режим по умолчанию визуально
		getModeCombo()->setCurrentIndex(0);
	}
}

void DrugSearchPage::setInitialFilter(const QString &text)
{
	BaseSearchPage::setInitialFilter(text);
}


