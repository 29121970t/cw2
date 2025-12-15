#pragma once

#include <QStandardItemModel>
#include <QTableView>
#include <QWidget>

#include "../widgets/ActionButtonsDelegate.h"

class BaseTablePage : public QWidget {
    Q_OBJECT
   public:
    explicit BaseTablePage(QWidget* parent = nullptr);

   protected:
    void setupTable();
    void setupActionsDelegate();
    void applyActionsDelegateToLastColumn();

    bool eventFilter(QObject* obj, QEvent* event) override;

    void selectRow(int row);

   public:
    QTableView* getTable() const { return table; }
    QStandardItemModel* getModel() const { return model; }
    Widgets::ActionButtonsDelegate* getActionsDelegate() const { return actionsDelegate; }

   protected slots:
    virtual void onRowAdd(int row) = 0;
    virtual void onRowEdit(int row) = 0;
    virtual void onRowDelete(int row) = 0;

   private:
    QTableView* table = nullptr;
    QStandardItemModel* model = nullptr;
    Widgets::ActionButtonsDelegate* actionsDelegate = nullptr;
};
