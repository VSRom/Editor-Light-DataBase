#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "Database.h"

class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

    // buttons
private slots:
    void add_task();
    void refresh_tasks();
    void clear_all_tasks();
    void clear_all_selected_tasks();

private:
    void setup_ui();
    void load_tasks();
    
    Database     db_;
    QListWidget *taskList_;
    QLineEdit   *taskInput_;
    QPushButton *addButton_;
    QPushButton *clear_all_;
    QPushButton *refreshButton_;
    QPushButton *clear_all_selected_;
};

#endif // MAIN_WINDOW_H