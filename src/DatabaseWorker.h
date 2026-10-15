#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QVariantList>
#include <QMap>
#include <QHash>
#include "TableRepository.h"

typedef QHash<QString, QVariant> Hash;
Q_DECLARE_METATYPE(Hash)

typedef QMap<QString, QVariant> Map;
Q_DECLARE_METATYPE(Map)

typedef QMap<QString, QString> MapString;
Q_DECLARE_METATYPE(MapString)


class DatabaseWorker : public QObject {
    Q_OBJECT

public:
    explicit DatabaseWorker(const QString& connectionName, const QString& dbType, QObject* parent = nullptr);
    ~DatabaseWorker();

public slots:
    void loadTables();
    void selectTable(quint64 requestId, const QString& table, const FilterList& filters = {}, int pageSize = 0, int offset = 0,
        const QString& search = {}, const QStringList& searchableColumns = {});

    void executeQuery(const QString& sql);
    void getColumns(quint64 requestId, const QString& table);
    void insertRow(const QString& table, const Hash& values);
    void updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues);
    void removeRow(const QString& table, const QString& idColumn, const QVariant& idValue);
    void removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues);
    void getTypesDb(quint64 requestId);
    void initConnection(const QString& driver, const QString& dbPath, const QString& db_type, const QString& host, const int port, const QString& log, const QString& pass);
    void dropTable(const QString& table);
    void renameTable(const QString& oldName, const QString& newName);

signals:
    void tablesLoaded(QStringList tables);
    void selectFinished(quint64 requestId, QList<QList<QVariant>> data, QStringList headers, int totalRows);
    void operationCompleted(bool success, const QString& message);
    void columnsLoaded(quint64 requestId, const QString& tableName, QList<TableRepository::ColumnInfo> columns);
    void typesDbLoaded(quint64 requestId, QStringList types);
    void errorOccurred(const QString& error);

private:
    // session_
    std::unique_ptr<TableRepository> explorer_ = nullptr;
    QString connection_name_;
    QString driver_;
    QString db_path_;
    QString dbType_;
};
