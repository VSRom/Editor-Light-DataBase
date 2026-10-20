#pragma once

#include "connection_state.h"
#include "query.h"
#include "result_set.h"
#include "sql_error.h"
#include "i_transaction.h"

#include <expected>
#include <memory>

namespace vsc {

// Driver implementations own the database handle and must obey their
// driver's thread-affinity rules. This interface contains no Qt types.
class IConnection {
public:
    virtual ~IConnection() = default;

    IConnection(const IConnection&) = delete;
    IConnection& operator=(const IConnection&) = delete;
    IConnection(IConnection&&) = delete;
    IConnection& operator=(IConnection&&) = delete;

    [[nodiscard]] virtual ConState state() const noexcept = 0;
    [[nodiscard]] virtual std::expected<ResultSet, SqlError>
    execute(const CmdQuery& query) = 0;
    [[nodiscard]] virtual std::expected<std::unique_ptr<ITransaction>, SqlError>
    beginTransaction() = 0;
    [[nodiscard]] virtual std::expected<void, SqlError> close() = 0;

protected:
    IConnection() = default;
};

} // namespace vsc
