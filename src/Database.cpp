#include "Database.h"
#include <QCoreApplication>

Database::Database()
{
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName("tasks.db");
    
    if (!db_.open())
        throw std::runtime_error("Cannot open database");

    // Создать таблицу если не существует
    QSqlQuery query;
    query.exec("CREATE TABLE IF NOT EXISTS tasks ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT,"
               "title TEXT NOT NULL,"
               "completed INTEGER DEFAULT 0)");
}

Database::~Database()
{
    if (db_.isOpen())
        db_.close();

}

bool Database::isOpen() const
{
    return db_.isOpen();
}

bool Database::add_task(const std::string& title)
{
    QSqlQuery query;
    query.prepare("INSERT INTO tasks (title, completed) VALUES (:title, :completed)");
    query.bindValue(":title", QString::fromStdString(title));
    query.bindValue(":completed", 0);
    return query.exec();
}

std::vector<Task> Database::getAllTasks()
{
    std::vector<Task> tasks;
    QSqlQuery query("SELECT id, title, completed FROM tasks");
    
    while (query.next())
    {
        tasks.emplace_back(
            query.value(0).toInt(),
            query.value(1).toString().toStdString(),
            query.value(2).toBool()
        );
    }
    return tasks;
}

bool Database::updateTask(int id, const std::string& title, bool completed)
{
    QSqlQuery query;
    query.prepare("UPDATE tasks SET title = :title, completed = :completed WHERE id = :id");
    query.bindValue(":id", id);
    query.bindValue(":title", QString::fromStdString(title));
    query.bindValue(":completed", completed ? 1 : 0);
    return query.exec();
}

bool Database::delete_task(int id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM tasks WHERE id = :id");
    query.bindValue(":id", id);
    return query.exec();
}