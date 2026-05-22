#include "Main_Window.h"
//===========================================================================================================
Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent), db_(), explorer_("main_connection")
{
    setup_ui();
    if (!db_.init_db("main_connection")) {
        QMessageBox::critical(this, "Ошибка подключения", "Не удалось открыть базу данных.\nПроверьте путь к файлу или имя подключения.", QMessageBox::Ok);
        this->close(); return; };
}
//===========================================================================================================
Main_Window::~Main_Window() {}
//===========================================================================================================
void Main_Window::onTableSelected(const QString &tableName)
{
    current_table_ = tableName;     // Сохраним текущую таблицу

    search_->clear();               // Очистим поиск

    // Шаги выше относятся к подключению onSearch!

    QSqlQueryModel *model = explorer_.select(tableName);    // Получаем данные

    data_view_->setModel(model);
}
//===========================================================================================================
void Main_Window::setup_ui()
{
    setWindowTitle("Manager DataBase");
    setMinimumSize(1024, 768);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(25);

    QGridLayout *sw = new QGridLayout();

    // Поисковая строка
    search_ = new QLineEdit();
    search_->setPlaceholderText("Search...");
    sw->addWidget(search_, 0, 0, 1, 3);
    
    // Данные
    data_view_ = new QTableView();
    sw->addWidget(data_view_, 1, 0, 1, 3);

    // Список таблиц
    table_list_ = new QListWidget();
    for (const QString &stroke : explorer_.getTables()) {
        if (!stroke.startsWith("sqlite_"))      // Исключаем системные объекты из списка таблиц
        table_list_->addItem(stroke);
    }

    sw->addWidget(table_list_, 0, 3, 1, 1); // Номер строки // Номер колонки // Сколько строк занять // Сколько колонок занять
    
    sw->setRowStretch(1, 1);
    sw->setColumnStretch(0, 1);
    sw->setColumnStretch(1, 1);
    sw->setColumnStretch(2, 1);
    sw->setColumnStretch(3, 1);
    mainLayout->addLayout(sw);

    connect(table_list_, &QListWidget::currentTextChanged, this, &Main_Window::onTableSelected);
    connect(search_, &QLineEdit::returnPressed, this, &Main_Window::onSearch); 
    //  Кто отправляет       // Сигнал     // Кто принимает // Слот
}
//================================================================================================================
void Main_Window::onSearch()
{
    QString stroke = search_->text();                     // Берем введеное пользователем значение

    if (stroke.isEmpty()) {
        onTableSelected(current_table_);
        return;
    }

    auto colum = explorer_.getColumns(current_table_); // Получаем колонки

    QMap<QString, QString> filters;
    for (const auto &col : colum)
        filters[col.name] = stroke;                     // Готовим параметры для поиска

    QSqlQueryModel *model = explorer_.select(current_table_, filters, " OR ");  // Отправляем готовые параметры в подготовку и исполнение

    data_view_->setModel(model);
}
//================================================================================================================