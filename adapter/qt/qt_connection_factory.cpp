#include "qt_connection_factory.h"
#include "qt_connection.h"

#include <QByteArray>
#include <QSqlDatabase>
#include <QSqlError>
#include <QUuid>

#include <cstddef>
#include <utility>

namespace {

QString driverDb(vsc::DbTypes type)
{
    switch (type) {
    case vsc::DbTypes::none:   return {};
    case vsc::DbTypes::sqlite: return QStringLiteral("QSQLITE");
    case vsc::DbTypes::mysql:  return QStringLiteral("QMYSQL");
    case vsc::DbTypes::psql:   return QStringLiteral("QPSQL");
    case vsc::DbTypes::oracle: return QStringLiteral("QOCI");
    case vsc::DbTypes::access: return QStringLiteral("QODBC");
    }

    return {};
}

QString toQtText(const vsc::Text& text)
{
    return QString::fromUtf8(
        text.data(),
        static_cast<qsizetype>(text.size()));
}

vsc::Text toVscText(const QString& text)
{
    const QByteArray utf8 = text.toUtf8();

    return vsc::Text{
        utf8.constData(),
        static_cast<std::size_t>(utf8.size())
    };
}

vsc::SqlError makeConnectionError(const QSqlError& error)
{
    return vsc::SqlError{
        vsc::SqlError::MessageError::connectionFailed,
        vsc::SqlError::SourceError::connection,
        toVscText(error.text()),
        toVscText(error.nativeErrorCode())
    };
}

void removeConnection(QSqlDatabase& database, const QString& connectionName)
{
    database.close();
    database = QSqlDatabase{};

    if (QSqlDatabase::contains(connectionName))
        QSqlDatabase::removeDatabase(connectionName);
}

}

namespace vsc::qt {

std::expected<std::unique_ptr<vsc::IConnection>, vsc::SqlError>
QtConnectionFactory::open(const vsc::ConnectionParameters& parameters)
{
    if (parameters.type == vsc::DbTypes::sqlite
        || parameters.type == vsc::DbTypes::access) {
        if (!parameters.pathDb || parameters.pathDb->empty()) {
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::connectionFailed,
                vsc::SqlError::SourceError::connection,
                "Database path is missing",
                {}
            });
        }
    } else if (parameters.type == vsc::DbTypes::mysql
               || parameters.type == vsc::DbTypes::psql
               || parameters.type == vsc::DbTypes::oracle) {
        if (!parameters.host || parameters.host->empty()
            || !parameters.port
            || *parameters.port <= 0 || *parameters.port > 65535
            || !parameters.dataBaseName || parameters.dataBaseName->empty()
            || !parameters.login || parameters.login->empty()) {
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::connectionFailed,
                vsc::SqlError::SourceError::connection,
                "Remote connection parameters are incomplete",
                {}
            });
        }
    } else {
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Unsupported database type",
            {}
        });
    }

    const QString dbDriver = driverDb(parameters.type);

    if (!QSqlDatabase::isDriverAvailable(dbDriver)) {
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::driverMissing,
            vsc::SqlError::SourceError::driver,
            "Qt SQL driver is not available",
            {}
        });
    }

    const QString connectionName = QStringLiteral("vsc_qt_")
        + QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlDatabase database = QSqlDatabase::addDatabase(dbDriver, connectionName);

    if (!database.isValid()) {
        removeConnection(database, connectionName);

        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Qt could not create a database connection",
            {}
        });
    }

    if (parameters.type == vsc::DbTypes::sqlite) {
        database.setDatabaseName(toQtText(*parameters.pathDb));
    } else if (parameters.type == vsc::DbTypes::access) {
        const QString path = toQtText(*parameters.pathDb);

        if (path.contains(u';') || path.contains(u'{') || path.contains(u'}')) {
            removeConnection(database, connectionName);

            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::connectionFailed,
                vsc::SqlError::SourceError::connection,
                "Access database path contains unsupported characters",
                {}
            });
        }

        database.setDatabaseName(
            QStringLiteral("DRIVER={Microsoft Access Driver (*.mdb, *.accdb)};DBQ=%1;")
                .arg(path));
    } else {
        database.setHostName(toQtText(*parameters.host));
        database.setPort(*parameters.port);
        database.setDatabaseName(toQtText(*parameters.dataBaseName));
        database.setUserName(toQtText(*parameters.login));

        if (parameters.password)
            database.setPassword(toQtText(*parameters.password));
    }

    if (!database.open()) {
        vsc::SqlError error = makeConnectionError(database.lastError());
        removeConnection(database, connectionName);

        return std::unexpected(std::move(error));
    }

    return std::make_unique<QtConnection>(std::move(database), connectionName);
}

}
