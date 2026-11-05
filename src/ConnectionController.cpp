#include "ConnectionController.h"
#include "qt/qt_connection_factory.h"

#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QSqlDatabase>
#include <QSqlError>
#include <QByteArray>
#include <cstddef>

namespace {
    vsc::Text toVscText(const QString& text) {
        const QByteArray utf8 = text.toUtf8();

        return vsc::Text{
            utf8.constData(),
            static_cast<std::size_t>(utf8.size())
        };
    }

    QString fromVscText(const vsc::Text& text) {
        return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
    }
}

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

QString ConnectionController::driverForIndex(int i) const {
    switch (i) {
    case 0: return "QSQLITE";
    case 1: return "QMYSQL";
    case 2: return "QPSQL";
    case 3: return "QODBC";
    case 4: return "QOCI";
    }

    return QString();
}
QString ConnectionController::dbTypeForIndex(int i) const {
    switch (i) {
    case 0: return "sqlite";
    case 1: return "mysql";
    case 2: return "postgresql";
    case 3: return "access";
    case 4: return "oracle";
    }

     return QString();
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

bool ConnectionController::tryOpenConnection(const QString& prefix, QString& error, QString& outDriver, QString& outDbType,
    QString& outDbPath, QString& outHost, int& outPort, QString& outLogin, QString& outPassword) {

    outDriver = driverForIndex(dbTypeIndex_);
    outDbType = dbTypeForIndex(dbTypeIndex_);

    if (outDriver.isEmpty()) {
        error = tr("Unknown database type");
        return false;
    }

    outDbPath = address_.trimmed();
    outHost = isRemote_ ? address_.trimmed() : QString();
    outPort = isRemote_ ? port_.toInt() : 0;
    outLogin = isRemote_ ? login_ : QString();
    outPassword = isRemote_ ? password_ : QString();
    if (outDbPath.isEmpty()) { error = isRemote_ ? tr("Host not specified") : tr("Database file path not specified.")
            ; return false;
    }

    if (!isRemote_) {
        QFileInfo fi(outDbPath);
        if (outDriver != "QSQLITE" && outDriver != "QODBC") {
            if (!fi.exists()) {
                error = tr("File not found: ") + outDbPath;
                return false;
            }

            if (!fi.isReadable()) {
                error = tr("No permission to read the file: ") + outDbPath;
                return false;
            }
        }
    }
    QString dbName = outDbPath;

    if (outDriver == "QODBC") {
        if (outDbPath.contains(';') || outDbPath.contains('{') || outDbPath.contains('}')) {
            error = tr("The path contains invalid characters: ';', '{', '}'");
            return false;
        }

        dbName = QString("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%1;").arg(outDbPath);
    }

    const QString cn = QString("%1_%2").arg(prefix).arg(++tempConnectionCounter_);

    if (QSqlDatabase::contains(cn)) {
        QSqlDatabase old = QSqlDatabase::database(cn);
        old.close();
        old = QSqlDatabase();
        QSqlDatabase::removeDatabase(cn);
    }

    bool ok = false;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase(outDriver, cn);
        db.setDatabaseName(dbName);

        if (isRemote_) {
            db.setHostName(outHost);
            db.setPort(outPort);
            db.setUserName(outLogin);
            db.setPassword(outPassword);
        }

        ok = db.open();
        if (!ok)
            error = db.lastError().text();

        db.close();
    }

    QSqlDatabase::removeDatabase(cn);
    return ok;
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
                return std::unexpected(tr("Unknown database type"));
    }

    const bool netDatabase = params.type == vsc::DbTypes::mysql
                          || params.type == vsc::DbTypes::psql
                          || params.type == vsc::DbTypes::oracle;

    if (netDatabase) {
        const QString host = address_.trimmed();
        const QString databaseName = databaseName_.trimmed();
        const QString login = login_.trimmed();

        if (host.isEmpty() || databaseName.isEmpty() || login.isEmpty())
            return std::unexpected(tr("Fill in host, database name, and login"));

        bool validPort = false;
        const int port = port_.trimmed().toInt(&validPort);

        if (!validPort || port < 1 || port > 65535)
            return std::unexpected(tr("Invalid port into database"));

        params.host = toVscText(host);
        params.dataBaseName = toVscText(databaseName);
        params.login = toVscText(login);
        params.port = port;

        if (!password_.isEmpty())
            params.password = toVscText(password_);
    } else {
        const QString pathDb = address_.trimmed();

        if (pathDb.isEmpty())
            return std::unexpected(tr("Invalid path to db"));

        params.pathDb = toVscText(pathDb);
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
            appendLog(tr("Connection error: ") + fromVscText(connection.error().message));
            return;
        }
        else
            appendLog(tr("Connected!"));


    }
}

void ConnectionController::requestConnect() {
    QString e, d, t, p, h, l, pw;
    int pt = 0;

    if (!tryOpenConnection("preflight_connection", e, d, t, p, h, pt, l, pw)) {
        appendLog(tr("Connection error: ") + e);

        emit connectionFailed(e);

        return;
    }

    appendLog(tr("Connection successful. Launching the main window..."));

    emit connected(d, t, h, pt, l, pw, p);
}
