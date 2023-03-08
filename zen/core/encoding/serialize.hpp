/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <cstdint>
#include <type_traits>

#include <zen/core/common/base.hpp>

//! \brief All functions dedicated to objects and types serialization
namespace zen::ser {

static constexpr uint32_t kMaxSerializedCompactSize{0x02000000};

//! \brief Scopes of serialization/deserialization
enum class Scope {
    kNetwork = (1 << 0),
    kStorage = (1 << 1),
    kHash = (1 << 2)
};

//! \brief Returns the serialized size of fundamental types
template <class T>
requires std::is_arithmetic_v<T>
inline uint32_t size(T) { return sizeof(T); }

//! \brief Returns the serialized size of fundamental types
//! \remarks Specialization for bool
template <>
inline uint32_t size(bool) {
    return 1U;
}

//! \brief Returns the size of a variable integer
inline uint32_t compact_size(uint64_t value) {
    if (value < 253)
        return 1;
    else if (value <= 0xFFFF)
        return 3;
    else if (value <= 0xFFFFFFFF)
        return 5;

    return 9;
}

}  // namespace zen::ser