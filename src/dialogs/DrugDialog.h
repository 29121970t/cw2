#pragma once

#include "BaseDialog.h"
#include <QLineEdit>
#include <QCheckBox>
#include "../models/Entities.h"

class DrugDialog : public BaseDialog {
	Q_OBJECT
public:
	explicit DrugDialog(QWidget *parent = nullptr);

	void setValue(const Models::Drug &d);
	Models::Drug value() const;
	void reset();

protected slots:
	void onAccept() override;

private:
	QLineEdit *eTrade = nullptr;
	QLineEdit *eMedical = nullptr;
	QLineEdit *eManufacturer = nullptr;
	QLineEdit *eForm = nullptr;
	QLineEdit *eCountry = nullptr;
	QCheckBox *cbRx = nullptr;
};


