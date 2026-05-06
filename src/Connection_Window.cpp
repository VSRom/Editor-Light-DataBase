#include "Connection_Window.h"
//===========================================================================================================
Connection_Window::Connection_Window(QWidget *parent)
	: QMainWindow(parent)
{ setupUI(); adjustSize(); }
//===========================================================================================================
Connection_Window::~Connection_Window() {}
//===========================================================================================================
void Connection_Window::UIforSqLite()
{
	bool isSqLite = (dbTypeCombo_->currentText() == "SQLite");

		dbAddressLine_->setPlaceholderText(isSqLite ? "Путь к файлу .db..." : "IP-address...");
		dbPortLine_->setEnabled(!isSqLite);
		loginLine_->setEnabled(!isSqLite);
		passwordLine_->setEnabled(!isSqLite);
	
		if (isSqLite) { dbPortLine_->clear(); loginLine_->clear(); passwordLine_->clear(); }
}
//===========================================================================================================
void Connection_Window::setupUI()
{
	setWindowTitle("Connection to DataBase");
	setMinimumSize(850, 450);
//===========================================================================================================
	// Create central widget
//===========================================================================================================
	QWidget *central = new QWidget(this);
	setCentralWidget(central);

	QVBoxLayout *mainLayout = new QVBoxLayout(central);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->setSpacing(25);
//===========================================================================================================
	//		Database Select 1.1.1	//	Enter Address:Port 1.1.2	//	Enter login:pass 1.1.3		// Select configure 1.1.4
//===========================================================================================================
	//	QHBoxLayout *topLayout = new QHBoxLayout();
	QGridLayout *aplp = new QGridLayout();
	aplp->addWidget(new QLabel("DataBase:"), 0, 0);

	dbTypeCombo_ = new QComboBox();
	dbTypeCombo_->addItems({ "SQLite", "MySQL", "PostgreSQL" }); //, "Access","Oracle" 
	aplp->addWidget(dbTypeCombo_, 0, 1, 1, 3);
	aplp->addWidget(new QLabel("Address:"), 1, 0);

	dbAddressLine_ = new QLineEdit();
	dbAddressLine_->setPlaceholderText("Введите IP-address...");
	aplp->addWidget(dbAddressLine_, 1, 1);

	aplp->addWidget(new QLabel("Port:"), 1, 2, Qt::AlignRight | Qt::AlignVCenter);

	dbPortLine_ = new QLineEdit();
	dbPortLine_->setPlaceholderText("Введите Port...");
	aplp->addWidget(dbPortLine_, 1, 3);

	aplp->addWidget(new QLabel("Login:"), 2, 0, Qt::AlignRight | Qt::AlignVCenter);

	loginLine_ = new QLineEdit();
	loginLine_->setPlaceholderText("Введите Login...");
	aplp->addWidget(loginLine_, 2, 1);

	aplp->addWidget(new QLabel("Password:"), 2, 2);

	passwordLine_ = new QLineEdit();
	passwordLine_->setEchoMode(QLineEdit::Password);
	passwordLine_->setPlaceholderText("Введите Password...");
	aplp->addWidget(passwordLine_, 2, 3);

// Create dir && path && create file settings
//=======================================================================================================
	QString config_dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
	QDir().mkpath(config_dir + "/Editor-Light_DataBase");
	QString path = config_dir + "/Editor-Light_DataBase/connections.ini";

	QSettings settings(path, QSettings::IniFormat);
	QStringList config_s = settings.childGroups();
//=======================================================================================================
	configCombo_ = new QComboBox();
	configCombo_->addItem("======Выберите конфигурационные настройки======");
	configCombo_->addItems(config_s);
	aplp->addWidget(configCombo_, 3, 0, 1, 4);
	aplp->setHorizontalSpacing(5);
	aplp->setVerticalSpacing(25);
	mainLayout->addLayout(aplp);
	//		for (int i = 0; i < configCombo_->count(); ++i) { configCombo_->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole); }
	connect(dbTypeCombo_, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, &Connection_Window::UIforSqLite);
	UIforSqLite();
//===========================================================================================================
	// 1.2 Add buttons
//===========================================================================================================
	QHBoxLayout *chrsscconnect = new QHBoxLayout();

	check_con_ = new QPushButton("Проверка соединения", this);
	connection_ = new QPushButton("Соединение", this);
	reset_ = new QPushButton("Очистка ввода", this);
	save_config_ = new QPushButton("Сохранение конфигурации", this);

	chrsscconnect->addWidget(check_con_);
	chrsscconnect->addWidget(connection_);
	chrsscconnect->addWidget(reset_);
	chrsscconnect->addWidget(save_config_);

	connect(check_con_, &QPushButton::clicked, this, &Connection_Window::check_con);
	connect(connection_, &QPushButton::clicked, this, &Connection_Window::connection);
	connect(reset_, &QPushButton::clicked, this, &Connection_Window::reset);
	connect(save_config_, &QPushButton::clicked, this, &Connection_Window::save_config);

	mainLayout->addLayout(chrsscconnect);
}
//===========================================================================================================
void Connection_Window::check_con()
{
	QString host = dbAddressLine_->text();
	int port = dbPortLine_->text().toInt();
	QString log = loginLine_->text();
	QString	pass = passwordLine_->text();

	QString driver_db;

	switch (dbTypeCombo_->currentIndex())
	{
		case 0: driver_db = "QSQLITE"; break;
		//case 1: driver_db = "ACCESS"; break;
		case 1: driver_db = "QMYSQL"; break;
		//case 3: driver_db = "ORACLE"; break;
		case 2: driver_db = "QPSQL"; break;
	}

	QSqlDatabase db = QSqlDatabase::addDatabase(driver_db, "test_connection");

	db.setDatabaseName(dbAddressLine_->text());
	db.setHostName(host);
	db.setPort(port);
	db.setUserName(log);
	db.setPassword(pass);

	if (driver_db != "QSQLITE")
	{
		db.setHostName(host);
		db.setPort(port);
		db.setUserName(log);
		db.setPassword(pass);
	}

	if (!db.open())
		qDebug() << "Ошибка подключения" << db.lastError().text();
	else
		qDebug() << "Подключено!";
}
//===========================================================================================================
void Connection_Window::connection()
{
	QString host = dbAddressLine_->text();
	int port = dbPortLine_->text().toInt();
	QString log = loginLine_->text();
	QString	pass = passwordLine_->text();

	QString driver_db;

	switch (dbTypeCombo_->currentIndex())
	{
		case 0: driver_db = "QSQLITE"; break;
		//case 1: driver_db = "ACCESS"; break;
		case 1: driver_db = "QMYSQL"; break;
		//case 3: driver_db = "ORACLE"; break;
		case 2: driver_db = "QPSQL"; break;
	}

	QSqlDatabase db = QSqlDatabase::addDatabase(driver_db, "main_connection");

	db.setDatabaseName(dbAddressLine_->text());
	db.setHostName(host);
	db.setPort(port);
	db.setUserName(log);
	db.setPassword(pass);

	if (driver_db != "QSQLITE")
	{
		db.setHostName(host);
		db.setPort(port);
		db.setUserName(log);
		db.setPassword(pass);
	}

	if (!db.open())
		qDebug() << "Ошибка подключения" << db.lastError().text();
	else
	{
		Main_Window *win = new Main_Window();
		win->show();
		this->close();
		win->setWindowIcon(QIcon(":/icons/app_icon.ico"));
	}
}
//===========================================================================================================
void Connection_Window::reset()
{
	dbAddressLine_->clear(); dbPortLine_->clear(); loginLine_->clear(); passwordLine_->clear();
	configCombo_->setCurrentIndex(0);
}
//===========================================================================================================
void Connection_Window::save_config()
{
	QMessageBox::StandardButton reply;
	reply = QMessageBox::question(this, "Сохранение конфигурации", "Сохраним?", QMessageBox::Yes | QMessageBox::No);
	if (reply == QMessageBox::Yes)
	{
	
	}
	
		QMessageBox::information(this, "Сохранение конфигурации", "Отменено");
}
//===========================================================================================================
