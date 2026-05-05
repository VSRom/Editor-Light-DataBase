#include "Connection_Window.h"

Connection_Window::Connection_Window(QWidget *parent)
	: QMainWindow(parent)
{ setupUI(); adjustSize(); }

Connection_Window::~Connection_Window() {}

void Connection_Window::UIforSqLite()
{
	bool isSqLite = (dbTypeCombo_->currentText() == "SQLite");

		dbAddressLine_->setPlaceholderText(isSqLite ? "Путь к файлу .db..." : "IP-address...");
		dbPortLine_->setEnabled(!isSqLite);
		loginLine_->setEnabled(!isSqLite);
		passwordLine_->setEnabled(!isSqLite);
	
		if (isSqLite) { dbPortLine_->clear(); loginLine_->clear(); passwordLine_->clear(); }
}

void Connection_Window::setupUI()
{
	setWindowTitle("Connection to DataBase");
	resize(400, 300);

//===========================================================================================================
	// Create central widget
//===========================================================================================================
	QWidget *central = new QWidget(this);
	setCentralWidget(central);

	QVBoxLayout *mainLayout = new QVBoxLayout(central);
	mainLayout->setContentsMargins(20, 20, 20, 20);
	mainLayout->setSpacing(3);
//===========================================================================================================
	//		Database Select 1.1.1
//===========================================================================================================
	QHBoxLayout *topLayout = new QHBoxLayout();
	topLayout->addWidget(new QLabel("Database Type:"));

	dbTypeCombo_ = new QComboBox();
	QSizePolicy policy_DB(QSizePolicy::Expanding, QSizePolicy::Fixed);
	dbTypeCombo_->setSizePolicy(policy_DB);

	dbTypeCombo_->addItems({ "SQLite", "MySQL", "PostgreSQL" }); //, "Access","Oracle" 
	topLayout->addWidget(dbTypeCombo_);

	mainLayout->addLayout(topLayout);
//===========================================================================================================
	//	Enter Address:Port 1.1.2
//===========================================================================================================
	QHBoxLayout *adrport = new QHBoxLayout();

	adrport->addWidget(new QLabel("Address:"));
	dbAddressLine_ = new QLineEdit();
	dbAddressLine_->setSizePolicy(policy_DB);
	dbAddressLine_->setPlaceholderText("Введите IP-address...");
	adrport->addWidget(dbAddressLine_);

	adrport->addWidget(new QLabel("Port:"));
	dbPortLine_ = new QLineEdit();
	dbPortLine_->setSizePolicy(policy_DB);
	dbPortLine_->setPlaceholderText("Введите Port...");
	adrport->addWidget(dbPortLine_);

	mainLayout->addLayout(adrport);
//===========================================================================================================
		// Enter login:pass 1.1.3
//===========================================================================================================
	QHBoxLayout *logopass = new QHBoxLayout();

	logopass->addWidget(new QLabel("Login:"));
	loginLine_ = new QLineEdit();
	loginLine_->setSizePolicy(policy_DB);
	loginLine_->setPlaceholderText("Введите Login...");
	logopass->addWidget(loginLine_);

	logopass->addWidget(new QLabel("Password:"));
	passwordLine_ = new QLineEdit();
	passwordLine_->setSizePolicy(policy_DB);
	passwordLine_->setEchoMode(QLineEdit::Password);
	passwordLine_->setPlaceholderText("Введите Password...");
	logopass->addWidget(passwordLine_);

	mainLayout->addLayout(logopass);
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
//===========================================================================================================
	// Select configure 1.1.4
//===========================================================================================================
	QHBoxLayout *config = new QHBoxLayout();

// Create dir && path && create file settings
//=======================================================================================================
	QString config_dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
	QDir().mkpath(config_dir + "/Editor-Light_DataBase");
	QString path = config_dir + "/Editor-Light_DataBase/connections.ini";

	QSettings settings(path, QSettings::IniFormat);
	QStringList config_s = settings.childGroups();
//=======================================================================================================
	configCombo_ = new QComboBox();
	configCombo_->setSizePolicy(policy_DB);
	configCombo_->addItem("======Выберите конфигурационные настройки======");
	configCombo_->addItems(config_s);

	//		for (int i = 0; i < configCombo_->count(); ++i) { configCombo_->setItemData(i, Qt::AlignCenter, Qt::TextAlignmentRole); }

	config->addWidget(configCombo_);
	mainLayout->addLayout(config);
//===========================================================================================================

	connect(dbTypeCombo_, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, &Connection_Window::UIforSqLite);
	UIforSqLite();
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
	//	if (reply == QMessageBox::Yes)
	//	{
	//	
	//	}
	//
		QMessageBox::information(this, "Сохранение конфигурации", "Отменено");
}
//===========================================================================================================
