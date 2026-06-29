#include "Main_Window.h"
#include "Create_Table.h"
#include <QInputDialog>
#include <QLabel>
#include <QSettings>
#include <QCoreApplication>
#include <QMessageBox>
#include <QCloseEvent>
#include <QSqlDatabase>
#include <QFontDatabase>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QSqlQueryModel>
#include <QHeaderView>
#include <QModelIndex>
#include <QMap>
#include <QMenu>
#include <QHash>
//===========================================================================================================
Main_Window::Main_Window(const QString db_type, const QString driver, QWidget *parent)
    : QMainWindow(parent), db_(), explorer_("main_connection", db_type), isModifyNote_(false)
{
    if (!db_.init_db("main_connection")) {
        QMessageBox::critical(this, "Ошибка подключения", "Не удалось открыть базу данных.\nПроверьте путь к файлу или имя подключения.", QMessageBox::Ok);
        this->close();
        return;
    }

    //Загрузка шрифтов!

    int Hack = QFontDatabase::addApplicationFont(":/resources/fonts/Hack.ttf");
    int Fira = QFontDatabase::addApplicationFont(":/resources/fonts/Fira.ttf");
    int Anon = QFontDatabase::addApplicationFont(":/resources/fonts/Anon.ttf");

    if (Hack == -1) qDebug() << "Ошибка загрузки Hack.ttf";
    if (Fira == -1) qDebug() << "Ошибка загрузки Fira.ttf";
    if (Anon == -1) qDebug() << "Ошибка загрузки Anon.ttf";

    hack_ = QFontDatabase::applicationFontFamilies(Hack).at(0);
    fira_ = QFontDatabase::applicationFontFamilies(Fira).at(0);
    anon_ = QFontDatabase::applicationFontFamilies(Anon).at(0);
    
    qDebug() << "Hack: " << hack_;
    qDebug() << "Fira: " << fira_;
    qDebug() << "Anon: " << anon_;

    notePath_ = QCoreApplication::applicationDirPath() + "/notepad.ini"; // Получили путь к папке для заметок

    setup_ui();
}
//===========================================================================================================
Main_Window::~Main_Window() {}
//===========================================================================================================
void Main_Window::onTableSelected(const QString &tableName)
{
    current_table_ = tableName;     // Сохраним текущую таблицу
    search_->clear();               // Очистим поиск

    const_ptr_.reset(explorer_.select(current_table_));
    proxyModel_->setSourceModel(const_ptr_.get());
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

///////////========================================================Онлайн редактирование БД========================================================///////////
    // Подключение сигнала двойного клика ЛКМ
    connect(data_view_, &QTableView::doubleClicked, this, &Main_Window::doubleClick);





///////////========================================================Онлайн редактирование БД========================================================///////////

    // Прогресс бар
    progress_ = new QProgressBar();
    progress_->setRange(0, 0);        //Режим "бегущей строки"
    progress_->setTextVisible(true);
    progress_->hide();                // Скрыт по умолчанию
    sw->addWidget(progress_, 2, 0, 1, 3);
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
    for (const QString &stroke : explorer_.getUserTables()) {
        if (!stroke.startsWith("sqlite_"))      // Исключаем системные объекты из списка таблиц
        table_list_->addItem(stroke);
    }
///////////========================================================Онлайн редактирование БД========================================================///////////

///////////==========================================================Контекстное меню БД========================================================///////////

    data_view_->setContextMenuPolicy(Qt::CustomContextMenu);    // Сами будем обрабатывать ПКМ
    connect(data_view_, &QTableView::customContextMenuRequested, this, &Main_Window::onDBContextMenu);

///////////==========================================================Контекстное меню БД========================================================///////////

    ///////////========================================================TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST
    ///////////========================================================TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST
    // Полный вывод того, что видит QSqlDatabase
    QStringList allTables = QSqlDatabase::database("main_connection").tables(QSql::Tables);
    qDebug() << "=== ВСЕ таблицы (QSqlDatabase::tables) ===";
    qDebug() << "Всего найдено:" << allTables.size();
    for (const QString& t : allTables) {
        qDebug() << "  [" << t << "]";
    }

    // Что возвращает getUserTables
    QStringList userTables = explorer_.getUserTables();
    qDebug() << "\n=== После фильтрации ===";
    qDebug() << "Всего:" << userTables.size();
    for (const QString& t : userTables) {
        qDebug() << "  " << t;
    }

    ///////////========================================================TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST
    ///////////========================================================TESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTESTTEST

    QGridLayout* ucrd = new QGridLayout();

    // Окно заметок + сохранение в notepad.ini файл + 3 варианта при закрытие приложения
    notepad_ = new QPlainTextEdit();
    font_select_ = new QComboBox();
    font_select_->addItems({ hack_, fira_, anon_ });
    font_select_->setToolTip("Выберите шрифт");

    notepad_->setFont(QFont(hack_, 12));
    connect(font_select_, &QComboBox::currentTextChanged, this, &Main_Window::onFontChanged);
    connect(notepad_, &QPlainTextEdit::textChanged, this, &Main_Window::onTextChanged);

   
    QSettings settings(notePath_, QSettings::IniFormat);
    QString text = settings.value("notepad/text", "").toString();
    QString font = settings.value("notepad/font", hack_).toString();
    int size = settings.value("notepad/size", 12).toInt();
    notepad_->setPlainText(text);
    notepad_->setFont(QFont(font, size));
    font_select_->setCurrentText(font);

    // 4 кнопки(Объединить, Создать, Переименовать, Удалить) "Между Заметками и Окном списка таблиц"
    
    unitedT_ = new QPushButton("Объединить\n таблицы", this);
    createT_ = new QPushButton("Создать\n таблицу", this);
    renameT_ = new QPushButton("Переименовать\n таблицу", this);
    deleteT_ = new QPushButton("Удалить\n таблицу", this);

    ucrd->addWidget(font_select_, 2, 0, 1, 2);
    ucrd->addWidget(unitedT_, 0, 0);
    ucrd->addWidget(createT_, 0, 1);
    ucrd->addWidget(renameT_, 1, 0);
    ucrd->addWidget(deleteT_, 1, 1);

    connect(unitedT_, &QPushButton::clicked, this, &Main_Window::tab_united);
    connect(createT_, &QPushButton::clicked, this, &Main_Window::tab_create);
    connect(renameT_, &QPushButton::clicked, this, &Main_Window::tab_rename);
    connect(deleteT_, &QPushButton::clicked, this, &Main_Window::tab_delete);

    sw->addLayout(ucrd, 2, 3, 1, 1);
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

    const_ptr_.reset(explorer_.select(current_table_));

    proxyModel_->setSourceModel(const_ptr_.get());
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
void Main_Window::tab_create() {
    Create_Table dialog(explorer_.get_types_db(), this);                      // Создаём диалог
    if (dialog.exec() == QDialog::Accepted) {                                 // Показываем и ждём результат
        QString sql = dialog.get_sql();
        if (sql.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Недостаточно данных для создания таблицы!");
            return;
        }

            if (!explorer_.exeQuery(sql)) {
                QMessageBox::critical(this, "Ошибка", "Не удалось создать таблицу");
                return;
            }

            refresh_table();

                QMessageBox::information(this, "Успех", "Таблица успешно создана!");
    }
}
//================================================================================================================
void Main_Window::tab_united() {

}
//================================================================================================================
void Main_Window::tab_rename() {
    QListWidgetItem* temp = table_list_->currentItem();
    bool ok;
    if (temp) {
        QString tabName = temp->text();
        QString newTabName = QInputDialog::getText(this, "Новое имя таблицы", "Ввод", QLineEdit::Normal, "", &ok);		//Встроенное окошко, что будет введено будет возвращено

        if (!ok || newTabName.isEmpty()) return; // Если нажал отмена или ничего не ввел

        QMessageBox::StandardButton reply = QMessageBox::question(this, "Предупреждение", QString("<p>После подтверждения имя таблицы '%1' изменится на '%2'</p>" "<p align='center'>Вы уверены?</p>").arg(tabName, newTabName), QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            explorer_.rename_table(tabName, newTabName);

            refresh_table();
        }
        else
            QMessageBox::information(this, "Отмена", "Операция отменена!");
    }
    else
        QMessageBox::warning(this, "Ошибка", "Таблица для изменение имени не выбрана!");
}
//================================================================================================================
void Main_Window::tab_delete(){

    QListWidgetItem* temp = table_list_->currentItem();
    if (temp) {
        QString tabName = temp->text();
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Предупреждение", QString("<p>После подтверждения все данные таблицы '%1' удалятся!</p>" "<p align='center'>Вы уверены?</p>").arg(tabName), QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            explorer_.drop_table(tabName);

            refresh_table();
        }
        else 
            QMessageBox::information(this, "Отмена", "Операция отменена!");
    }
    else
        QMessageBox::warning(this, "Ошибка", "Таблица для удаления не выбрана!");
}
//================================================================================================================
void Main_Window::refresh_table() {
table_list_->clear();
for (const QString& name : explorer_.getUserTables())
if (!name.startsWith("sqlite_"))
table_list_->addItem(name);
}
//================================================================================================================
void Main_Window::onFontChanged(const QString& fontName) {
    QFont font(fontName, 12);
    notepad_->setFont(font);
    isModifyNote_ = true;
}
//================================================================================================================
void Main_Window::closeEvent(QCloseEvent* event) {

    if (isModifyNote_ == true) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Сохранение заметок", QString("Заметки были изменены\n Сохранить?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

        if (reply == QMessageBox::Save) {
            save_note();
            event->accept();
        }
        else if (reply == QMessageBox::Discard)
            event->accept();
        else if (reply == QMessageBox::Cancel)
            event->ignore();
    }
    else event->accept();
}
//================================================================================================================
void Main_Window::onTextChanged() {
    isModifyNote_ = true;
}
//================================================================================================================
void Main_Window::save_note() {
    QSettings settings(notePath_, QSettings::IniFormat);
    settings.setValue("notepad/text", notepad_->toPlainText());
    settings.setValue("notepad/font", notepad_->font().family());
    settings.setValue("notepad/size", notepad_->font().pointSize());
    settings.sync();

    isModifyNote_ = false;
}
//================================================================================================================
void Main_Window::doubleClick(const QModelIndex& index) {
    if (index.column() == 0)
        QMessageBox::warning(this, "Ошибка", "Нельзя изменить id");
    else {
        QString data = index.data().toString();
        QInputDialog inDialog(this);
        inDialog.setWindowTitle("Новое значение");
        inDialog.setLabelText("Ввод");
        inDialog.setTextValue(data);
        inDialog.resize(500, 150);
        
        if (inDialog.exec() == QDialog::Accepted) {
            QString newData = inDialog.textValue();
            QString nameRow = proxyModel_->headerData(index.column(), Qt::Horizontal).toString();

            if (newData == data) return;
            else {
                QVariant idRow = proxyModel_->index(index.row(), 0).data();
                QMap<QString, QVariant> newVal;
                newVal[nameRow] = newData;
                if (!explorer_.update(current_table_, "id", idRow, newVal)) {
                    QMessageBox::critical(this, "Ошибка", "Новые данные не сохранились");
                    return;
                }
                const_ptr_.reset(explorer_.select(current_table_));
                proxyModel_->setSourceModel(const_ptr_.get());
            }
        }
    }
}
//================================================================================================================
void Main_Window::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Delete) {                                           // Проверка на нажатие кнопки Delete
        QModelIndexList selectRows = data_view_->selectionModel()->selectedRows();  // Получили список выделенных строк
        if (selectRows.isEmpty()) {
            QMessageBox::warning(this, "Ошибка", "Строка для удаления не выделена");
            return;
        }
        // Список выделенных строк
        QList<QVariant> delRows;
        for (int i = 0; i < selectRows.size(); i++) {
            QModelIndex idIdx = proxyModel_->index(selectRows[i].row(), 0);     // Получаем индекс в колонке 0 для текущей строки
            QVariant idVal = proxyModel_->data(idIdx);                          // Извлекаем значение ID из этой ячейки
            delRows.append(idVal);
        }
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Подтверждение удаления", "Удалить выбранную(ые) строку(и)?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            for (const QVariant& idVal : delRows) {
                if (!explorer_.remove(current_table_, "id", idVal)) {
                    QMessageBox::critical(this, "Ошибка", "Невозможно удалить строку: " + idVal.toString());
                    return;
                }
            }// Обновление БД
            const_ptr_.reset(explorer_.select(current_table_));
            proxyModel_->setSourceModel(const_ptr_.get());
            return;
        }
        QMainWindow::keyPressEvent(event);
    }
}
//================================================================================================================
void Main_Window::onDBContextMenu(const QPoint& pos) {
    QMenu menu(this);

    menu.addAction("Добавить строку", this, &Main_Window::onAddRow);
    menu.addSeparator();
    menu.addAction("Добавить столбец", this, &Main_Window::onAddCol);

    menu.exec(data_view_->viewport()->mapToGlobal(pos));    //Показывает меню по координатам из pos на видимой области(viewport())
}
//================================================================================================================
void Main_Window::onAddRow() {
    QHash<QString, QVariant> newRow;
    QList<Table_Explorer::ColumnInfo> cols = explorer_.getColumns(current_table_);  // Получили структуру таблицы

    for (const auto& col : cols) {
        if (col.name != "id")
            newRow.insert(col.name, QVariant());
    }
    
    if (!explorer_.insert(current_table_, newRow)) {
        QMessageBox::critical(this, "Ошибка", "Не удаётся добавить строку");
        return;
    }
    int lastRow = proxyModel_->rowCount() - 1;  // Количество строк
    QModelIndex lastIdx = proxyModel_->index(lastRow, 0);

    // Сброс фильтра перед прокруткой
    proxyModel_->setFilterFixedString("");
    search_->clear();

    data_view_->scrollTo(lastIdx, QAbstractItemView::PositionAtCenter); // Прокрутка таблицы
    data_view_->selectRow(lastRow); // Выделение строки

    const_ptr_.reset(explorer_.select(current_table_));
    proxyModel_->setSourceModel(const_ptr_.get());
}
//================================================================================================================
void Main_Window::onAddCol() {
    bool ok;
    QString nameCol = QInputDialog::getText(this, "Новый столбец", "Имя", QLineEdit::Normal, "", &ok);  // Ввели имя
    if (!ok || nameCol.isEmpty()) return;

    QStringList types = explorer_.get_types_db();
    QString colType = QInputDialog::getItem(this, "Тип данных", "Выберите тип: ", types, 0, false, &ok);    // Выбор типа столбца
    if (!ok) return;

    QString sql = QString("ALTER TABLE \"%1\" ADD COLUMN \"%2\" %3").arg(current_table_, nameCol, colType); // Запрос на добавление столбца

    if (!explorer_.exeQuery(sql)) {
        QMessageBox::critical(this, "Ошибка", "Не удалось создать столбец");
        return;
    }

    const_ptr_.reset(explorer_.select(current_table_));
    proxyModel_->setSourceModel(const_ptr_.get());
}
//================================================================================================================