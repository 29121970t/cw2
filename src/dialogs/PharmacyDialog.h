#pragma once

#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QTimer>

#include "../models/Entities.h"
#include "../widgets/MapWebView.h"
#include "../widgets/ScheduleTable.h"
#include "BaseDialog.h"

class PharmacyDialog : public BaseDialog {
    Q_OBJECT
   public:
    explicit PharmacyDialog(QWidget* parent = nullptr);
    void setValue(const Models::Pharmacy& p);
    Models::Pharmacy value() const;
    // Clear all fields for creating a new pharmacy
    void reset();

   protected slots:
    void onAccept() override;

   private slots:
    void onAddressDebounced();
    void reverseGeocode(double la, double lo);
    void forwardGeocode(const QString& addr);
    void geolocateByIpIfNeeded();

   private:
    QLineEdit* eName = nullptr;
    QLineEdit* eAddress = nullptr;
    QLineEdit* ePhone = nullptr;
    Widgets::ScheduleTable* schedule = nullptr;
    Widgets::MapWebView* mapPicker = nullptr;
    double lat = 0.0;
    double lon = 0.0;
    QNetworkAccessManager* net = nullptr;
    QTimer* addressDebounce = nullptr;
};
