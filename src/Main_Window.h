#pragma once
//================================================================================================================
#include "Database.h"
#include "Table_Explorer.h"
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QStandardPaths>
#include <QSettings>
#include <qdir.h>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
//================================================================================================================
class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

private:
    void setup_ui();

    Database     db_;
    Table_Explorer explorer_;

    QLineEdit *search_;
};
//================================================================================================================
