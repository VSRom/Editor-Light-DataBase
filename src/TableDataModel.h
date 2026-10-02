#pragma once
#include <QAbstractTableModel>
#include <QList>
#include <QStringList>
#include <QVariantList>
#include <QVector>

class TableDataModel : public QAbstractTableModel {
    Q_OBJECT
        Q_PROPERTY(int rows READ rowCount NOTIFY modelChanged)
        Q_PROPERTY(int cols READ columnCount NOTIFY modelChanged)
        Q_PROPERTY(int pkColumn READ pkColumn NOTIFY modelChanged)
public:
    enum Roles {
        DisplayRole = Qt::DisplayRole,
        RawValueRole = Qt::UserRole + 1,
        ColumnNameRole, TypeRole, IsPkRole, IsNullableRole, IsNullRole, RowIdRole, SelectedRole
    };

    explicit TableDataModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    int columnCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setTable(const QStringList& headers, const QList<QList<QVariant>>& rawRows);
    void setColumnsMeta(const QVariantList& meta, int pkCol);
    void setSearchTerm(const QString& term);
    void clearTable();

    int pkColumn() const { return pkCol_; }

    Q_INVOKABLE int rowCountInv() const { return rowCount(); }
    Q_INVOKABLE int columnCountInv() const { return columnCount(); }
    Q_INVOKABLE QString headerAt(int col) const;
    Q_INVOKABLE QString displayAt(int row, int col) const;
    Q_INVOKABLE QVariant rawAt(int row, int col) const;
    Q_INVOKABLE QString columnNameAt(int col) const;
    Q_INVOKABLE QString typeAt(int col) const;
    Q_INVOKABLE bool isPkAt(int col) const;
    Q_INVOKABLE bool isNullAt(int row, int col) const;
    Q_INVOKABLE QVariant rowIdAt(int row) const;
    Q_INVOKABLE bool isSelectedAt(int row) const;
    Q_INVOKABLE void selectOnly(int row);
    Q_INVOKABLE void toggleSelected(int row);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE QVariantList selectedRowIds() const;

signals:
    void modelChanged();

private:
    void rebuildDisplay();
    QStringList headers_;
    QList<QList<QVariant>> srcRaw_;
    QList<QStringList> srcDisp_;
    QList<bool> srcNull_;
    QList<QList<QVariant>> dispRaw_;
    QList<QStringList> dispDisp_;
    QList<bool> dispNull_;
    QVector<int> dispToSrc_;
    QVector<bool> selected_;
    QVariantList meta_;
    QString searchTerm_;
    int pkCol_ = -1;
};