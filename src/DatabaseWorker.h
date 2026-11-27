#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>
#include <QVariant>
#include <QVariantList>
#include <QMap>
#include <QHash>
#include <memory>
#include <utility>

#include "TableRepository.h"
#include "vsc/i_connection.h"
#include "vsc/connection_params.h"


typedef QHash<QString, QVariant> Hash;
Q_DECLARE_METATYPE(Hash)

typedef QMap<QString, QVariant> Map;
Q_DECLARE_METATYPE(Map)

typedef QMap<QString, QString> MapString;
Q_DECLARE_METATYPE(MapString)


class DatabaseWorker : public QObject {
    Q_OBJECT

public:
    explicit DatabaseWorker(QObject* parent = nullptr);
    ~DatabaseWorker() = default;

public slots:
    void loadTables();
    void selectTable(quint64 requestId, const QString& table, const FilterList& filters = {}, int pageSize = 0, int offset = 0, const QString& search = {}, const QStringList& searchableColumns = {});
    void queryConsole(quint64 requestId, const QString& textQuery, vsc::CmdQuery::Mode m);
    void getColumns(quint64 requestId, const QString& table);
    void getTypesDb(quint64 requestId);
    void initConnection(const vsc::ConnectionParameters& params);

    void executeQuery(const QString& sql);
    void insertRow(const QString& table, const Hash& values);
    void updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues);
    void removeRow(const QString& table, const QString& idColumn, const QVariant& idValue);
    void removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues);
    void dropTable(const QString& table);
    void renameTable(const QString& oldName, const QString& newName);


signals:
    void tablesLoaded(QStringList tables);
    void tablesError(const QString& error);
    void selectFinished(quint64 requestId, QList<QList<QVariant>> data, QStringList headers, int totalRows);
    void selectError(quint64 requestId, const QString& error);
    void operationCompleted(bool success, const QString& message);
    void rowsMayHaveChanged(const QString& table);
    void columnsLoaded(quint64 requestId, const QString& tableName, QList<TableRepository::ColumnInfo> columns);
    void columnsError(quint64 requestId, const QString& error);
    void typesDbLoaded(quint64 requestId, QStringList types);
    void errorOccurred(const QString& error);
    void connectionOpened();
    void tableDropped(const QString& table);
    void tableRenamed(const QString& oldName, const QString& newName);
    void queryError(quint64 requestId, const QString& error);
    void queryRowsReady(quint64 requestId, const QStringList& headers, const QList<QList<QVariant>>& rows);
    void queryAffected(quint64 requestId, qint64 affectedRows);
    void queryFinished(quint64 requestId);

private:
    template<typename Operation, typename OnSuccess, typename OnError>
    void runCommand(Operation&&, OnError&&, OnSuccess&&);

    template<typename Operation, typename OnSuccess>
    void runCommand(Operation&&, OnSuccess&&);

    std::unique_ptr<vsc::IConnection> connection_ = nullptr;
    std::unique_ptr<TableRepository> explorer_ = nullptr;
};
