#include "TableDataModel.h"

TableDataModel::TableDataModel(QObject* parent) : QAbstractTableModel(parent) {}

int TableDataModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : dispRaw_.size();
}

int TableDataModel::columnCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : headers_.size();
}

QVariant TableDataModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) return {};
    const int r = index.row(), c = index.column();
    if (r < 0 || r >= dispRaw_.size() || c < 0 || c >= headers_.size()) return {};
    switch (role) {
    case DisplayRole: return dispDisp_[r].value(c);
    case RawValueRole: return dispRaw_[r].value(c);
    case IsNullRole: return dispNull_.value(r * headers_.size() + c, true);
    case ColumnNameRole: return columnNameAt(c);
    case TypeRole: return typeAt(c);
    case IsPkRole: return isPkAt(c);
    case IsNullableRole: return meta_.value(c).toMap().value("isNullable", true);
    case RowIdRole: return rowIdAt(r);
    case SelectedRole: return selected_.value(r, false);
    }
    return {};
}

QVariant TableDataModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section >= 0 && section < headers_.size())
        return headers_.at(section);
    return {};
}

QHash<int, QByteArray> TableDataModel::roleNames() const {
    return {
        {DisplayRole, "display"}, {RawValueRole, "rawValue"},
        {ColumnNameRole, "columnName"}, {TypeRole, "columnType"},
        {IsPkRole, "isPk"}, {IsNullableRole, "isNullable"},
        {IsNullRole, "isNull"}, {RowIdRole, "rowId"}, {SelectedRole, "selected"}
    };
}

void TableDataModel::setTable(const QStringList& headers, const QList<QList<QVariant>>& rawRows) {
    beginResetModel();
    headers_ = headers;
    srcRaw_ = rawRows;
    srcDisp_.clear();
    srcNull_.clear();
    const int cols = headers_.size();
    for (const auto& row : srcRaw_) {
        QStringList d;
        for (int c = 0; c < cols; ++c) {
            const QVariant v = row.value(c);
            const bool isNull = !v.isValid() || v.isNull();
            d << (isNull ? QString() : v.toString());
            srcNull_.append(isNull);
        }
        srcDisp_.append(d);
    }
    rebuildDisplay();
    endResetModel();
    emit modelChanged();
}

void TableDataModel::rebuildDisplay() {
    dispRaw_.clear();
    dispDisp_.clear();
    dispNull_.clear();
    dispToSrc_.clear();
    const int cols = headers_.size();
    const QString term = searchTerm_;
    for (int sr = 0; sr < srcRaw_.size(); ++sr) {
        bool match = term.isEmpty();
        if (!match) {
            for (int c = 0; c < cols && !match; ++c) {
                if (srcDisp_[sr].value(c).contains(term, Qt::CaseInsensitive)) match = true;
            }
        }
        if (!match) continue;
        dispRaw_.append(srcRaw_[sr]);
        dispDisp_.append(srcDisp_[sr]);
        for (int c = 0; c < cols; ++c) dispNull_.append(srcNull_.value(sr * cols + c, true));
        dispToSrc_.append(sr);
    }
    selected_ = QVector<bool>(dispRaw_.size(), false);
}

void TableDataModel::setColumnsMeta(const QVariantList& meta, int pkCol) {
    meta_ = meta;
    pkCol_ = pkCol;
    emit modelChanged();
}

void TableDataModel::setSearchTerm(const QString& term) {
    if (searchTerm_ == term) return;
    beginResetModel();
    searchTerm_ = term;
    rebuildDisplay();
    endResetModel();
    emit modelChanged();
}

void TableDataModel::clearTable() {
    setTable({}, {});
    setColumnsMeta({}, -1);
}

QString TableDataModel::headerAt(int col) const { return headers_.value(col); }
QString TableDataModel::displayAt(int row, int col) const {
    return (row >= 0 && row < dispDisp_.size()) ? dispDisp_[row].value(col) : QString();
}
QVariant TableDataModel::rawAt(int row, int col) const {
    return (row >= 0 && row < dispRaw_.size()) ? dispRaw_[row].value(col) : QVariant();
}
QString TableDataModel::columnNameAt(int col) const {
    return meta_.value(col).toMap().value("name", headers_.value(col)).toString();
}
QString TableDataModel::typeAt(int col) const {
    return meta_.value(col).toMap().value("type").toString();
}
bool TableDataModel::isPkAt(int col) const {
    return meta_.value(col).toMap().value("isPrimaryKey", false).toBool();
}
bool TableDataModel::isNullAt(int row, int col) const {
    return (row < 0 || col < 0 || col >= headers_.size()) ? true : dispNull_.value(row * headers_.size() + col, true);
}
QVariant TableDataModel::rowIdAt(int row) const {
    if (pkCol_ < 0 || row < 0 || row >= dispToSrc_.size()) return {};
    const int sr = dispToSrc_[row];
    return srcRaw_.value(sr).value(pkCol_);
}
bool TableDataModel::isSelectedAt(int row) const { return selected_.value(row, false); }

void TableDataModel::selectOnly(int row) {
    if (row < 0 || row >= selected_.size()) return;
    for (int i = 0; i < selected_.size(); ++i) selected_[i] = (i == row);
    emit dataChanged(index(0, 0), index(selected_.size() - 1, 0), { SelectedRole });
}

void TableDataModel::toggleSelected(int row) {
    if (row < 0 || row >= selected_.size()) return;
    selected_[row] = !selected_[row];
    emit dataChanged(index(row, 0), index(row, 0), { SelectedRole });
}

void TableDataModel::clearSelection() {
    if (selected_.isEmpty()) return;
    selected_.fill(false);
    emit dataChanged(index(0, 0), index(selected_.size() - 1, 0), { SelectedRole });
}

QVariantList TableDataModel::selectedRowIds() const {
    QVariantList out;
    for (int i = 0; i < selected_.size(); ++i)
        if (selected_[i]) out << rowIdAt(i);
    return out;
}