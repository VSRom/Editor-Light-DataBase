#include "Database_Worker.h"
#include <QSqlError>
//================================================================================================================
Database_Worker::Database_Worker(const QString& connectionName, const QString& dbType, QObject* parent)
	: QObject(parent), explorer_(nullptr), dbType_(dbType)
{ }
//================================================================================================================
void Database_Worker::loadTables() {
	QStringList tables = explorer_->getUserTables();
	emit tablesLoaded(tables);
}
//================================================================================================================
Database_Worker::~Database_Worker() {
    delete explorer_;
}
//================================================================================================================
void Database_Worker::selectTable(const QString& table) {
	QSqlQueryModel* model = explorer_->select(table);
	QList<QList<QVariant>> data;
	QStringList headers;

	for (int i = 0; i < model->columnCount(); i++)
		headers << model->headerData(i, Qt::Horizontal).toString();

	for (int row = 0; row < model->rowCount(); row++) {
		QList<QVariant> rowData;
		for (int col = 0; col < model->columnCount(); col++)
			rowData << model->data(model->index(row, col));
		data << rowData;
	}
	delete model;
	emit selectFinished(data, headers);
}
//================================================================================================================
void Database_Worker::executeQuery(const QString& sql) {
	bool success = explorer_->exeQuery(sql);
	emit operationCompleted(success, success ? "Запрос выполнен" : "Ошибка: " + sql);
}
//================================================================================================================
void Database_Worker::getColumns(const QString& table) {
	QList<Table_Explorer::ColumnInfo> cols = explorer_->getColumns(table);
    emit columnsLoaded(table ,cols);
}
//================================================================================================================
void Database_Worker::insertRow(const QString& table, const QHash<QString, QVariant>& values) {
	bool success = explorer_->insert(table, values);
	emit operationCompleted(success, success ? "Строка добавлена" : "Ошибка добавления строки");
}
//================================================================================================================
void Database_Worker::updateRow(const QString& table, const QString& idColumn, const QVariant& idValue, const QMap<QString, QVariant>& newValues) {
	bool success = explorer_->update(table, idColumn, idValue, newValues);
	emit operationCompleted(success, success ? "Строка обновлена" : "Ошибка обновления строки");
}
//================================================================================================================
void Database_Worker::removeRow(const QString& table, const QString& idColumn, const QVariant& idValue) {
	bool success = explorer_->remove(table, idColumn, idValue);
	emit operationCompleted(success, success ? "Строка удалена" : "Ошибка удаления строки");
}
//================================================================================================================
void Database_Worker::getTypesDb() {
	emit typesDbLoaded(explorer_->get_types_db());
}
//================================================================================================================
void Database_Worker::dropTable(const QString& table) {
	bool success = explorer_->drop_table(table);
	emit operationCompleted(success, success ? "Таблица удалена" : "Ошибка удаления таблицы");
}
//================================================================================================================
void Database_Worker::renameTable(const QString& oldName, const QString& newName) {
	bool success = explorer_->rename_table(oldName, newName);
	emit operationCompleted(success, success ? "Таблица переименована" : "Ошибка переименования");
}
//================================================================================================================
void Database_Worker::initConnection(const QString &driver, const QString &dbPath, const QString &db_type, const QString &host, const int port, const QString &log, const QString &pass) {
	connection_name_ = "worker_connection";
	dbType_ = db_type;
	db_path_ = dbPath;
	driver_ = driver;
	if (QSqlDatabase::contains(connection_name_))
		QSqlDatabase::removeDatabase(connection_name_);

	QSqlDatabase db = QSqlDatabase::addDatabase(driver_, connection_name_);
	db.setDatabaseName(db_path_);

	if (!host.isEmpty()) {
		db.setHostName(host);
		db.setPort(port);
		db.setUserName(log);
		db.setPassword(pass);
	}

	if (!db.open()) {
		emit errorOccurred("Не удалось открыть БД в воркере: " + db.lastError().text());
		return;
	}

	explorer_ = new Table_Explorer(connection_name_, dbType_);
	loadTables();
}
//================================================================================================================
