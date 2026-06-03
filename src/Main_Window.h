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
#include <QListWidget>
#include <QTableView>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
//================================================================================================================
// Состояния для QProgressBar
enum class PB_Status {
    Idle, Searching, Saving, Loading
};
//================================================================================================================
class Main_Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Main_Window(QWidget *parent = nullptr);
    ~Main_Window();

    void onTableSelected(const QString &tableName);

private slots:
    void onSearch();
    void startProgressBar(PB_Status pbs);
    void stopProgressBar();
    void tab_create();
    void tab_united();
    void tab_rename();
    void tab_delete();

private:
    void setup_ui();

    Database     db_;                           // База Данных
    Table_Explorer explorer_;                   // Обозреватель
    QLineEdit* search_;                         // Поиск
    QListWidget* table_list_;                   // Список таблиц
    QTableView* data_view_;                     // Данные
    QPlainTextEdit* notepad_;                   // Заметки
    QProgressBar* progress_;                    // Прогресс Бар
    PB_Status current_pb_ = PB_Status::Idle;    // Текущее состояние прогресс Бара
    QString current_table_;                     // Текущая таблица
    QSortFilterProxyModel* proxyModel_;         // Для поиска в любом регистра

    QSqlQueryModel* const_ptr_ = nullptr;       // Чистка моделей

    QPushButton* unitedT_;                      // 2.2.1 united tables
    QPushButton* createT_;			            // 2.2.2 create table
    QPushButton* renameT_;                      // 2.2.3 rename table
    QPushButton* deleteT_;                      // 2.2.4 delete table
};
//================================================================================================================
