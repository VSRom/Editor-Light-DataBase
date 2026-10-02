#include "MainController.h"
#include "Database_Worker.h"
#include "Table_Explorer.h"
#include <QThread>
#include <QMetaObject>
#include <QMetaType>
#include <QTimer>
#include <QSettings>
#include <QCoreApplication>

MainController::MainController(const QStringList& fonts, QObject* parent)
    : QObject(parent), fonts_(fonts) {
    qRegisterMetaType<Hash>("Hash");
    qRegisterMetaType<Map>("Map");
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString,QString>");
    qRegisterMetaType<FilterList>("FilterList");
    qRegisterMetaType<Table_Explorer::ColumnInfo>("Table_Explorer::ColumnInfo");
    qRegisterMetaType<QList<Table_Explorer::ColumnInfo>>("QList<Table_Explorer::ColumnInfo>");

    tablesModel_ = new StringListModel(this);
    tableModel_ = new TableDataModel(this);
    filtersModel_ = new FilterModel(this);
    connect(filtersModel_, &FilterModel::filtersChanged, this,
        [this] { if (filterTimer_) filterTimer_->start(); });

    filterTimer_ = new QTimer(this); filterTimer_->setSingleShot(true); filterTimer_->setInterval(400);
    connect(filterTimer_, &QTimer::timeout, this, &MainController::flushFilters);

    noteTimer_ = new QTimer(this); noteTimer_->setSingleShot(true); noteTimer_->setInterval(800);
    connect(noteTimer_, &QTimer::timeout, this, &MainController::saveNote);

    notePath_ = QCoreApplication::applicationDirPath() + "/notepad.ini";
    loadNote();
}
MainController::~MainController() { endSession(); }

int MainController::totalPages() const { int t = (totalRows_ + pageSize_ - 1) / pageSize_; return t <= 0 ? 1 : t; }

quint64 MainController::startRequest(Kind k, const QString& table, const QStringList& expected, const QVariantMap& payload) {
    const quint64 id = nextId_++;
    pending_.insert(id, PendingReq{ k, table, expected, {}, payload });
    return id;
}
void MainController::setCurrentTable(const QString& v) {
    if (currentTable_ == v)return;
    currentTable_ = v;
    emit currentTableChanged();
}
void MainController::setSearchText(const QString& v) {
    if (searchText_ == v)return;
    searchText_ = v;
    emit searchTextChanged();
    filterTimer_->start();
}
void MainController::setBusy(bool v) {
    if (busy_ == v)return;
        busy_ = v;
        emit busyChanged();
}
void MainController::beginSession(const QString& driver, const QString& dbType, const QString& host,
    int port, const QString& login, const QString& password, const QString& dbPath) {
    endSession();
    dbTypeCache_ = dbType;
    setBusy(true); errorMessage_.clear(); emit errorChanged();
    statusText_ = "Подключение к базе данных..."; emit statusChanged();
    workerThread_ = new QThread(this);
    worker_ = new Database_Worker("worker_connection", dbType);
    worker_->moveToThread(workerThread_);
    connect(worker_, &Database_Worker::tablesLoaded, this, &MainController::onTablesLoaded);
    connect(worker_, &Database_Worker::selectFinished, this, &MainController::onSelectFinished);
    connect(worker_, &Database_Worker::columnsLoaded, this, &MainController::onColumnsLoaded);
    connect(worker_, &Database_Worker::typesDbLoaded, this, &MainController::onTypesDbLoaded);
    connect(worker_, &Database_Worker::operationCompleted, this, &MainController::onOperationCompleted);
    connect(worker_, &Database_Worker::errorOccurred, this, &MainController::onWorkerError);
    workerThread_->start();
    QMetaObject::invokeMethod(worker_, "initConnection", Qt::QueuedConnection,
        Q_ARG(QString, driver), Q_ARG(QString, dbPath), Q_ARG(QString, dbType),
        Q_ARG(QString, host), Q_ARG(int, port), Q_ARG(QString, login), Q_ARG(QString, password));
}

void MainController::endSession() {
    if (workerThread_) { workerThread_->quit(); workerThread_->wait(3000); }
    if (worker_) { delete worker_; worker_ = nullptr; }
    if (workerThread_) { delete workerThread_; workerThread_ = nullptr; }
    tablesModel_->clearStrings(); tableModel_->clearTable(); filtersModel_->clearAll();
    columns_.clear(); emit columnsChanged();
    setCurrentTable(QString()); setSearchText(QString());
    currentPage_ = 0; totalRows_ = 0; emit pageInfoChanged();
    setBusy(false); pending_.clear();
}

void MainController::refreshTables() {
    if (!worker_ || !workerThread_ || !workerThread_->isRunning()) return;
    statusText_ = "Обновление списка таблиц..."; emit statusChanged();
    QMetaObject::invokeMethod(worker_, "loadTables", Qt::QueuedConnection);
}

void MainController::selectTable(const QString& name)
{
    if (name.isEmpty()) return;
    setCurrentTable(name);
    setSearchText(QString());
    filtersModel_->clearAll();
    currentPage_ = 0; emit pageInfoChanged();
    reloadCurrentPage();
    const quint64 id = startRequest(LoadPk, name);
    QMetaObject::invokeMethod(worker_, "getColumns", Qt::QueuedConnection, Q_ARG(quint64, id), Q_ARG(QString, name));
}

void MainController::applySearch() { currentPage_ = 0; emit pageInfoChanged(); reloadCurrentPage(); }
void MainController::addFilter() { filtersModel_->addRow(); }
void MainController::removeFilter(int row) { filtersModel_->removeRow(row); }
void MainController::setFilterColumn(int r, const QString& c) { filtersModel_->setColumn(r, c); }
void MainController::setFilterOp(int r, const QString& o) { filtersModel_->setOp(r, o); }
void MainController::setFilterValue(int r, const QString& v) { filtersModel_->setValue(r, v); }
void MainController::applyFilters() { currentPage_ = 0; emit pageInfoChanged(); reloadCurrentPage(); }
void MainController::flushFilters() { applyFilters(); }
void MainController::prevPage() { if (canPrev()) { currentPage_--; emit pageInfoChanged(); reloadCurrentPage(); } }
void MainController::nextPage() { if (canNext()) { currentPage_++; emit pageInfoChanged(); reloadCurrentPage(); } }

void MainController::reloadCurrentPage() {
    if (currentTable_.isEmpty() || !worker_) return;
    setBusy(true);
    QStringList sc;
    for (const QVariant& v : columns_)
        sc << v.toMap().value("name").toString();

    const quint64 id = startRequest(SelectPage, currentTable_);
    QMetaObject::invokeMethod(worker_, "selectTable", Qt::QueuedConnection,
        Q_ARG(quint64, id), Q_ARG(QString, currentTable_), Q_ARG(FilterList, filtersModel_->collect()),
        Q_ARG(int, pageSize_), Q_ARG(int, currentPage_ * pageSize_), Q_ARG(QString, searchText_), Q_ARG(QStringList, sc));
}

void MainController::requestCreateTable() {
    const quint64 id = startRequest(CreateTable);
    QMetaObject::invokeMethod(worker_, "getTypesDb",
        Qt::QueuedConnection, Q_ARG(quint64, id));
}
void MainController::requestAddColumn(const QString& colName) {
    pendingColumnName_ = colName;
    const quint64 id = startRequest(AddColumn);
    QMetaObject::invokeMethod(worker_, "getTypesDb", Qt::QueuedConnection, Q_ARG(quint64, id));
}
void MainController::requestAddRow() {
    if (currentTable_.isEmpty()) {
        emit showError("Таблица не выбрана");
        return;
    }
    const quint64 id = startRequest(AddRow, currentTable_);
    QMetaObject::invokeMethod(worker_, "getColumns", Qt::QueuedConnection, Q_ARG(quint64, id),
        Q_ARG(QString, currentTable_));
}
void MainController::requestMergeTables(const QStringList& names) {
    if (names.size() < 2) {
        emit showError("Невозможно объединить менее 2 таблиц");
        return;
    }
    const quint64 id = startRequest(MergeTables, QString(), names);
    for (const QString& n : names)
        QMetaObject::invokeMethod(worker_, "getColumns", Qt::QueuedConnection, Q_ARG(quint64, id), Q_ARG(QString, n));
}
void MainController::renameTable(const QString& o, const QString& n) {
    if (!worker_)return;
    QMetaObject::invokeMethod(worker_, "renameTable", Qt::QueuedConnection, Q_ARG(QString, o), Q_ARG(QString, n));
}
void MainController::dropTable(const QString& name) {
    if (!worker_)return;
    QMetaObject::invokeMethod(worker_, "dropTable", Qt::QueuedConnection, Q_ARG(QString, name));
}

void MainController::updateCellRequested(int row, int col) {
    if (tableModel_->pkColumn() < 0) {
        emit showError("У таблицы нет первичного ключа, редактирование недоступно");
        return;
    }
    if (col == tableModel_->pkColumn()) {
        emit showError("Нельзя изменить id");
        return;
    }
    const QString colName = tableModel_->columnNameAt(col);
    const QString old = tableModel_->displayAt(row, col);
    const QVariant id = tableModel_->rowIdAt(row);
    const quint64 rid = startRequest(AddRow, currentTable_, {}, { {"rowId",id}, {"col",colName}, {"old",old} });

    emit askInput("Новое значение", "Ввод", old, (int)rid);
}

void MainController::deleteSelectedRowsRequested() {
    if (tableModel_->pkColumn() < 0) { emit showError("У таблицы нет первичного ключа, редактирование недоступно"); return; }
    const QVariantList ids = tableModel_->selectedRowIds();
    if (ids.isEmpty()) { emit showError("Строка для удаления не выделена"); return; }
    const quint64 rid = startRequest(AddRow, currentTable_, {}, { {"ids",QVariant(ids)} });
    emit askConfirm("Подтверждение удаления", "Удалить выбранную(ые) строку(и)?", (int)rid);
}

void MainController::confirmResult(int requestId, bool accepted) {
    const quint64 id = (quint64)requestId; auto it = pending_.find(id); if (it == pending_.end())return;
    PendingReq r = it.value(); pending_.erase(it);
    if (r.kind == AddRow && r.payload.contains("ids")) {
        if (accepted && worker_) QMetaObject::invokeMethod(worker_, "removeRows", Qt::QueuedConnection, Q_ARG(QString, currentTable_), Q_ARG(QString, pkName()), Q_ARG(QVariantList, r.payload.value("ids").toList()));
        else emit showInfo("Операция отменена");
    }
    else emit showInfo("Операция отменена");
}

void MainController::inputResult(int requestId, const QString& text, bool accepted) {
    const quint64 id = (quint64)requestId; auto it = pending_.find(id); if (it == pending_.end())return;
    PendingReq r = it.value(); pending_.erase(it);
    if (!accepted) { emit showInfo("Операция отменена"); return; }
    if (r.kind == AddRow && r.payload.contains("col")) {
        const QString old = r.payload.value("old").toString();
        if (text == old) return;
        Map newVal; newVal[r.payload.value("col").toString()] = text;
        if (worker_) QMetaObject::invokeMethod(worker_, "updateRow", Qt::QueuedConnection, Q_ARG(QString, currentTable_), Q_ARG(QString, pkName()), Q_ARG(QVariant, r.payload.value("rowId")), Q_ARG(Map, newVal));
    }
}

QString MainController::pkName() const { for (const QVariant& v : columns_) { const QVariantMap m = v.toMap(); if (m.value("isPrimaryKey").toBool()) return m.value("name").toString(); } return QString(); }

void MainController::createTableFromDraft(const QVariantMap& draft) { const QString sql = buildCreateTableSql(draft); if (sql.isEmpty())return; if (worker_) QMetaObject::invokeMethod(worker_, "executeQuery", Qt::QueuedConnection, Q_ARG(QString, sql)); }
void MainController::mergeTablesFromDraft(const QVariantMap& draft) { const QString sql = buildMergeTablesSql(draft); if (sql.isEmpty())return; if (worker_) QMetaObject::invokeMethod(worker_, "executeQuery", Qt::QueuedConnection, Q_ARG(QString, sql)); }
void MainController::addRowFromDraft(const QVariantMap& values) { Hash h; for (auto it = values.constBegin(); it != values.constEnd(); ++it) h[it.key()] = it.value(); if (worker_) QMetaObject::invokeMethod(worker_, "insertRow", Qt::QueuedConnection, Q_ARG(QString, currentTable_), Q_ARG(Hash, h)); }
void MainController::addColumnFromDraft(const QString& colName, const QString& type) { const QString sql = QString("ALTER TABLE %1 ADD COLUMN %2 %3").arg(Table_Explorer::safeName(currentTable_), Table_Explorer::safeName(colName), type); if (worker_) QMetaObject::invokeMethod(worker_, "executeQuery", Qt::QueuedConnection, Q_ARG(QString, sql)); }
QString MainController::safeName(const QString& n) const { return Table_Explorer::safeName(n); }

QString MainController::buildCreateTableSql(const QVariantMap& draft) const {
    const QString nameTab = draft.value("tableName").toString().trimmed(); if (nameTab.isEmpty())return QString();
    auto pkType = [&](const QString& t)->QString { if (t == "sqlite")return "INTEGER PRIMARY KEY"; if (t == "mysql")return "INT PRIMARY KEY AUTO_INCREMENT"; if (t == "postgresql")return "SERIAL PRIMARY KEY"; if (t == "access")return "COUNTER PRIMARY KEY"; if (t == "oracle")return "NUMBER GENERATED ALWAYS AS IDENTITY PRIMARY KEY"; return QString(); };
    QStringList cols; cols << QString("%1 %2").arg(Table_Explorer::safeName("id"), pkType(dbTypeCache_));
    const QVariantList cl = draft.value("columns").toList();
    for (const QVariant& cv : cl) { const QVariantMap c = cv.toMap(); const QString nm = c.value("name").toString().trimmed(); const QString ty = c.value("type").toString(); if (nm.toLower() == "id" || nm.isEmpty())continue; cols << QString("%1 %2").arg(Table_Explorer::safeName(nm), ty); }
    return QString("CREATE TABLE IF NOT EXISTS %1 (%2);").arg(Table_Explorer::safeName(nameTab), cols.join(", "));
}

QString MainController::buildMergeTablesSql(const QVariantMap& draft) const {
    const QString nameTab = draft.value("tableName").toString().trimmed(); if (nameTab.isEmpty())return QString();
    const QString join = draft.value("join").toString(); const QVariantList tabs = draft.value("tables").toList(); if (tabs.size() < 2)return QString();
    QStringList selectCols; for (const QVariant& tv : tabs) { const QVariantMap t = tv.toMap(); const QString al = t.value("alias").toString(); const QVariantList cs = t.value("columns").toList(); for (const QVariant& cv : cs) selectCols << al + "." + Table_Explorer::safeName(cv.toString()); }
    if (selectCols.isEmpty())return QString();
    const QVariantMap first = tabs.first().toMap();
    QString from = "FROM " + Table_Explorer::safeName(first.value("table").toString()) + " " + first.value("alias").toString();
    QStringList avail; avail << first.value("alias").toString();
    for (int i = 1; i < tabs.size(); ++i) {
        const QVariantMap t = tabs[i].toMap(); const QString al = t.value("alias").toString(); from += " " + join + " JOIN " + Table_Explorer::safeName(t.value("table").toString()) + " " + al;
        QStringList cond; const QVariantList ons = draft.value("conditions").toList();
        for (const QVariant& ov : ons) {
            const QVariantMap o = ov.toMap(); const QString l = o.value("leftTable").toString(), r = o.value("rightTable").toString(), lc = o.value("leftCol").toString(), rc = o.value("rightCol").toString(), op = o.value("op").toString();
            if (lc.isEmpty() || rc.isEmpty())continue; QString second; if (l == al)second = r; else if (r == al)second = l; else continue; if (!avail.contains(second))continue; if (QStringList({ "=","<>","<",">","<=",">=" }).contains(op)) cond << l + "." + Table_Explorer::safeName(lc) + " " + op + " " + r + "." + Table_Explorer::safeName(rc);
        }
        if (cond.isEmpty())return QString{}; from += " ON (" + cond.join(" AND ") + ")"; avail << al;
    }
    return QString("CREATE TABLE %1 AS SELECT %2 %3").arg(Table_Explorer::safeName(nameTab), selectCols.join(", "), from);
}

void MainController::loadNote() { QSettings s(notePath_, QSettings::IniFormat); noteText_ = s.value("notepad/text", "").toString(); noteFont_ = s.value("notepad/font", fonts_.value(0)).toString(); noteSize_ = s.value("notepad/size", 12).toInt(); noteDirty_ = false; emit noteTextChanged(); emit noteFontChanged(); emit noteSizeChanged(); emit noteDirtyChanged(); }
void MainController::setUserNoteText(const QString& t) { if (noteText_ == t)return; noteText_ = t; noteDirty_ = true; emit noteTextChanged(); emit noteDirtyChanged(); noteTimer_->start(); }
void MainController::setUserNoteFont(const QString& f) { if (noteFont_ == f)return; noteFont_ = f; noteDirty_ = true; emit noteFontChanged(); emit noteDirtyChanged(); noteTimer_->start(); }
void MainController::saveNote() { QSettings s(notePath_, QSettings::IniFormat); s.setValue("notepad/text", noteText_); s.setValue("notepad/font", noteFont_); s.setValue("notepad/size", noteSize_); s.sync(); if (noteDirty_) { noteDirty_ = false; emit noteDirtyChanged(); } }

void MainController::onTablesLoaded(const QStringList& tables) { tablesModel_->setStrings(tables); statusText_ = "Таблиц загружено: " + QString::number(tables.size()); emit statusChanged(); setBusy(false); if (!currentTable_.isEmpty() && !tables.contains(currentTable_)) setCurrentTable(QString()); }

void MainController::onSelectFinished(quint64 requestId, const QList<QList<QVariant>>& data, const QStringList& headers, int totalRows) {
    auto it = pending_.find(requestId); if (it == pending_.end() || it->kind != SelectPage || it->table != currentTable_) return;
    pending_.erase(it); totalRows_ = totalRows; emit pageInfoChanged();
    tableModel_->setTable(headers, data); tableModel_->setSearchTerm(searchText_);
    setBusy(false);
}

void MainController::onColumnsLoaded(quint64 requestId, const QString& tableName, const QList<Table_Explorer::ColumnInfo>& cols) {
    auto it = pending_.find(requestId); if (it == pending_.end()) return; PendingReq r = it.value();
    if (r.kind == LoadPk) {
        if (tableName == currentTable_) {
            columns_.clear(); for (const auto& c : cols) columns_ << QVariantMap{ {"name",c.name},{"type",c.type},{"isNullable",c.isNullable},{"isPrimaryKey",c.isPrimaryKey} };
            int pk = -1; for (int i = 0; i < cols.size(); ++i) if (cols[i].isPrimaryKey) { pk = i; break; } tableModel_->setColumnsMeta(columns_, pk); filtersModel_->setColumnNames([&] {QStringList n; for (const auto& c : cols)n << c.name; return n; }()); emit columnsChanged();
        }
        pending_.erase(it); return;
    }
    if (r.kind == AddRow) { pending_.erase(it); QVariantList cl; for (const auto& c : cols) cl << QVariantMap{ {"name",c.name},{"type",c.type},{"isNullable",c.isNullable},{"isPrimaryKey",c.isPrimaryKey} }; emit openAddRowDialog(cl); return; }
    if (r.kind == MergeTables) {
        QVariantMap per = r.collected; per[tableName] = [&] {QVariantList l; for (const auto& c : cols)l << QVariantMap{ {"name",c.name},{"type",c.type} }; return QVariant(l); }();
        it.value().collected = per;
        if (per.size() == r.expected.size()) { pending_.erase(it); emit openMergeDialog(per); }
        return;
    }
    pending_.erase(it);
}

void MainController::onTypesDbLoaded(quint64 requestId, const QStringList& types) {
    types_ = types; emit typesChanged();
    auto it = pending_.find(requestId); if (it == pending_.end())return; PendingReq r = it.value(); pending_.erase(it);
    if (r.kind == CreateTable) emit openCreateDialog(types);
    else if (r.kind == AddColumn) emit openAddColDialog(types);
}

void MainController::onOperationCompleted(bool success, const QString& message) {
    if (!success) { errorMessage_ = message; emit errorChanged(); emit showError(message); return; }
    statusText_ = message; emit statusChanged(); refreshTables(); if (!currentTable_.isEmpty()) reloadCurrentPage();
}

void MainController::onWorkerError(const QString& error) { errorMessage_ = error; emit errorChanged(); setBusy(false); emit sessionFailed(error); }