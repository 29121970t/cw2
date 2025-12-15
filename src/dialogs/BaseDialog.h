#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

class BaseDialog : public QDialog {
    Q_OBJECT
   public:
    explicit BaseDialog(const QString& title, QWidget* parent = nullptr);

   protected:
    void setupButtons() const;
    void setupLayout();

    QFormLayout* getFormLayout() const { return formLayout; }
    QVBoxLayout* getMainLayout() const { return mainLayout; }
    QDialogButtonBox* getButtonBox() const { return buttonBox; }

   protected slots:
    virtual void onAccept() = 0;

   private:
    QFormLayout* formLayout = nullptr;
    QVBoxLayout* mainLayout = nullptr;
    QDialogButtonBox* buttonBox = nullptr;
};
