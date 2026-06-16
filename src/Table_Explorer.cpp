#include "Table_Explorer.h"
#include <QSqlError>
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
    QStringList AllTables = QSqlDatabase::database(connectionName_).tables(QSql::Tables);
    QStringList userTables = {};

    for (const QString& temp : AllTables) {
        if (dbType_ == "sqlite" && temp.startsWith("sqlite_"))
            continue;

        if (dbType_ == "access" && temp.startsWith("MSys_"))
            continue;

        if (dbType_ == "oracle" && (temp.startsWith("SYS_") || temp.startsWith("BIN$")))
            continue;

        userTables.append(temp);
    }
    return userTables;
}
//================================================================================================================
QList<Table_Explorer::ColumnInfo> Table_Explorer::getColumns(const QString &tableName) const {
    QList<ColumnInfo> cols;

    QString driver = QSqlDatabase::database(connectionName_).driverName();
    QSqlQuery q(QSqlDatabase::database(connectionName_));

    if (driver == "QSQLITE") {
        q.exec(QString("PRAGMA table_info(%1)").arg(tableName));
        while (q.next()) {
            cols.append({ q.value(1).toString(), q.value(2).toString(), !q.value(3).toBool() });
        }
    }
    else if (driver == "QMYSQL") {
        q.exec(QString("SHOW COLUMNS FROM %1").arg(tableName));
        while (q.next()) {
            cols.append({ q.value(0).toString(), q.value(1).toString(), q.value(2).toString() == "YES" });
        }
    }
    else if (driver == "QPSQL") {
        q.exec(QString(
            "SELECT column_name, data_type, is_nullable"
            "FROM information_schema.columns"
            "WHERE table_name = '%1'").arg(tableName));
        while (q.next()) {
            cols.append({ q.value(0).toString(), q.value(1).toString(), q.value(2).toString() == "YES" });
        }
    }
    else if (driver == "QOCI") {
        q.exec(QString(
            "SELECT column_name, data_type, nullable "
            "FROM user_tab_columns "
            "WHERE table_name = '%1' "
            "ORDER BY column_id").arg(tableName.toUpper()));

        while (q.next()) {
            cols.append({
                q.value(0).toString(), q.value(1).toString(), q.value(2).toString() == "Y" });
        }
    }
    else
        return cols;
}
//================================================================================================================
QSqlQueryModel *Table_Explorer::select(const QString &table, const QMap<QString, QString> &filters, const QString &logic) const {
    
    QString sql = QString("SELECT * FROM %1").arg(table);

    if (!filters.isEmpty()) {
        sql += " WHERE ";
        QStringList conditions;
        for (auto it = filters.constBegin(); it != filters.constEnd(); ++it)
            conditions << QString("%1 LIKE ? COLLATE NOCASE").arg(it.key());

        sql += conditions.join(logic);                                      // Для использования OR или AND
    }

    auto *model = new QSqlQueryModel();
    QSqlQuery qs(QSqlDatabase::database(connectionName_));
    qs.prepare(sql);

    if (!filters.isEmpty()) {
        int idx = 0;
        for (auto it = filters.constBegin(); it != filters.constEnd(); ++it, ++idx) {
            qs.bindValue(idx, "%" + it.value() + "%");
        }
    }

    if (!qs.exec()) {
        qDebug() << "Query error:" << qs.lastError().text();
    }

    model->setQuery(qs);
    //  qDebug() << model->rowCount();  // Сколько выдаёт строк после поиска
    return model;
}
//================================================================================================================
bool Table_Explorer::insert(const QString &table, const QMap<QString, QVariant> &values) const
{
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QStringList place(values.size(), "?");
    QString colum = values.keys().join(", ");
    QString placer = place.join(", ");                              // Подготовка данных

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table, colum, placer);

    qs.prepare(sql);                                                // Подготовка запроса

    const QList<QVariant> val = values.values();                    // Привязка значений (Извлечение данных и привязка через bindValue)
    for (int i = 0; i < val.size(); ++i)
    qs.bindValue(i, val.at(i));

    bool exe = qs.exec();                                           // Один вызов!

    if (!exe) {
        qDebug() << "Query error:" << qs.lastError().text();
        return false;
    }

    return exe;
}
//================================================================================================================
bool Table_Explorer::update(const QString &table, const QString &idColumn, const QVariant &idValue, const QMap<QString, QVariant> &newValues) const
{
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QStringList list{};

    for (auto it = newValues.constBegin(); it != newValues.constEnd(); it++)
        list << QString("%1 = ?").arg(it.key());                // Взяли все значения в строку с разделителем %1 = ?

    QString result = list.join(", ");                           // Склеили полученную выше строку с разделителем ,

    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = ?").arg(table, result, idColumn);

    qs.prepare(sql);                                            // Подготовка запроса

    const QList<QVariant> val = newValues.values();             // Привязка значений (Извлечение данных и привязка через bindValue)
    for (int i = 0; i < val.size(); ++i)
        qs.bindValue(i, val.at(i));

    qs.bindValue(newValues.size(), idValue);                    // Привязали idValue в запрос = ?

    bool exe = qs.exec();

    if (!exe) {
        qDebug() << "Query error:" << qs.lastError().text();
        return false;
    }

    return exe;
}
//================================================================================================================
bool Table_Explorer::remove(const QString &table, const QString &idColumn, const QVariant &idValue) const
{
    QSqlQuery qs(QSqlDatabase::database(connectionName_));

    QString sql = QString("DELETE FROM %1 WHERE %2 = ?").arg(table, idColumn);

    qs.prepare(sql);

    qs.bindValue(0, idValue);               // привязка значения к (= ?[0])

    bool exe = qs.exec();

    if (!exe) {
        qDebug() << "Query error:" << qs.lastError().text();
        return false;
    }

    return exe;
}
//================================================================================================================