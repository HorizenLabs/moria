/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <zen/core/common/assert.hpp>
#include <zen/core/common/endian.hpp>
#include <zen/core/types/hash.hpp>

namespace zen {

template <uint32_t BITS>
Hash<BITS>::Hash(ByteView init) {
    ZEN_ASSERT(init.length() <= kSize);
    std::memcpy(&bytes_[0], init.data(), init.size());
}

template <uint32_t BITS>
Hash<BITS>::Hash(uint64_t value) {
    const auto offset{size() - sizeof(value)};
    endian::store_big_u64(&bytes_[offset], value);
}

template <uint32_t BITS>
tl::expected<Hash<BITS>, DecodingError> Hash<BITS>::from_hex(std::string_view input) noexcept {
    const auto parsed_bytes{hex::decode(input)};
    if (!parsed_bytes) return tl::unexpected(parsed_bytes.error());
    return Hash(ByteView(*parsed_bytes));
}

template <uint32_t BITS>
std::string Hash<BITS>::to_hex(bool with_prefix) const noexcept {
    return zen::hex::encode({&bytes_[0], kSize}, with_prefix);
}

}  // namespace zen
