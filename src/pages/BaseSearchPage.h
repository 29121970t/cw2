#pragma once

#include "BaseTablePage.h"
#include <QLineEdit>
#include <QComboBox>
#include "../models/Repository.h"
#include "../core/ServiceLocator.h"

class BaseSearchPage : public BaseTablePage {
	Q_OBJECT
public:
	explicit BaseSearchPage(QWidget *parent = nullptr);

	virtual void setInitialFilter(const QString &text) const;

protected:
	// Common search setup
	void setupSearch();
	
	// Accessors
	QLineEdit* getSearchEdit() const { return searchEdit; }
	QComboBox* getModeCombo() const { return modeCombo; }
	Models::Repository* getRepository() const { return repo; }

protected slots:
	virtual void filterChanged(const QString &text) = 0;
	virtual void modeChanged(int index) = 0;

private:
	Models::Repository *repo = nullptr;
	QComboBox *modeCombo = nullptr;
	QLineEdit *searchEdit = nullptr;
};

