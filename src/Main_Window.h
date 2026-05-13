#pragma once
//================================================================================================================
#include <QMainWindow>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "Database.h"
#include "Table_Explorer.h"
//================================================================================================================
class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

    // buttons
private slots:
    void add_task();


private:
    void setup_ui();

    
    Database     db_;
    Table_Explorer explorer_;
};
//================================================================================================================
