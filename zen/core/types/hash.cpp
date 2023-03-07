/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "hash.hpp"

#include <zen/core/common/endian.hpp>

namespace zen {

Hash::Hash(ByteView init) {
    if (init.length() > kHashLength) return;
    const auto offset{size() - init.size()};
    std::memcpy(&bytes_[offset], init.data(), init.size());
}

Hash::Hash(uint64_t value) { endian::store_big_u64(&bytes_[24], value); }

tl::expected<Hash, DecodingError> Hash::from_hex(std::string_view input) noexcept {
    const auto parsed_bytes{zen::hex::decode(input)};
    if (!parsed_bytes) return tl::unexpected(parsed_bytes.error());
    return Hash(ByteView(*parsed_bytes));
}

std::string Hash::to_hex(bool with_prefix) const noexcept {
    return zen::hex::encode({&bytes_[0], kHashLength}, with_prefix);
}

}  // namespace zen
