#include "Table_Explorer.h"
#include <QSqlError>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include <QSqlIndex>
//================================================================================================================
Table_Explorer::Table_Explorer(const QString &connectionName, const QString& dbType) : connectionName_(connectionName), dbType_(dbType)
{
    if (dbType_ == "sqlite")
        types_db_ = { "INTEGER", "REAL", "TEXT", "BLOB" };

    else if (dbType_ == "mysql")
        types_db_ = { "INT", "BIGINT", "VARCHAR(255)", "TEXT", "BLOB", "DATE", "DATETIME", "BOOLEAN" };

    else if (dbType_ == "postgresql")
        types_db_ = { "INTEGER", "SERIAL", "VARCHAR(255)", "TEXT", "BOOLEAN", "DATE", "TIMESTAMP" };

    else if (dbType_ == "access")
        types_db_ = { "INTEGER", "LONG", "TEXT", "MEMO", "YESNO","DATETIME" };

    else if (dbType_ == "oracle")
        types_db_ = { "NUMBER", "VARCHAR2(255)", "CLOB", "BLOB", "DATE", "TIMESTAMP" };

    else
        types_db_ = { "INTEGER", "TEXT", "REAL", "BLOB" };
}
//================================================================================================================
QStringList Table_Explorer::getUserTables() const {
    QSqlDatabase db = QSqlDatabase::database(connectionName_);
    QStringList AllTables = db.tables(QSql::Tables);
    QStringList userTables;

    if (dbType_ == "postgresql") {

        QSqlQuery query(QSqlDatabase::database(connectionName_));
        bool success = query.exec(R"(
            SELECT table_name 
            FROM information_schema.tables 
            WHERE table_schema = 'public' 
            AND table_type = 'BASE TABLE'
            ORDER BY table_name
        )");

        if (!success) {
            qDebug() << "Ошибка запроса таблиц:" << query.lastError().text();
            return userTables;
        }

        while (query.next()) {
            userTables.append(query.value(0).toString());
        }
    }
    else {
        QStringList AllTables = QSqlDatabase::database(connectionName_).tables(QSql::Tables);

        for (const QString& temp : AllTables) {
            if ((dbType_ == "sqlite" || dbType_ == "QSQLITE") && temp.startsWith("sqlite_"))
                continue;

            if (dbType_ == "access" && temp.startsWith("MSys_"))
                continue;

            if (dbType_ == "oracle" && (temp.startsWith("SYS_") || temp.startsWith("BIN$")))
                continue;

            userTables.append(temp);
        }
    }
    return userTables;
}
//================================================================================================================
QList<Table_Explorer::ColumnInfo> Table_Explorer::getColumns(const QString &tableName) const {
    QList<ColumnInfo> cols;

    QString driver = QSqlDatabase::database(connectionName_).driverName();
    QSqlQuery q(QSqlDatabase::database(connectionName_));

    QSqlDatabase db = QSqlDatabase::database(connectionName_);
    QSqlIndex primaryKey = db.primaryIndex(tableName);
    QStringList pkColumns;

    for (int i = 0; i < primaryKey.count(); i++)
        pkColumns.append(primaryKey.fieldName(i));  // Получили список колонок первичного ключа

    if (driver == "QSQLITE") {
        q.exec(QString("PRAGMA table_info(%1)").arg(safeName(tableName)));
        while (q.next()) {
            QString colName = q.value(1).toString();
            cols.append({ colName, q.value(2).toString(), !q.value(3).toBool(), pkColumns.contains(colName, Qt::CaseInsensitive)});
        }
    }

    else if (driver == "QMYSQL") {
        q.exec(QString("SHOW COLUMNS FROM %1").arg(safeName(tableName)));
        while (q.next()) {
            QString colName = q.value(0).toString();
            cols.append({ colName, q.value(1).toString(), q.value(2).toString() == "YES", pkColumns.contains(colName, Qt::CaseInsensitive) });
        }
    }

    else if (driver == "QPSQL") {
        q.exec(QString(
            "SELECT column_name, data_type, is_nullable "
            "FROM information_schema.columns "
            "WHERE table_name = '%1'").arg(tableName));
        while (q.next()) {
            QString colName = q.value(0).toString();
            cols.append({ colName, q.value(1).toString(), q.value(2).toString() == "YES", pkColumns.contains(colName, Qt::CaseInsensitive) });
        }
    }

    else if (driver == "QOCI") {
        q.exec(QString(
            "SELECT column_name, data_type, nullable "
            "FROM user_tab_columns "
            "WHERE table_name = '%1' "
            "ORDER BY column_id").arg(tableName.toUpper()));
        while (q.next()) {
            QString colName = q.value(0).toString();
            cols.append({ q.value(0).toString(), q.value(1).toString(), q.value(2).toString() == "Y", pkColumns.contains(colName, Qt::CaseInsensitive) });
        }
    }
    else if (driver == "QODBC") {
        q.exec(QString(
            "SELECT COLUMN_NAME, DATA_TYPE, IS_NULLABLE "
            "FROM INFORMATION_SCHEMA.COLUMNS "
            "WHERE TABLE_NAME = '%1'").arg(tableName));
        while (q.next()) {
            QString colName = q.value(0).toString();
            cols.append({ colName, q.value(1).toString(), q.value(2).toString() == "YES", pkColumns.contains(colName, Qt::CaseInsensitive)  });
       }
    }
        return cols;
}
//================================================================================================================
QSqlQueryModel *Table_Explorer::select(const QString &table, const FilterList &filters, const QString &logic, int pageSize, int offset) const {
    QString sql = QString("SELECT * FROM %1").arg(safeName(table));

    if (!filters.isEmpty()) {
        sql += " WHERE ";

        QStringList conditions;

        for (const auto &f : filters) {
            if (f.operator_ == "LIKE") {
                QString op = (dbType_ == "postgresql") ? "ILIKE" : "LIKE";
                QString collate = (dbType_ == "sqlite") ? " COLLATE NOCASE" : "";
                conditions << QString("%1 %2 ?%3").arg(safeName(f.colName_), op, collate);
            }
            else
                conditions << QString("%1 %2 ?").arg(safeName(f.colName_), f.operator_);
        }
        sql += conditions.join(logic);
    }

    if (pageSize > 0)
        sql += QString(" LIMIT %1 OFFSET %2").arg(pageSize).arg(offset);

    auto *model = new QSqlQueryModel();
    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    qs.prepare(sql);

    if (!filters.isEmpty()) {
        int idx = 0;
        for (const auto &f : filters) {
            if (f.operator_ == "LIKE")
                qs.bindValue(idx, "%" + f.value_ + "%");
            else
                qs.bindValue(idx, f.value_);
            ++idx;
        }
    }

    if (!qs.exec()) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
    }

    model->setQuery(qs);
    return model;
}
//================================================================================================================
bool Table_Explorer::insert(const QString &table, const QMap<QString, QVariant> &values) const {
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QStringList place(values.size(), "?");
    QStringList tempColum = values.keys();
    QStringList resultColum{};

    for (QString col : tempColum) {
        QString column = safeName(col);
            resultColum += column;
    }

    QString colum = resultColum.join(", ");
    QString placer = place.join(", ");                              // Подготовка данных

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(safeName(table), colum, placer);

    qs.prepare(sql);                                                // Подготовка запроса

    const QList<QVariant> val = values.values();                    // Привязка значений (Извлечение данных и привязка через bindValue)
    for (int i = 0; i < val.size(); ++i)
    qs.bindValue(i, val.at(i));

    bool exe = qs.exec();                                           // Один вызов!

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}
//================================================================================================================
bool Table_Explorer::insert(const QString& table, const QHash<QString, QVariant>& values) const {
    QMap<QString, QVariant> map;
    for (auto it = values.begin(); it != values.end(); it++)
        map[it.key()] = it.value();

    return insert(table, map);
}
//================================================================================================================
bool Table_Explorer::update(const QString &table, const QString &idColumn, const QVariant &idValue, const QMap<QString, QVariant> &newValues) const
{
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QStringList list{};

    for (auto it = newValues.constBegin(); it != newValues.constEnd(); it++)
        list << QString("%1 = ?").arg(safeName(it.key()));            // Взяли все значения в строку с разделителем %1 = ?

    QString result = list.join(", ");                           // Склеили полученную выше строку с разделителем ,

    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = ?").arg(safeName(table), result, safeName(idColumn));

    qs.prepare(sql);                                            // Подготовка запроса

    const QList<QVariant> val = newValues.values();             // Привязка значений (Извлечение данных и привязка через bindValue)
    for (int i = 0; i < val.size(); ++i)
        qs.bindValue(i, val.at(i));

    qs.bindValue(newValues.size(), idValue);                    // Привязали idValue в запрос = ?

    bool exe = qs.exec();

    if (!exe) {
#ifdef QT_DEBUG
        qDebug() << "Query error:" << qs.lastError().text();
#endif
        return false;
    }

    return exe;
}
//================================================================================================================
bool Table_Explorer::remove(const QString &table, const QString &idColumn, const QVariant &idValue) const {
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
//================================================================================================================
bool Table_Explorer::drop_table(const QString& table) const {
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
//================================================================================================================
bool Table_Explorer::rename_table(const QString& table, const QString& new_name_table) const {
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
//================================================================================================================
bool Table_Explorer::exeQuery(const QString& sql) const {
    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    bool exe = qs.exec(sql);
    return exe;
}
//================================================================================================================
int Table_Explorer::countRows(const QString &table, const FilterList &filters) const {  // Пагинация
    QString sql = QString("SELECT COUNT(*) FROM %1").arg(safeName(table));

    if (!filters.isEmpty()) {
        sql += " WHERE ";

        QStringList conditions;

        for (const auto &f : filters) {
            if (f.operator_ == "LIKE") {
                QString op = (dbType_ == "postgresql") ? "ILIKE" : "LIKE";
                QString collate = (dbType_ == "sqlite") ? " COLLATE NOCASE" : "";
                conditions << QString("%1 %2 ?%3").arg(safeName(f.colName_), op, collate);
            }
            else
                conditions << QString("%1 %2 ?").arg(safeName(f.colName_), f.operator_);
        }
        sql += conditions.join(" AND ");
    }

    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    qs.prepare(sql);

    if (!filters.isEmpty()) {
        int idx = 0;
        for (const auto &f : filters) {
            if (f.operator_ == "LIKE")
                qs.bindValue(idx, "%" + f.value_ + "%");
            else
                qs.bindValue(idx, f.value_);
            ++idx;
        }
    }

    if (qs.exec() && qs.next())
        return qs.value(0).toInt();
    else
        return 0;
}
//================================================================================================================
QString Table_Explorer::safeName(const QString& name) {
    if (name.isEmpty()) 
        return QString();

    QString copy = name;
    QString result{};
    copy.replace("\"", "\"\"");
    result = "\"" + copy + "\"";

    return result;
}
//================================================================================================================