#include "MapWebView.h"
#include <QVBoxLayout>
#include <QUrl>
#include <QByteArray>
#include <QProcessEnvironment>
#include <QTimer>
#include <QVariant>
#include <QApplication>
#include <QFile>

#include <QStackedWidget>

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
	using QObject::QObject;
public slots:
	void pick(double la, double lo) {
		emit picked(la, lo);
	}
signals:
	void picked(double latitude, double longitude);
};

MapWebView::MapWebView(QWidget *parent)
	: QWidget(parent), web(
		new QWebEngineView{this}),
	stack(new QStackedWidget(this))
{
	stack->addWidget(web);
	auto v = new QVBoxLayout();
	v->setContentsMargins(0,0,0,0);
	v->addWidget(stack, 1);
	setLayout(v);
}

MapWebView::~MapWebView()
{
	if (web) {
		web->stop();
		web->page()->setUrl(QUrl("about:blank"));
		QApplication::processEvents();
	}
}

void MapWebView::setLocation(double latitude, double longitude)
{
	if (const auto apiKey = QProcessEnvironment::systemEnvironment().value("GOOGLE_MAPS_API_KEY"); apiKey.isEmpty()) {
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
		channel = new QWebChannel(this);
		auto *bridge = new MapBridge(this);
		connect(bridge, &MapBridge::picked, this, [this](double la, double lo){
			 emit coordinatePicked(la, lo);
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
		channel = new QWebChannel(this);
		auto *bridge = new MapBridge(this);
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


