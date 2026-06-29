#pragma once
//================================================================================================================
#include "Database.h"
#include "Table_Explorer.h"
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QPlainTextEdit>
#include <QProgressBar>
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
    explicit Main_Window(const QString db_type, const QString driver, QWidget* parent = nullptr);
    ~Main_Window();

private slots:
    void onSearch();
    void startProgressBar(PB_Status pbs);
    void stopProgressBar();
    void tab_create();
    void tab_united();
    void tab_rename();
    void tab_delete();
    void onFontChanged(const QString& fontName);
    void onTextChanged();
    void onTableSelected(const QString& tableName);
    void doubleClick(const QModelIndex& index);         // Дабл клик ЛКМ редактирование БД
    void onDBContextMenu(const QPoint& pos);            // Клик ПКМ окна БД
    void onAddRow();
    void onAddCol();
    //void onTableContextMenu(const QPoint& pos);       // Клик ПКМ окна таблиц

private:
    void setup_ui();
    void save_note();
    void refresh_table();

    Database     db_;                           // База Данных
    Table_Explorer explorer_;                   // Обозреватель
    QLineEdit* search_;                         // Поиск
    QListWidget* table_list_;                   // Список таблиц
    QTableView* data_view_;                     // Данные

    QProgressBar* progress_;                    // Прогресс Бар
    PB_Status current_pb_ = PB_Status::Idle;    // Текущее состояние прогресс Бара

    QString current_table_;                     // Текущая таблица
    QSortFilterProxyModel* proxyModel_;         // Для поиска в любом регистра

    QPlainTextEdit* notepad_;                   // Заметки
    QString notePath_;                          // Путь для заметок
    QComboBox* font_select_;                    // Выбор шрифта для заметок
    bool isModifyNote_;                         // Заметки изменены

    std::unique_ptr<QSqlQueryModel> const_ptr_;

    QPushButton* unitedT_;                      // 2.2.1 united tables
    QPushButton* createT_;			            // 2.2.2 create table
    QPushButton* renameT_;                      // 2.2.3 rename table
    QPushButton* deleteT_;                      // 2.2.4 delete table

    // Шрифты
    QString hack_;
    QString fira_;
    QString anon_;

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};
//================================================================================================================
