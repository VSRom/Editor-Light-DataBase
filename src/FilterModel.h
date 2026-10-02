#pragma once
#include <QAbstractListModel>
#include <QStringList>
#include "Table_Explorer.h"

class FilterModel : public QAbstractListModel {
    Q_OBJECT
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum Roles { ColumnRole = Qt::DisplayRole, OpRole, ValueRole };

    struct Row { QString column, op, value; };

    explicit FilterModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addRow();
    Q_INVOKABLE void removeRow(int row);
    Q_INVOKABLE void setColumn(int row, const QString& col);
    Q_INVOKABLE void setOp(int row, const QString& op);
    Q_INVOKABLE void setValue(int row, const QString& val);
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE QStringList operators() const;
    Q_INVOKABLE void setColumnNames(const QStringList& cols);
    Q_INVOKABLE QStringList columnNames() const { return columnNames_; }

    FilterList collect() const;

signals:
    void countChanged();
    void filtersChanged();

private:
    QList<Row> rows_;
    QStringList columnNames_;
};