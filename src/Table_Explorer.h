#pragma once
//================================================================================================================
#include <QString>
#include <QStringList>
#include <QSqlQueryModel>
#include <QMap>
#include <QVariant>
#include <QList>
#include <QHash>
//================================================================================================================
class Table_Explorer
{
public:
    explicit Table_Explorer(const QString &connectionName, const QString& dbType = "sqlite");

    struct ColumnInfo { QString name; QString type; bool isNullable; };
    QList<ColumnInfo> getColumns(const QString &tableName) const;
    QStringList getUserTables() const;
    QStringList get_types_db() const { return types_db_; }

    QSqlQueryModel *select(const QString &table, const QMap<QString, QString> &filters = {}, const QString &logic = {}) const;

    bool insert(const QString &table, const QMap<QString, QVariant> &values) const;
    bool insert(const QString& table, const QHash<QString, QVariant>& values) const;

    bool update(const QString &table, const QString &idColumn, const QVariant &idValue, const QMap<QString, QVariant> &newValues) const;
    bool remove(const QString &table, const QString &idColumn, const QVariant &idValue) const;
    bool drop_table(const QString& table) const;
    bool rename_table(const QString& table, const QString& new_name_table) const;
    bool exeQuery(const QString& sql) const;

private:
    QString connectionName_;
    QString dbType_;
    QStringList types_db_;
};
//================================================================================================================