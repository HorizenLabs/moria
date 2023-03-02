/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <zen/core/common/base.hpp>

#include "tl/expected.hpp"

namespace zen {

//! \brief Abridges a string to given length and eventually adds an ellipsis if input length is gt required length
//! \remarks Should length be equal to zero then no abridging occurs
[[nodiscard]] std::string abridge(std::string_view input, size_t length);

//! \brief Parses a string input value representing a size in human-readable format with qualifiers. eg "256MB"
[[nodiscard]] tl::expected<uint64_t, std::string> parse_binary_size(const std::string& input);

//! \brief Transforms a size value into it's decimal string representation with binary suffix
//! \see https://en.wikipedia.org/wiki/Binary_prefix
std::string to_string_binary(size_t input);

//! \brief Builds a randomized string of alpha num chars (lowercase) of arbitrary length
std::string get_random_alpha_string(size_t length);

}  // namespace zen
