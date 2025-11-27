#include "MapWebView.h"
#include <QVBoxLayout>
#include <QUrl>
#include <QByteArray>
#include <QProcessEnvironment>
#include <QTimer>
#include <QVariant>
#include <QApplication>

namespace Widgets {

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
	: QWidget(parent), web(new QWebEngineView(this))
{
	auto v = new QVBoxLayout;
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
	// Simple embedded map that works without API key
	const QString html = QString(
		"<!doctype html><html><head><meta charset='utf-8'>"
		"<style>"
		"html,body,#wrap{height:100%%;margin:0;padding:0;}"
		"#wrap{position:relative;}"
		"#map{position:absolute;top:0;left:0;right:0;bottom:0;border:0;width:100%%;height:100%%;}"
		"</style></head>"
		"<body><div id='wrap'><iframe id='map' frameborder='0' loading='lazy' "
		"src='https://maps.google.com/maps?q=%1,%2&z=15&output=embed' allowfullscreen></iframe></body></html>"
	).arg(QString::number(lat, 'f', 6), QString::number(lon, 'f', 6));
	web->setHtml(html, QUrl("https://maps.google.com/"));
}

void MapWebView::loadErrorPage(const QString &message)
{
	const QString inputsHtml = pickMode
		? QString("<div class='muted' style='margin-bottom:10px; font-size:13px;'>Введите координаты вручную (по умолчанию показаны текущие/стандартные):</div>"
		          "<div><input id='lat' placeholder='Широта' type='number' step='0.000001' value='%1'>"
		          "<input id='lon' placeholder='Долгота' type='number' step='0.000001' value='%2'>"
		          "<button onclick='applyCoords()'>Применить</button></div>")
		      .arg(QString::number(lat,'f',6), QString::number(lon,'f',6))
		: QString();
	const QString html = QString(
		"<!doctype html><html><head><meta charset='utf-8'>"
		"<style>"
		"html,body{height:100%%;margin:0;padding:0;background:#1e2022;color:#e5e7eb;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif}"
		"#error{display:flex;height:100%%;align-items:center;justify-content:center;text-align:center;padding:16px;}"
		".muted{opacity:.8}"
		"input{background:#242628;border:1px solid #3a3d41;border-radius:8px;color:#e5e7eb;padding:6px 8px;margin:0 4px;width:140px;outline:none}"
		"button{background:#22c55e;border:1px solid #16a04e;border-radius:8px;color:#000;padding:8px 12px;margin-left:8px;cursor:pointer}"
		"button:active{transform:translateY(1px)}"
		"</style></head>"
		"<body><div id='error'><div>"
		"<div style='font-size:13px;margin-bottom:8px;'>%1</div>"
		"%2"
		"</div></div>"
		"<script>"
		"function applyCoords(){var la=parseFloat(document.getElementById('lat').value);"
		"var lo=parseFloat(document.getElementById('lon').value);"
		"if(isNaN(la)||isNaN(lo)) return; if(window.QWebChannel){new QWebChannel(qt.webChannelTransport,function(ch){if(ch.objects.bridge){ch.objects.bridge.pick(la,lo);}});} }"
		"</script>"
		"</body></html>"
	).arg(message.toHtmlEscaped(), inputsHtml);
	if (pickMode && !channel) {
		channel = new QWebChannel(this);
		auto *bridge = new MapBridge(this);
		connect(bridge, &MapBridge::picked, this, [this](double la, double lo){
			lat = la; lon = lo; emit coordinatePicked(la, lo);
		});
		channel->registerObject("bridge", bridge);
		web->page()->setWebChannel(channel);
	}
	web->setHtml(html, QUrl("about:blank"));
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
	const QString html = QString(
		"<!doctype html><html><head><meta charset='utf-8'>"
		"<style>"
		"html,body{height:100vh;width:100vw;margin:0;padding:0;overflow:hidden;background:#1e2022;color:#e5e7eb;font-family:system-ui,-apple-system,Segoe UI,Roboto,sans-serif}"
		"#container{position:relative;height:100vh;width:100vw;}"
		"#map{position:absolute;top:0;left:0;right:0;bottom:0;}"
		"#error{display:none;position:absolute;inset:0;align-items:center;justify-content:center;text-align:center;padding:16px;}"
		"#error.visible{display:flex;}"
		".muted{opacity:.8}"
		"</style>"
		"%5"
		"<script>"
		"var map, marker, pickMode=%4;"
		"function init(){var c={lat:%2,lng:%3};"
		"try{"
		"map=new google.maps.Map(document.getElementById('map'),{zoom:15,center:c,gestureHandling:'greedy',streetViewControl:false});"
		"marker=new google.maps.Marker({position:c,map:map});"
		"if(pickMode && window.QWebChannel){new QWebChannel(qt.webChannelTransport,function(ch){window.bridge=ch.objects.bridge;});"
		"map.addListener('click',function(e){marker.setPosition(e.latLng); if(window.bridge){bridge.pick(e.latLng.lat(),e.latLng.lng());}});} "
		"}catch(e){showError('Не удалось инициализировать Google Maps. '+e.message);} }"
		"function showError(msg){document.getElementById('map').style.display='none';var el=document.getElementById('error');el.className='visible';"
		"var inputs = pickMode ? '<div><input id=\"lat\" placeholder=\"Широта\" type=\"number\" step=\"0.000001\">"
		"<input id=\"lon\" placeholder=\"Долгота\" type=\"number\" step=\"0.000001\">"
		"<button onclick=\"applyCoords()\">Применить</button></div>' : '';"
		"el.innerHTML='<div><div style=\"font-size:15px;margin-bottom:8px;\">'+msg+'</div>"
		"<div class=\"muted\" style=\"margin-bottom:10px;\">Проверьте GOOGLE_MAPS_API_KEY или сетевое соединение.</div>'+inputs+'</div>';}"
		"function applyCoords(){var la=parseFloat(document.getElementById('lat').value);var lo=parseFloat(document.getElementById('lon').value);"
		"if(isNaN(la)||isNaN(lo)) return; if(window.QWebChannel){new QWebChannel(qt.webChannelTransport,function(ch){if(ch.objects.bridge){ch.objects.bridge.pick(la,lo);}});} }"
		"window.gm_authFailure=function(){showError('Ошибка аутентификации Google Maps API: неверный или ограниченный ключ.');};"
		"</script>"
		"<script async defer src='https://maps.googleapis.com/maps/api/js?key=%1&callback=init'></script>"
		"</head><body><div id='container'><div id='map'></div><div id='error'></div></div></body></html>"
	).arg(apiKey, QString::number(lat,'f',6), QString::number(lon,'f',6), pickMode ? "true" : "false",
	     pickMode ? "<script src='qrc:///qtwebchannel/qwebchannel.js'></script>" : "");
	web->setHtml(html, QUrl("about:blank"));
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


