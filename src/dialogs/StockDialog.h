#pragma once

#include <QComboBox>
#include <QDoubleSpinBox>

#include "../models/Entities.h"
#include "BaseDialog.h"

class StockDialog : public BaseDialog {
    Q_OBJECT
   public:
    explicit StockDialog(QWidget* parent = nullptr);

    void setInitial(qint64 drugId, quint32 pharmacyId, double price, bool allowChangeDrug);
    Models::Stock value() const;

   protected slots:
    void onAccept() override;

   private:
    QComboBox* cbDrug = nullptr;
    QComboBox* cbPharmacy = nullptr;
    QDoubleSpinBox* spPrice = nullptr;
};
