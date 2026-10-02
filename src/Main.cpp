#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQuickWindow>
#include <QIcon>
#include <QDir>
#include <QUrl>
#include <QFontDatabase>
#include <QtQml>

#include "ConnectionController.h"
#include "MainController.h"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("Manager DataBase");

    const QString exeDir = QCoreApplication::applicationDirPath();
    QByteArray currentPath = qgetenv("PATH");
    currentPath += ";" + QDir::toNativeSeparators(exeDir + "/drivers/mysql").toLocal8Bit();
    currentPath += ";" + QDir::toNativeSeparators(exeDir + "/drivers/pgsql").toLocal8Bit();
    currentPath += ";" + QDir::toNativeSeparators(exeDir + "/drivers/oracle").toLocal8Bit();
    qputenv("PATH", currentPath);

    QCoreApplication::addLibraryPath(exeDir + "/sqldrivers");

    QQuickStyle::setStyle("Basic");

    QStringList fonts;
    for (const QString& fontFile : { ":/resources/fonts/Hack.ttf",
                                     ":/resources/fonts/Fira.ttf",
                                     ":/resources/fonts/Anon.ttf" }) {
        const int id = QFontDatabase::addApplicationFont(fontFile);
        if (id == -1) continue;
        const QStringList families = QFontDatabase::applicationFontFamilies(id);
        if (!families.isEmpty()) fonts << families.at(0);
    }

    ConnectionController connectionController;
    MainController mainController(fonts);

    qmlRegisterSingletonInstance("App", 1, 0, "ConnectionController", &connectionController);
    qmlRegisterSingletonInstance("App", 1, 0, "MainController", &mainController);
    qmlRegisterSingletonType(QUrl("qrc:/qml_desktop/Theme.qml"), "App", 1, 0, "Theme");

    QQmlApplicationEngine engine;
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); }, Qt::QueuedConnection);

    engine.load(QUrl("qrc:/qml_desktop/App.qml"));

    if (engine.rootObjects().isEmpty()) return -1;
    if (auto* w = qobject_cast<QQuickWindow*>(engine.rootObjects().first()))
        w->setIcon(QIcon(":/icons/app_icon.ico"));

    return app.exec();
}