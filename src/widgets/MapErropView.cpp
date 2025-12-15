#include "MapErropView.h"

namespace Widgets {
MapErropView::MapErropView(/* args */)
    : errorMassage(new QLabel(this)),
      laPicker(new QDoubleSpinBox{this}),
      loPicker(new QDoubleSpinBox{this}),
      pickersLayout(new QFormLayout()) {
    errorMassage->setText("Ошибка сети");
    errorMassage->setAlignment(Qt::AlignCenter);

    loPicker->setRange(-180, 180);
    loPicker->setSingleStep(0.000001);
    loPicker->setDecimals(6);
    loPicker->setMaximumWidth(90);

    laPicker->setRange(-90, 90);
    laPicker->setSingleStep(0.000001);
    laPicker->setDecimals(6);
    laPicker->setMaximumWidth(90);

    auto handleInputChange = [this]() { emit inputChanged(laPicker->value(), loPicker->value()); };
    connect(laPicker, &QDoubleSpinBox::valueChanged, this, handleInputChange);
    connect(loPicker, &QDoubleSpinBox::valueChanged, this, handleInputChange);

    auto h = new QBoxLayout(QBoxLayout::TopToBottom);
    h->setAlignment(Qt::AlignCenter);
    pickersLayout->addRow(errorMassage);
    pickersLayout->addRow(tr("Широта"), laPicker);
    pickersLayout->addRow(tr("Долгота"), loPicker);
    h->addLayout(pickersLayout);
    h->setStretch(1, 1);
    setLayout(h);
}
MapErropView::~MapErropView() {}

void MapErropView::setErrorMassage(const QString& string) { errorMassage->setText(string); }
void MapErropView::setVisiblePickers(bool val) {
    pickersLayout->setRowVisible(1, val);
    pickersLayout->setRowVisible(2, val);
}
void MapErropView::setPickerValues(double la, double lo) {
    laPicker->setValue(la);
    loPicker->setValue(lo);
}
}  // namespace Widgets