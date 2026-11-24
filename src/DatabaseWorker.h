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
    void selectTable(quint64 requestId, const QString& table, const FilterList& filters = {}, int pageSize = 0, int offset = 0,
        const QString& search = {}, const QStringList& searchableColumns = {});

    void executeQuery(const QString& sql);
    void getColumns(quint64 requestId, const QString& table);
    void insertRow(const QString& table, const Hash& values);
    void updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues);
    void removeRow(const QString& table, const QString& idColumn, const QVariant& idValue);
    void removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues);
    void getTypesDb(quint64 requestId);
    void initConnection(const vsc::ConnectionParameters& params);
    void dropTable(const QString& table);
    void renameTable(const QString& oldName, const QString& newName);

signals:
    void tablesLoaded(QStringList tables);
    void selectFinished(quint64 requestId, QList<QList<QVariant>> data, QStringList headers, int totalRows);
    void selectError(quint64 requestId, const QString& error);
    void operationCompleted(bool success, const QString& message);
    void rowsMayHaveChanged(const QString& table);
    void columnsLoaded(quint64 requestId, const QString& tableName, QList<TableRepository::ColumnInfo> columns);
    void typesDbLoaded(quint64 requestId, QStringList types);
    void errorOccurred(const QString& error);
    void connectionOpened();

private:
    std::unique_ptr<vsc::IConnection> connection_ = nullptr;
    std::unique_ptr<TableRepository> explorer_ = nullptr;
};
