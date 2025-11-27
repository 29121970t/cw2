#include "BaseDialog.h"
#include <memory>

BaseDialog::BaseDialog(const QString &title, QWidget *parent)
	: QDialog(parent)
{
	auto layout = std::make_unique<QFormLayout>();
	formLayout = layout.release();

	auto main = std::make_unique<QVBoxLayout>(this);
	mainLayout = main.release();

	auto buttons = std::make_unique<QDialogButtonBox>(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	buttonBox = buttons.release();

	setWindowTitle(title);
	setupButtons();
}

void BaseDialog::setupButtons() const
{
	connect(buttonBox, &QDialogButtonBox::accepted, this, &BaseDialog::onAccept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void BaseDialog::setupLayout()
{
	mainLayout->addLayout(formLayout);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);
}

