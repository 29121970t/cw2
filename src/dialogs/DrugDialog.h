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
	QLineEdit *eTrade;
	QLineEdit *eMedical;
	QLineEdit *eManufacturer;
	QLineEdit *eForm;
	QLineEdit *eCountry;
	QCheckBox *cbRx;
};


