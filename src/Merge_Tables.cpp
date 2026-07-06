/*
#include "Merge_Tables.h"
#include "Table_Explorer.h"
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
//===========================================================================================================
Merge_Tables::Merge_Tables(const QStringList& selTables, Table_Explorer* explorer, QWidget* parent)
	:QDialog(parent), tables_(selTables), explorer_(explorer)  {
	setWindowTitle("Объединение таблиц");
	setMinimumSize(800, 600);
	setup_ui();
}
Merge_Tables::~Merge_Tables() {}
//===========================================================================================================
void Merge_Tables::setup_ui() {

}
//================================================================================================================
QString Merge_Tables::get_sql() const {	// Сборка запроса для создания таблицы
	QString name_row = {};
	QString type_row = {};
	QStringList temp_sql_get = {};

	QString nameTab = tables_->text().trimmed();	// Получили текст из строки имени таблицы
	if (!nameTab.isEmpty()) {
		for (int i = 0; i < col_row_.size(); i++) {
			name_row = col_row_[i].nameEdit_->text().trimmed();
			type_row = col_row_[i].typeCombo_->currentText();

			if (!name_row.isEmpty())
				temp_sql_get.append(QString("\"%1\" %2").arg(name_row, type_row));
		}
	}

	if (temp_sql_get.isEmpty())
		return QString();

	return QString("CREATE TABLE IF NOT EXISTS \"%1\" (%2);").arg(nameTab, temp_sql_get.join(", "));
}
//================================================================================================================
void Merge_Tables::updateComKeys() {

}
//================================================================================================================
*/