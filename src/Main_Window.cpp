#include "Main_Window.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), db_() {
    setupUI();
    loadTasks();
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    setWindowTitle("Task Manager");
    setMinimumSize(800, 600);

    // Виджеты
    taskList_ = new QListWidget(this);
    taskInput_ = new QLineEdit(this);
    taskInput_->setPlaceholderText("Введите задачу...");
    
    addButton_ = new QPushButton("Добавить", this);
    deleteButton_ = new QPushButton("Удалить", this);
    refreshButton_ = new QPushButton("Обновить", this);

    // Компоновка
    auto* buttonsLayout = new QHBoxLayout();
    buttonsLayout->addWidget(addButton_);
    buttonsLayout->addWidget(deleteButton_);
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

    // Сигналы и слоты
    connect(addButton_, &QPushButton::clicked, this, &MainWindow::addTask);
    connect(deleteButton_, &QPushButton::clicked, this, &MainWindow::deleteTask);
    connect(refreshButton_, &QPushButton::clicked, this, &MainWindow::refreshTasks);
    connect(taskInput_, &QLineEdit::returnPressed, this, &MainWindow::addTask);
}

void MainWindow::loadTasks() {
    taskList_->clear();
    auto tasks = db_.getAllTasks();
    
    for (const auto& task : tasks) {
        QString text = QString::fromStdString(task.title());
        if (task.completed()) {
            text += " ✓";
        }
        taskList_->addItem(text);
    }
}

void MainWindow::addTask() {
    QString title = taskInput_->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название задачи!");
        return;
    }
    
    if (db_.addTask(title.toStdString())) {
        taskInput_->clear();
        refreshTasks();
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось добавить задачу!");
    }
}

void MainWindow::deleteTask() {
    int row = taskList_->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите задачу для удаления!");
        return;
    }
    
    auto tasks = db_.getAllTasks();
    if (row < static_cast<int>(tasks.size())) {
        if (db_.deleteTask(tasks[row].id())) {
            refreshTasks();
        }
    }
}

void MainWindow::refreshTasks() {
    loadTasks();
}