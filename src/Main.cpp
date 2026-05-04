#include <QApplication>
#include "Connection_Window.h"
//  #include "Main_Window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Connection_Window window;
    window.setWindowIcon(QIcon(":/icons/app_icon.ico"));
    window.show();
    //  Main_Window window;

    return app.exec();
}