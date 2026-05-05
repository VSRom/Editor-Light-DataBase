#include <QApplication>
#include "Connection_Window.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    auto *window = new Connection_Window();
    window->setWindowIcon(QIcon(":/icons/app_icon.ico"));
    window->show();

    return app.exec();
}