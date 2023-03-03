/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#ifndef ZEN_CORE_COMMON_BASE_HPP_
#define ZEN_CORE_COMMON_BASE_HPP_

#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

#include <intx/intx.hpp>

#if defined(__wasm__)
#define ZEN_THREAD_LOCAL static
#else
#define ZEN_THREAD_LOCAL thread_local
#endif

namespace zen {

using BlockNum = uint32_t;

template <class T>
concept UnsignedIntegral = std::unsigned_integral<T> || std::same_as<T, intx::uint128> ||
                           std::same_as<T, intx::uint256> || std::same_as<T, intx::uint512>;

inline constexpr size_t kHashLength{32};  // Length (in bytes) of a hash

using Bytes = std::basic_string<uint8_t>;

class ByteView : public std::basic_string_view<uint8_t> {
  public:
    constexpr ByteView() noexcept = default;

    constexpr ByteView(const std::basic_string_view<uint8_t>& other) noexcept
        : std::basic_string_view<uint8_t>{other.data(), other.length()} {}

    constexpr ByteView(const Bytes& str) noexcept : std::basic_string_view<uint8_t>{str.data(), str.length()} {}

    constexpr ByteView(const uint8_t* data, size_type length) noexcept
        : std::basic_string_view<uint8_t>{data, length} {}

    template <std::size_t N>
    constexpr ByteView(const uint8_t (&array)[N]) noexcept : std::basic_string_view<uint8_t>{array, N} {}

    template <std::size_t N>
    constexpr ByteView(const std::array<uint8_t, N>& array) noexcept
        : std::basic_string_view<uint8_t>{array.data(), N} {}

    [[nodiscard]] bool is_null() const noexcept { return data() == nullptr; }
};

// https://en.wikipedia.org/wiki/Binary_prefix

inline constexpr uint64_t kKibi{1024};
inline constexpr uint64_t kMebi{1024 * kKibi};
inline constexpr uint64_t kGibi{1024 * kMebi};
inline constexpr uint64_t kTebi{1024 * kGibi};

constexpr uint64_t operator"" _Kibi(unsigned long long x) { return x * kKibi; }

constexpr uint64_t operator"" _Mebi(unsigned long long x) { return x * kMebi; }

constexpr uint64_t operator"" _Gibi(unsigned long long x) { return x * kGibi; }

constexpr uint64_t operator"" _Tebi(unsigned long long x) { return x * kTebi; }

}  // namespace zen
#endif  // ZEN_CORE_COMMON_BASE_HPP_
