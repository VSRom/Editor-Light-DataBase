#pragma once
//================================================================================================================
#include "Database.h"
#include "Database_Worker.h"
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTableView>
#include <QSortFilterProxyModel>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QStandardItemModel>
#include <QThread>
//================================================================================================================
class Main_Window : public QMainWindow {
    Q_OBJECT
public:
    explicit Main_Window(const QString driver, const QString db_type, const QString host, const int port,
                        const QString log, const QString pass, QWidget* parent = nullptr);
    ~Main_Window();

private slots:
    void onSearch();
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

    // Поток
    void onTablesLoaded(QStringList tables);
    void onSelectFinished(QList<QList<QVariant>> data, QStringList headers);
    void onColumnsLoaded(const QString& tableName, QList<Table_Explorer::ColumnInfo> cols);
    //void onErrorOccurred(QString);
    void onOperationCompleted(bool success, const QString& message);
    void onTypesDbLoaded(QStringList types);

private:
    void setup_ui();
    void save_note();
    void refresh_table();

    Database     db_;                           // База Данных

    // Поток
    QThread* worker_thread_;
    Database_Worker* worker_;
    QLineEdit* search_;                         // Поиск
    QListWidget* table_list_;                   // Список таблиц
    QTableView* data_view_;                     // Данные
    QString current_table_;                     // Текущая таблица
    QSortFilterProxyModel* proxyModel_;         // Для поиска в любом регистра
    QPlainTextEdit* notepad_;                   // Заметки
    QString notePath_;                          // Путь для заметок
    QComboBox* font_select_;                    // Выбор шрифта для заметок
    bool isModifyNote_;                         // Заметки изменены
    std::unique_ptr<QStandardItemModel> const_ptr_;
    QPushButton* unitedT_;                      // 2.2.1 united tables
    QPushButton* createT_;			            // 2.2.2 create table
    QPushButton* renameT_;                      // 2.2.3 rename table
    QPushButton* deleteT_;                      // 2.2.4 delete table
    // Шрифты
    QString hack_;
    QString fira_;
    QString anon_;
    // Запоминание текста поиска
    QString search_text_;
    // Запоминание операции
    QString pending_action_;
    // Запоминание колонки
    QString pending_column_name_;
    // БД и Драйвер
    QString db_path_;
    QString driver_;
    QString db_type_;
    QString host_;
    int port_;
    QString log_;
    QString pass_;
    QMap<QString, QList<Table_Explorer::ColumnInfo>> m_pendingMergeTables;  // Накопление колонок каждой выделенной таблицы
    QStringList m_mergeTablesNames;     // Список имен таблиц которые выбрал пользователь

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
};
//================================================================================================================
