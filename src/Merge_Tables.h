#pragma once
//================================================================================================================
#include "Table_Explorer.h"
#include <QDialog>
#include <QStringList>
//================================================================================================================
class Merge_Tables : public QDialog {
	Q_OBJECT

public:
	explicit Merge_Tables(const QStringList& selTables, Table_Explorer* explorer, QWidget* parent = nullptr);
	~Merge_Tables();

private slots:
	void updateComKeys();

private:
	void setup_ui();
	QString get_sql() const;

	QStringList tables_;													// Список таблиц
	Table_Explorer* explorer_;											// Обозреватель
	QMap<QString, QList<Table_Explorer::ColumnInfo>> tableColumns_;		// Колонки
	QTabWidget* tabWidget_;												// Вкладки
	QLineEdit* nameEdit_;												// Поле Имя
	QMap<QString, QList<QCheckBox*>> checkBox_;							// Чек таблицы
	QComboBox* keyTables;

};
//================================================================================================================

