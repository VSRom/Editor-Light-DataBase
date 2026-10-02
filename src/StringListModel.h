#pragma once
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

class StringListModel : public QAbstractListModel {
    Q_OBJECT
        Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    enum Roles { DisplayRole = Qt::DisplayRole, SelectedRole = Qt::UserRole + 1 };

    explicit StringListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = {}) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setStrings(const QStringList& list);
    Q_INVOKABLE void clearStrings();
    Q_INVOKABLE int indexOf(const QString& value) const;
    Q_INVOKABLE void selectOnly(int row);
    Q_INVOKABLE void toggleSelected(int row);
    Q_INVOKABLE void clearSelection();
    Q_INVOKABLE QStringList selectedStrings() const;
    Q_INVOKABLE QStringList strings() const { return items_; }

signals:
    void countChanged();
    void selectionChanged();

private:
    QStringList items_;
    QVector<bool> selected_;
};