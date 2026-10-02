#include "ConnectionController.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>

static const QStringList kDbDisplayNames = { "SQLite", "MySQL", "PostgreSQL", "Access", "Oracle" };

ConnectionController::ConnectionController(QObject* parent) : QObject(parent) {
    reloadConfigs();
    appendLog("Доступные драйверы: " + QSqlDatabase::drivers().join(", "));
}

void ConnectionController::setDbTypeIndex(int v) { if (dbTypeIndex_ == v)return; dbTypeIndex_ = v; emit dbTypeIndexChanged(); }
void ConnectionController::setIsRemote(bool v) { if (isRemote_ == v)return; isRemote_ = v; emit isRemoteChanged(); }
void ConnectionController::setAddress(const QString& v) { if (address_ == v)return; address_ = v; emit addressChanged(); }
void ConnectionController::setPort(const QString& v) { if (port_ == v)return; port_ = v; emit portChanged(); }
void ConnectionController::setLogin(const QString& v) { if (login_ == v)return; login_ = v; emit loginChanged(); }
void ConnectionController::setPassword(const QString& v) { if (password_ == v)return; password_ = v; emit passwordChanged(); }
void ConnectionController::setCurrentConfig(const QString& v) { if (currentConfig_ == v)return; currentConfig_ = v; emit currentConfigChanged(); }

QString ConnectionController::driverForIndex(int i) const {
    switch (i) { case 0:return "QSQLITE"; case 1:return "QMYSQL"; case 2:return "QPSQL"; case 3:return "QODBC"; case 4:return "QOCI"; }
                       return QString();
}
QString ConnectionController::dbTypeForIndex(int i) const {
    switch (i) { case 0:return "sqlite"; case 1:return "mysql"; case 2:return "postgresql"; case 3:return "access"; case 4:return "oracle"; }
                       return QString();
}
QString ConnectionController::displayTypeForIndex(int i) const { return (i >= 0 && i < kDbDisplayNames.size()) ? kDbDisplayNames.at(i) : QString(); }

QString ConnectionController::configFilePath() const {
    QString d = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(d + "/configs");
    return d + "/configs/connections.ini";
}
QStringList ConnectionController::readConfigs() const { QSettings s(configFilePath(), QSettings::IniFormat); return s.childGroups(); }
void ConnectionController::reloadConfigs() { configs_ = readConfigs(); emit configsChanged(); }
bool ConnectionController::hasConfig(const QString& n) const { return configs_.contains(n); }

void ConnectionController::loadConfig(const QString& name) {
    if (name.isEmpty() || !hasConfig(name)) return;
    QSettings s(configFilePath(), QSettings::IniFormat);
    if (!s.childGroups().contains(name)) return;
    s.beginGroup(name);
    const int idx = kDbDisplayNames.indexOf(s.value("dbType").toString());
    if (idx >= 0) setDbTypeIndex(idx);
    setIsRemote(s.value("isRemote").toBool());
    setAddress(s.value("address").toString());
    setPort(s.value("port").toString());
    setLogin(s.value("login").toString());
    setPassword(s.value("password").toString());
    s.endGroup();
    setCurrentConfig(name);
    appendLog("Загружена конфигурация: '" + name + "'");
}

void ConnectionController::saveConfig(const QString& name) {
    const QString n = name.trimmed();
    if (n.isEmpty()) return;
    QSettings s(configFilePath(), QSettings::IniFormat);
    s.beginGroup(n);
    s.setValue("dbType", displayTypeForIndex(dbTypeIndex_));
    s.setValue("isRemote", isRemote_);
    s.setValue("address", address_);
    s.setValue("port", port_);
    s.setValue("login", login_);
    s.setValue("password", password_);
    s.endGroup();
    s.sync();
    reloadConfigs();
    setCurrentConfig(n);
    appendLog("Конфигурационные настройки сохранены!");
}

void ConnectionController::resetForm() { setAddress({}); setPort({}); setLogin({}); setPassword({}); setCurrentConfig({}); }
void ConnectionController::appendLog(const QString& m) { logText_ += m; logText_ += "\n"; emit logTextChanged(); }
void ConnectionController::appendLogMessage(const QString& m) { appendLog(m); }

bool ConnectionController::tryOpenConnection(const QString& prefix, QString& error, QString& outDriver, QString& outDbType,
    QString& outDbPath, QString& outHost, int& outPort, QString& outLogin, QString& outPassword) {
    outDriver = driverForIndex(dbTypeIndex_);
    outDbType = dbTypeForIndex(dbTypeIndex_);
    if (outDriver.isEmpty()) { error = "Неизвестный тип БД"; return false; }
    outDbPath = address_.trimmed();
    outHost = isRemote_ ? address_.trimmed() : QString();
    outPort = isRemote_ ? port_.toInt() : 0;
    outLogin = isRemote_ ? login_ : QString();
    outPassword = isRemote_ ? password_ : QString();
    if (outDbPath.isEmpty()) { error = isRemote_ ? "Не указан хост" : "Не указан путь к файлу БД"; return false; }
    if (!isRemote_) {
        QFileInfo fi(outDbPath);
        if (outDriver != "QSQLITE" && outDriver != "QODBC") {
            if (!fi.exists()) { error = "Файл не найден: " + outDbPath; return false; }
            if (!fi.isReadable()) { error = "Нет прав на чтение файла: " + outDbPath; return false; }
        }
    }
    QString dbName = outDbPath;
    if (outDriver == "QODBC") {
        if (outDbPath.contains(';') || outDbPath.contains('{') || outDbPath.contains('}')) { error = "Путь содержит недопустимые символы: ';', '{', '}'"; return false; }
        dbName = QString("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%1;").arg(outDbPath);
    }
    const QString cn = QString("%1_%2").arg(prefix).arg(++tempConnectionCounter_);
    if (QSqlDatabase::contains(cn)) { { QSqlDatabase od = QSqlDatabase::database(cn); od.close(); } QSqlDatabase::removeDatabase(cn); }
    bool ok = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(outDriver, cn);
        db.setDatabaseName(dbName);
        if (isRemote_) { db.setHostName(outHost); db.setPort(outPort); db.setUserName(outLogin); db.setPassword(outPassword); }
        ok = db.open();
        if (!ok) error = db.lastError().text();
        db.close();
    }
    QSqlDatabase::removeDatabase(cn);
    return ok;
}

void ConnectionController::checkConnection() {
    QString e, d, t, p, h, l; int pt = 0;
    if (tryOpenConnection("test_connection", e, d, t, p, h, pt, l, password_)) appendLog("Подключено!");
    else appendLog("Ошибка подключения: " + e);
}
void ConnectionController::requestConnect() {
    QString e, d, t, p, h, l; int pt = 0; QString pw;
    if (!tryOpenConnection("preflight_connection", e, d, t, p, h, pt, l, pw)) { appendLog("Ошибка подключения: " + e); emit connectionFailed(e); return; }
    appendLog("Подключение успешно. Запуск основного окна...");
    emit connected(d, t, h, pt, l, pw, p);
}