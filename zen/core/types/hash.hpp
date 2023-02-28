/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#ifndef ZEN_CORE_TYPES_HASH_HPP_
#define ZEN_CORE_TYPES_HASH_HPP_

#include <array>
#include <ranges>

#include <zen/core/common/assert.hpp>
#include <zen/core/common/base.hpp>
#include <zen/core/encoding/hex.hpp>

namespace zen {
class Hash {
  public:
    Hash() = default;

    //! \brief Creates a Hash from given input
    //! \remarks If len of input exceeds kHashLength then input is disregarded otherwise input is left padded with
    //! zeroes
    explicit Hash(ByteView init);

    //! \brief Converting constructor from unsigned integer value.
    //! \details This constructor assigns the value to the last 8 bytes [24:31] in big endian order
    explicit Hash(uint64_t value);

    //! \brief Returns a hash loaded from a hex string
    static tl::expected<Hash, DecodingError> from_hex(std::string_view input) noexcept;

    //! \brief Returns the hexadecimal representation of this hash
    [[nodiscard]] std::string to_hex(bool with_prefix = false) const noexcept;

    //! \brief The size of a Hash
    static constexpr size_t size() { return kHashLength; }

    inline uint8_t operator[](size_t index) const { return bytes_.at(index); }
    auto operator<=>(const Hash&) const = default;

    inline explicit operator bool() const noexcept {
        return std::ranges::find_if_not(bytes_, [](const uint8_t b) { return b != 0; }) == bytes_.end();
    }

  private:
    std::array<uint8_t, kHashLength> bytes_{0};
};
}  // namespace zen

#endif  // ZEN_CORE_TYPES_HASH_HPP_
