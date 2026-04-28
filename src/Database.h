#ifndef DATABASE_H
#define DATABASE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <vector>
#include "Task.h"

class Database {
public:
    Database();
    ~Database();
    
    bool isOpen() const;
    bool add_task(const std::string& title);
    std::vector<Task> getAllTasks();
    bool updateTask(int id, const std::string& title, bool completed);
    bool delete_task(int id);

private:
    QSqlDatabase db_;
};

#endif // DATABASE_H