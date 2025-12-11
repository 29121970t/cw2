#include "PharmacyDialog.h"
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QTimeEdit>
#include <QLabel>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>
#include "../core/ServiceLocator.h"


PharmacyDialog::PharmacyDialog(QWidget *parent)
	: BaseDialog(tr("Аптека"), parent),
	  eName(new QLineEdit(this)),
	  eAddress(new QLineEdit(this)),
	  ePhone(new QLineEdit(this)),
	  schedule(new Widgets::ScheduleTable(this)),
	  mapPicker(new Widgets::MapWebView(this)),
	  net(Core::ServiceLocator::get<QNetworkAccessManager>()),
	  addressDebounce(new QTimer(this))
{
	setMinimumSize(900, 820);
	eName->setMaxLength(128);
	eAddress->setMaxLength(160);
	ePhone->setMaxLength(24);
	eName->setPlaceholderText(tr("Название аптеки"));
	eAddress->setPlaceholderText(tr("Город, улица, дом"));
	ePhone->setPlaceholderText(tr("+375 (xx) xxx-xx-xx"));
	ePhone->setValidator(new QRegularExpressionValidator(QRegularExpression("^[+0-9 ()-]{6,20}$"), ePhone));

	getFormLayout()->addRow(tr("Название"), eName);
	getFormLayout()->addRow(tr("Адрес"), eAddress);
	getFormLayout()->addRow(tr("Телефон"), ePhone);
	schedule->setMaximumWidth(320);
	schedule->setMaximumHeight(220);
	getFormLayout()->addRow(tr("Часы работы"), schedule);
	auto mapLabel = new QLabel(tr("Расположение (Google Maps): кликните по карте, чтобы выбрать"), this);
	mapLabel->setWordWrap(true);
	getFormLayout()->addRow(mapLabel);
	mapPicker->setMinimumHeight(320);
	mapPicker->setPickMode(true);
	connect(mapPicker, &Widgets::MapWebView::coordinatePicked, this, [this](double la, double lo){
		lat = la; lon = lo;
		reverseGeocode(la, lo);
	});
	getFormLayout()->addRow(mapPicker);

	setupLayout();

	// Debounced forward geocoding from address
	addressDebounce->setInterval(700);
	addressDebounce->setSingleShot(true);
	connect(addressDebounce, &QTimer::timeout, this, &PharmacyDialog::onAddressDebounced);
	connect(eAddress, &QLineEdit::textEdited, this, [this](){ addressDebounce->start(); });

	// Try to detect approximate user location for new pharmacies
	QTimer::singleShot(0, this, &PharmacyDialog::geolocateByIpIfNeeded);
}

void PharmacyDialog::setValue(const Models::Pharmacy &p)
{
	eName->setText(p.name);
	eAddress->setText(p.address);
	ePhone->setText(p.phone);
	lat = p.latitude; lon = p.longitude;
	mapPicker->setLocation(lat, lon);
	schedule->setHours(p.hours);
}

Models::Pharmacy PharmacyDialog::value() const
{
	Models::Pharmacy p;
	p.name = eName->text();
	p.address = eAddress->text();
	p.phone = ePhone->text();
	p.latitude = lat;
	p.longitude = lon;
	p.hours = schedule->hours();
	return p;
}

void PharmacyDialog::reset()
{
	eName->clear();
	eAddress->clear();
	ePhone->clear();
	lat = 0.0;
	lon = 0.0;
	mapPicker->setLocation(lat, lon);
	// reset schedule to defaults 08:00-22:00
	QVector<QPair<QTime,QTime>> h(7, {QTime(8,0), QTime(22,0)});
	schedule->setHours(h);
}

void PharmacyDialog::onAccept()
{
	if (eName->text().trimmed().isEmpty() ||
	    eAddress->text().trimmed().isEmpty() ||
	    ePhone->text().trimmed().isEmpty()) {
		QMessageBox::warning(this, tr("Проверка ввода"), tr("Название, адрес и телефон обязательны."));
		return;
	}
	if (!ePhone->hasAcceptableInput()) {
		QMessageBox::warning(this, tr("Проверка ввода"), tr("Телефон имеет неверный формат."));
		return;
	}
	// Validate schedule: open < close
	for (int i=0;i<7;++i) {
		const auto *e1 = qobject_cast<QTimeEdit*>(schedule->cellWidget(i,1));
		const auto *e2 = qobject_cast<QTimeEdit*>(schedule->cellWidget(i,2));
		if (!e1 || !e2) continue;
		if (!(e1->time() < e2->time())) {
			QMessageBox::warning(this, tr("Проверка ввода"), tr("Для всех дней время открытия должно быть меньше времени закрытия."));
			return;
		}
	}
	accept();
}

void PharmacyDialog::onAddressDebounced()
{
	const QString addr = eAddress->text().trimmed();
	if (addr.isEmpty()) return;
	forwardGeocode(addr);
}

void PharmacyDialog::reverseGeocode(double la, double lo)
{
	const QString key = QProcessEnvironment::systemEnvironment().value("GOOGLE_MAPS_API_KEY");
	if (key.isEmpty()) return;
	QUrl url("https://maps.googleapis.com/maps/api/geocode/json");
	QUrlQuery q; q.addQueryItem("latlng", QString::number(la,'f',6)+","+QString::number(lo,'f',6));
	q.addQueryItem("language", "ru");
	q.addQueryItem("key", key);
	url.setQuery(q);
	QNetworkRequest req(url);
	auto *reply = net->get(req);
	connect(reply, &QNetworkReply::finished, this, [this, reply](){
		const QByteArray data = reply->readAll();
		reply->deleteLater();
		QJsonDocument doc = QJsonDocument::fromJson(data);
		if (!doc.isObject()) return;
		QJsonArray results = doc.object().value("results").toArray();
		if (results.isEmpty()) return;
		const QString formatted = results.first().toObject().value("formatted_address").toString();
		if (!formatted.isEmpty() && eAddress->text() != formatted) {
			eAddress->setText(formatted);
		}
	});
}

void PharmacyDialog::forwardGeocode(const QString &addr)
{
	const QString key = QProcessEnvironment::systemEnvironment().value("GOOGLE_MAPS_API_KEY");
	if (key.isEmpty()) return;
	QUrl url("https://maps.googleapis.com/maps/api/geocode/json");
	QUrlQuery q; q.addQueryItem("address", addr); q.addQueryItem("language", "ru"); q.addQueryItem("key", key);
	url.setQuery(q);
	QNetworkRequest req(url);
	auto *reply = net->get(req);
	connect(reply, &QNetworkReply::finished, this, [this, reply](){
		const QByteArray data = reply->readAll();
		reply->deleteLater();
		QJsonDocument doc = QJsonDocument::fromJson(data);
		if (!doc.isObject()) return;
		QJsonArray results = doc.object().value("results").toArray();
		if (results.isEmpty()) return;
		auto loc = results.first().toObject().value("geometry").toObject().value("location").toObject();
		double la = loc.value("lat").toDouble();
		double lo = loc.value("lng").toDouble();
		if (la != 0.0 || lo != 0.0) {
			lat = la; lon = lo;
			mapPicker->setLocation(lat, lon);
		}
	});
}

void PharmacyDialog::geolocateByIpIfNeeded()
{
	if (lat != 0.0 || lon != 0.0) return; // already set (edit mode or prefilled)
	const QString key = QProcessEnvironment::systemEnvironment().value("GOOGLE_MAPS_API_KEY");
	if (key.isEmpty()) return;

	QUrl url(QString("https://www.googleapis.com/geolocation/v1/geolocate?key=%1").arg(key));
	QNetworkRequest req(url);
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	QByteArray body("{\"considerIp\": true}");
	auto *reply = net->post(req, body);
	connect(reply, &QNetworkReply::finished, this, [this, reply](){
		const QByteArray data = reply->readAll();
		reply->deleteLater();
		QJsonDocument doc = QJsonDocument::fromJson(data);
		if (!doc.isObject()) return;
		QJsonObject loc = doc.object().value("location").toObject();
		if (loc.isEmpty()) return;
		double la = loc.value("lat").toDouble();
		double lo = loc.value("lng").toDouble();
		if (la == 0.0 && lo == 0.0) return;
		lat = la; lon = lo;
		mapPicker->setLocation(lat, lon);
		reverseGeocode(lat, lon);
	});
}


