#include "DatabaseWorker.h"
#include "qt/qt_connection_factory.h"
#include "qt/qt_text_codec.h"
#include "qt/qt_value_codec.h"

DatabaseWorker::DatabaseWorker(QObject* parent)
    : QObject(parent) {
}

void DatabaseWorker::loadTables() {
    if (!explorer_){
        emit tablesError(tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->getUserTables();

    if (!result) {
        emit tablesError("Failed to load tables: " + vsc::qt::toQtText(result.error().message));
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

void DatabaseWorker::queryConsole(quint64 requestId, const QString& textQuery, vsc::CmdQuery::Mode m)
{
    if (!explorer_) {
        queryError(requestId, tr("Database repository is not initialized"));
        return;
    }

    auto result = explorer_->querySql(textQuery, m);

    if(!result)
    {
        emit queryError(requestId, "Failed to query: " + vsc::qt::toQtText(result.error().message));
        return;
    }

    switch(result->kind){
        case vsc::ResultSet::ResultKind::rows: {
            QStringList headers{};

            for (const auto& col : std::as_const(result->cols))
                headers += vsc::qt::toQtText(col.nameColumn);

            QList<QList<QVariant>> resultRows{};

            for (const auto& row : std::as_const(result->rows)) {
                QList<QVariant> resultRow{};

                for (const auto& cell : std::as_const(row)) {
                    auto converted = vsc::qt::toQtValue(cell);

                    if (!converted) {
                        emit queryError(requestId, vsc::qt::toQtText(converted.error().message));
                        return;
                    }

                    resultRow += *converted;
                }
                resultRows += resultRow;
            }
            emit queryRowsReady(requestId, headers, resultRows);
            return;
        }
        case vsc::ResultSet::ResultKind::affected: emit queryAffected(requestId, result->affectedRows); return;
        case vsc::ResultSet::ResultKind::none: emit queryFinished(requestId); return;
            default: return;
    }
}

void DatabaseWorker::executeQuery(const QString& sql)
{
    auto operation = [&] {
        return explorer_->exeQuery(sql);
    };

    auto success = [this]() {
        emit operationCompleted(true, tr("Query executed"));
    };

    runCommand(operation, success);
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

void DatabaseWorker::insertRow(const QString& table, const Hash& values)
{
    auto operation = [&] {
        return explorer_->insert(table, values);
    };

    auto success = [this]() {
        emit operationCompleted(true, tr("Row added"));
    };

    runCommand(operation, success);
}

void DatabaseWorker::updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues)
{
    auto operation = [&] {
        return explorer_->update(table, idColumn, idValue, newValues);
    };

    auto success = [this]() {
        emit operationCompleted(true, tr("Row updated"));
    };

    runCommand(operation, success);
}

void DatabaseWorker::removeRow(const QString& table, const QString& idColumn, const QVariant& idValue)
{
    auto operation = [&] {
        return explorer_->remove(table, idColumn, idValue);
    };

    auto success = [this]() {
        emit operationCompleted(true, tr("Row deleted"));
    };

    runCommand(operation, success);
}

void DatabaseWorker::removeRows(const QString& table, const QString& idColumn, const QVariantList& idValues)
{
    auto operation = [&] {
        return explorer_->removeRows(table, idColumn, idValues);
    };

    auto error = [&](const QString& messageError) {
        emit operationCompleted(false, tr("Failed to delete rows; some rows may have already been deleted: ") + messageError);
        emit rowsMayHaveChanged(table);
    };

    auto success = [this]() {
        emit operationCompleted(true, tr("Rows deleted"));
    };

    runCommand(operation, error, success);
}

void DatabaseWorker::getTypesDb(quint64 requestId) {
    if (!explorer_)
        return;

    emit typesDbLoaded(requestId, explorer_->get_types_db());
}

void DatabaseWorker::dropTable(const QString& table)
{
    auto operation = [&] {
        return explorer_->drop_table(table);
    };

    auto success = [&]() {
        emit tableDropped(table);
    };

    runCommand(operation, success);
}

void DatabaseWorker::renameTable(const QString& oldName, const QString& newName)
{
    auto operation = [&]() {
        return explorer_->rename_table(oldName, newName);
    };

    auto success = [&]() {
        emit tableRenamed(oldName, newName);
    };

    runCommand(operation, success);
}

template<typename Operation, typename OnSuccess, typename OnError>
void DatabaseWorker::runCommand(Operation&& operation, OnError&& error, OnSuccess&& success)
{
    if (!explorer_) {
        emit operationCompleted(false, tr("Database repository is not initialized"));
        return;
    }

    auto tryOperation = std::forward<Operation>(operation)();

    if (!tryOperation) {
        std::forward<OnError>(error)(vsc::qt::toQtText(tryOperation.error().message));
        return;
    }

    std::forward<OnSuccess>(success)();
}

template<typename Operation, typename OnSuccess>
void DatabaseWorker::runCommand(Operation&& operation, OnSuccess&& success)
{
    auto error = [this](const QString& messageError) {
        emit operationCompleted(false, messageError);
    };

    runCommand(std::forward<Operation>(operation), error, std::forward<OnSuccess>(success));
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
