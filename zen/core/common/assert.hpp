/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#ifndef ZEN_CORE_COMMON_ASSERT_HPP_
#define ZEN_CORE_COMMON_ASSERT_HPP_

#include <source_location>
#include <string_view>

namespace zen {
void abort_due_to_assertion_failure(std::string_view message,
                                    std::source_location location = std::source_location::current());
}  // namespace zen

//! \brief Always aborts program execution on assertion failure, even when NDEBUG is defined.
#define ZEN_ASSERT(expr)      \
    if ((expr)) [[likely]]    \
        static_cast<void>(0); \
    else                      \
        ::zen::abort_due_to_assertion_failure(#expr, std::source_location::current())

#endif  // ZEN_CORE_COMMON_ASSERT_HPP_
