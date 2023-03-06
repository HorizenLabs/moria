/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "hmac_sha_512.hpp"

#include <zen/core/common/cast.hpp>

namespace zen::crypto {

HmacSha512::HmacSha512(const ByteView initial_data) { init(initial_data); }

HmacSha512::HmacSha512(const std::string_view initial_data) { init(initial_data); }

void HmacSha512::init(const ByteView initial_data) {
    inner.init();
    outer.init();

    Bytes rkey;
    rkey.reserve(SHA512_CBLOCK);

    if (initial_data.length() > SHA512_CBLOCK) {
        inner.update(initial_data);
        rkey.assign(inner.finalize());
        inner.init(); // Reset
    } else {
        rkey.assign(initial_data);
    }
    rkey.resize(SHA512_CBLOCK, 0);

    for (size_t i{0}; i < rkey.size(); ++i) {
        rkey[i] ^= 0x5c;
    }
    outer.update(rkey);

    for (size_t i{0}; i < rkey.size(); ++i) {
        rkey[i] ^= 0x5c ^ 0x36;
    }
    inner.update(rkey);
}

void HmacSha512::init(const std::string_view initial_data) { init(string_view_to_byte_view(initial_data)); }

void HmacSha512::update(ByteView data) noexcept { inner.update(data); }

void HmacSha512::update(std::string_view data) noexcept { update(string_view_to_byte_view(data)); }

Bytes HmacSha512::finalize() noexcept {
    outer.update(inner.finalize());
    return outer.finalize();
}
}  // namespace zen::crypto
