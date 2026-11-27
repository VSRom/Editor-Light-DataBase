#include "qt_connection.h"
#include "qt/qt_text_codec.h"
#include "qt_value_codec.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QByteArray>
#include <QSqlRecord>
#include <QSqlField>
#include <QMetaType>
#include <span>
#include <string_view>
#include <cstdint>
#include <QSqlIndex>
#include <QSqlDriver>

namespace {
    vsc::SqlError makeStatementError(const QSqlError& error)
    {
        return vsc::SqlError{
            vsc::SqlError::MessageError::internal,
            vsc::SqlError::SourceError::statement,
            vsc::qt::toVscText(error.text()),
            vsc::qt::toVscText(error.nativeErrorCode())
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

    const QString sqlText = vsc::qt::toQtText(query.sqlText);

    if (!qtQuery.prepare(sqlText))
        return std::unexpected(std::move(makeStatementError(qtQuery.lastError())));

    //  vsc::Value -> QVariant
    for (qsizetype i = 0; i < query.sqlParams.size(); i++) {
        auto value = toQtValue(query.sqlParams[i]);

        if (!value)
            return std::unexpected(std::move(value.error()));

        qtQuery.bindValue(static_cast<int>(i), *value);
    }

    if (!qtQuery.exec())
        return std::unexpected(std::move(makeStatementError(qtQuery.lastError())));

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

        vsc::ColumnMetaData col { result.allocator().resource() };

        col.nameColumn = vsc::qt::toVscText(colName, result.allocator());
        col.type = vsc::qt::toVscText(typeName, result.allocator());
        col.ordinal = i;
        col.isPk = false;
        col.isNullable = field.requiredStatus() != QSqlField::Required;

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
                return std::unexpected(std::move(convertCell.error()));

            row.push_back(std::move(*convertCell));
        }
        result.rows.push_back(std::move(row));
    }
    return result;
}

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

vsc::IDatabaseMetadata* vsc::qt::QtConnection::metadata() noexcept {
    return this;
}

std::expected<vsc::TableList, vsc::SqlError> vsc::qt::QtConnection::listUserTables() {
    if (state_ != vsc::ConState::open || !database_.isValid() || !database_.isOpen())
    {
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::notOpen,
            vsc::SqlError::SourceError::connection,
            "Database is not valid",
            {}
        });
    }

    TableList result;
    const QString driver = database_.driverName();
    const QStringList allTables = database_.tables(QSql::Tables);

    for (const auto& tableName : std::as_const(allTables)) {
        if (driver == "QSQLITE" && tableName.startsWith("sqlite_", Qt::CaseInsensitive))
            continue;
        if (driver == "QODBC" && tableName.startsWith("MSys_", Qt::CaseInsensitive))
            continue;
        if (driver == "QOCI" && ( tableName.startsWith("SYS_", Qt::CaseInsensitive) || tableName.startsWith("BIN$", Qt::CaseInsensitive) ))
            continue;

        result.tables.push_back(vsc::qt::toVscText(tableName, result.allocator()));
    }
    return result;
}

std::expected<vsc::TableMetadata, vsc::SqlError> vsc::qt::QtConnection::describeTable(std::string_view tableName) {
    if (state_ != vsc::ConState::open || !database_.isValid() || !database_.isOpen())
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::notOpen,
            vsc::SqlError::SourceError::connection,
            "Database is not valid",
            {}
        });

    if (tableName.empty())
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::syntax,
            vsc::SqlError::SourceError::statement,
            "Table name is empty",
            {}
        });


    vsc::TableMetadata result;
    const QString qtTableName = QString::fromUtf8(tableName.data(), static_cast<qsizetype>(tableName.size()));

    result.tableName = vsc::qt::toVscText(qtTableName, result.allocator());

    const QSqlIndex primaryKey = database_.primaryIndex(qtTableName);
    QStringList pkColumns;

    for (int i = 0; i < primaryKey.count(); ++i)
        pkColumns.append(primaryKey.fieldName(i));

    const QString driver = database_.driverName();
    QSqlQuery query{database_};

    if (driver == "QSQLITE") {
        const QString escapedTable = database_.driver()->escapeIdentifier(
            qtTableName,
            QSqlDriver::TableName
            );

        if (!query.exec(QString("PRAGMA table_info(%1)").arg(escapedTable)))
            return std::unexpected(std::move(makeStatementError(query.lastError())));
    }
    else if (driver == "QMYSQL") {
        const QString escapedTable = database_.driver()->escapeIdentifier(
            qtTableName,
            QSqlDriver::TableName
            );

        if (!query.exec(QString("SHOW COLUMNS FROM %1").arg(escapedTable)))
            return std::unexpected(std::move(makeStatementError(query.lastError())));
    }
    else if (driver == "QPSQL") {
        if (!query.prepare(
                "SELECT column_name, data_type, is_nullable "
                "FROM information_schema.columns "
                "WHERE table_name = :tableName "
                "AND table_schema = 'public' "
                "ORDER BY ordinal_position"
                ))
            return std::unexpected(std::move(makeStatementError(query.lastError())));

        query.bindValue(":tableName", qtTableName);

        if (!query.exec())
            return std::unexpected(std::move(makeStatementError(query.lastError())));
    }
    else if (driver == "QOCI") {
        if (!query.prepare(
                "SELECT column_name, data_type, nullable "
                "FROM user_tab_columns "
                "WHERE table_name = :tableName "
                "ORDER BY column_id"
                ))
            return std::unexpected(std::move(makeStatementError(query.lastError())));

        query.bindValue(":tableName", qtTableName.toUpper());

        if (!query.exec())
            return std::unexpected(std::move(makeStatementError(query.lastError())));
    }
    else if (driver == "QODBC") {
        if (!query.prepare(
                "SELECT COLUMN_NAME, DATA_TYPE, IS_NULLABLE "
                "FROM INFORMATION_SCHEMA.COLUMNS "
                "WHERE TABLE_NAME = :tableName "
                "ORDER BY ORDINAL_POSITION"
                ))
            return std::unexpected(std::move(makeStatementError(query.lastError())));

        query.bindValue(":tableName", qtTableName);

        if (!query.exec())
            return std::unexpected(std::move(makeStatementError(query.lastError())));
    }
    else {
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Metadata is unsupported for this driver",
            {}
        });
    }

    int ordinal = 0;

    while (query.next()) {
        const bool isSqlite = driver == "QSQLITE";
        const QString columnName = query.value(isSqlite ? 1 : 0).toString();
        const QString typeName = query.value(isSqlite ? 2 : 1).toString();

        bool isNullable = true;

        if (isSqlite)
            isNullable = !query.value(3).toBool();
        else if (driver == "QOCI")
            isNullable = query.value(2).toString().compare("Y", Qt::CaseInsensitive) == 0;
        else
            isNullable = query.value(2).toString().compare("YES", Qt::CaseInsensitive) == 0;

        vsc::ColumnMetaData column{result.allocator().resource()};
        column.nameColumn = vsc::qt::toVscText(columnName, result.allocator());
        column.type = vsc::qt::toVscText(typeName, result.allocator());
        column.isPk = pkColumns.contains(columnName, Qt::CaseInsensitive);
        column.isNullable = isNullable;
        column.ordinal = ordinal++;

        result.columns.push_back(std::move(column));
    }

    return result;
}