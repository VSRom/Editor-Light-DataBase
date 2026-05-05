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
#include "Main_Window.h"

class Connection_Window : public QMainWindow
{
	Q_OBJECT

public:
	explicit Connection_Window(QWidget *parent = nullptr);
	~Connection_Window();

private slots:

	// 1.2 Add buttons
	void check_con();					// 1.2.1 check the connection
	void connection();					// 1.2.2 Connection to BD
	void reset();						// 1.2.3 Clear enter
	void save_config();					// 1.2.4 save configure to *.ini

private:
	void setupUI();
	void UIforSqLite();

	// 1.1 Add window lines
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

};