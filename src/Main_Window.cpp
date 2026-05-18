#include "Main_Window.h"
//===========================================================================================================
Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent), db_(), explorer_(QSqlDatabase::database("main_connection"))
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
    setMinimumSize(800, 600);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(25);

    QGridLayout *sw = new QGridLayout();

    search_ = new QLineEdit();
    search_->setPlaceholderText("Search...");
    sw->addWidget(search_, 0, 0, 1, 3);

    mainLayout->addLayout(sw);

}
//================================================================================================================