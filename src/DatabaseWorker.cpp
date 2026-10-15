#include "DatabaseWorker.h"
#include <QSqlError>
#include <QSqlQueryModel>
#include <QSqlDatabase>

DatabaseWorker::DatabaseWorker(const QString& connectionName, const QString& dbType, QObject* parent)
    : QObject(parent), explorer_(nullptr), dbType_(dbType) {
}

DatabaseWorker::~DatabaseWorker() { }

void DatabaseWorker::loadTables() {
    if (!explorer_)
        return;

    emit tablesLoaded(explorer_->getUserTables());
}

void DatabaseWorker::selectTable(quint64 requestId, const QString& table, const FilterList& filters, int pageSize,
                                  int offset, const QString& search, const QStringList& searchableColumns)
{
    if (!explorer_)
        return;

    std::unique_ptr<QSqlQueryModel> model = explorer_->select(table, filters, " AND ", pageSize, offset, search, searchableColumns);

    int totalRows = explorer_->countRows(table, filters, search, searchableColumns);

    QList<QList<QVariant>> data;
    QStringList headers;

    for (int i = 0; i < model->columnCount(); i++)
        headers << model->headerData(i, Qt::Horizontal).toString();

    for (int row = 0; row < model->rowCount(); row++) {
        QList<QVariant> rowData;

        for (int col = 0; col < model->columnCount(); col++)
            rowData << model->data(model->index(row, col));

        data << rowData;
    }

    emit selectFinished(requestId, data, headers, totalRows);
}

void DatabaseWorker::executeQuery(const QString& sql) {
    if (!explorer_)
        return;

    bool success = explorer_->exeQuery(sql);

    emit operationCompleted(success, success ? tr("Query executed") : tr("Error: ") + sql);
}

void DatabaseWorker::getColumns(quint64 requestId, const QString& table) {
    if (!explorer_)
        return;

    emit columnsLoaded(requestId, table, explorer_->getColumns(table));
}

void DatabaseWorker::insertRow(const QString& table, const Hash& values) {
    if (!explorer_)
        return;

    bool success = explorer_->insert(table, values);

    emit operationCompleted(success, success ? tr("Row added") : tr("Failed to add row"));
}

void DatabaseWorker::updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues) {
    if (!explorer_)
        return;

    bool success = explorer_->update(table, idColumn, idValue, newValues);

    emit operationCompleted(success, success ? tr("Row updated") : tr("Failed to update row"));
}

void DatabaseWorker::removeRow(const QString& table, const QString& idColumn, const QVariant& idValue) {
    if (!explorer_)
        return;

    bool success = explorer_->remove(table, idColumn, idValue);

    emit operationCompleted(success, success ? tr("Row deleted") : tr("Failed to delete row"));
}

void DatabaseWorker::removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues) {
    if (!explorer_)
        return;

    bool success = explorer_->removeRows(table, idColumn, idValues);

    emit operationCompleted(success, success ? tr("Rows deleted") : tr("Failed to delete rows"));
}

void DatabaseWorker::getTypesDb(quint64 requestId) {
    if (!explorer_)
        return;

    emit typesDbLoaded(requestId, explorer_->get_types_db());
}

void DatabaseWorker::dropTable(const QString& table) {
    if (!explorer_)
        return;

    bool success = explorer_->drop_table(table);

    emit operationCompleted(success, success ? tr("Table deleted") : tr("Failed to delete table"));
}

void DatabaseWorker::renameTable(const QString& oldName, const QString& newName) {
    if (!explorer_)
        return;
    bool success = explorer_->rename_table(oldName, newName);

    emit operationCompleted(success, success ? tr("Table renamed") : tr("Failed to rename table"));
}

void DatabaseWorker::initConnection(const QString& driver, const QString& dbPath, const QString& db_type, const QString& host, const int port, const QString& log, const QString& pass) {
    /* Как разрешить с connection_name */
    connection_name_ = "worker_connection";
    dbType_ = db_type;
    db_path_ = dbPath;
    driver_ = driver;

    if (QSqlDatabase::contains(connection_name_))
        QSqlDatabase::removeDatabase(connection_name_);

    QSqlDatabase db = QSqlDatabase::addDatabase(driver_, connection_name_);
    db.setDatabaseName(db_path_);

    if (!host.isEmpty()) {
        db.setHostName(host);
        db.setPort(port);
        db.setUserName(log);
        db.setPassword(pass);
    }

    if (!db.open()) {
        emit errorOccurred(tr("Failed to open database in worker: ") + db.lastError().text());
        QSqlDatabase::removeDatabase(connection_name_);
        /* Добавить deleteLater() */
        connection_name_ = "";
        return;
    }

    explorer_ = std::make_unique<TableRepository>(connection_name_, dbType_);

    loadTables();
}