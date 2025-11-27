#include "StockDialog.h"
#include "../core/ServiceLocator.h"
#include "../utils/QtHelpers.h"
#include <QMessageBox>

StockDialog::StockDialog(QWidget *parent)
	: BaseDialog(tr("Наличие/Цена"), parent),
	  cbDrug(Utils::QtHelpers::makeOwned<QComboBox>(this)),
	  cbPharmacy(Utils::QtHelpers::makeOwned<QComboBox>(this)),
	  spPrice(Utils::QtHelpers::makeOwned<QDoubleSpinBox>(this))
{
	spPrice->setRange(0, 100000); spPrice->setDecimals(2);

	getFormLayout()->addRow(tr("Препарат"), cbDrug);
	getFormLayout()->addRow(tr("Аптека"), cbPharmacy);
	getFormLayout()->addRow(tr("Цена"), spPrice);

	setupLayout();
}

void StockDialog::setInitial(quint32 drugId, quint32 pharmacyId, double price, bool allowChangeDrug)
{
	// Populate options from shared Repository via Service Locator
	Models::Repository *r = Core::ServiceLocator::get<Models::Repository>();
	if (!r) {
		cbDrug->clear();
		cbPharmacy->clear();
		spPrice->setValue(price);
		return;
	}

	cbDrug->clear();
	for (const auto &d : r->allDrugs()) {
		cbDrug->addItem(QString("%1 — %2").arg(d.tradeName, d.medicalName), d.id);
	}
	cbPharmacy->clear();
	for (const auto &p : r->allPharmacies()) {
		cbPharmacy->addItem(p.name, p.id);
	}
	int di = cbDrug->findData(drugId);
	if (di >= 0) cbDrug->setCurrentIndex(di);
	int pi = cbPharmacy->findData(pharmacyId);
	if (pi >= 0) cbPharmacy->setCurrentIndex(pi);
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


