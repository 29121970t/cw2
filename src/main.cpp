#include <QApplication>
#include <QUrl>
#include <QWebEngineView>
#include <QNetworkAccessManager>
#include "MainWindow.h"
#include "core/ServiceLocator.h"
#include "models/Repository.h"

using namespace Models;

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	QApplication::setApplicationName("Информационная система лекарственных препаратов");
	QApplication::setOrganizationName("CourseProject");

	// Apply modern dark theme with green accents
	QApplication::setStyle("Fusion");
	{
		const QColor bg        = QColor(30, 32, 34);     // window background
		const QColor panel     = QColor(40, 42, 46);     // panels/cards
		const QColor base      = QColor(36, 38, 41);     // inputs
		const QColor text      = QColor(235, 235, 235);  // primary text
		const QColor midText   = QColor(200, 200, 200);  // secondary
		const QColor disabled  = QColor(130, 130, 130);
		const QColor accent    = QColor(34, 197, 94);    // green accent (#22c55e)
		const QColor accentD   = QColor(22, 160, 78);

		QPalette p;
		p.setColor(QPalette::Window, bg);
		p.setColor(QPalette::WindowText, text);
		p.setColor(QPalette::Base, base);
		p.setColor(QPalette::AlternateBase, panel);
		p.setColor(QPalette::ToolTipBase, panel);
		p.setColor(QPalette::ToolTipText, text);
		p.setColor(QPalette::Text, text);
		p.setColor(QPalette::Button, panel);
		p.setColor(QPalette::ButtonText, text);
		p.setColor(QPalette::Highlight, accent);
		p.setColor(QPalette::HighlightedText, QColor(0,0,0));
		p.setColor(QPalette::BrightText, text);
		p.setColor(QPalette::PlaceholderText, disabled);
		p.setColor(QPalette::Link, accent);
		p.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
		p.setColor(QPalette::Disabled, QPalette::Text, disabled);
		p.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
		app.setPalette(p);

		// Subtle styling for widgets to match palette
		const QString css = QString(
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

	// Pre-initialize Qt WebEngine at app start to avoid first-use glitches (e.g., leaving fullscreen)
	{
		QWebEngineView prewarm;
		prewarm.setAttribute(Qt::WA_DontShowOnScreen, true);
		prewarm.resize(1, 1);
		prewarm.load(QUrl("about:blank"));
		// Let the event loop process initialization tasks once
		app.processEvents();
	}

	// Register application services
	{
		auto repo = std::make_shared<Models::Repository>();
		if (!repo->load()) { repo->seedSampleData(); repo->save(); }
		Core::ServiceLocator::registerService<Models::Repository>(repo);
		auto net = std::make_shared<QNetworkAccessManager>();
		Core::ServiceLocator::registerService<QNetworkAccessManager>(net);
	}

	MainWindow w;
	w.resize(1200, 720);
	w.show();
	
	int result = app.exec();
	
	// Clean up WebEngine resources before application exit to avoid OpenGL warnings
	QApplication::processEvents();
	
	return result;
}


