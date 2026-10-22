#pragma once

#include "i_connection_factory.h"
#include "i_task_scheduler.h"
#include "transaction_impl.h"
#include "query.h"
#include "result_set.h"
#include "sql_error.h"

#include <memory>
#include <future>
#include <expected>

namespace vsc {
    class VscEngine {
    public:
        VscEngine(std::unique_ptr<IConnectionFactory> factory, std::shared_ptr<ITaskScheduler> scheduler)
            : factory_(std::move(factory)), scheduler_(std::move(scheduler))
        { }

        auto executeAsync(const CmdQuery& query) {
            auto promise = std::make_shared<std::promise<std::expected<ResultSet, SqlError>>>();
            auto future = promise->get_future();

            scheduler_->schedule([this, query, promise]() {
                ConnectionParameters params{};
                auto conResult  = factory_->open(params);

                if(!conResult) {
                    promise->set_value(std::unexpected(conResult.error()));
                    return;
                }

                auto connection = std::move(conResult.value());
                auto executionResult = connection->execute(query);
                promise->set_value(std::move(executionResult));
            });

            return future;
        }

        auto beginTransactionAsync(std::uint32_t workerId) {
            auto promise = std::make_shared<std::promise<std::expected<std::unique_ptr<ITransaction>, SqlError>>>();
            auto future = promise->get_future();

            scheduler_->schedule([this, workerId, promise]() {
                ConnectionParameters params{};
                auto conResult  = factory_->open(params);

                if (!conResult) {
                    promise->set_value(std::unexpected(conResult.error()));
                    return;
                }

                auto objTransaction = std::make_unique<TransactionImpl>(std::move(conResult.value()), workerId);
                CmdQuery query{ .sqlText = "BEGIN TRANSACTION", .mode = CmdQuery::Mode::write };
                auto startResult = objTransaction->getConnection().execute(query);

                if (!startResult)
                    promise->set_value(std::unexpected(startResult.error()));
                else
                    promise->set_value(std::move(objTransaction));

            }, workerId);

            return future;
        }

    private:
        std::unique_ptr<IConnectionFactory> factory_;
        std::shared_ptr<ITaskScheduler> scheduler_;
    };
}