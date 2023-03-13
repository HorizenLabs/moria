/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <array>
#include <bit>
#include <type_traits>

#include <zen/core/common/base.hpp>
#include <zen/core/serialization/base.hpp>

//! \brief All functions dedicated to objects and types serialization
namespace zen::ser {

//! \brief Returns the serialized size of arithmetic types
//! \remarks Do not define serializable classes members as size_t as it might lead to wrong results on
//! MacOS/Xcode bundles
template <class T>
requires std::is_arithmetic_v<T>
inline uint32_t ser_sizeof(T) { return sizeof(T); }

//! \brief Returns the serialized size of arithmetic types
//! \remarks Specialization for bool which is stored in at least 1 byte
template <>
inline uint32_t ser_sizeof(bool) {
    return sizeof(char);
}

//! \brief Returns the serialzed size of a compacted integral
//! \remarks Mostly used in P2P messages to prepend a list of elements with the count of elements.
//! Not to be confused with varint which is used in storage serialization
inline uint32_t ser_csizeof(uint64_t value) {
    if (value < 253)
        return 1;
    else if (value <= 0xffff)
        return 3;
    else if (value <= 0xffffffff)
        return 5;

    return 9;
}

//! \brief Lowest level serialization for integral arithmetic types
template <class Stream, typename T>
requires std::is_integral_v<T>
inline void write_data(Stream& s, T obj) { s.write(reinterpret_cast<uint8_t*>(&obj), ser_sizeof(obj)); }

//! \brief Lowest level serialization for bool
template <class Stream>
inline void write_data(Stream& s, bool obj) {
    const uint8_t out{static_cast<uint8_t>(obj ? 0x0 : 0x1)};
    write_data(s, out);
}

//! \brief Lowest level serialization for float
template <class Stream>
inline void write_data(Stream& s, float obj) {
    auto casted{std::bit_cast<uint32_t>(obj)};
    write_data(s, casted);
}

//! \brief Lowest level serialization for double
template <class Stream>
inline void write_data(Stream& s, double obj) {
    auto casted{std::bit_cast<uint64_t>(obj)};
    write_data(s, casted);
}

//! \brief Lowest level serialization for compact integer
template <class Stream>
inline void write_compact(Stream& s, uint64_t obj) {
    auto size{ser_csizeof(obj)};

    union caster_t {
        uint64_t u64;
        std::array<uint32_t, 2> u32;
        std::array<uint16_t, 4> u16;
        std::array<uint8_t, 8> u8;
    };

    const caster_t caster{obj};
    switch (size) {
        case 1:
            write_data(s, caster.u8[0]);
            break;
        case 3:
            write_data(s, uint8_t{253});
            write_data(s, caster.u16[0]);
            break;
        case 5:
            write_data(s, uint8_t{254});
            write_data(s, caster.u32[0]);
            break;
        default:
            write_data(s, uint8_t{255});
            write_data(s, caster.u64);
    }
}

//! \brief Lowest level deserialization for integral arithmetic types
template <typename T, class Stream>
requires std::is_integral_v<T>
inline tl::expected<T, DeserializationError> read_data(Stream& s) {
    T ret{};
    const uint32_t count{ser_sizeof(ret)};
    const auto read_result{s.read(count)};
    if (!read_result) return tl::unexpected(read_result.error());
    std::memcpy(&ret, read_result->data(), count);
    s.shrink();  // Remove consumed data
    return ret;
}

//! \brief Lowest level deserialization for compact integer
template <class Stream>
inline tl::expected<uint64_t, DeserializationError> read_compact(Stream& s) {
    const auto size{read_data<uint8_t>(s)};
    if (!size) return tl::unexpected(size.error());

    uint64_t ret;
    if (*size < 253) {
        ret = *size;
    } else if (*size == 253) {
        const auto value{read_data<uint16_t>(s)};
        if (!value) return tl::unexpected(value.error());
        if (*value < 253) return tl::unexpected(DeserializationError::kNonCanonicalCompactSize);
        ret = *value;
    } else if (*size == 254) {
        const auto value{read_data<uint32_t>(s)};
        if (!value) return tl::unexpected(value.error());
        if (*value < 0x10000U) return tl::unexpected(DeserializationError::kNonCanonicalCompactSize);
        ret = *value;
    } else {
        const auto value{read_data<uint64_t>(s)};
        if (!value) return tl::unexpected(value.error());
        if (*value < 0x100000000ULL) return tl::unexpected(DeserializationError::kNonCanonicalCompactSize);
        ret = *value;
    }
    if (ret > kMaxSerializedCompactSize) return tl::unexpected(DeserializationError::kCompactSizeTooBig);
    return ret;
}
}  // namespace zen::ser
