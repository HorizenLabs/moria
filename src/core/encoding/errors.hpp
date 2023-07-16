/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <cstdint>

namespace zenpp {

enum class DecodingError : uint32_t {
    kSuccess,
    kInvalidHexDigit,
    kInvalidInput,
    kInputTooShort,
    kInvalidBase58Input,
    kInvalidBase58Checksum,
    kInvalidBase64Input,
    kInvalidAmountRange
};

enum class EncodingError : uint32_t {
    kSuccess,
    kInputTooLong,
    kUnexpectedError,
};
}  // namespace zenpp
