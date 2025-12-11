#include "StockDialog.h"
#include "../core/ServiceLocator.h"
#include "../models/DrugRepository.h"
#include "../models/PharmacyRepository.h"

#include <QMessageBox>

StockDialog::StockDialog(QWidget *parent)
	: BaseDialog(tr("Наличие/Цена"), parent),
	  cbDrug(new QComboBox(this)),
	  cbPharmacy(new QComboBox(this)),
	  spPrice(new QDoubleSpinBox(this))
{
	spPrice->setRange(0, 100000); spPrice->setDecimals(2);

	getFormLayout()->addRow(tr("Препарат"), cbDrug);
	getFormLayout()->addRow(tr("Аптека"), cbPharmacy);
	getFormLayout()->addRow(tr("Цена"), spPrice);

	setupLayout();
}

void StockDialog::setInitial(quint32 drugId, quint32 pharmacyId, double price, bool allowChangeDrug)
{
	// Populate options from repositories via Service Locator
	const auto *drugRepo = Core::ServiceLocator::get<Models::DrugRepository>();
	const auto *pharmacyRepo = Core::ServiceLocator::get<Models::PharmacyRepository>();
	if (!drugRepo || !pharmacyRepo) {
		cbDrug->clear();
		cbPharmacy->clear();
		spPrice->setValue(price);
		return;
	}

	cbDrug->clear();
	for (const auto &d : drugRepo->allDrugs()) {
		cbDrug->addItem(QString("%1 — %2").arg(d.tradeName, d.medicalName), d.id);
	}
	cbPharmacy->clear();
	for (const auto &p : pharmacyRepo->allPharmacies()) {
		cbPharmacy->addItem(p.name, p.id);
	}
	if (const int di = cbDrug->findData(drugId); di >= 0) {
		cbDrug->setCurrentIndex(di);
	}
	if (const int pi = cbPharmacy->findData(pharmacyId); pi >= 0) {
		cbPharmacy->setCurrentIndex(pi);
	}
	cbDrug->setEnabled(allowChangeDrug);
	cbPharmacy->setEnabled(false); // this dialog is usually context of pharmacy
	spPrice->setValue(price);
}

Models::Stock StockDialog::value() const
{
	Models::Stock v;
	v.drugId = cbDrug->currentData().toUInt();
	v.pharmacyId = cbPharmacy->currentData().toUInt();
	v.price = spPrice->value();
	return v;
}

void StockDialog::onAccept()
{
	if (cbDrug->currentData().toUInt() == 0 || cbPharmacy->currentData().toUInt() == 0) {
		QMessageBox::warning(this, tr("Проверка ввода"), tr("Выберите препарат и аптеку."));
		return;
	}
	if (spPrice->value() <= 0.0) {
		QMessageBox::warning(this, tr("Проверка ввода"), tr("Цена должна быть больше нуля."));
		return;
	}
	accept();
}


