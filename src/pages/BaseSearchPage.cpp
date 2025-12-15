#include "BaseSearchPage.h"

BaseSearchPage::BaseSearchPage(QWidget *parent)
	: BaseTablePage(parent),
	  modeCombo(new QComboBox(this)),
	  searchEdit(new QLineEdit(this)),
	  addButton(new QPushButton("Добавить", this))
{
	connect(searchEdit, &QLineEdit::textChanged, this, &BaseSearchPage::filterChanged);
	connect(modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &BaseSearchPage::modeChanged);
	connect(addButton, &QPushButton::released, this, &BaseSearchPage::addElement);
}

void BaseSearchPage::setInitialFilter(const QString &text) const
{
	searchEdit->setText(text);
}

