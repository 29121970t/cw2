#include <QApplication>
#include <QUrl>
#include <QWebEngineView>
#include <QNetworkAccessManager>
#include <QFile>
#include <QDataStream>
#include <QDir>
#include <QStandardPaths>
#include "MainWindow.h"
#include "core/ServiceLocator.h"
#include "models/DrugRepository.h"
#include "models/PharmacyRepository.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("Информационная система лекарственных препаратов");
	QApplication::setOrganizationName("CourseProject");

	
	QApplication::setStyle("Fusion");
	{
		const auto bg        = QColor(30, 32, 34);     
		const auto panel     = QColor(40, 42, 46);     
		const auto base      = QColor(36, 38, 41);     
		const auto text      = QColor(235, 235, 235);  
		const auto disabled  = QColor(130, 130, 130);
		const auto accent    = QColor(34, 197, 94);    
		const auto accentD   = QColor(22, 160, 78);

		auto palette = QPalette{};
		palette.setColor(QPalette::Window, bg);
		palette.setColor(QPalette::WindowText, text);
		palette.setColor(QPalette::Base, base);
		palette.setColor(QPalette::AlternateBase, panel);
		palette.setColor(QPalette::ToolTipBase, panel);
		palette.setColor(QPalette::ToolTipText, text);
		palette.setColor(QPalette::Text, text);
		palette.setColor(QPalette::Button, panel);
		palette.setColor(QPalette::ButtonText, text);
		palette.setColor(QPalette::Highlight, accent);
		palette.setColor(QPalette::HighlightedText, QColor(0,0,0));
		palette.setColor(QPalette::BrightText, text);
		palette.setColor(QPalette::PlaceholderText, disabled);
		palette.setColor(QPalette::Link, accent);
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
		palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
		palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
		QApplication::setPalette(palette);

		
		const auto css = QString(
			"QWidget { background-color:%1; color:%2; }"
			"QLabel { background: transparent; }"
			"QGroupBox, QToolBar { background-color:%3; }"
			"QLineEdit, QComboBox, QTextEdit, QPlainTextEdit, QSpinBox, QDoubleSpinBox, QTimeEdit {"
			"  background-color:%4; border:1px solid #3a3d41; border-radius:8px; padding:4px 6px;"
			"}"
			"QAbstractSpinBox::up-button, QAbstractSpinBox::down-button {"
			"  subcontrol-origin: padding; width:18px; border:none; background:transparent; margin:1px;"
			"}"
			"QAbstractSpinBox::up-arrow { image:url(:/icons/icons/chevron-up.svg); }"
			"QAbstractSpinBox::down-arrow { image:url(:/icons/icons/chevron-down.svg); }"
			"QComboBox QAbstractItemView {"
			"  background-color:%3; color:%2; border:1px solid #3a3d41; border-radius:8px; padding:4px;"
			"}"
			"QPushButton { background-color:%3; border:1px solid #3a3d41; border-radius:6px; padding:6px 10px; }"
			"QPushButton:hover { border-color:%5; }"
			"QPushButton:pressed { background-color:%4; }"
			"QPushButton[accent=\"true\"], .accent { background-color:%5; color:#000; border:1px solid %6; }"
			"QMenu { background-color:%3; color:%2; border:1px solid #3a3d41; border-radius:8px; }"
			"QHeaderView::section { background:%3; color:%2; padding:6px; border:0px; border-bottom:1px solid #3a3d41; }"
			"QTableView { gridline-color:#3a3d41; selection-background-color:%5; selection-color:#000; }"
			"QToolTip { background-color:%3; color:%2; border:1px solid #3a3d41; }"
		).arg(bg.name(), text.name(), panel.name(), base.name(), accent.name(), accentD.name());
		app.setStyleSheet(css);
	}

	
	{
		auto prewarm = QWebEngineView{};
		prewarm.setAttribute(Qt::WA_DontShowOnScreen, true);
		prewarm.resize(1, 1);
		prewarm.load(QUrl("about:blank"));
		
		QApplication::processEvents();
	}

	
	{
		
		auto migrateFromOldFile = []() -> bool {
			const auto base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
			const QString oldFile = base + QDir::separator() + "drug_system.bin";
			QFile f(oldFile);
			if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
				return false;
			}
			QDataStream in(&f);
			in.setVersion(QDataStream::Qt_6_0);
			quint32 version = 0;
			in >> version;
			if (version != 2 && version != 1) {
				return false;
			}
			
			
			quint32 drugCount = 0;
			in >> drugCount;
			QVector<Models::Drug> drugs;
			drugs.reserve(drugCount);
			for (quint32 i = 0; i < drugCount; ++i) {
				Models::Drug d;
				if (version == 1) {
					in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
					   >> d.dosageForm >> d.country >> d.prescriptionRequired;
					double legacyMin = 0;
					double legacyMax = 0;
					in >> legacyMin >> legacyMax;
				} else {
					in >> d.id >> d.tradeName >> d.medicalName >> d.manufacturer
					   >> d.dosageForm >> d.country >> d.prescriptionRequired;
				}
				drugs.push_back(d);
			}
			
			
			quint32 pharmacyCount = 0;
			in >> pharmacyCount;
			QVector<Models::Pharmacy> pharmacies;
			pharmacies.reserve(pharmacyCount);
			for (quint32 i = 0; i < pharmacyCount; ++i) {
				Models::Pharmacy p;
				in >> p;
				pharmacies.push_back(p);
			}
			
			
			quint32 stockCount = 0;
			in >> stockCount;
			QVector<Models::Stock> stocks;
			stocks.reserve(stockCount);
			for (quint32 i = 0; i < stockCount; ++i) {
				Models::Stock s;
				in >> s;
				stocks.push_back(s);
			}
			
			
			auto drugRepo = std::make_shared<Models::DrugRepository>();
			for (const auto &d : drugs) {
				drugRepo->addDrug(d);
			}
			drugRepo->save();
			
			auto pharmacyRepo = std::make_shared<Models::PharmacyRepository>();
			for (const auto &p : pharmacies) {
				pharmacyRepo->addPharmacy(p);
			}
			for (const auto &s : stocks) {
				pharmacyRepo->setStock(s.pharmacyId, s.drugId, s.price);
			}
			pharmacyRepo->save();
			
			
			f.close();
			QFile::rename(oldFile, oldFile + ".migrated");
			return true;
		};
		
		auto drugRepo = std::make_shared<Models::DrugRepository>();
		auto pharmacyRepo = std::make_shared<Models::PharmacyRepository>();
		
		
		bool drugLoaded = drugRepo->load();
		bool pharmacyLoaded = pharmacyRepo->load();
		
		if (!drugLoaded || !pharmacyLoaded) {
			
			if (migrateFromOldFile()) {
				drugLoaded = drugRepo->load();
				pharmacyLoaded = pharmacyRepo->load();
			}
		}
		
		if (!drugLoaded) {
			drugRepo->seedSampleData();
			drugRepo->save();
		}
		if (!pharmacyLoaded) {
			pharmacyRepo->seedSampleData(drugRepo->allDrugs());
			pharmacyRepo->save();
		}
		
		Core::ServiceLocator::registerService<Models::DrugRepository>(drugRepo);
		Core::ServiceLocator::registerService<Models::PharmacyRepository>(pharmacyRepo);
		
		auto net = std::make_shared<QNetworkAccessManager>();
		Core::ServiceLocator::registerService<QNetworkAccessManager>(net);
	}

	auto window = MainWindow{};
	window.resize(1200, 720);
	window.show();
	
	const auto result = QApplication::exec();
	
	
	QApplication::processEvents();
	
	return result;
}


