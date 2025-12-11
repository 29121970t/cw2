#include "DrugDialog.h"
#include <QMessageBox>


DrugDialog::DrugDialog(QWidget *parent)
	: BaseDialog(tr("Препарат"), parent),
	  eTrade(new QLineEdit(this)),
	  eMedical(new QLineEdit(this)),
	  eManufacturer(new QLineEdit(this)),
	  eForm(new QLineEdit(this)),
	  eCountry(new QLineEdit(this)),
	  cbRx(new QCheckBox(tr("Требуется рецепт"), this))
{
	eTrade->setPlaceholderText(tr("Торговое название"));
	eMedical->setPlaceholderText(tr("Международное непатентованное наименование"));
	eManufacturer->setPlaceholderText(tr("Производитель"));
	eForm->setPlaceholderText(tr("Форма выпуска, дозировка"));
	eCountry->setPlaceholderText(tr("Страна производства"));
	for (auto *le : {eTrade, eMedical, eManufacturer, eForm, eCountry}) {
		le->setMaxLength(128);
	}

	getFormLayout()->addRow(tr("Торговое наименование"), eTrade);
	getFormLayout()->addRow(tr("МНН"), eMedical);
	getFormLayout()->addRow(tr("Производитель"), eManufacturer);
	getFormLayout()->addRow(tr("Форма выпуска"), eForm);
	getFormLayout()->addRow(tr("Страна"), eCountry);
	getFormLayout()->addRow(cbRx);

	setupLayout();
}

void DrugDialog::setValue(const Models::Drug &d)
{
	eTrade->setText(d.tradeName);
	eMedical->setText(d.medicalName);
	eManufacturer->setText(d.manufacturer);
	eForm->setText(d.dosageForm);
	eCountry->setText(d.country);
	cbRx->setChecked(d.prescriptionRequired);
}

Models::Drug DrugDialog::value() const
{
	Models::Drug d;
	d.tradeName = eTrade->text();
	d.medicalName = eMedical->text();
	d.manufacturer = eManufacturer->text();
	d.dosageForm = eForm->text();
	d.country = eCountry->text();
	d.prescriptionRequired = cbRx->isChecked();
	return d;
}

void DrugDialog::reset()
{
	eTrade->clear();
	eMedical->clear();
	eManufacturer->clear();
	eForm->clear();
	eCountry->clear();
	cbRx->setChecked(false);
}

void DrugDialog::onAccept()
{
	if (eTrade->text().trimmed().isEmpty() ||
	    eMedical->text().trimmed().isEmpty() ||
	    eManufacturer->text().trimmed().isEmpty() ||
	    eForm->text().trimmed().isEmpty() ||
	    eCountry->text().trimmed().isEmpty()) {
		QMessageBox::warning(this, tr("Проверка ввода"), tr("Заполните все текстовые поля."));
		return;
	}
	accept();
}


