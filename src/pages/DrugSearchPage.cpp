#include "DrugSearchPage.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include "../dialogs/DrugDialog.h"

DrugSearchPage::DrugSearchPage(QWidget *parent)
	: BaseSearchPage(parent),
	  dlg(new DrugDialog(this))
{
	setupUi();
	refresh();
}

void DrugSearchPage::setupUi()
{
	modeCombo->addItems({tr("По препарату"), tr("По аптеке")});
	searchEdit->setPlaceholderText(tr("Поиск по названию препарата (торговому или МНН)..."));

	setupTable();
	table->horizontalHeader()->setSortIndicatorShown(true);
	table->setSortingEnabled(true);
	setupActionsDelegate();

	auto v = new QVBoxLayout;
	auto top = new QHBoxLayout;
	top->addWidget(modeCombo);
	top->addWidget(searchEdit, 1);
	v->addLayout(top);
	v->addWidget(table, 1);
	setLayout(v);

	setupSearch();
	connect(table, &QTableView::doubleClicked, this, &DrugSearchPage::openPharmacies);
}

void DrugSearchPage::fillModel(const QVector<Models::Drug> &rows)
{
	model->clear();
	model->setHorizontalHeaderLabels({tr("ID"), tr("Наименование (торговое)"), tr("МНН"), tr("Производитель"), tr("Форма"),
	                                  tr("Страна"), tr("Рецепт"), QString()});
	for (const auto &d : rows) {
		QList<QStandardItem*> items;
		items << new QStandardItem(QString::number(d.id));
		items << new QStandardItem(d.tradeName);
		items << new QStandardItem(d.medicalName);
		items << new QStandardItem(d.manufacturer);
		items << new QStandardItem(d.dosageForm);
		items << new QStandardItem(d.country);
		items << new QStandardItem(d.prescriptionRequired ? tr("Да") : tr("Нет"));
		items << new QStandardItem(QString());
		model->appendRow(items);
	}
	applyActionsDelegateToLastColumn();
}

void DrugSearchPage::refresh()
{
	fillModel(repo->allDrugs());
}

void DrugSearchPage::filterChanged(const QString &text)
{
	if (text.trimmed().isEmpty()) {
		refresh();
		return;
	}
	QVector<Models::Drug> filtered;
	for (const auto &d : repo->allDrugs()) {
		if (d.tradeName.contains(text, Qt::CaseInsensitive) ||
		    d.medicalName.contains(text, Qt::CaseInsensitive)) {
			filtered.push_back(d);
		}
	}
	fillModel(filtered);
}

quint32 DrugSearchPage::currentDrugId() const
{
	const auto sel = table->selectionModel()->selectedRows();
	if (sel.isEmpty()) return 0;
	return model->item(sel.first().row(), 0)->text().toUInt();
}

void DrugSearchPage::addDrug()
{
	dlg->reset();
	if (dlg->exec() == QDialog::Accepted) {
		Models::Drug d = dlg->value();
		repo->addDrug(d);
		repo->save();
		refresh();
	}
}

void DrugSearchPage::editDrug()
{
	auto id = currentDrugId();
	if (!id) return;
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
		repo->removeDrug(id);
		repo->save();
		refresh();
	}
}

void DrugSearchPage::openPharmacies()
{
	if (modeCombo->currentIndex() == 1) {
		emit switchToPharmacySearch(searchEdit->text());
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
		emit switchToPharmacySearch(searchEdit->text());
		// вернуть режим по умолчанию визуально
		modeCombo->setCurrentIndex(0);
	}
}

void DrugSearchPage::setInitialFilter(const QString &text)
{
	BaseSearchPage::setInitialFilter(text);
}


