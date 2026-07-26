#include "Merge_Tables.h"
#include <QTabWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QSet>
#include <QList>
#include <QWidget>
//===========================================================================================================
Merge_Tables::Merge_Tables(const QStringList& selTables, const QList<QList<Table_Explorer::ColumnInfo>> allColumns, QWidget* parent)
    :QDialog(parent)  {


    setup_ui();
}
//===========================================================================================================
Merge_Tables::~Merge_Tables() {}
//===========================================================================================================
void Merge_Tables::setup_ui() {
    setWindowTitle("Merge Tables");
    setMinimumSize(850, 450);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(25);

    QHBoxLayout* nameLayout = new QHBoxLayout();
    nameEdit_ = new QLineEdit();
    nameEdit_->setPlaceholderText("Введите имя новой таблицы");
    nameLayout->addWidget(nameEdit_);
    mainLayout->addLayout(nameLayout);

    QHBoxLayout *choiseJOIN = new QHBoxLayout();
    joinTypeCombo_ = new QComboBox();
    joinTypeCombo_->setPlaceholderText("Выберите тип JOIN");
    joinTypeCombo_->addItems({"LEFT", "RIGHT", "INNER"});
    choiseJOIN->addWidget(joinTypeCombo_);
    mainLayout->addLayout(choiseJOIN);

    tabWidget_ = new QTabWidget();
    mainLayout->addWidget(tabWidget_);

    for (int i = 0; i < mergeInfo_.size(); i++) {
        QWidget* tab = new QWidget();
        QVBoxLayout* tabLayout = new QVBoxLayout(tab);

        // Продолжение для Вывода всех таблиц во вскрываемых вкладках на одном окне
    }
}
//================================================================================================================
QString Merge_Tables::get_sql() const {	// Сборка запроса для создания таблицы
    QString nameRow = {};
    QString typeRow = {};
    QStringList tempSqlGet = {};

    //QString nameTab = tables_->text().trimmed();	// Получили текст из строки имени таблицы
    //if (!nameTab.isEmpty()) {
    //	for (int i = 0; i < col_row_.size(); i++) {
    //		name_row = col_row_[i].nameEdit_->text().trimmed();
    //		type_row = col_row_[i].typeCombo_->currentText();

    //		if (!name_row.isEmpty())
    //			temp_sql_get.append(QString("\"%1\" %2").arg(name_row, type_row));
    //	}
    //}

    if (tempSqlGet.isEmpty())
		return QString();

    //return QString("CREATE TABLE IF NOT EXISTS \"%1\" (%2);").arg(nameTab, temp_sql_get.join(", "));
}
//================================================================================================================

void Merge_Tables::updatePreview() {

}
//================================================================================================================
void Merge_Tables::updateComKeys() {

}
//================================================================================================================

