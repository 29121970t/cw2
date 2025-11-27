#include "BaseSearchPage.h"

BaseSearchPage::BaseSearchPage(QWidget *parent)
	: BaseTablePage(parent),
	  repo(Core::ServiceLocator::get<Models::Repository>()),
	  modeCombo(new QComboBox(this)),
	  searchEdit(new QLineEdit(this))
{
}

void BaseSearchPage::setupSearch()
{
	connect(searchEdit, &QLineEdit::textChanged, this, &BaseSearchPage::filterChanged);
	connect(modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &BaseSearchPage::modeChanged);
}

void BaseSearchPage::setInitialFilter(const QString &text)
{
	searchEdit->setText(text);
}

