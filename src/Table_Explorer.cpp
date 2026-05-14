#include "Table_Explorer.h"
//================================================================================================================
Table_Explorer::Table_Explorer(QSqlDatabase &db) : db_T(db)
{
}
//================================================================================================================
QStringList Table_Explorer::getTables() const {
    return db_T.tables(QSql::Tables);
}
//================================================================================================================
QList<Table_Explorer::ColumnInfo> Table_Explorer::getColumns(const QString &tableName) const {
    QList<ColumnInfo> cols;

    QString driver = db_T.driverName();
    QSqlQuery q(db_T);

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
            "SELECT column_name, data_type, is_nullable "
            "FROM information_schema.columns "
            "WHERE table_name = '%1'").arg(tableName));
        while (q.next()) {
            cols.append({ q.value(0).toString(), q.value(1).toString(), q.value(2).toString() == "YES" });
        }
    }
    return cols;
}
//================================================================================================================
QSqlQueryModel *Table_Explorer::select(const QString &table, const QMap<QString, QString> &filters) const {
    QString sql = QString("SELECT * FROM %1").arg(table);

    if (!filters.isEmpty()) {
        sql += " WHERE ";
        QStringList conditions;
        for (auto it = filters.constBegin(); it != filters.constEnd(); ++it)
            conditions << QString("%1 LIKE ?").arg(it.key());

        sql += conditions.join(" AND ");
    }

    auto *model = new QSqlQueryModel();
    QSqlQuery q(db_T);
    q.prepare(sql);

    if (!filters.isEmpty()) {
        int idx = 0;
        for (auto it = filters.constBegin(); it != filters.constEnd(); ++it, ++idx) {
            q.bindValue(idx, "%" + it.value() + "%");
        }
    }

    if (!q.exec()) {
        qDebug() << "Query error:" << q.lastError().text();
    }

    model->setQuery(q);
    return model;
}
//================================================================================================================
bool Table_Explorer::insert(const QString &table, const QMap<QString, QVariant> &values) const
{
    QSqlQuery qs(db_T);

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
    QSqlQuery qs(db_T);

    QStringList list{};

    for (auto it = newValues.constBegin(); it != newValues.constEnd(); it++)
        list << QString("%1 = ?").arg(it.key());                // Взяли все значения в строку с разделителем %1 = ?

    QString result = list.join(", ");                           // Склеили полученную выше строку с разделителем ,

    QString sql = QString("UPDATE %1 SET %2 WHERE %3 = ?").arg(table, result, idColumn);

    qs.prepare(sql);                                                // Подготовка запроса

    const QList<QVariant> val = newValues.values();                    // Привязка значений (Извлечение данных и привязка через bindValue)
    for (int i = 0; i < val.size(); ++i)
        qs.bindValue(i, val.at(i));

    qs.bindValue(newValues.size(), idValue);                        // Привязали idValue в запрос

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

}
//================================================================================================================