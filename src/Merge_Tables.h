#pragma once
//================================================================================================================
#include "Table_Explorer.h"
#include <QDialog>
#include <QStringList>
#include <QCheckBox>
#include <QLineEdit>
#include <QTabWidget>
#include <QGridLayout>
#include <QTextEdit>
//================================================================================================================
class Merge_Tables : public QDialog {
	Q_OBJECT

public:
    explicit Merge_Tables(const QStringList& selectTables, const QList<QList<Table_Explorer::ColumnInfo>> allColumns, QWidget* parent = nullptr);

    virtual ~Merge_Tables();

    struct TableMergeInfo {
        QString tableName;
        QString tableAlias;
        QList<Table_Explorer::ColumnInfo> columns;

        //Элементы UI
        QGridLayout* layout;
        QList<QCheckBox*> columnsCheck;
        QComboBox* keyCombo;
    };

    QString get_sql() const;

private slots:
    void updatePreview();   // Предпросмотр объединения

private:
    void extracted();
    //===========================================================================================================
    void setup_ui();

    QStringList tables_; // Список таблиц

    QMap<QString, QList<Table_Explorer::ColumnInfo>> tableColumns_; // Колонки

    QTabWidget *tabWidget_ = nullptr; // Вкладки
    QLineEdit *nameEdit_ = nullptr;   // Поле Имя
    QList<TableMergeInfo> mergeInfo_; // Список структур
    QTextEdit *sqlPreview_ = nullptr;
    QComboBox *joinTypeCombo_ = nullptr;
};
//================================================================================================================