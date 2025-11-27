#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QFormLayout>

class BaseDialog : public QDialog {
	Q_OBJECT
public:
	explicit BaseDialog(const QString &title, QWidget *parent = nullptr);

protected:
	// Common dialog setup
	void setupButtons() const;
	void setupLayout();
	
	// Accessors
	QFormLayout* getFormLayout() const { return formLayout; }
	QVBoxLayout* getMainLayout() const { return mainLayout; }
	QDialogButtonBox* getButtonBox() const { return buttonBox; }

protected slots:
	virtual void onAccept() = 0;

private:
	QFormLayout *formLayout = nullptr;
	QVBoxLayout *mainLayout = nullptr;
	QDialogButtonBox *buttonBox = nullptr;
};

