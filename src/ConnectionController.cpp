#include "ConnectionController.h"
#include "qt/qt_connection_factory.h"
#include "qt/qt_text_codec.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QSqlDatabase>
#include <cstddef>

static const QStringList kDbDisplayNames = { "SQLite", "MySQL", "PostgreSQL", "Access", "Oracle" };

ConnectionController::ConnectionController(QObject* parent) : QObject(parent) {
    reloadConfigs();
    appendLog(tr("Available drivers: ") + QSqlDatabase::drivers().join(", "));
}

void ConnectionController::setDbTypeIndex(int v) {
    if (dbTypeIndex_ == v)
        return;

    dbTypeIndex_ = v;

    setIsRemote(v == 1 || v == 2 || v == 4);

    emit dbTypeIndexChanged();
}

void ConnectionController::setIsRemote(bool v) {
    if (isRemote_ == v)
        return;

    isRemote_ = v;

    emit isRemoteChanged();
}

void ConnectionController::setDatabaseName(const QString& v) {
    if (databaseName_ == v)
        return;

    databaseName_ = v;

    emit databaseNameChanged();
}

void ConnectionController::setAddress(const QString& v) {
    if (address_ == v)
        return;

    address_ = v;

    emit addressChanged();
}

void ConnectionController::setPort(const QString& v) {
    if (port_ == v)
        return;

    port_ = v;

    emit portChanged();
}

void ConnectionController::setLogin(const QString& v) {
    if (login_ == v)
        return;

    login_ = v;

    emit loginChanged();
}

void ConnectionController::setPassword(const QString& v) {
    if (password_ == v)
        return;

    password_ = v;

    emit passwordChanged();
}

void ConnectionController::setCurrentConfig(const QString& v) {
    if (currentConfig_ == v)
        return;

    currentConfig_ = v;

    emit currentConfigChanged();
}

QString ConnectionController::displayTypeForIndex(int i) const {
    return (i >= 0 && i < kDbDisplayNames.size()) ? kDbDisplayNames.at(i) : QString();
}

QString ConnectionController::configFilePath() const {
    QString d = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir().mkpath(d + "/configs");

    return d + "/configs/connections.ini";
}

QStringList ConnectionController::readConfigs() const {
    QSettings s(configFilePath(), QSettings::IniFormat);
    return s.childGroups();
}

void ConnectionController::reloadConfigs() {
    configs_ = readConfigs();

    emit configsChanged();
}

bool ConnectionController::hasConfig(const QString& n) const {
    return configs_.contains(n);
}

void ConnectionController::loadConfig(const QString& name) {
    if (name.isEmpty() || !hasConfig(name))
        return;

    QSettings s(configFilePath(), QSettings::IniFormat);

    if (!s.childGroups().contains(name))
        return;

    s.beginGroup(name);
    const int idx = kDbDisplayNames.indexOf(s.value("dbType").toString());

    if (idx >= 0)
        setDbTypeIndex(idx);

    setDatabaseName(s.value("databaseName").toString());
    setAddress(s.value("address").toString());
    setPort(s.value("port").toString());
    setLogin(s.value("login").toString());
    setPassword(s.value("password").toString());
    s.endGroup();
    setCurrentConfig(name);

    appendLog(tr("Configuration loaded: '") + name + "'");
}

void ConnectionController::saveConfig(const QString& name) {
    const QString n = name.trimmed();
    if (n.isEmpty()) return;
    QSettings s(configFilePath(), QSettings::IniFormat);
    s.beginGroup(n);
    s.setValue("dbType", displayTypeForIndex(dbTypeIndex_));
    s.setValue("isRemote", isRemote_);
    s.setValue("databaseName", databaseName_);
    s.setValue("address", address_);
    s.setValue("port", port_);
    s.setValue("login", login_);
    s.setValue("password", password_);
    s.endGroup();
    s.sync();
    reloadConfigs();
    setCurrentConfig(n);
    appendLog(tr("Configuration settings saved!"));
}

void ConnectionController::resetForm() {
    setDatabaseName({});
    setAddress({});
    setPort({});
    setLogin({});
    setPassword({});
    setCurrentConfig({});
}

void ConnectionController::appendLog(const QString& m) {
    logText_ += m;
    logText_ += "\n";

    emit logTextChanged();
}

void ConnectionController::appendLogMessage(const QString& m) {
    appendLog(m);
}

std::expected<vsc::ConnectionParameters, QString> ConnectionController::makeConnectionParameters() const {
    vsc::ConnectionParameters params;

    switch(dbTypeIndex_) {
        case 0: params.type = vsc::DbTypes::sqlite; break;
        case 1: params.type = vsc::DbTypes::mysql;  break;
        case 2: params.type = vsc::DbTypes::psql;   break;
        case 3: params.type = vsc::DbTypes::access; break;
        case 4: params.type = vsc::DbTypes::oracle; break;

            default:
                return std::unexpected(std::move(tr("Unknown database type")));
    }

    const bool netDatabase = params.type == vsc::DbTypes::mysql
                          || params.type == vsc::DbTypes::psql
                          || params.type == vsc::DbTypes::oracle;

    if (netDatabase) {
        const QString host = address_.trimmed();
        const QString databaseName = databaseName_.trimmed();
        const QString login = login_.trimmed();

        if (host.isEmpty() || databaseName.isEmpty() || login.isEmpty())
            return std::unexpected(std::move(tr("Fill in host, database name, and login")));

        bool validPort = false;
        const int port = port_.trimmed().toInt(&validPort);

        if (!validPort || port < 1 || port > 65535)
            return std::unexpected(std::move(tr("Invalid port into database")));

        params.host = vsc::qt::toVscText(host);
        params.dataBaseName = vsc::qt::toVscText(databaseName);
        params.login = vsc::qt::toVscText(login);
        params.port = port;

        if (!password_.isEmpty())
            params.password = vsc::qt::toVscText(password_);
    } else {
        const QString pathDb = address_.trimmed();

        if (pathDb.isEmpty())
            return std::unexpected(std::move(tr("Invalid path to db")));

        params.pathDb = vsc::qt::toVscText(pathDb);
    }
    return params;
}

void ConnectionController::checkConnection() {
    auto params = makeConnectionParameters();

    if (!params) {
        appendLog(tr("Connection error: ") + params.error());
        return;
    }
    else {
        vsc::qt::QtConnectionFactory factory;
        auto connection = factory.open(*params);

        if (!connection) {
            appendLog(tr("Connection error: ") + vsc::qt::toQtText(connection.error().message));
            return;
        }
        else
            appendLog(tr("Connected!"));
    }
}

void ConnectionController::requestConnect() {
    auto params = makeConnectionParameters();

    if (!params) {
        appendLog(tr("Connection error: ") + params.error());
        emit connectionFailed(params.error());
        return;
    }

    appendLog(tr("Starting worker session..."));
    emit sessionRequested(*params);
}
