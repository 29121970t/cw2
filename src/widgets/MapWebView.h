#pragma once

#include <QWidget>
#include <QWebEngineView>
#include <QWebChannel>

namespace Widgets {

class MapWebView : public QWidget {
	Q_OBJECT
public:
	explicit MapWebView(QWidget *parent = nullptr);
	~MapWebView() override;
	void setLocation(double latitude, double longitude);
	void setPickMode(bool enabled) { pickMode = enabled; loadInteractive(); }

signals:
	void coordinatePicked(double latitude, double longitude);

private:
	QWebEngineView *web;
	QWebChannel *channel = nullptr;
	double lat = 0.0;
	double lon = 0.0;
	bool pickMode = false;
	void loadEmbedded();
	void loadInteractive();
	void loadErrorPage(const QString &message);
};

} // namespace Widgets


