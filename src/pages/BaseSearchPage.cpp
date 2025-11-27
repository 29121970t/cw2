#include "BaseSearchPage.h"
#include "../core/ServiceLocator.h"
#include "../utils/QtHelpers.h"

BaseSearchPage::BaseSearchPage(QWidget *parent)
	: BaseTablePage(parent),
	  repo(Core::ServiceLocator::get<Models::Repository>()),
	  modeCombo(Utils::QtHelpers::makeOwned<QComboBox>(this)),
	  searchEdit(Utils::QtHelpers::makeOwned<QLineEdit>(this))
{
}

void BaseSearchPage::setupSearch() const
{
	connect(searchEdit, &QLineEdit::textChanged, this, &BaseSearchPage::filterChanged);
	connect(modeCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, &BaseSearchPage::modeChanged);
}

void BaseSearchPage::setInitialFilter(const QString &text) const
{
	searchEdit->setText(text);
}

