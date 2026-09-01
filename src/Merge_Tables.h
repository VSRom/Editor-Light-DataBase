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
    explicit Merge_Tables(const QMap<QString, QList<Table_Explorer::ColumnInfo>> tableColumns, QWidget* parent = nullptr);
    ~Merge_Tables();

    struct TableMergeInfo {
        QString tableName;
        QString tableAlias;

        QList<QCheckBox*> columnsCheck;
    };

    struct TableConditionEdit {
        QComboBox *leftTable_ = nullptr;
        QComboBox *leftCol_ = nullptr;
        QComboBox *operator_ = nullptr;
        QComboBox *rightTable_ = nullptr;
        QComboBox *rightCol_ = nullptr;

        QWidget *container_ = nullptr;
        QPushButton *btnDel_ = nullptr;
    };

    QString get_sql() const;

private slots:
    void addConditionRow();

private:
    void setup_ui();
    QStringList getNameRows(const QString &alias) const;

    QMap<QString, QList<Table_Explorer::ColumnInfo>> tableColumns_; // Колонки
    QList<TableConditionEdit*> listStruct_;

    QTabWidget *tabWidget_ = nullptr; // Вкладки
    QLineEdit *nameEdit_ = nullptr;   // Поле Имя
    QList<TableMergeInfo> mergeInfo_; // Список структур
    QComboBox *joinTypeCombo_ = nullptr;
   // QLineEdit *joinConditionEdit_ = nullptr; // Поле для ввода условия объединения (например, "t1.id = t2.id")
    QPushButton *btnOk_ = nullptr;
    QPushButton *btnCancel_ = nullptr;

    QPushButton *btnAdd_ = nullptr;
};
//================================================================================================================
