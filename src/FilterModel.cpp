#include "FilterModel.h"

FilterModel::FilterModel(QObject* parent) : QAbstractListModel(parent) {}

int FilterModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : rows_.size();
}

QVariant FilterModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= rows_.size()) return {};
    const Row& r = rows_.at(index.row());
    switch (role) {
    case ColumnRole: return r.column;
    case OpRole: return r.op;
    case ValueRole: return r.value;
    }
    return {};
}

QHash<int, QByteArray> FilterModel::roleNames() const {
    return { {ColumnRole, "column"}, {OpRole, "op"}, {ValueRole, "value"} };
}

void FilterModel::addRow() {
    beginInsertRows({}, rows_.size(), rows_.size());
    rows_.append({ columnNames_.value(0), "=", QString() });
    endInsertRows();
    emit countChanged();
    emit filtersChanged();
}

void FilterModel::removeRow(int row) {
    if (row < 0 || row >= rows_.size()) return;
    beginRemoveRows({}, row, row);
    rows_.removeAt(row);
    endRemoveRows();
    emit countChanged();
    emit filtersChanged();
}

void FilterModel::setColumn(int row, const QString& col) {
    if (row < 0 || row >= rows_.size() || rows_[row].column == col) return;
    rows_[row].column = col;
    emit dataChanged(index(row), index(row), { ColumnRole });
    emit filtersChanged();
}

void FilterModel::setOp(int row, const QString& op) {
    if (row < 0 || row >= rows_.size() || rows_[row].op == op) return;
    rows_[row].op = op;
    emit dataChanged(index(row), index(row), { OpRole });
    emit filtersChanged();
}

void FilterModel::setValue(int row, const QString& val) {
    if (row < 0 || row >= rows_.size() || rows_[row].value == val) return;
    rows_[row].value = val;
    emit dataChanged(index(row), index(row), { ValueRole });
    emit filtersChanged();
}

void FilterModel::clearAll() {
    if (rows_.isEmpty()) return;
    beginResetModel();
    rows_.clear();
    endResetModel();
    emit countChanged();
    emit filtersChanged();
}

QStringList FilterModel::operators() const {
    return { "=", "!=", "<", ">", "<=", ">=", "LIKE" };
}

void FilterModel::setColumnNames(const QStringList& cols) {
    columnNames_ = cols;
    for (Row& r : rows_)
        if (!columnNames_.contains(r.column)) r.column = columnNames_.value(0);
    emit filtersChanged();
}

FilterList FilterModel::collect() const {
    FilterList out;
    for (const Row& r : rows_) {
        if (r.column.isEmpty() || r.value.trimmed().isEmpty()) continue;
        Filtration f;
        f.colName_ = r.column;
        f.operator_ = r.op;
        f.value_ = r.value.trimmed();
        out.append(f);
    }
    return out;
}