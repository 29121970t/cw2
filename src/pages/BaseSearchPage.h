#pragma once

#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

#include "BaseTablePage.h"
class BaseSearchPage : public BaseTablePage {
    Q_OBJECT
   public:
    explicit BaseSearchPage(QWidget* parent = nullptr);

    virtual void setInitialFilter(const QString& text) const;
    bool getAllowAdd() const { return allowAdd; }
	void setAllowAdd(bool val)  { allowAdd = val; addButton->setVisible(val);}
   protected:
    // Common search setup
    void setupSearch() const;

    // Accessors
    QLineEdit* getSearchEdit() const { return searchEdit; }
    QComboBox* getModeCombo() const { return modeCombo; }
    QPushButton* getAddButton() const { return addButton; }
	


   protected slots:
    virtual void filterChanged(const QString& text) = 0;
    virtual void modeChanged(int index) = 0;
    virtual void addElement() = 0;


   private:
    QComboBox* modeCombo = nullptr;
    QLineEdit* searchEdit = nullptr;
    QPushButton* addButton = nullptr;
    bool allowAdd = false;
};
