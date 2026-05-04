#include "Connection_Window.h"

Connection_Window::Connection_Window(QWidget *parent)
	: QMainWindow(parent)
{ setupUI(); adjustSize(); }

Connection_Window::~Connection_Window() {}

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

	dbTypeCombo_->addItems({ "SQLite", "Access", "MySQL", "Oracle", "PostgreSQL" });
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
	config->addWidget(new QLabel("Select configure"));
	configCombo_ = new QComboBox();
	configCombo_->setSizePolicy(policy_DB);
	configCombo_->addItem("===Выберите конфигурацию===");
	configCombo_->addItems(config_s);
	config->addWidget(configCombo_);
	
	mainLayout->addLayout(config);
//===========================================================================================================
}