/*
   Copyright 2023 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <optional>
#include <string>

#include <infra/network/addresses.hpp>

namespace zenpp::nat {

enum class NatType {
    kNone,  // No network address translation: local IP as public IP
    kAuto,  // Detect public IP address using IPify.org
    kIp     // Use provided IP address as public IP
};

struct Option {
    NatType _type{NatType::kAuto};
    std::optional<net::IPAddress> _address{std::nullopt};
};

//! \brief Used by CLI to convert a string to a NAT Option
bool lexical_cast(const std::string& input, Option& value);

}  // namespace zenpp::nat