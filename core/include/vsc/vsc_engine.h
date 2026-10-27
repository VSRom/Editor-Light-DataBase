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
        VscEngine(std::unique_ptr<IConnectionFactory> factory, std::shared_ptr<ITaskScheduler> scheduler
                  , ConnectionParameters params)
            : factory_(std::move(factory)), scheduler_(std::move(scheduler)), params_(std::move(params))
        { }

        auto executeAsync(const CmdQuery& query) {
            auto promise = std::make_shared<std::promise<std::expected<ResultSet, SqlError>>>();
            auto future = promise->get_future();

            scheduler_->schedule([this, query, promise]() {
                auto conResult  = factory_->open(params_);

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
                auto conResult  = factory_->open(params_);

                if (!conResult) {
                    promise->set_value(std::unexpected(conResult.error()));
                    return;
                }

                auto created = TransactionImpl::create(std::move(conResult.value()), workerId);

                if (!created)
                    promise->set_value(std::unexpected(created.error()));
                else
                    promise->set_value(std::move(created.value()));


            }, workerId);

            return future;
        }

        ~VscEngine() { scheduler_->drain(); }

    private:
        std::unique_ptr<IConnectionFactory> factory_;
        std::shared_ptr<ITaskScheduler> scheduler_;
        ConnectionParameters params_;
    };
}