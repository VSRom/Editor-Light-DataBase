#pragma once

#include <QSqlDatabase>
//===========================================================================================================
class Database {
public:
    Database();
    ~Database();

    bool isOpen() const;
    bool init_db(const QString &connect_name);

private:
    QSqlDatabase db_Q;
};
//===========================================================================================================