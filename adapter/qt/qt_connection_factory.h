#pragma once

#include "vsc/i_connection_factory.h"
#include <expected>
#include <memory>

namespace vsc::qt {
    class QtConnectionFactory final : public vsc::IConnectionFactory {

    public:
        std::expected<std::unique_ptr<vsc::IConnection>, vsc::SqlError>
        open(const vsc::ConnectionParameters& parameters) override;


    };
}
