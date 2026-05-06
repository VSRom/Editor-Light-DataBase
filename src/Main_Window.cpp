#include "Main_Window.h"
#include <QMessageBox>
#include <vector>
//===========================================================================================================
Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent), db_()
{
    setup_ui();
    if (!db_.init_db("main_connection")) {
        QMessageBox::critical(this, "Ошибка подключения", "Не удалось открыть базу данных.\nПроверьте путь к файлу или имя подключения.", QMessageBox::Ok);
        this->close(); return; };
}
//===========================================================================================================
Main_Window::~Main_Window() {}
//===========================================================================================================
void Main_Window::setup_ui()
{
    setWindowTitle("Manager DataBase");
    setMinimumSize(1920, 1080);

}