#include "qt_connection.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QByteArray>
#include <QSqlRecord>
#include <QSqlField>
#include <QMetaType>
#include <span>
#include <string_view>
#include <cstdint>

namespace {

vsc::Text toVscText(const QString& text)
{
    const QByteArray utf8 = text.toUtf8();

    return vsc::Text{
        utf8.constData(),
        static_cast<std::size_t>(utf8.size())
    };
}

vsc::SqlError makeStatementError(const QSqlError& error)
{
    return vsc::SqlError{
        vsc::SqlError::MessageError::internal,
        vsc::SqlError::SourceError::statement,
        toVscText(error.text()),
        toVscText(error.nativeErrorCode())
    };
}

}

vsc::qt::QtConnection::QtConnection(QSqlDatabase database, QString connectionName)
    : database_(std::move(database)), connectionName_(std::move(connectionName)),
    state_(database_.isValid() && database_.isOpen() ? vsc::ConState::open : vsc::ConState::failed)
    { }

vsc::qt::QtConnection::~QtConnection() {
    (void)close();
}

vsc::ConState vsc::qt::QtConnection::state() const noexcept {
    return state_;
}
//================================================================================================//================================================================================================
//================================================================================================//================================================================================================
//================================================================================================//================================================================================================
std::expected<vsc::ResultSet, vsc::SqlError> vsc::qt::QtConnection::execute(const vsc::CmdQuery& query) {
    if (state_ != vsc::ConState::open || !database_.isValid() || !database_.isOpen())
    {
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::notOpen,
            vsc::SqlError::SourceError::connection,
            "Database is not valid",
            {}
        });
    }

    QSqlQuery qtQuery{database_};

    const QString sqlText = QString::fromUtf8(
        query.sqlText.data(),
        static_cast<qsizetype>(query.sqlText.size())
        );

    if (!qtQuery.prepare(sqlText))
        return std::unexpected(makeStatementError(qtQuery.lastError()));

    //  vsc::Value -> QVariant
    for (qsizetype i = 0; i < query.sqlParams.size(); i++) {
        auto value = toQtValue(query.sqlParams[i]);

        if (!value)
            return std::unexpected(value.error());

        qtQuery.bindValue(static_cast<int>(i), *value);
    }

    if (!qtQuery.exec())
        return std::unexpected(makeStatementError(qtQuery.lastError()));

    vsc::ResultSet result;

    if (!qtQuery.isSelect()) {
        result.kind = vsc::ResultSet::ResultKind::affected;
        result.affectedRows = qtQuery.numRowsAffected();

        const QVariant id = qtQuery.lastInsertId();

        if (id.isValid() && !id.isNull())
            result.lastInsertId = id.toLongLong();

        return result;
    }

    result.kind = vsc::ResultSet::ResultKind::rows;
    auto* resource = result.allocator().resource();

    const QSqlRecord record = qtQuery.record();
    const int colCount = record.count();

    for (int i = 0; i < colCount; ++i)
    {
        const QSqlField field = record.field(i);
        const QString colName = field.name();
        const QMetaType metaType = field.metaType();
        const char* rawTypeName = metaType.name();
        const QString typeName = rawTypeName ? QString::fromLatin1(rawTypeName) : QString{};

        vsc::ColumnMetaData col { outStringPmr(colName, result.allocator()),
                                  outStringPmr(typeName, result.allocator()), false,
                                  field.requiredStatus() != QSqlField::Required, i
                                };


        result.cols.push_back(std::move(col));
    }



    while(qtQuery.next()) {
        std::pmr::vector<vsc::Value> row{resource};
        row.reserve(colCount);

        for(int i = 0; i < colCount; ++i)
        {
            const QVariant cell = qtQuery.value(i);
            auto convertCell = fromQtValue(cell, result.allocator());

            if (!convertCell)
                return std::unexpected(convertCell.error());

            row.push_back(std::move(*convertCell));
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}
//================================================================================================//================================================================================================
//================================================================================================//================================================================================================
//================================================================================================//================================================================================================
std::expected<std::unique_ptr<vsc::ITransaction>, vsc::SqlError> vsc::qt::QtConnection::beginTransaction()
{
    return std::unexpected(vsc::SqlError{
        vsc::SqlError::MessageError::unsupported,
        vsc::SqlError::SourceError::transaction,
        "Transactions are not implemented yet",
        {}
    });
}

std::expected<void, vsc::SqlError> vsc::qt::QtConnection::close() {
    if (state_ == vsc::ConState::disposed)
        return {};

    if (database_.isValid())
        database_.close();

    const QString name = connectionName_;
    database_ = QSqlDatabase{};

    if (!name.isEmpty())
        QSqlDatabase::removeDatabase(name);

    connectionName_.clear();

    state_ = vsc::ConState::disposed;

    return {};
}

std::expected<QVariant, vsc::SqlError> vsc::qt::QtConnection::toQtValue(const vsc::Value& value) {
    switch(value.type()) {
        case ValueType::null: return QVariant{};

        case ValueType::integer: {
            if (auto number = value.asInt64())
                return QVariant::fromValue<qlonglong>(*number);

            break;
        }

        case ValueType::real: {
                if (auto number = value.asDouble())
                    return QVariant::fromValue<double>(*number);

            break;
        }

        case ValueType::boolean: {
            if (auto number = value.asBool())
                return QVariant::fromValue<bool>(*number);

            break;
        }

        case ValueType::text: {
            if (auto number  = value.asString())
                return QVariant{ QString::fromUtf8(number->data(),
                                static_cast<qsizetype>(number->size()))
                        };

            break;
        }

        case vsc::ValueType::blob: {
            if (auto blob = value.asBlob()) {
                const char* data = reinterpret_cast<const char*>(blob->data());

                return QVariant{ QByteArray{ data, static_cast<qsizetype>(blob->size()) } };
            }

            break;
        }

        case ValueType::decimal: {
            if (auto decimal = value.asDecimal()) {
                return QVariant{ QString::fromUtf8(decimal->repr.data(),
                                static_cast<qsizetype>(decimal->repr.size()))
                        };
            }

            break;
        }
    }

    return std::unexpected(vsc::SqlError{
        vsc::SqlError::MessageError::internal,
        vsc::SqlError::SourceError::statement,
        "Value conversion failed",
        {}
    });
}

std::expected<vsc::Value, vsc::SqlError> vsc::qt::QtConnection::fromQtValue(const QVariant& value, vsc::Text::allocator_type alloc) {
    if (!value.isValid() || value.isNull())
        return vsc::makeNull();

    switch (value.metaType().id()) {
    case QMetaType::Int:
    case QMetaType::LongLong:
        return vsc::makeInt(value.toLongLong());

    case QMetaType::Double:
    case QMetaType::Float:
        return vsc::makeReal(value.toDouble());

    case QMetaType::Bool:
        return vsc::makeBool(value.toBool());

    case QMetaType::QString: {
        const QByteArray utf8 = value.toString().toUtf8();

        return vsc::makeText(
            alloc,
            std::string_view{
                utf8.constData(),
                static_cast<std::size_t>(utf8.size())
            });
    }

    case QMetaType::QByteArray: {
        const QByteArray bytes = value.toByteArray();

        const auto* data =
            reinterpret_cast<const std::uint8_t*>(bytes.constData());

        return vsc::makeBlob(
            alloc,
            std::span<const std::uint8_t>{
                data,
                static_cast<std::size_t>(bytes.size())
            });
    }

    default:
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Unsupported Qt value type",
            {}
        });
    }
}

vsc::Text vsc::qt::QtConnection::outStringPmr(const QString& str, vsc::Text::allocator_type alloc) {
    vsc::Text output{alloc};
    const QByteArray utf8 = str.toUtf8();

    output.assign(utf8.constData(), static_cast<std::size_t>(utf8.size()));

    return output;
}