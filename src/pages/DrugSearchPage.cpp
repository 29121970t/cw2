#include "DrugSearchPage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include "../dialogs/DrugDialog.h"
#include "../core/ServiceLocator.h"
#include "../utils/QtHelpers.h"

DrugSearchPage::DrugSearchPage(QWidget *parent)
	: BaseSearchPage(parent),
	  drugRepo(Core::ServiceLocator::get<Models::DrugRepository>()),
	  dlg(Utils::QtHelpers::makeOwned<DrugDialog>(this))
{
	setupUi();
	refresh();
}

void DrugSearchPage::setupUi()
{
	auto &modeCombo = *getModeCombo();
	auto &searchField = *getSearchEdit();
	auto &tableView = *getTable();

	modeCombo.addItems({tr("По препарату"), tr("По аптеке")});
	searchField.setPlaceholderText(tr("Поиск по названию препарата (торговому или МНН)..."));

	tableView.horizontalHeader()->setSortIndicatorShown(true);
	tableView.setSortingEnabled(true);

	auto v = new QVBoxLayout();
	auto top = new QHBoxLayout();

	top->addWidget(&modeCombo);
	top->addWidget(&searchField, 1);
	v->addLayout(top);
	v->addWidget(&tableView, 1);
	setLayout(v);

	connect(&tableView, &QTableView::doubleClicked, this, &DrugSearchPage::openPharmacies);
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
	//shit
	applyActionsDelegateToLastColumn();
	//raw fix
	getTable()->setColumnHidden(0, true);
}
//make more complex
void DrugSearchPage::refresh()
{
	if (!drugRepo) return;
	fillModel(drugRepo->allDrugs());
}
//file search?
void DrugSearchPage::filterChanged(const QString &text)
{
	if (!drugRepo) return;
	if (text.trimmed().isEmpty()) {
		refresh();
		return;
	}
	QVector<Models::Drug> filtered;
	for (const auto &d : drugRepo->allDrugs()) {
		if (d.tradeName.contains(text, Qt::CaseInsensitive) ||
		    d.medicalName.contains(text, Qt::CaseInsensitive)) {
			filtered.push_back(d);
		}
	}
	fillModel(filtered);
}
//store id as class member?
quint32 DrugSearchPage::currentDrugId() const
{
	const auto *tableView = getTable();
	const auto sel = tableView->selectionModel()->selectedRows();
	if (sel.isEmpty()) return 0;
	return getModel()->item(sel.first().row(), 0)->text().toUInt();
}

void DrugSearchPage::addDrug()
{
	if (!drugRepo) return;
	dlg->reset();
	if (dlg->exec() == QDialog::Accepted) {
		Models::Drug d = dlg->value();
		drugRepo->addDrug(d);
		drugRepo->save();
		refresh();
	}
}

void DrugSearchPage::editDrug()
{
	if (!drugRepo) return;
	auto id = currentDrugId();
	if (!id) return;
	auto *d = drugRepo->findDrug(id);
	if (!d) return;
	dlg->setValue(*d);
	if (dlg->exec() == QDialog::Accepted) {
		*d = dlg->value(); d->id = id;
		drugRepo->updateDrug(*d);
		drugRepo->save();
		refresh();
	}
}

void DrugSearchPage::deleteDrug()
{
	if (!drugRepo) return;
	auto id = currentDrugId();
	if (!id) return;
	if (QMessageBox::question(this, tr("Удалить"), tr("Удалить выбранный препарат?")) == QMessageBox::Yes) {
		drugRepo->removeDrug(id);
		drugRepo->save();
		refresh();
	}
}

void DrugSearchPage::openPharmacies()
{
	if (getModeCombo()->currentIndex() == 1) {
		emit switchToPharmacySearch(getSearchEdit()->text());
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

