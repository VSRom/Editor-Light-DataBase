#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <QStandardPaths>
#include <QSettings>
#include <qdir.h>
#include <QPushButton>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTextEdit>
#include "Main_Window.h"
#include "Table_Explorer.h"
#include <QCheckBox>
#include <QInputDialog>
#include <QStandardPaths>
//===========================================================================================================
class Connection_Window : public QMainWindow
{
	Q_OBJECT

public:
	explicit Connection_Window(QWidget *parent = nullptr);
	QString get_dbType() const { return dbType_; }
	QString get_driver() const { return driver_; }

	~Connection_Window();

private slots:
						// 1.2 buttons

	void check_con();					// 1.2.1 check the connection
	void connection();					// 1.2.2 Connection to BD
	void reset();						// 1.2.3 Clear enter
	void save_config();					// 1.2.4 save configure to *.ini


private:
	void setupUI();
						// 1.1 window lines

	QComboBox *dbTypeCombo_;			// 1.1.1 DB type dropdown
	QLineEdit *dbAddressLine_;			// 1.1.2 Address DB
	QLineEdit *dbPortLine_;				// 1.1.2 Port DB
	QLineEdit *loginLine_;				// 1.1.3 Login
	QLineEdit *passwordLine_;			// 1.1.3 Password
	QComboBox *configCombo_;			// 1.1.4 Select configure

	QPushButton *check_con_;			// 1.2.1 check the connection
	QPushButton *connection_;			// 1.2.2 Connection to BD;
	QPushButton *reset_;				// 1.2.3 Clear enter
	QPushButton *save_config_;			// 1.2.4 save configure to *.ini_;
						// 1.1.9 Text Edit
	QTextEdit *text_edit_;
	QString dbType_;            // Определение типа БД для определения списка поддерживаемых типов столбцов таблицы
	QString driver_;			// Драйвер БД

	QCheckBox* remoteCheck_;		// Переключатель local/net db

	// Для скрытия лейблов
	QLabel* login_;
	QLabel* pass_;
	QLabel* port_;
};
//===========================================================================================================
