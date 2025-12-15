#pragma once

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QStackedWidget>
#include <QWebChannel>
#include <QWebEngineView>
#include <QWidget>
#include "MapErropView.h"

namespace Widgets {


class MapWebView : public QWidget {
    Q_OBJECT
   public:
    explicit MapWebView(QWidget* parent = nullptr);
    ~MapWebView() override;
    void setLocation(double latitude, double longitude);
    void setPickMode(bool enabled) {
        pickMode = enabled;
        errorPage->setVisiblePickers(enabled);
        loadInteractive();
    }

   signals:
    void coordinatePicked(double latitude, double longitude);

   private:
    QWebEngineView* web = nullptr;
    QWebChannel* channel = nullptr;
    QStackedWidget* stack = nullptr;
    MapErropView* errorPage = nullptr;

    double lat = 0.0;
    double lon = 0.0;
    bool pickMode = false;
    void loadInteractive();
    void loadErrorPage(const QString& message);
};

}  // namespace Widgets
