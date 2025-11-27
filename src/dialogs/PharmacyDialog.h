#pragma once

#include "BaseDialog.h"
#include <QLineEdit>
#include "../models/Entities.h"
#include "../models/Repository.h"
#include "../widgets/ScheduleTable.h"
#include "../widgets/MapWebView.h"
#include <QNetworkAccessManager>
#include <QTimer>

class PharmacyDialog : public BaseDialog {
	Q_OBJECT
public:
	explicit PharmacyDialog(QWidget *parent = nullptr);
	void setValue(const Models::Pharmacy &p);
	Models::Pharmacy value() const;
	// Clear all fields for creating a new pharmacy
	void reset();

protected slots:
	void onAccept() override;

private slots:
	void onAddressDebounced();
	void reverseGeocode(double la, double lo);
	void forwardGeocode(const QString &addr);
	void geolocateByIpIfNeeded();

private:
	QLineEdit *eName;
	QLineEdit *eAddress;
	QLineEdit *ePhone;
	Widgets::ScheduleTable *schedule;
	Widgets::MapWebView *mapPicker;
	double lat = 0.0;
	double lon = 0.0;
	QNetworkAccessManager *net;
	QTimer *addressDebounce;
};


