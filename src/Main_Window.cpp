#include "Main_Window.h"
#include <QMessageBox>
#include <vector>

Main_Window::Main_Window(QWidget *parent)
    : QMainWindow(parent), db_()
{
    setup_ui();
    load_tasks();
}

Main_Window::~Main_Window() {}

void Main_Window::setup_ui()
{
    setWindowTitle("Task Manager");
    setMinimumSize(800, 600);

    // Widgets
    taskList_ = new QListWidget(this);
    taskInput_ = new QLineEdit(this);
    taskInput_->setPlaceholderText("Введите задачу...");
    
    addButton_ = new QPushButton("Добавить", this);
    clear_all_ = new QPushButton("Удалить весь список", this);
    refreshButton_ = new QPushButton("Обновить", this);
    clear_all_selected_ = new QPushButton("Удалить выбранные задачи", this);

    // Layout
    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(clear_all_selected_);
    buttonsLayout->addWidget(addButton_);
    buttonsLayout->addWidget(clear_all_);
    buttonsLayout->addWidget(refreshButton_);

    auto* inputLayout = new QHBoxLayout();
    inputLayout->addWidget(taskInput_);
    inputLayout->addWidget(addButton_);

    auto* mainLayout = new QVBoxLayout();
    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(taskList_);
    mainLayout->addLayout(buttonsLayout);

    auto* centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    // Signals and slots
    connect(addButton_, &QPushButton::clicked, this, &Main_Window::add_task);
    connect(clear_all_, &QPushButton::clicked, this, &Main_Window::clear_all_tasks);
    connect(refreshButton_, &QPushButton::clicked, this, &Main_Window::refresh_tasks);
    connect(clear_all_selected_, &QPushButton::clicked, this, &Main_Window::clear_all_selected_tasks);

    connect(taskInput_, &QLineEdit::returnPressed, this, &Main_Window::add_task);

    // Чтобы выделить все строки(элементы списка)
    taskList_->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void Main_Window::load_tasks()
{
    taskList_->clear();
    auto tasks = db_.getAllTasks();
    
    for (const auto& task : tasks)
    {
        QString text = QString::fromStdString(task.title());

        if (task.completed()) {
            text += " ✓";
        }

        auto *item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, task.id());
        taskList_->addItem(item);
    }
}

void Main_Window::add_task()
{
    QString title = taskInput_->text().trimmed();

    if (title.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Введите название задачи!");
        return;
    }
    
    if (db_.add_task(title.toStdString()))
    {
        taskInput_->clear();
        refresh_tasks();
    }
    else
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить задачу!");
}

void Main_Window::clear_all_tasks()
{
    int total = taskList_->count();

    if (total == 0)
    {
        QMessageBox::warning(this, "Ошибка", "Нет задач для удаления");
        return;
    }
    db_.delete_all_tasks();
    taskList_->clear();
}

void Main_Window::clear_all_selected_tasks()
{
    if (taskList_->currentRow() < 0)
    {
        QMessageBox::warning(this, "Ошибка", "Выберите задачи для удаления!");
        return;
    }

    // Список выделенных элементов
    QList<QListWidgetItem *> list_tasks = taskList_->selectedItems();

    if (list_tasks.isEmpty())
        return;

    for (QListWidgetItem *temp_list : list_tasks)

    {   // Берем id/text из элемента и отправляем в удаление из БД
        db_.delete_task(temp_list->data(Qt::UserRole).toInt());

        // Получаем номер строки
        auto indx = taskList_->row(temp_list);

        // Убираем элемент из виджета
        taskList_->takeItem(indx);

        delete temp_list;

    }
}

void Main_Window::refresh_tasks()
{
    load_tasks();
}