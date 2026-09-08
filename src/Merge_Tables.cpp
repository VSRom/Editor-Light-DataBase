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

    conditionContainer_ = new QWidget();
    QVBoxLayout *onLayout = new QVBoxLayout(conditionContainer_);
    mainLayout->addWidget(new QLabel("Условия связи ON"));
    mainLayout->addWidget(conditionContainer_);
    
    btnAdd_ = new QPushButton("+ Добавить условие");
    mainLayout->addWidget(btnAdd_);
    connect(btnAdd_, &QPushButton::clicked, this, &Merge_Tables::addConditionRow);


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
    addConditionRow();
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
QString Merge_Tables::get_sql() const {	            // Сборка запроса для создания таблицы
    QString nameTab = nameEdit_->text().trimmed();	// Получили текст из строки имени таблицы

    if (nameTab.isEmpty())    return QString();

    QStringList selectCols;

    for (const auto& mergiL : mergeInfo_) {
        for (QCheckBox* box : mergiL.columnsCheck)
            if (box->isChecked()) selectCols.append(mergiL.tableAlias + ".\"" + box->text() + "\"");
    }

    if (selectCols.isEmpty()) return QString();

    QString fromClause("FROM \"" + mergeInfo_[0].tableName + "\" " + mergeInfo_[0].tableAlias);

    for (int i = 1; i < mergeInfo_.size(); i++) {
        fromClause += " " + joinTypeCombo_->currentText() + " JOIN \"" + mergeInfo_[i].tableName + "\" " + mergeInfo_[i].tableAlias;
        QString currentAlias = mergeInfo_[i].tableAlias;
        QStringList conditionsForThisTable;
        QStringList availableAliases;

        for (int j = 0; j < i; j++)
            availableAliases += mergeInfo_[j].tableAlias;

        for (const auto* structas : listStruct_) {
            QString left = structas->leftTable_->currentData().toString();
            QString right = structas->rightTable_->currentData().toString();
            QString rightCol = structas->rightCol_->currentText();
            QString leftCol = structas->leftCol_->currentText();
            QString oper = structas->operator_->currentText();

            if (leftCol.isEmpty() || rightCol.isEmpty())
                continue;

            QString secondAlias;
            if (left == currentAlias)
                secondAlias = right;
            else if (right == currentAlias)
                secondAlias = left;
            else
                continue;

            if (availableAliases.contains(secondAlias)) {
                QString condition = left + ".\"" + leftCol + "\" " + oper + " " + right + ".\"" + rightCol + "\"";
                conditionsForThisTable.append(condition);
            }
        }

            if (conditionsForThisTable.isEmpty())
                return QString{};

            fromClause += " ON (" + conditionsForThisTable.join(" AND ") + ")";
    }
    return QString("CREATE TABLE \"%1\" AS SELECT %2 %3").arg(nameTab, selectCols.join(", "), fromClause);
}
//================================================================================================================
void Merge_Tables::addConditionRow() {
    TableConditionEdit *structura = new TableConditionEdit();
    QStringList operators = { "=", "<>", "<", ">", "<=", ">=" };
    structura->container_ = new QWidget();

    QHBoxLayout *layoutStructContainer = new QHBoxLayout(structura->container_);
    structura->leftTable_ = new QComboBox(structura->container_);
    structura->leftCol_ = new QComboBox(structura->container_);
    structura->operator_ = new QComboBox(structura->container_);
    structura->rightTable_ = new QComboBox(structura->container_);
    structura->rightCol_ = new QComboBox(structura->container_);
    structura->btnDel_ = new QPushButton("X", structura->container_);

    layoutStructContainer->addWidget(structura->leftTable_);
    structura->leftTable_->setMinimumWidth(120);
    layoutStructContainer->addWidget(new QLabel("."));
    structura->leftCol_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layoutStructContainer->addWidget(structura->leftCol_);

    layoutStructContainer->addWidget(structura->operator_);
    structura->operator_->setFixedWidth(80);
    structura->operator_->addItems(operators);

    layoutStructContainer->addWidget(structura->rightTable_);
    structura->rightTable_->setMinimumWidth(120);
    layoutStructContainer->addWidget(new QLabel("."));
    structura->rightCol_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layoutStructContainer->addWidget(structura->rightCol_);

    layoutStructContainer->addWidget(structura->btnDel_);

    connect(structura->leftTable_, &QComboBox::currentIndexChanged, this, [this, structura]() {
        QString alias = structura->leftTable_->currentData().toString();
        structura->leftCol_->clear();
        structura->leftCol_->addItems(getNameRows(alias));
        });

    connect(structura->rightTable_, &QComboBox::currentIndexChanged, this, [this, structura]() {
        QString alias = structura->rightTable_->currentData().toString();
        structura->rightCol_->clear();
        structura->rightCol_->addItems(getNameRows(alias));
        });

    for (const auto &mergiL : mergeInfo_) {
        structura->leftTable_->addItem(mergiL.tableName, mergiL.tableAlias);
        structura->rightTable_->addItem(mergiL.tableName, mergiL.tableAlias);
    }

    structura->leftTable_->setCurrentIndex(0);
    structura->rightTable_->setCurrentIndex(0);

    connect(structura->btnDel_, &QPushButton::clicked, this, [this, structura]() {

        int i = listStruct_.indexOf(structura);
        if (i != -1) {
            listStruct_.removeAt(i);
            structura->container_->hide();
            structura->container_->deleteLater();
            delete structura;
        } });

        conditionContainer_->layout()->addWidget(structura->container_);
        listStruct_.append(structura);
}
//================================================================================================================
