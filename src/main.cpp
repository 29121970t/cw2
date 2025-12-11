#include <qdebug.h>

#include <QApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QWebEngineView>

#include "MainWindow.h"
#include "core/ServiceLocator.h"
#include "models/DrugRepository.h"
#include "models/PharmacyRepository.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QApplication::setApplicationName("Информационная система лекарственных препаратов");
    QApplication::setOrganizationName("29121970tt");

    QApplication::setStyle("Fusion");
    {
        const auto bg = QColor(30, 32, 34);

        const auto panel = QColor(40, 42, 46);
        const auto base = QColor(36, 38, 41);

        const auto text = QColor(235, 235, 235);
        const auto disabled = QColor(130, 130, 130);
        const auto accent = QColor(23, 166, 76);
        const auto accentD = QColor(22, 160, 78);

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
        palette.setColor(QPalette::HighlightedText, QColor(0, 0, 0));
        palette.setColor(QPalette::BrightText, text);
        palette.setColor(QPalette::PlaceholderText, disabled);
        palette.setColor(QPalette::Link, accent);
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabled);
        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);
        palette.setColor(QPalette::Disabled, QPalette::WindowText, disabled);

        QApplication::setPalette(palette);
        QFile file(":style/index.qss");
        if (file.open(QFile::ReadOnly)) {
            app.setStyleSheet(file.readAll());
        }
        else{
            qInfo() << "cannot open stylesheet file ";
        }
    }

    {
        auto drugRepo = std::make_shared<Models::DrugRepository>();
        auto pharmacyRepo = std::make_shared<Models::PharmacyRepository>();

        bool drugLoaded = drugRepo->load();
        bool pharmacyLoaded = pharmacyRepo->load();

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
    app.installEventFilter(&window);

    const auto result = QApplication::exec();
    QApplication::processEvents();

    return result;
}
