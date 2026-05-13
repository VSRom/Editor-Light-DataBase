#include "Database.h"
#include <QCoreApplication>
//===========================================================================================================
Database::Database()
{ }
//===========================================================================================================
Database::~Database()
{ }
//===========================================================================================================
bool Database::isOpen() const
{
    return db_Q.isOpen();
}
//===========================================================================================================
bool Database::add_task(const std::string& title)
{
    QSqlQuery db(db_Q);
    db.prepare("INSERT INTO tasks (title, completed) VALUES (:title, :completed)");
    db.bindValue(":title", QString::fromStdString(title));
    db.bindValue(":completed", 0);
    return db.exec();
}
//===========================================================================================================
std::vector<Task> Database::getAllTasks()
{
    std::vector<Task> tasks;
    QSqlQuery db(db_Q);
    db.prepare("SELECT id, title, completed FROM tasks");

    while (db.next())
    {
        tasks.emplace_back(
            db.value(0).toInt(),
            db.value(1).toString().toStdString(),
            db.value(2).toBool()
        );
    }
    return tasks;
}
//===========================================================================================================
bool Database::updateTask(int id, const std::string& title, bool completed)
{
    QSqlQuery db(db_Q);
    db.prepare("UPDATE tasks SET title = :title, completed = :completed WHERE id = :id");
    db.bindValue(":id", id);
    db.bindValue(":title", QString::fromStdString(title));
    db.bindValue(":completed", completed ? 1 : 0);
    return db.exec();
}
//===========================================================================================================
bool Database::delete_task(int id)
{
    QSqlQuery db(db_Q);
    db.prepare("DELETE FROM tasks WHERE id = :id");
    db.bindValue(":id", id);
    return db.exec();
}
//===========================================================================================================
bool Database::delete_all_tasks()
{
    QSqlQuery db(db_Q);
    db.prepare("DELETE FROM tasks");
    return db.exec();
}
//===========================================================================================================
bool Database::init_db(const QString &connect_name)
{
    db_Q = QSqlDatabase::database(connect_name);

    if (!db_Q.open())
        return false;

    else
        return true;
}
//===========================================================================================================