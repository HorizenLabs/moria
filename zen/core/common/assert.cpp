/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "assert.hpp"

#include <iostream>

namespace zen {
void abort_due_to_assertion_failure(std::string_view message, std::source_location location) {
    std::cerr << "Assert failed: " << message << "\n"
              << "Source: " << location.file_name() << ", line " << location.line() << std::endl;
    std::abort();
}
}  // namespace zen