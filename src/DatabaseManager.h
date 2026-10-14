#pragma once

#include <memory>
#include <string>

class DatabaseManager {

    enum class State {
        empty,
        init,
        error,
        closed
    };

public:
    DatabaseManager(std::unique_ptr<DatabaseManager> dbMan = nullptr);

    DatabaseManager(const DatabaseManager& other) = delete;
    DatabaseManager& operator=(const DatabaseManager& other) = delete;
    DatabaseManager(DatabaseManager&& other) noexcept = delete;
    DatabaseManager& operator=(DatabaseManager&& other) noexcept = delete;

    ~DatabaseManager();

private:
    std::unique_ptr<DatabaseManager> databaseManager_ = nullptr;
    std::string nameConnect_;
};

