#pragma once

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include "../widgets/ActionButtonsDelegate.h"

class BaseTablePage : public QWidget {
	Q_OBJECT
public:
	explicit BaseTablePage(QWidget *parent = nullptr);

protected:
	// Common table setup
	void setupTable();
	void setupActionsDelegate();
	void applyActionsDelegateToLastColumn();
	
	// Common event filtering for hover detection
	bool eventFilter(QObject *obj, QEvent *event) override;
	
	// Common row selection
	void selectRow(int row);
	
	// Accessors
	QTableView* getTable() const { return table; }
	QStandardItemModel* getModel() const { return model; }
	Widgets::ActionButtonsDelegate* getActionsDelegate() const { return actionsDelegate; }

protected slots:
	// Virtual slots for derived classes to implement
	virtual void onRowAdd(int row) = 0;
	virtual void onRowEdit(int row) = 0;
	virtual void onRowDelete(int row) = 0;

protected:
	QTableView *table;
	QStandardItemModel *model;
	Widgets::ActionButtonsDelegate *actionsDelegate;
};

