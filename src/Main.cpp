#include "Connection_Window.h"
#include <QApplication>
#include <qfile.h>
#include <QIcon> 
#include <QDir>
//===========================================================================================================
int main(int argc, char* argv[]) {
    QString exeDir = QCoreApplication::applicationDirPath();
    QByteArray currentPath = qgetenv("PATH");
    currentPath += ";" + QDir::toNativeSeparators(exeDir + "/drivers/mysql").toLocal8Bit();
    currentPath += ";" + QDir::toNativeSeparators(exeDir + "/drivers/pgsql").toLocal8Bit();
    currentPath += ";" + QDir::toNativeSeparators(exeDir + "/drivers/oracle").toLocal8Bit();
    qputenv("PATH", currentPath);

    QApplication app(argc, argv);
    QFile file(":/resources/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(file.readAll());
        file.close();
    }
    auto* window = new Connection_Window();
    window->setWindowIcon(QIcon(":/icons/app_icon.ico"));
    window->show();
    return app.exec();
}
//===========================================================================================================