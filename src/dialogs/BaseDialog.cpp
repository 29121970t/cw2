#include "BaseDialog.h"

#include <memory>

BaseDialog::BaseDialog(const QString& title, QWidget* parent)
    : QDialog(parent),
      formLayout(new QFormLayout()),
      mainLayout(new QVBoxLayout()),
      buttonBox(new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this)) {
    setWindowTitle(title);
    setupButtons();
	setupLayout();
}

void BaseDialog::setupButtons() const {
    connect(buttonBox, &QDialogButtonBox::accepted, this, &BaseDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void BaseDialog::setupLayout() {
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);
}
