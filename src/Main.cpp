#include <QApplication>
#include "Main_Window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    Main_Window window;

    window.setWindowIcon(QIcon(":/icons/app_icon.ico"));

    window.show();
    
    return app.exec();
}