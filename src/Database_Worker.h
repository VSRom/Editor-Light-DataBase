#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QMap>
#include <QHash>
#include "Table_Explorer.h"
//================================================================================================================
typedef QHash<QString, QVariant> Hash;
typedef QMap<QString, QVariant> Map;
//================================================================================================================
Q_DECLARE_METATYPE(Hash)
Q_DECLARE_METATYPE(Map)
//================================================================================================================
class Database_Worker : public QObject {
    Q_OBJECT
public:
    explicit Database_Worker(const QString& connectionName, const QString& dbType, QObject* parent = nullptr);
    ~Database_Worker();

public slots:
    void loadTables();
    void selectTable(const QString& table);
    void executeQuery(const QString& sql);
    void getColumns(const QString& table);
    void insertRow(const QString& table, const QHash<QString, QVariant>& values);
    void updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues);
    void removeRow(const QString& table, const QString& idColumn, const QVariant& idValue);
    void getTypesDb();
    void initConnection(const QString& driver, const QString& dbPath);
    void dropTable(const QString& table);
    void renameTable(const QString& oldName, const QString& newName);

signals:
    void tablesLoaded(QStringList tables);
    void selectFinished(QList<QList<QVariant>> data, QStringList headers);
    void operationCompleted(bool success, const QString& message);
    void columnsLoaded(const QString& tableName, QList<Table_Explorer::ColumnInfo> columns);
    void typesDbLoaded(QStringList types);
    void errorOccurred(const QString& error);

private:
    Table_Explorer* explorer_;
    QString connection_name_;
    QString driver_;
    QString db_path_;
    QString dbType_;
};
//================================================================================================================
