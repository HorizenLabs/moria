/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "sha_512.hpp"

#include <zen/core/common/assert.hpp>
#include <zen/core/common/cast.hpp>
#include <zen/core/common/endian.hpp>

namespace zen::crypto {

thread_local ObjectPool<SHA512_CTX> Sha512::ctx_pool_{};

Sha512::Sha512() { init(); }

Sha512::~Sha512() {
    if (ctx_) {
        ctx_pool_.add(ctx_.release());
    }
}
void Sha512::init() noexcept {
    if (!ctx_) {
        ctx_.reset(ctx_pool_.acquire());
        if (!ctx_) ctx_ = std::make_unique<SHA512_CTX>();
        ZEN_ASSERT(ctx_.get() != nullptr);
    }
    SHA512_Init(ctx_.get());
    bytes_ = 0;
    buffer_offset_ = 0;
}

void Sha512::update(ByteView data) noexcept {
    // If some room left in buffer fill it
    if (buffer_offset_ != 0) {
        const size_t room_size{std::min(buffer_.size() - buffer_offset_, data.size())};
        memcpy(&buffer_[buffer_offset_], data.data(), room_size);
        data.remove_prefix(room_size);  // Already consumed
        buffer_offset_ += room_size;
        bytes_ += room_size;
        if (buffer_offset_ == buffer_.size()) {
            SHA512_Transform(ctx_.get(), buffer_.data());
            buffer_offset_ = 0;
        }
    }

    // Process remaining data in chunks
    while (data.size() >= SHA512_CBLOCK) {
        bytes_ += SHA512_CBLOCK;
        SHA512_Transform(ctx_.get(), data.data());
        data.remove_prefix(SHA512_CBLOCK);
    }

    // Accumulate leftover in buffer
    if (!data.empty()) {
        memcpy(&buffer_[0], data.data(), data.size());
        buffer_offset_ = data.size();
        bytes_ += data.size();
    }
}

void Sha512::update(std::string_view data) noexcept { update(string_view_to_byte_view(data)); }

Bytes Sha512::finalize() noexcept {
    static const std::array<uint8_t, SHA512_CBLOCK> pad{0x80};

    Bytes sizedesc(16, '\0');
    endian::store_big_u64(&sizedesc[8], bytes_ << 3);
    update({&pad[0], 1 + ((239 - (bytes_ % SHA512_CBLOCK)) % SHA512_CBLOCK)});
    update({&sizedesc[0], sizedesc.size()});
    return finalize_nopadding(false);
}
Bytes Sha512::finalize_nopadding(bool compression) const noexcept {
    if (compression) {
        ZEN_ASSERT(bytes_ == SHA512_CBLOCK);
    }

    Bytes ret(SHA512_DIGEST_LENGTH, '\0');
    for (int i{0}; i < 8; ++i) {
        endian::store_big_u64(&ret[i << 3], ctx_->h[i]);
    }
    return ret;
}

}  // namespace zen::crypto
