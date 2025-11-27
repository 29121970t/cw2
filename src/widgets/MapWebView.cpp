#include "MapWebView.h"
#include <QVBoxLayout>
#include <QUrl>
#include <QByteArray>
#include <QProcessEnvironment>
#include <QTimer>
#include <QVariant>
#include <QApplication>
#include <QFile>
#include "../utils/QtHelpers.h"

namespace Widgets {

namespace {

QString loadTemplate(const QString &path)
{
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		return {};
	}
	return QString::fromUtf8(file.readAll());
}

} // namespace

class MapBridge : public QObject {
	Q_OBJECT
public:
	explicit MapBridge(QObject *parent=nullptr) : QObject(parent) {}
public slots:
	void pick(double la, double lo) {
		emit picked(la, lo);
	}
signals:
	void picked(double latitude, double longitude);
};

MapWebView::MapWebView(QWidget *parent)
	: QWidget(parent), web(Utils::QtHelpers::makeOwned<QWebEngineView>(this))
{
	auto v = Utils::QtHelpers::makeOwned<QVBoxLayout>();
	v->setContentsMargins(0,0,0,0);
	v->addWidget(web, 1);
	setLayout(v);
}

MapWebView::~MapWebView()
{
	// Explicitly close the web view before destruction to avoid OpenGL context warnings
	if (web) {
		// Stop loading and clear the page
		web->stop();
		web->page()->setUrl(QUrl("about:blank"));
		// Process events to ensure cleanup happens
		QApplication::processEvents();
		// The web view will be automatically deleted as a child widget
	}
}

void MapWebView::setLocation(double latitude, double longitude)
{
	lat = latitude; lon = longitude;
	const QString apiKey = QProcessEnvironment::systemEnvironment().value("GOOGLE_MAPS_API_KEY");
	if (apiKey.isEmpty()) {
		loadErrorPage(QString::fromUtf8("Ключ Google Maps не указан. Установите переменную окружения GOOGLE_MAPS_API_KEY. "
		                                "Карта не будет загружена без действительного ключа."));
		return;
	}
	loadInteractive();
}

void MapWebView::loadEmbedded()
{
	const QString tpl = loadTemplate(QStringLiteral(":/map/embedded.html"));
	if (tpl.isEmpty()) {
		return;
	}
	const QString html = tpl.arg(QString::number(lat, 'f', 6),
	                             QString::number(lon, 'f', 6));
	web->setHtml(html, QUrl("qrc:/map/"));
}

void MapWebView::loadErrorPage(const QString &message)
{
	const QString tpl = loadTemplate(QStringLiteral(":/map/error.html"));
	if (tpl.isEmpty()) {
		return;
	}
	const QString html = tpl.arg(message.toHtmlEscaped(),
	                             pickMode ? QStringLiteral("true") : QStringLiteral("false"),
	                             QString::number(lat, 'f', 6),
	                             QString::number(lon, 'f', 6));
	if (pickMode && !channel) {
		channel = Utils::QtHelpers::makeOwned<QWebChannel>(this);
		auto *bridge = Utils::QtHelpers::makeOwned<MapBridge>(this);
		connect(bridge, &MapBridge::picked, this, [this](double la, double lo){
			lat = la; lon = lo; emit coordinatePicked(la, lo);
		});
		channel->registerObject("bridge", bridge);
		web->page()->setWebChannel(channel);
	}
	web->setHtml(html, QUrl("qrc:/map/"));
}

void MapWebView::loadInteractive()
{
	const QString apiKey = QProcessEnvironment::systemEnvironment().value("GOOGLE_MAPS_API_KEY");
	if (apiKey.isEmpty()) { loadErrorPage(QString::fromUtf8("Ключ Google Maps не указан. Установите GOOGLE_MAPS_API_KEY.")); return; }
	// Setup webchannel bridge for click picking if requested
	if (!channel) {
		channel = Utils::QtHelpers::makeOwned<QWebChannel>(this);
		auto *bridge = Utils::QtHelpers::makeOwned<MapBridge>(this);
		connect(bridge, &MapBridge::picked, this, [this](double la, double lo){
			lat = la; lon = lo; emit coordinatePicked(la, lo);
		});
		channel->registerObject("bridge", bridge);
		web->page()->setWebChannel(channel);
	}
	const QString tpl = loadTemplate(QStringLiteral(":/map/interactive.html"));
	if (tpl.isEmpty()) {
		return;
	}
	const QString html = tpl.arg(apiKey,
	                             QString::number(lat,'f',6),
	                             QString::number(lon,'f',6),
	                             pickMode ? "true" : "false");
	web->setHtml(html, QUrl("qrc:/map/"));
	// Verify API loaded; if failed (bad key/restrictions) display readable message
	connect(web, &QWebEngineView::loadFinished, this, [this](bool ok){
		if (!ok) return;
		QTimer::singleShot(1200, this, [this](){
			if (!web) return;
			web->page()->runJavaScript("!!(window.google && google.maps && typeof google.maps.Map==='function') ? 'ok' : 'fail'",
				[this](const QVariant &v){
					if (v.toString() != "ok") {
						web->page()->runJavaScript("("+QStringLiteral("function(){var el=document.getElementById('error');"
							"if(!el)return; document.getElementById('map').style.display='none';"
							"el.className='visible'; el.innerHTML='<div><div style=\"font-size:15px;margin-bottom:8px;\">Google Maps script failed to load.</div>"
							"<div class=\"muted\">Possible invalid GOOGLE_MAPS_API_KEY or blocked network.</div></div>'; })()")+ ";");
					}
				});
		});
	});
}

} // namespace Widgets

#include "MapWebView.moc"


