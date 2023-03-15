/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <array>
#include <ranges>

#include <zen/core/common/assert.hpp>
#include <zen/core/common/base.hpp>
#include <zen/core/encoding/hex.hpp>

namespace zen {
template <uint32_t BITS>
class Hash {
  public:
    static_assert(BITS >= 8 && BITS % 8 == 0, "Must be a multiple of 8");
    enum : uint32_t {
        kSize = BITS / 8
    };

    using iterator_type = typename std::array<uint8_t, kSize>::iterator;
    using const_iterator_type = typename std::array<uint8_t, kSize>::const_iterator;

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

    //! \brief An alias for to_hex with no prefix
    [[nodiscard]] std::string to_string() const noexcept { return to_hex(false); }

    //! \brief The size of a Hash
    static constexpr size_t size() { return kSize; }

    //! \brief Returns the hash to its pristine state (i.e. all zeroes)
    void reset() { memset(&bytes_, 0, kSize); }

    iterator_type begin() { return bytes_.begin(); }

    iterator_type end() { return bytes_.end(); }

    const_iterator_type cbegin() {return bytes_.cbegin(); }

    const_iterator_type cend() {return bytes_.cend(); }

    inline uint8_t operator[](size_t index) const { return bytes_.at(index); }
    auto operator<=>(const Hash&) const = default;

    inline explicit operator bool() const noexcept {
        return std::ranges::find_if_not(bytes_, [](const uint8_t b) { return b != 0; }) == bytes_.end();
    }

  private:
    alignas(uint32_t) std::array<uint8_t, kSize> bytes_{0};
};

using h160 = Hash<160>;
using h256 = Hash<256>;

}  // namespace zen
