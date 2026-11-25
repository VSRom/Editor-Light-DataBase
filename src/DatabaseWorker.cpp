#include <QSqlQueryModel>

#include "DatabaseWorker.h"
#include "qt/qt_connection_factory.h"
#include "qt/qt_text_codec.h"

DatabaseWorker::DatabaseWorker(QObject* parent)
    : QObject(parent) {
}

void DatabaseWorker::loadTables() {
    if (!explorer_)
        return;

    auto result = explorer_->getUserTables();

    if (!result) {
        emit errorOccurred("Failed to load tables: " + vsc::qt::toQtText(result.error().message));
        return;
    }

    emit tablesLoaded(*result);
}

void DatabaseWorker::selectTable(quint64 requestId, const QString& table, const FilterList& filters, int pageSize,
                                  int offset, const QString& search, const QStringList& searchableColumns)
{
    if (!explorer_) {
        selectError(requestId, tr("Database repository is not initialized"));
        return;
    }

    auto page = explorer_->select(table, filters, " AND ", pageSize, offset, search, searchableColumns);

    if (!page) {
        QString error = vsc::qt::toQtText(page.error().message);
        emit selectError(requestId, error);
        return;
    }

    auto request = explorer_->countRows(table, filters, search, searchableColumns);

    if (!request) {
        QString error = vsc::qt::toQtText(request.error().message);
        emit selectError(requestId, error);
        return;
    }

    emit selectFinished(requestId, page->rows, page->headers, *request);
}

void DatabaseWorker::executeQuery(const QString& sql) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->exeQuery(sql);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, error);
        return;
    }

    emit operationCompleted(true, tr("Query executed"));
}

void DatabaseWorker::getColumns(quint64 requestId, const QString& table) {
    if (!explorer_) {
        emit columnsError(requestId, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->getColumns(table);

    if (!result) {
        emit columnsError(requestId, vsc::qt::toQtText(result.error().message));
        return;
    }

    emit columnsLoaded(requestId, table, *result);
}

void DatabaseWorker::insertRow(const QString& table, const Hash& values) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->insert(table, values);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, error);
        return;
    }

    emit operationCompleted(true, tr("Row added"));
}

void DatabaseWorker::updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->update(table, idColumn, idValue, newValues);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, error);
        return;
    }

    emit operationCompleted(true, tr("Row updated"));
}

void DatabaseWorker::removeRow(const QString& table, const QString& idColumn, const QVariant& idValue) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->remove(table, idColumn, idValue);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, error);
        return;
    }

    emit operationCompleted(true, tr("Row deleted"));
}

void DatabaseWorker::removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->removeRows(table, idColumn, idValues);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, tr("Failed to delete rows; some rows may have already been deleted: ") + error);
        emit rowsMayHaveChanged(table);
        return;
    }

    emit operationCompleted(true, tr("Rows deleted"));
}

void DatabaseWorker::getTypesDb(quint64 requestId) {
    if (!explorer_)
        return;

    emit typesDbLoaded(requestId, explorer_->get_types_db());
}

void DatabaseWorker::dropTable(const QString& table) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->drop_table(table);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, error);
        return;
    }

    emit tableDropped(table);
}

void DatabaseWorker::renameTable(const QString& oldName, const QString& newName) {
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->rename_table(oldName, newName);

    if(!result) {
        QString error = vsc::qt::toQtText(result.error().message);
        emit operationCompleted(false, error);
        return;
    }

    emit tableRenamed(oldName, newName);
}

void DatabaseWorker::initConnection(const vsc::ConnectionParameters& params) {
    vsc::qt::QtConnectionFactory factory;
    auto result = factory.open(params);

    if (!result) {
        emit errorOccurred("Failed to open database in worker: " + vsc::qt::toQtText(result.error().message));
        return;
    }

    explorer_.reset();
    connection_.reset();
    connection_ = std::move(result.value());
    explorer_ = std::make_unique<TableRepository>(connection_.get(), params.type);

    emit connectionOpened();

    loadTables();
}
