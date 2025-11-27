#pragma once

#include "BaseDialog.h"
#include <QComboBox>
#include <QDoubleSpinBox>
#include "../models/Entities.h"
#include "../models/Repository.h"

class StockDialog : public BaseDialog {
	Q_OBJECT
public:
	explicit StockDialog(QWidget *parent = nullptr);

	void setInitial(quint32 drugId, quint32 pharmacyId, double price, bool allowChangeDrug);
	Models::Stock value() const;

protected slots:
	void onAccept() override;

private:
	QComboBox *cbDrug;
	QComboBox *cbPharmacy;
	QDoubleSpinBox *spPrice;
};


