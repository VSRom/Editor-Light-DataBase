#include <QApplication>
#include <qfile.h>
#include <QTextStream>
#include "Connection_Window.h"
//===========================================================================================================
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile file(":/resources/style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        app.setStyleSheet(file.readAll());
        file.close();
    }

    auto *window = new Connection_Window();
    window->setWindowIcon(QIcon(":/icons/app_icon.ico"));
    window->show();

    return app.exec();
}
//===========================================================================================================