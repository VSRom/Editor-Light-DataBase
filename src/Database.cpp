#include "Database.h"
#include <QDebug>
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
bool Database::init_db(const QString &connect_name)
{
    db_Q = QSqlDatabase::database(connect_name);

    qDebug() << "Connection name:" << connect_name;
    qDebug() << "Driver:" << db_Q.driverName();
    qDebug() << "Database name:" << db_Q.databaseName();
    qDebug() << "Is open:" << db_Q.isOpen();

    if (!db_Q.open())
        return false;

    else
        return true;
}
//===========================================================================================================