#include "qt_connection_factory.h"

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

        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Qt connection implementation is not initialized yet",
            {}
        });
    }

}
