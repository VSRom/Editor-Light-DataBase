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
Merge_Tables::Merge_Tables(const QMap<QString, QList<Table_Explorer::ColumnInfo>> tableColumns, QWidget* parent)
    :QDialog(parent), tableColumns_(tableColumns) {

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

    int aliasIndex = 1;
    // Перебираем входные данные
    for (auto it = tableColumns_.constBegin(); it != tableColumns_.constEnd(); it++) {
        QString tableName = it.key(); // Получаем имя таблицы
        QList<Table_Explorer::ColumnInfo> cols = it.value(); // Получаем список колонок(полученной таблицы)
        
        // Контейнер для хранения данных об этой таблице
        TableMergeInfo info;
        info.tableName = tableName;
        info.tableAlias = "t" + QString::number(aliasIndex);    // Присваиваем псевдоним
        
        // Визуальная вкладка для э той таблицы
        QWidget *tab = new QWidget();
        QVBoxLayout *tabLayout = new QVBoxLayout(tab);
        tabLayout->setAlignment(Qt::AlignTop);
        
        // Перебираем всю колонки и создаём галочки
        for (auto col : cols) {
            QCheckBox *box = new QCheckBox(col.name);
            box->setChecked(true);
            tabLayout->addWidget(box);
            info.columnsCheck.append(box);
        }

        tabLayout->addStretch();
        tabWidget_->addTab(tab, QString("%1 (как %2)").arg(tableName, info.tableAlias));
        mergeInfo_.append(info);
        aliasIndex++;
    }
   // Поле условия ON
    QHBoxLayout *onLayout = new QHBoxLayout();
    onLayout->addWidget(new QLabel("Условие связи (ON), например: t1.id = t2.user_id :"));

    //  joinConditionEdit_ = new QLineEdit();
    //  joinConditionEdit_->setPlaceholderText("t1.id = t2.user_id");
    //  onLayout->addWidget(joinConditionEdit_);

    mainLayout->addLayout(onLayout);
    
    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnOk_ = new QPushButton("Создать таблицу");
    btnCancel_ = new QPushButton("Отмена");
    btnLayout->addWidget(btnOk_);
    btnLayout->addWidget(btnCancel_);
    mainLayout->addLayout(btnLayout);

    connect(btnOk_, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel_, &QPushButton::clicked, this, &QDialog::reject);
}
//================================================================================================================
QStringList Merge_Tables::getNameRows(const QString& alias) const {
    QStringList resultColCheck;

    for (const TableMergeInfo& tempMergeInfo : mergeInfo_) {
        if (tempMergeInfo.tableAlias == alias)
            for (QCheckBox *box : tempMergeInfo.columnsCheck)
                resultColCheck += box->text();
    }
    return resultColCheck;
}
//================================================================================================================
QString Merge_Tables::get_sql() const {	// Сборка запроса для создания таблицы
    QString nameTab = nameEdit_->text().trimmed();	// Получили текст из строки имени таблицы
    //  QString condiUnif = joinConditionEdit_->text().trimmed();
    QStringList selectCols;

    for (const auto &mergiL : mergeInfo_) {
        for (QCheckBox *box : mergiL.columnsCheck)
            if (box->isChecked()) selectCols.append(mergiL.tableAlias + "." + box->text());
    }
    if (selectCols.isEmpty()) return QString();
    if (nameTab.isEmpty())    return QString();
    //if (condiUnif.isEmpty())  return QString();

    QString fromClause("FROM \"" + mergeInfo_[0].tableName + "\" " + mergeInfo_[0].tableAlias);

    for (int i = 1; i != mergeInfo_.size(); i++)
        fromClause += " " + joinTypeCombo_->currentText() + " JOIN \"" + mergeInfo_[i].tableName + "\" " + mergeInfo_[i].tableAlias;
    //fromClause += " ON (" + condiUnif + ")";
    
    if (fromClause.isEmpty())    return QString();


    return QString("CREATE TABLE \"%1\" AS SELECT %2 %3").arg(nameTab, selectCols.join(", "), fromClause);
}
//================================================================================================================
void Merge_Tables::addConditionRow() {
    TableConditionEdit* structura = new TableConditionEdit();


}
