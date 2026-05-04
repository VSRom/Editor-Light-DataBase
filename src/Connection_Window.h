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


class Connection_Window : public QMainWindow
{
	Q_OBJECT

public:
	explicit Connection_Window(QWidget *parent = nullptr);
	~Connection_Window();

private:
	void setupUI();

	// 1.1
	QComboBox *dbTypeCombo_;			// 1.1.1 DB type dropdown
	QLineEdit *dbAddressLine_;			// 1.1.2 Address DB
	QLineEdit *dbPortLine_;				// 1.1.2 Port DB
	QLineEdit *loginLine_;				// 1.1.3 Login
	QLineEdit *passwordLine_;			// 1.1.3 Password
	QComboBox *configCombo_;			// 1.1.4 Select configure
};