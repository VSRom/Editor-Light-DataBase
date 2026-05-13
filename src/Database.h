#pragma once
//===========================================================================================================
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <vector>
#include "Task.h"
#include <QMessageBox>
#include <QSqlQueryModel>
//===========================================================================================================
class Database {
public:
    Database();
    ~Database();
    


    bool isOpen() const;
    bool add_task(const std::string& title);
    std::vector<Task> getAllTasks();
    bool updateTask(int id, const std::string& title, bool completed);
    bool delete_task(int id);
    bool delete_all_tasks();
    bool init_db(const QString &connect_name);
    QStringList get_table_list();
    QStringList get_table_col(const QString &table_name);
    QSqlQueryModel* select_all(const QString &table_name);

private:
    QSqlDatabase db_Q;
};
//===========================================================================================================