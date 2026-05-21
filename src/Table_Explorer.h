#pragma once
//================================================================================================================
#include <QString>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include <QMap>
#include <QVariant>
#include <QList>
#include <QSqlQuery>
#include <QDebug>
//================================================================================================================
class Table_Explorer
{
public:
    explicit Table_Explorer(const QString &connectionName);

    struct ColumnInfo { QString name; QString type; bool isNullable; };
    QList<ColumnInfo> getColumns(const QString &tableName) const;
    QStringList getTables() const;

    QSqlQueryModel *select(const QString &table, const QMap<QString, QString> &filters = {}, const QString &logic = {}) const;
    bool insert(const QString &table, const QMap<QString, QVariant> &values) const;
    bool update(const QString &table, const QString &idColumn, const QVariant &idValue, const QMap<QString, QVariant> &newValues) const;
    bool remove(const QString &table, const QString &idColumn, const QVariant &idValue) const;

private:
    QString connectionName_;
};
//================================================================================================================