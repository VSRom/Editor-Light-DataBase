#pragma once

#include "vsc/i_connection.h"
#include <QSqlDatabase>
#include <QString>
#include <QVariant>
#include <utility>

namespace vsc::qt {

    class QtConnection final : public vsc::IConnection {

    public:
        QtConnection(QSqlDatabase database, QString connectionName);
        ~QtConnection() override;

        QtConnection(const QtConnection&) = delete;
        QtConnection& operator=(const QtConnection&) = delete;
        QtConnection(QtConnection&&) = delete;
        QtConnection& operator=(QtConnection&&) = delete;


        ConState state() const noexcept override;
        std::expected<ResultSet, SqlError>
        execute(const CmdQuery &query) override;
        std::expected<std::unique_ptr<ITransaction>, SqlError> beginTransaction() override;
        std::expected<void, SqlError> close() override;



    private:
        QSqlDatabase database_;
        QString connectionName_;
        vsc::ConState state_{ vsc::ConState::closed };

        static std::expected<QVariant, vsc::SqlError> toQtValue(const vsc::Value& value);
        static std::expected<vsc::Value, vsc::SqlError> fromQtValue(const QVariant& value, vsc::Text::allocator_type alloc);
        static vsc::Text outStringPmr(const QString& str, vsc::Text::allocator_type alloc);
    };

}   // owner connection