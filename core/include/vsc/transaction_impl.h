#pragma once

#include "i_transaction.h"
#include "i_connection.h"
#include "query.h"
#include "sql_error.h"

#include <memory>
#include <atomic>
#include <expected>

namespace vsc {
    class TransactionImpl final : public ITransaction {
    public:
        static std::expected<std::unique_ptr<TransactionImpl>, SqlError> create(
            std::unique_ptr<IConnection> connection, std::uint32_t workerId) {
            CmdQuery q{ .sqlText = "BEGIN", .mode = CmdQuery::Mode::write };
            auto r = connection->execute(q);
            if (!r)
                return std::unexpected(std::move(r.error()));
            return std::unique_ptr<TransactionImpl>(new TransactionImpl(std::move(connection), workerId));
        }

        ~TransactionImpl() override {
            if (!isCompleted_)
                static_cast<void>(rollback());
        }

        std::expected<void, SqlError> commit() override {
            if (!isCompleted_.exchange(true)) {
                CmdQuery query{"COMMIT"};
                query.mode = CmdQuery::Mode::write;
                auto exp = connection_->execute(query);
                return exp.transform([](const ResultSet&){});
            }
            else
                return std::unexpected(SqlError{ SqlError::MessageError::internal
                                                , SqlError::SourceError::transaction
                                                , "Transaction already finalized", ""
                });
        }

        std::expected<void, SqlError> rollback() override {
            if (!isCompleted_.exchange(true)) {
                CmdQuery query{"ROLLBACK"};
                query.mode = CmdQuery::Mode::write;
                auto exp = connection_->execute(query);
                return exp.transform([](const ResultSet&){});
            }
            else
                return std::unexpected(SqlError{ SqlError::MessageError::internal
                                                , SqlError::SourceError::transaction
                                                , "Transaction already finalized", ""
                });
        }

        [[nodiscard]] std::uint32_t getWorkerId() const noexcept { return workerId_; }

    private:
        TransactionImpl(std::unique_ptr<IConnection> connection, std::uint32_t workerId)
            : connection_(std::move(connection)), workerId_(workerId)
        { }

        std::unique_ptr<IConnection> connection_;
        std::uint32_t workerId_;
        std::atomic<bool> isCompleted_{false};
    };
}