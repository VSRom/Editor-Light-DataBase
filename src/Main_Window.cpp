#include "Main_Window.h"
#include "Create_Table.h"
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

    // Новый источник
    proxyModel_->setSourceModel(model);
    proxyModel_->setFilterFixedString("");
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

    // Прогресс бар
    progress_ = new QProgressBar();
    progress_->setRange(0, 0);        //Режим "бегущей строки"
    progress_->setTextVisible(true);
    progress_->hide();                // Скрыт по умолчанию
    sw->addWidget(progress_, 2, 0, 1, 3);
    //sw->addWidget(new QLabel("ПРОГРЕСС"), 2, 0, 1, 3);
    // Стилизация прогресс бара
    progress_->setAlignment(Qt::AlignCenter);

    // Скрыть заголовок с дублированием id
    data_view_->verticalHeader()->setVisible(false);

    // Для поиска по любому регистру
    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setFilterCaseSensitivity(Qt::CaseInsensitive); // Игнор регистра
    proxyModel_->setFilterKeyColumn(-1); // Поиск по всем колонкам
    data_view_->setModel(proxyModel_);

    // Список таблиц
    table_list_ = new QListWidget();
    for (const QString &stroke : explorer_.getTables()) {
        if (!stroke.startsWith("sqlite_"))      // Исключаем системные объекты из списка таблиц
        table_list_->addItem(stroke);
    }

    // Окно заметок
    notepad_ = new QPlainTextEdit();

    // 4 кнопки(Объединить, Создать, Переименовать, Удалить) "Между Заметками и Окном списка таблиц"

    QHBoxLayout* ucrd = new QHBoxLayout();

    unitedT_ = new QPushButton("Объединить таблицы", this);
    createT_ = new QPushButton("Создать таблицу", this);
    renameT_ = new QPushButton("Переименовать таблицу", this);
    deleteT_ = new QPushButton("Удалить таблицу", this);

    ucrd->addWidget(unitedT_);
    ucrd->addWidget(createT_);
    ucrd->addWidget(renameT_);
    ucrd->addWidget(deleteT_);

    //  connect(unitedT_, &QPushButton::clicked, this, &Create_Table::tab_united);
    connect(createT_, &QPushButton::clicked, this, &Create_Table::tab_create);
    //  connect(renameT_, &QPushButton::clicked, this, &Create_Table::tab_rename);
    //  connect(deleteT_, &QPushButton::clicked, this, &Create_Table::tab_delete);


    sw->addLayout(ucrd, 2, 3, 1, 1);

    // Добавить шрифты пользователю на выбор // Mono ( Hack Feronoma Anonimus ) //////////////////////////////////////////////////////////////////

    sw->addWidget(notepad_, 3, 3, 2, 1);

    sw->addWidget(table_list_, 0, 3, 2, 1); // Номер строки // Номер колонки // Сколько строк занять // Сколько колонок занять
    
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

    startProgressBar(PB_Status::Searching);

    // Исправление утечки памяти
    if (const_ptr_) const_ptr_->clear();

    QSqlQueryModel *model = explorer_.select(current_table_);  // Отправляем готовые параметры в подготовку и исполнение

    const_ptr_ = model;

    proxyModel_->setSourceModel(const_ptr_);
    proxyModel_->setFilterFixedString(stroke);

    stopProgressBar();
}
//================================================================================================================
void Main_Window::startProgressBar(PB_Status pbs) {
    current_pb_ = pbs;

    switch (pbs) {
        case PB_Status::Searching: progress_->setFormat("Поиск"); break;
        case PB_Status::Saving: progress_->setFormat("Сохранение"); break;
        case PB_Status::Loading: progress_->setFormat("Загрузка"); break;
        default: progress_->setFormat("");
    }

    //  Idle, Searching, Saving, Loading
    progress_->show();
}
//================================================================================================================
void Main_Window::stopProgressBar() {
    progress_->reset(); // Сбросывает значение и остановка анимации
    progress_->hide();
    current_pb_ = PB_Status::Idle;
}
//================================================================================================================