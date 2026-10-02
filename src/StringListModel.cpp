#include "StringListModel.h"

StringListModel::StringListModel(QObject* parent) : QAbstractListModel(parent) {}

int StringListModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : items_.size();
}

QVariant StringListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= items_.size()) return {};
    switch (role) {
    case DisplayRole: return items_.at(index.row());
    case SelectedRole: return selected_.value(index.row(), false);
    }
    return {};
}

QHash<int, QByteArray> StringListModel::roleNames() const {
    return { {DisplayRole, "display"}, {SelectedRole, "selected"} };
}

void StringListModel::setStrings(const QStringList& list) {
    beginResetModel();
    items_ = list;
    selected_ = QVector<bool>(list.size(), false);
    endResetModel();
    emit countChanged();
    emit selectionChanged();
}

void StringListModel::clearStrings() { setStrings({}); }

int StringListModel::indexOf(const QString& value) const { return items_.indexOf(value); }

void StringListModel::selectOnly(int row) {
    if (row < 0 || row >= selected_.size()) return;
    for (int i = 0; i < selected_.size(); ++i) selected_[i] = (i == row);
    emit dataChanged(index(0), index(selected_.size() - 1), { SelectedRole });
    emit selectionChanged();
}

void StringListModel::toggleSelected(int row) {
    if (row < 0 || row >= selected_.size()) return;
    selected_[row] = !selected_[row];
    emit dataChanged(index(row), index(row), { SelectedRole });
    emit selectionChanged();
}

void StringListModel::clearSelection() {
    if (selected_.isEmpty()) return;
    selected_.fill(false);
    emit dataChanged(index(0), index(selected_.size() - 1), { SelectedRole });
    emit selectionChanged();
}

QStringList StringListModel::selectedStrings() const {
    QStringList out;
    for (int i = 0; i < items_.size(); ++i)
        if (selected_.value(i, false)) out << items_.at(i);
    return out;
}