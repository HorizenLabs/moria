/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <CLI/CLI.hpp>

#include <infra/nat/option.hpp>

namespace zenpp::cmd::common {
struct NatOptionValidator : public CLI::Validator {
    explicit NatOptionValidator();
};
} // namespace zenpp::cmd::common