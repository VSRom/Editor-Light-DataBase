#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlIndex>
#include <string_view>
#include <utility>
#include <limits>

#include "qt/qt_value_codec.h"
#include "vsc/i_database_metadata.h"
#include "qt/qt_text_codec.h"
#include "TableRepository.h"

TableRepository::TableRepository(vsc::IConnection* connection, vsc::DbTypes type)
    :  connection_(connection), type_(type)
{
    if (type_ == vsc::DbTypes::sqlite)
        types_db_ = { "INTEGER", "REAL", "TEXT", "BLOB" };
    else if (type_== vsc::DbTypes::mysql)
        types_db_ = { "INT", "BIGINT", "VARCHAR(255)", "TEXT", "BLOB", "DATE", "DATETIME", "BOOLEAN" };
    else if (type_ == vsc::DbTypes::psql)
        types_db_ = { "INTEGER", "SERIAL", "VARCHAR(255)", "TEXT", "BOOLEAN", "DATE", "TIMESTAMP" };
    else if (type_ == vsc::DbTypes::access)
        types_db_ = { "INTEGER", "LONG", "TEXT", "MEMO", "YESNO","DATETIME" };
    else if (type_ == vsc::DbTypes::oracle)
        types_db_ = { "NUMBER", "VARCHAR2(255)", "CLOB", "BLOB", "DATE", "TIMESTAMP" };
    else
        types_db_ = { "INTEGER", "TEXT", "REAL", "BLOB" };
}

std::expected<QStringList, vsc::SqlError> TableRepository::getUserTables() const {
    if(!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Not connection",
            {}
        });

    auto metadata = connection_->metadata();

    if(!metadata)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Connection does not expose database metadata",
            {}
        });

    auto coreTables = metadata->listUserTables();

    if(!coreTables)
        return std::unexpected(std::move(coreTables.error()));

    QStringList result{};

    for(const auto& table : std::as_const(coreTables->tables))
        result += vsc::qt::toQtText(table);

    return result;
}

std::expected<QList<TableRepository::ColumnInfo>, vsc::SqlError> TableRepository::getColumns(const QString &tableName) const {
    if(!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Connection is missing",
            {}
        });

    auto metadata = connection_->metadata();

    if(!metadata)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::unsupported,
            vsc::SqlError::SourceError::driver,
            "Connection does not expose database metadata",
            {}
        });

    auto coreTableName = vsc::qt::toVscText(tableName);
    auto schema = metadata->describeTable(std::string_view{coreTableName.data(), coreTableName.size()});

    if(!schema)
        return std::unexpected(std::move(schema.error()));

    QList<ColumnInfo> result{};

    for (const auto& col : std::as_const(schema->columns)) {
        ColumnInfo tempResult;

        tempResult.name = vsc::qt::toQtText(col.nameColumn);
        tempResult.type = vsc::qt::toQtText(col.type);
        tempResult.isNullable = col.isNullable;
        tempResult.isPrimaryKey = col.isPk;

        result += tempResult;
    }
    return result;
}

std::expected<TableRepository::TablePage, vsc::SqlError> TableRepository::select(const QString &table, const FilterList &filters, const QString &logic, int pageSize,
                                       int offset, const QString& search, const QStringList& searchableColumns) const {

    QStringList params;
    QString where = buildWhereClause(filters, logic, search, searchableColumns, params);

    QString sql = QString("SELECT * FROM %1").arg(safeName(table));
    sql += where;

    if (!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Connection is missing",
            {}
        });

    if (pageSize > 0)
        sql += QString(" LIMIT %1 OFFSET %2").arg(pageSize).arg(offset);

    vsc::CmdQuery query{};
    query.sqlText = vsc::qt::toVscText(sql);

    vsc::Text::allocator_type alloc{query.sqlParams.get_allocator().resource()};

    for (auto& temp : std::as_const(params)) {
        auto converted = vsc::qt::fromQtValue(QVariant{temp}, alloc);

        if (!converted)
            return std::unexpected(converted.error());

        query.sqlParams.push_back(*converted);
    }

    auto coreResult = connection_->execute(query);

    if (!coreResult)
        return std::unexpected(coreResult.error());

    TablePage pages{};

    for (const auto& col : std::as_const(coreResult->cols))
        pages.headers.push_back(vsc::qt::toQtText(col.nameColumn));

    for (const auto& row : std::as_const(coreResult->rows)) {
        QList<QVariant> result{};

        for (const auto& cell : std::as_const(row)) {
            auto converted = vsc::qt::toQtValue(cell);

            if (!converted)
                return std::unexpected(converted.error());

            result += *converted;
        }
        pages.rows.push_back(result);
    }
    return pages;
}

std::expected<void, vsc::SqlError> TableRepository::insert(const QString &table, const QMap<QString, QVariant> &values) const {
    if (!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Connection is missing",
            {}
        });

    if (values.isEmpty())
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::internal,
            vsc::SqlError::SourceError::statement,
            "Values is empty",
            {}
        });

    QStringList place(values.size(), "?");
    QStringList tempColum = values.keys();
    QStringList resultColum{};

    for (const auto& col : std::as_const(tempColum)) {
        QString column = safeName(col);
            resultColum += column;
    }

    QString colum = resultColum.join(", ");
    QString placer = place.join(", ");

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(safeName(table), colum, placer);

    vsc::CmdQuery query{};

    query.sqlText = vsc::qt::toVscText(sql);
    query.mode = vsc::CmdQuery::Mode::write;
    vsc::Text::allocator_type alloc{query.sqlParams.get_allocator().resource()};

    for (const auto& value : values) {
        auto converted = vsc::qt::fromQtValue(QVariant{value}, alloc);

        if (!converted)
            return std::unexpected(converted.error());

        query.sqlParams.push_back(*converted);
    }

    auto coreResult = connection_->execute(query);

    if (!coreResult)
        return std::unexpected(coreResult.error());

    return {};
}

std::expected<void, vsc::SqlError> TableRepository::insert(const QString& table, const QHash<QString, QVariant>& values) const {
    QMap<QString, QVariant> map;
    for (auto it = values.begin(); it != values.end(); it++)
        map[it.key()] = it.value();

    return insert(table, map);
}

std::expected<void, vsc::SqlError> TableRepository::update(const QString &table, const QString &idColumn, const QVariant &idValue, const QMap<QString, QVariant> &newValues) const {
    if (!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Connection is missing",
            {}
        });

    if (newValues.isEmpty() || table.isEmpty() || idColumn.isEmpty() || idValue.isNull() || !idValue.isValid())
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::internal,
            vsc::SqlError::SourceError::statement,
            "Values is empty",
            {}
        });

    QStringList list{};

    for (auto it = newValues.constBegin(); it != newValues.constEnd(); it++)
        list << QString("%1 = ?").arg(safeName(it.key()));

    QString result = list.join(", ");
    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = ?").arg(safeName(table), result, safeName(idColumn));

    vsc::CmdQuery query{};
    query.sqlText = vsc::qt::toVscText(sql);
    query.mode = vsc::CmdQuery::Mode::write;
    vsc::Text::allocator_type alloc{query.sqlParams.get_allocator().resource()};

    for (const auto& value : newValues) {
        auto converted = vsc::qt::fromQtValue(QVariant{value}, alloc);

        if (!converted)
            return std::unexpected(converted.error());

        query.sqlParams.push_back(*converted);
    }

    auto resus = vsc::qt::fromQtValue(idValue, alloc);

    if (!resus)
        return std::unexpected(resus.error());

    query.sqlParams.push_back(*resus);

    auto coreResult = connection_->execute(query);

    if (!coreResult)
        return std::unexpected(coreResult.error());

    return{};
}

std::expected<void, vsc::SqlError> TableRepository::remove(const QString &table, const QString &idColumn, const QVariant &idValue) const {
    if (!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Connection is missing",
            {}
        });

    if (table.isEmpty() || idColumn.isEmpty() || idValue.isNull() || !idValue.isValid())
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::internal,
            vsc::SqlError::SourceError::statement,
            "Values is empty",
            {}
        });

    QString sql = QString("DELETE FROM %1 WHERE %2 = ?").arg(safeName(table), safeName(idColumn));

    vsc::CmdQuery query{};

    query.sqlText = vsc::qt::toVscText(sql);
    query.mode = vsc::CmdQuery::Mode::write;

    vsc::Text::allocator_type alloc{query.sqlParams.get_allocator().resource()};

    auto resus = vsc::qt::fromQtValue(idValue, alloc);

    if (!resus)
        return std::unexpected(resus.error());

    query.sqlParams.push_back(*resus);

    auto coreResult = connection_->execute(query);

    if (!coreResult)
        return std::unexpected(coreResult.error());

    return{};
}

std::expected<void, vsc::SqlError> TableRepository::removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues) const {
    if (idValues.isEmpty())
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::internal,
            vsc::SqlError::SourceError::statement,
            "No rows selected for deletion",
            {}
        });

    for (const QVariant& v : idValues) {
        auto result = remove(table, idColumn, v);

        if (!result)
            return std::unexpected(result.error());
    }

    return {};
}

std::expected<void, vsc::SqlError> TableRepository::drop_table(const QString& table) const {
    QSqlQuery qs(QSqlDatabase::database());

    if (table.isEmpty()) return false;

    QString sql = QString("DROP TABLE %1").arg(safeName(table));
    bool exe = qs.exec(sql);

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}

std::expected<void, vsc::SqlError> TableRepository::rename_table(const QString& table, const QString& new_name_table) const {
    QSqlQuery qs(QSqlDatabase::database());

    if (table.isEmpty() || new_name_table.isEmpty()) return false;

    QString sql = QString("ALTER TABLE %1 RENAME TO %2").arg(safeName(table), safeName(new_name_table));
    bool exe = qs.exec(sql);

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}

bool TableRepository::exeQuery(const QString& sql) const {
    QSqlQuery qs(QSqlDatabase::database());
    bool exe = qs.exec(sql);
    return exe;
}

std::expected<int, vsc::SqlError> TableRepository::countRows(const QString &table, const FilterList &filters,
    const QString& search, const QStringList& searchableColumns) const
{

    QStringList params;
    QString where = buildWhereClause(filters, QStringLiteral(" AND "), search, searchableColumns, params);
    QString sql = QString("SELECT COUNT(*) FROM %1").arg(safeName(table));
    sql += where;

    if (!connection_)
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::connectionFailed,
            vsc::SqlError::SourceError::connection,
            "Connection is missing",
            {}
        });

    vsc::CmdQuery query{};
    query.sqlText = vsc::qt::toVscText(sql);
    vsc::Text::allocator_type alloc{query.sqlParams.get_allocator().resource()};

    for (auto& temp : std::as_const(params)) {
        auto converted = vsc::qt::fromQtValue(QVariant{temp}, alloc);

        if (!converted)
            return std::unexpected(converted.error());

        query.sqlParams.push_back(*converted);
    }

    auto coreResult = connection_->execute(query);

    if (!coreResult)
        return std::unexpected(coreResult.error());

    if(!coreResult->rows.empty() && !coreResult->rows[0].empty()) {
        auto count = coreResult->rows[0][0].asInt64();

        if (!count)
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::internal,
                vsc::SqlError::SourceError::statement,
                "COUNT returned no value",
                {}
            });

        if (*count < 0 || *count > std::numeric_limits<int>::max())
            return std::unexpected(vsc::SqlError{
                vsc::SqlError::MessageError::internal,
                vsc::SqlError::SourceError::statement,
                "COUNT returned no value",
                {}
            });
        else
            return static_cast<int>(*count);
    }
    else
        return std::unexpected(vsc::SqlError{
            vsc::SqlError::MessageError::internal,
            vsc::SqlError::SourceError::statement,
            "COUNT returned no value",
            {}
        });
}

QString TableRepository::safeName(const QString& name) {
    if (name.isEmpty()) 
        return QString();

    QString copy = name;
    QString result{};
    copy.replace("\"", "\"\"");
    result = "\"" + copy + "\"";

    return result;
}

QString TableRepository::buildSearchCondition(const QString& search, const QStringList& cols, QStringList& outParams) const {
    if (search.isEmpty() || cols.isEmpty()) return QString();

    QString esc = search;
    const bool useEscape = !(type_ == vsc::DbTypes::access);

    if (useEscape) {
        esc.replace("\\", "\\\\").replace("%", "\\%").replace("_", "\\_");
    }

    const QString pattern = "%" + esc + "%";
    const QString op = (type_ == vsc::DbTypes::psql) ? "ILIKE" : "LIKE";
    const QString tail = useEscape ? " ESCAPE '\\'" : "";

    QStringList ors;

    for (const auto& c : std::as_const(cols)) {
        QString castExpr;
        if (type_ == vsc::DbTypes::sqlite|| type_ == vsc::DbTypes::psql)
            castExpr = QString("CAST(%1 AS TEXT)").arg(safeName(c));

        else if (type_ == vsc::DbTypes::mysql)
            castExpr = QString("CAST(%1 AS CHAR)").arg(safeName(c));

        else if (type_ == vsc::DbTypes::oracle)
            castExpr = QString("TO_CHAR(%1)").arg(safeName(c));
        else
            castExpr = safeName(c);

        ors << QString("%1 %2 ?%3").arg(castExpr, op, tail);

        outParams << pattern;
    }
    return "(" + ors.join(" OR ") + ")";
}

QString TableRepository::buildWhereClause(const FilterList& filters, const QString& logic, const QString& search
                                         , const QStringList& searchableColumns, QStringList& outParams) const {
    QStringList conditions;
    outParams.clear();

    for(const auto& f : std::as_const(filters)) {
        if(f.operator_ == "LIKE") {
            QString op = (type_ == vsc::DbTypes::psql) ? "ILIKE" : "LIKE";
            QString collate = (type_ == vsc::DbTypes::sqlite) ? " COLLATE NOCASE " : "";

            conditions << QString("%1 %2 ?%3").arg(safeName(f.colName_), op, collate);
            outParams << "%" + f.value_ + "%";
        }
        else {
            conditions << QString("%1 %2 ?").arg(safeName(f.colName_), f.operator_);
            outParams << f.value_;
        }
    }

    QStringList searchParams;
    QString searchCond = buildSearchCondition(search, searchableColumns, searchParams);

    if (!searchCond.isEmpty()) {
        conditions << searchCond;
        outParams += searchParams;
    }

    if (conditions.isEmpty())
        return QString();

    const QString joiner = logic.isEmpty() ? QStringLiteral(" AND ") : logic;

    return " WHERE " + conditions.join(joiner);
}
