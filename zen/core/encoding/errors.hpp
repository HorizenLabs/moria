/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#ifndef ZEN_CORE_ENCODING_ERRORS_HPP_
#define ZEN_CORE_ENCODING_ERRORS_HPP_

#include <cstdint>

namespace zen {

enum class DecodingError : uint32_t {
    kSuccess,
    kInvalidHexDigit
};

enum class EncodingError : uint32_t {
    kSuccess,
};
}  // namespace zen

#endif  // ZEN_CORE_ENCODING_ERRORS_HPP_
