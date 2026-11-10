#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlIndex>
#include <string_view>
#include <utility>

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

std::unique_ptr<QSqlQueryModel> TableRepository::select(const QString &table, const FilterList &filters, const QString &logic, int pageSize,
                                       int offset, const QString& search, const QStringList& searchableColumns) const {

    QStringList params;
    QString where = buildWhereClause(filters, logic, search, searchableColumns, params);

    QString sql = QString("SELECT * FROM %1").arg(safeName(table));
    sql += where;

    if (pageSize > 0)
        sql += QString(" LIMIT %1 OFFSET %2").arg(pageSize).arg(offset);

    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    qs.prepare(sql);

    for (int i = 0; i < params.size(); i++)
        qs.bindValue(i, params.at(i));

    auto model = std::make_unique<QSqlQueryModel>();

    if (!qs.exec()) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
        qDebug() << "SQL: " << sql;
        qDebug() << "Params: " << params;
#endif
    }

    model->setQuery(qs);
    return model;
}

bool TableRepository::insert(const QString &table, const QMap<QString, QVariant> &values) const {
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

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

    qs.prepare(sql);

    const QList<QVariant> val = values.values();

    for (int i = 0; i < val.size(); ++i)
        qs.bindValue(i, val.at(i));

    bool exe = qs.exec();

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}

bool TableRepository::insert(const QString& table, const QHash<QString, QVariant>& values) const {
    QMap<QString, QVariant> map;
    for (auto it = values.begin(); it != values.end(); it++)
        map[it.key()] = it.value();

    return insert(table, map);
}

bool TableRepository::update(const QString &table, const QString &idColumn, const QVariant &idValue, const QMap<QString, QVariant> &newValues) const
{
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QStringList list{};

    for (auto it = newValues.constBegin(); it != newValues.constEnd(); it++)
        list << QString("%1 = ?").arg(safeName(it.key()));

    QString result = list.join(", ");
    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = ?").arg(safeName(table), result, safeName(idColumn));

    qs.prepare(sql);

    const QList<QVariant> val = newValues.values();
    for (int i = 0; i < val.size(); ++i)
        qs.bindValue(i, val.at(i));

    qs.bindValue(newValues.size(), idValue);

    bool exe = qs.exec();

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}

bool TableRepository::remove(const QString &table, const QString &idColumn, const QVariant &idValue) const {
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QString sql = QString("DELETE FROM %1 WHERE %2 = ?").arg(safeName(table), safeName(idColumn));
    qs.prepare(sql);

    qs.bindValue(0, idValue);               // привязка значения к (= ?[0])

    bool exe = qs.exec();

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}

bool TableRepository::removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues) const {
    bool all = true;

    for (const QVariant& v : idValues)
        all = remove(table, idColumn, v) && all;

    return all;
}

bool TableRepository::drop_table(const QString& table) const {
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

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

bool TableRepository::rename_table(const QString& table, const QString& new_name_table) const {
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

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
    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    bool exe = qs.exec(sql);
    return exe;
}

int TableRepository::countRows(const QString &table, const FilterList &filters,
    const QString& search, const QStringList& searchableColumns) const
{

    QStringList params;
    QString where = buildWhereClause(filters, QStringLiteral(" AND "), search, searchableColumns, params);
    QString sql = QString("SELECT COUNT(*) FROM %1").arg(safeName(table));
    sql += where;

    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    qs.prepare(sql);

    for (int i = 0; i < params.size(); i++)
        qs.bindValue(i, params.at(i));

    if (qs.exec() && qs.next())
        return qs.value(0).toInt();
    else
        return 0;
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

            conditions << QString("%1 %2 %3").arg(safeName(f.colName_), op, collate);
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
