#pragma once
#include <QObject>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QHash>
#include "Table_Explorer.h"
#include "StringListModel.h"
#include "TableDataModel.h"
#include "FilterModel.h"

class QThread;
class QTimer;
class Database_Worker;

class MainController : public QObject {
    Q_OBJECT

        Q_PROPERTY(QStringList fonts READ fonts CONSTANT)
        Q_PROPERTY(StringListModel* tablesModel READ tablesModel CONSTANT)
        Q_PROPERTY(TableDataModel* tableModel  READ tableModel  CONSTANT)
        Q_PROPERTY(FilterModel* filtersModel READ filtersModel CONSTANT)
        Q_PROPERTY(QVariantList columns READ columns NOTIFY columnsChanged)
        Q_PROPERTY(QStringList types READ types NOTIFY typesChanged)
        Q_PROPERTY(QString currentTable READ currentTable WRITE setCurrentTable NOTIFY currentTableChanged)
        Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
        Q_PROPERTY(int currentPage READ currentPage NOTIFY pageInfoChanged)
        Q_PROPERTY(int totalPages  READ totalPages  NOTIFY pageInfoChanged)
        Q_PROPERTY(int totalRows   READ totalRows   NOTIFY pageInfoChanged)
        Q_PROPERTY(int pageSize    READ pageSize    CONSTANT)
        Q_PROPERTY(bool canPrev READ canPrev NOTIFY pageInfoChanged)
        Q_PROPERTY(bool canNext READ canNext NOTIFY pageInfoChanged)
        Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
        Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
        Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorChanged)
        Q_PROPERTY(QString noteText READ noteText WRITE setUserNoteText NOTIFY noteTextChanged)
        Q_PROPERTY(QString noteFont READ noteFont WRITE setUserNoteFont NOTIFY noteFontChanged)
        Q_PROPERTY(int noteSize READ noteSize NOTIFY noteSizeChanged)
        Q_PROPERTY(bool noteDirty READ noteDirty NOTIFY noteDirtyChanged)

public:
    explicit MainController(const QStringList& fonts, QObject* parent = nullptr);
    ~MainController() override;

    QStringList fonts() const { return fonts_; }
    StringListModel* tablesModel() const { return tablesModel_; }
    TableDataModel* tableModel()  const { return tableModel_; }
    FilterModel* filtersModel() const { return filtersModel_; }
    QVariantList columns() const { return columns_; }
    QStringList types() const { return types_; }
    QString currentTable() const { return currentTable_; }
    QString searchText() const { return searchText_; }
    int currentPage() const { return currentPage_; }
    int totalPages() const;
    int totalRows() const { return totalRows_; }
    int pageSize() const { return pageSize_; }
    bool canPrev() const { return currentPage_ > 0; }
    bool canNext() const { return currentPage_ < totalPages() - 1; }
    bool busy() const { return busy_; }
    QString statusText() const { return statusText_; }
    QString errorMessage() const { return errorMessage_; }
    QString noteText() const { return noteText_; }
    QString noteFont() const { return noteFont_; }
    int noteSize() const { return noteSize_; }
    bool noteDirty() const { return noteDirty_; }

public slots:
    void setCurrentTable(const QString& v);
    void setSearchText(const QString& v);
    void setBusy(bool v);

public:
    Q_INVOKABLE void beginSession(const QString& driver, const QString& dbType, const QString& host,
        int port, const QString& login, const QString& password, const QString& dbPath);
    Q_INVOKABLE void endSession();
    Q_INVOKABLE void refreshTables();
    Q_INVOKABLE void selectTable(const QString& name);
    Q_INVOKABLE void applySearch();
    Q_INVOKABLE void addFilter();
    Q_INVOKABLE void removeFilter(int row);
    Q_INVOKABLE void setFilterColumn(int row, const QString& col);
    Q_INVOKABLE void setFilterOp(int row, const QString& op);
    Q_INVOKABLE void setFilterValue(int row, const QString& val);
    Q_INVOKABLE void applyFilters();
    Q_INVOKABLE void prevPage();
    Q_INVOKABLE void nextPage();
    Q_INVOKABLE void requestCreateTable();
    Q_INVOKABLE void requestAddColumn(const QString& colName);
    Q_INVOKABLE void requestAddRow();
    Q_INVOKABLE void requestMergeTables(const QStringList& names);
    Q_INVOKABLE void renameTable(const QString& oldN, const QString& newN);
    Q_INVOKABLE void dropTable(const QString& name);
    Q_INVOKABLE void updateCellRequested(int row, int col);
    Q_INVOKABLE void deleteSelectedRowsRequested();
    Q_INVOKABLE void createTableFromDraft(const QVariantMap& draft);
    Q_INVOKABLE void mergeTablesFromDraft(const QVariantMap& draft);
    Q_INVOKABLE void addRowFromDraft(const QVariantMap& values);
    Q_INVOKABLE void addColumnFromDraft(const QString& colName, const QString& type);
    Q_INVOKABLE void setUserNoteText(const QString& t);
    Q_INVOKABLE void setUserNoteFont(const QString& f);
    Q_INVOKABLE void saveNote();
    Q_INVOKABLE QString safeName(const QString& n) const;

    Q_INVOKABLE void confirmResult(int requestId, bool accepted);
    Q_INVOKABLE void inputResult(int requestId, const QString& text, bool accepted);

signals:
    void columnsChanged();
    void typesChanged();
    void currentTableChanged();
    void searchTextChanged();
    void pageInfoChanged();
    void busyChanged();
    void statusChanged();
    void errorChanged();
    void noteTextChanged();
    void noteFontChanged();
    void noteSizeChanged();
    void noteDirtyChanged();
    void sessionFailed(const QString& error);

    void askConfirm(const QString& title, const QString& message, int requestId);
    void askInput(const QString& title, const QString& label, const QString& def, int requestId);
    void openCreateDialog(const QStringList& types);
    void openMergeDialog(const QVariantMap& columnsPerTable);
    void openAddRowDialog(const QVariantList& columns);
    void openAddColDialog(const QStringList& types);
    void showError(const QString& msg);
    void showInfo(const QString& msg);
    void requestCloseNote(int requestId);

private slots:
    void onTablesLoaded(const QStringList& tables);
    void onSelectFinished(quint64 requestId, const QList<QList<QVariant>>& data, const QStringList& headers, int totalRows);
    void onColumnsLoaded(quint64 requestId, const QString& tableName, const QList<Table_Explorer::ColumnInfo>& cols);
    void onTypesDbLoaded(quint64 requestId, const QStringList& types);
    void onOperationCompleted(bool success, const QString& message);
    void onWorkerError(const QString& error);
    void flushFilters();

private:
    enum Kind { LoadPk, AddRow, CreateTable, AddColumn, MergeTables, SelectPage };
    struct PendingReq { Kind kind; QString table; QStringList expected; QVariantMap collected; QVariantMap payload; };

    quint64 startRequest(Kind k, const QString& table = {}, const QStringList& expected = {}, const QVariantMap& payload = {});
    void reloadCurrentPage();
    void loadNote();
    QString pkName() const;
    QString buildCreateTableSql(const QVariantMap& draft) const;
    QString buildMergeTablesSql(const QVariantMap& draft) const;

    QStringList fonts_, types_;
    QVariantList columns_;
    StringListModel* tablesModel_ = nullptr;
    TableDataModel* tableModel_ = nullptr;
    FilterModel* filtersModel_ = nullptr;
    QThread* workerThread_ = nullptr;
    Database_Worker* worker_ = nullptr;
    QTimer* filterTimer_ = nullptr;
    QTimer* noteTimer_ = nullptr;
    QString currentTable_, searchText_, statusText_, errorMessage_;
    QString noteText_, noteFont_, notePath_, pendingColumnName_;
    int currentPage_ = 0, totalRows_ = 0, pageSize_ = 1000, noteSize_ = 12;
    bool busy_ = false, noteDirty_ = false;
    quint64 nextId_ = 1;
    QHash<quint64, PendingReq> pending_;
    QString dbTypeCache_;
};