#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(std::unique_ptr<DatabaseManager> dbMan)
    : dbMan(databaseManager_)
{


}
