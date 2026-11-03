#include "qt_connection_factory.h"
#include "qt_connection.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QUuid>
#include <QByteArray>

namespace {

    QString driverDb(vsc::DbTypes type) {
        switch(type) {

        case vsc::DbTypes::none:   return QString{};
        case vsc::DbTypes::sqlite: return "QSQLITE";
        case vsc::DbTypes::mysql:  return "QMYSQL";
        case vsc::DbTypes::psql:   return "QPSQL";
        case vsc::DbTypes::oracle: return "QOCI";
        case vsc::DbTypes::access: return "QODBC";
        }
        return QString{};
    }
}

namespace vsc::qt {

    std::expected<std::unique_ptr<vsc::IConnection>, vsc::SqlError>
    QtConnectionFactory::open(const vsc::ConnectionParameters& parameters) {
        if (parameters.type == vsc::DbTypes::sqlite
            || parameters.type == vsc::DbTypes::access)
        {
            if (!parameters.pathDb || parameters.pathDb->empty())
            {
                return std::unexpected(vsc::SqlError{
                    vsc::SqlError::MessageError::connectionFailed,
                    vsc::SqlError::SourceError::connection,
                    "Database path is missing",
                    {}
                });
            }
        }
        else if (parameters.type == vsc::DbTypes::mysql
                 || parameters.type == vsc::DbTypes::psql
                 || parameters.type == vsc::DbTypes::oracle)
        {
            if (!parameters.host || parameters.host->empty()
                || !parameters.port
                || !parameters.dataBaseName || parameters.dataBaseName->empty()
                || !parameters.login || parameters.login->empty())
            {
                return std::unexpected(vsc::SqlError{
                    vsc::SqlError::MessageError::connectionFailed,
                    vsc::SqlError::SourceError::connection,
                    "Remote connection parameters are incomplete",
                    {}
                });
            }
        }
        else {
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::unsupported,
                vsc::SqlError::SourceError::driver,
                "Unsupported database type",
                {}
            });
        }


        QString dbDriver = driverDb(parameters.type);

        if (!QSqlDatabase::isDriverAvailable(dbDriver)) {
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::driverMissing,
                vsc::SqlError::SourceError::driver,
                "Unsupported database type",
                {}
            });
        }

        const QString connectionName = QStringLiteral("vsc_qt_") + QUuid::createUuid().toString(QUuid::WithoutBraces);
        QSqlDatabase database = QSqlDatabase::addDatabase(dbDriver, connectionName);

        if (!database.isValid()) {
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::connectionFailed,
                vsc::SqlError::SourceError::connection,
                "Connection failed to database",
                {}
            });
        }


        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Qt connection implementation is not initialized yet",
            {}
        });
    }
}
