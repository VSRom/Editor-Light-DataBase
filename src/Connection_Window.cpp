#include "Connection_Window.h"
//===========================================================================================================
Connection_Window::Connection_Window(QWidget *parent)
	: QMainWindow(parent)
{ setupUI(); adjustSize(); }
//===========================================================================================================
Connection_Window::~Connection_Window() {}
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
	QGridLayout *aplp = new QGridLayout();
	aplp->addWidget(new QLabel("DataBase:"), 0, 0);

	dbTypeCombo_ = new QComboBox();
	dbTypeCombo_->addItems({ "SQLite", "MySQL", "PostgreSQL", "Access", "Oracle"});
	aplp->addWidget(dbTypeCombo_, 0, 1, 1, 3);

	aplp->addWidget(new QLabel("Address:"), 1, 0);
	dbAddressLine_ = new QLineEdit();
	dbAddressLine_->setPlaceholderText("IP...");
	aplp->addWidget(dbAddressLine_, 1, 1);

	port_ = new QLabel("Port:");
	aplp->addWidget((port_), 1, 2, Qt::AlignRight | Qt::AlignVCenter);
	dbPortLine_ = new QLineEdit();
	dbPortLine_->setPlaceholderText("Port...");
	aplp->addWidget(dbPortLine_, 1, 3);

	login_ = new QLabel("Login:");
	aplp->addWidget((login_), 2, 0, Qt::AlignRight | Qt::AlignVCenter);
	loginLine_ = new QLineEdit();
	loginLine_->setPlaceholderText("Login...");
	aplp->addWidget(loginLine_, 2, 1);


	pass_ = new QLabel("Password:");
	aplp->addWidget((pass_), 2, 2);
	passwordLine_ = new QLineEdit();
	passwordLine_->setEchoMode(QLineEdit::Password);
	passwordLine_->setPlaceholderText("Password...");
	aplp->addWidget(passwordLine_, 2, 3);

	remoteCheck_ = new QCheckBox("Сетевое подключение");
	aplp->addWidget(remoteCheck_, 3, 0, 1, 4);

	remoteCheck_->setChecked(true);
// Create dir && path && create file settings
//=======================================================================================================
	QString config_dir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
	QDir().mkpath(config_dir + "/Editor Light DataBase");
	QString path = config_dir + "/Editor Light DataBase/connections.ini";

	QSettings settings(path, QSettings::IniFormat);
	QStringList config_s = settings.childGroups();
//=======================================================================================================
	configCombo_ = new QComboBox();
	configCombo_->addItem("Выберите конфигурационные настройки");
	configCombo_->setItemData(0, Qt::AlignCenter, Qt::TextAlignmentRole);			// Центровка текста addItem
	configCombo_->addItems(config_s);
	aplp->addWidget(configCombo_, 4, 0, 1, 4);
	aplp->setHorizontalSpacing(5);
	aplp->setVerticalSpacing(25);
	mainLayout->addLayout(aplp);

/////////////////Заполнение полей подключения из конфигурационного файла///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	connect(configCombo_, &QComboBox::currentTextChanged, this, [this, path](const QString& configName) {
		if (configName == "Выберите конфигурационные настройки" || configName.isEmpty() ) return;

		QSettings settings(path, QSettings::IniFormat);
		
		if (settings.childGroups().contains(configName)) {
			settings.beginGroup(configName);

			int index = dbTypeCombo_->findText(settings.value("dbType").toString());
			if (index != -1) dbTypeCombo_->setCurrentIndex(index);
			remoteCheck_->setChecked(settings.value("isRemote").toBool());
			dbAddressLine_->setText(settings.value("address").toString());
			dbPortLine_->setText(settings.value("port").toString());
			loginLine_->setText(settings.value("login").toString());
			passwordLine_->setText(settings.value("password").toString());

			settings.endGroup();
			text_edit_->append("Загружена конфигурация: " + configName + "'");
		}});

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	connect(remoteCheck_, &QCheckBox::toggled, this, [this](bool isRemote) {
		dbPortLine_->setVisible(isRemote);
		loginLine_->setVisible(isRemote);
		passwordLine_->setVisible(isRemote);
		port_->setVisible(isRemote);
		login_->setVisible(isRemote);
		pass_->setVisible(isRemote);

		if (isRemote)
			dbAddressLine_->setPlaceholderText("IP или Hostname...");
		else {
			dbAddressLine_->setPlaceholderText("Путь к файлу БД...");
			dbPortLine_->clear();
			loginLine_->clear();
			passwordLine_->clear();
		}
		});
//===========================================================================================================
	// 1.2 Add buttons
//===========================================================================================================
	QHBoxLayout *chrsscconnect = new QHBoxLayout();

	reset_ = new QPushButton("Очистка ввода", this);
	save_config_ = new QPushButton("Сохранение конфигурации", this);
	check_con_ = new QPushButton("Проверка соединения", this);
	connection_ = new QPushButton("Соединение", this);

	chrsscconnect->addWidget(reset_);
	chrsscconnect->addWidget(save_config_);
	chrsscconnect->addWidget(check_con_);
	chrsscconnect->addWidget(connection_);

	connect(reset_, &QPushButton::clicked, this, &Connection_Window::reset);
	connect(save_config_, &QPushButton::clicked, this, &Connection_Window::save_config);
	connect(check_con_, &QPushButton::clicked, this, &Connection_Window::check_con);
	connect(connection_, &QPushButton::clicked, this, &Connection_Window::connection);

	mainLayout->addLayout(chrsscconnect);

//===========================================================================================================
	// 1.1.9 Add QTextEdit
//===========================================================================================================
	text_edit_ = new QTextEdit(this);

	mainLayout->addWidget(text_edit_);
}
//===========================================================================================================
void Connection_Window::check_con()
{
	QString host = dbAddressLine_->text();
	int port = dbPortLine_->text().toInt();
	QString log = loginLine_->text();
	QString	pass = passwordLine_->text();


	switch (dbTypeCombo_->currentIndex())
	{
		case 0: driver_ = "QSQLITE"; break;
		case 1: driver_ = "QODBC";  break;
		case 2: driver_ = "QMYSQL"; break;
		case 3: driver_ = "QOCI"; break;
		case 4: driver_ = "QPSQL"; break;
		default: text_edit_->append("Неизвестный тип БД"); return;
	}


	QString conName = "test_connection";
	bool isRemote = remoteCheck_->isChecked();
	/// Проверка существования файла
	QString dbPath = dbAddressLine_->text();

	if (!isRemote) {
		QFileInfo fileInfo(dbPath);

		if (!fileInfo.exists()) {
			text_edit_->append("Файл не найден: " + dbPath);
			return;
		}

		if (!fileInfo.isReadable()) {
			text_edit_->append("Нет прав на чтение файла: " + dbPath);
			return;
		}
	}

	if (QSqlDatabase::contains(conName)) {
		{
			QSqlDatabase db_old = QSqlDatabase::database(conName);
			db_old.close();
		}
		QSqlDatabase::removeDatabase(conName);
	}

	QSqlDatabase db = QSqlDatabase::addDatabase(driver_, conName);
	db.setDatabaseName(dbAddressLine_->text());

	if (isRemote)
	{
		db.setHostName(host);
		db.setPort(port);
		db.setUserName(log);
		db.setPassword(pass);
	}

	if (!db.open())
		text_edit_->append(QString("Ошибка подключения: ") + db.lastError().text());
	else {
		text_edit_->append(QString("Подключено!"));
		db.close();
	}
}
//===========================================================================================================
void Connection_Window::connection()
{
	QString host = dbAddressLine_->text();
	int port = dbPortLine_->text().toInt();
	QString log = loginLine_->text();
	QString	pass = passwordLine_->text();

	switch (dbTypeCombo_->currentIndex())
	{
		case 0: driver_ = "QSQLITE"; dbType_ = "sqlite";   break;
		case 1: driver_ = "QODBC"; dbType_ = "access";	   break;
		case 2: driver_ = "QMYSQL"; dbType_ = "mysql";     break;
		case 3: driver_ = "QOCI"; dbType_ = "oracle";      break;
		case 4: driver_ = "QPSQL"; dbType_ = "postgresql"; break;
		default: text_edit_->append("Неизвестный тип БД"); return;
	}

	QString conName = "main_connection";

	bool isRemote = remoteCheck_->isChecked();

	QSqlDatabase db;

	if (QSqlDatabase::contains(conName))
		db = QSqlDatabase::database(conName);
	else
		db = QSqlDatabase::addDatabase(driver_, conName);

	if (!isRemote) {
		QFileInfo fileInfo(dbAddressLine_->text());

		if (!fileInfo.exists()) {
			text_edit_->append("Файл не найден: " + dbAddressLine_->text());
			return;
		}
	}

	db.setDatabaseName(dbAddressLine_->text());

	if (isRemote) {
		db.setHostName(dbAddressLine_->text());
		db.setPort(dbPortLine_->text().toInt());
		db.setUserName(loginLine_->text());
		db.setPassword(passwordLine_->text());
	}

	if (!db.open())
		text_edit_->append(QString("Ошибка подключения: ") + db.lastError().text());
	else
	{
		Main_Window *win = new Main_Window(driver_, dbType_);	// Передали в основное окно данные о типе БД
		win->setAttribute(Qt::WA_DeleteOnClose);				// Аттрибут самоудаления
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
//////////////////////////////////// Перезаписываем старый конфиг или создаём новый!?//////////////////////////////////////////////////////////////////////////

	QString configName = configCombo_->currentText();

	if (configName.isEmpty() || configName == "Выберите конфигурационные настройки") {
		bool ok;
		configName = QInputDialog::getText(this, "Новая конфигурация", "Введите имя", QLineEdit::Normal, "", &ok);		//Встроенное окошко, что будет введено будет возвращено

		if (!ok || configName.isEmpty()) return; // Если нажал отмена или ничего не ввел
	}
	// Выбрал существующую конфигурацию
	else {
		QMessageBox::StandardButton reply = QMessageBox::question(this, "Перезапись"
			,QString("Перезаписать конфигурацию '%1'?").arg(configName)
			,QMessageBox::Yes | QMessageBox::No);

		// Если нажал Нет
		if (reply != QMessageBox::Yes) return;

	}
/////////////////////////////////////////Открываем файл настроек////////////////////////////////////////////////////////////////////////////////////////////////

	QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation); // Получили путь к папке для конфигов

	QString path = configDir + "/Editor Light DataBase/connections.ini";					// Формируем путь

	QSettings settings(path, QSettings::IniFormat);

/////////////////////////////////////////Запись данных в файл///////////////////////////////////////////////////////////////////////////////////////////////////

	settings.beginGroup(configName);	// Открыли группу в ini-file [*****]

	settings.setValue("dbType", dbTypeCombo_->currentText() );
	settings.setValue("isRemote", remoteCheck_->isChecked() );
	settings.setValue("address", dbAddressLine_->text() );
	settings.setValue("port", dbPortLine_->text() );
	settings.setValue("login", loginLine_->text() );
	settings.setValue("password", passwordLine_->text() );

	settings.endGroup();				// Закрыли группу
	settings.sync();					// Сохранили на диск

/////////////////////////////////////////Обновим список////////////////////////////////////////////////////////////////////////////////////////////////////////

	configCombo_->clear(); // Стёрли всё
	configCombo_->addItem("Выберите конфигурационные настройки");
	configCombo_->setItemData(0, Qt::AlignCenter, Qt::TextAlignmentRole);
	configCombo_->addItems(settings.childGroups());	// Читаем ini-file, и возвращаем список всех секций
	configCombo_->setCurrentText(configName);		// Делает активным текст что сохранен в configName
	text_edit_->append("Конфигурационные настройки сохранены!");
}
//===========================================================================================================
