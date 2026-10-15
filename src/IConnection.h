#pragma once

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <expected>

class IConnection {

public:
    IConnection();

    virtual ConState state() const noexcept;
    virtual SqlError lastError() const;
    virtual std::expected<ResultSet, SqlError> execute(const CmdQuery&);
    virtual HandleTransaction transactionHandle();
    virtual void close();

    IConnection(const IConnection& other) = delete;
    IConnection& operator=(const IConnection& other) = delete;
    IConnection(IConnection&& other) noexcept = delete;
    IConnection& operator=(IConnection&& other) noexcept = delete;
    virtual ~IConnection() = default;
};

