#pragma once
#include <QScrollArea>
#include <QDialog>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QList>
#include <QLabel>
#include <QHBoxLayout>
#include <QMessageBox>
//===========================================================================================================
class Create_Table : public QDialog {
	Q_OBJECT

public:
	explicit Create_Table(QWidget* parent = nullptr, const QStringList& types);
	~Create_Table();

	QString get_sql const();


private slots:
	void add_col_row();				// Добавить столбец/строку

private:
	void setup_ui();

	struct Col_Row {				// Структура для создаваемых столбцов
		QHBoxLayout* layout_;
		QComboBox* typeCombo_;
		QLineEdit* nameEdit_;
		QPushButton* removeBtn_;
	};

	QLineEdit* tableName_;			// задаваемое имя таблицы
	QVBoxLayout* col_layout_;		// внутри ScrollArea
	QList<Col_Row> col_row_;		// список строк/столбцов
	QPushButton* add_col_;
	QStringList types_db_;			// Типы БД принимаем из Main_Window->Table_Explorer->Create_Table
	QStringList sql_get_;			// Для сборки запроса добавления

};
//===========================================================================================================