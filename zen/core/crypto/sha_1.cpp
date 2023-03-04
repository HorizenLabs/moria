/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "sha_1.hpp"

#include <zen/core/common/assert.hpp>
#include <zen/core/common/cast.hpp>
#include <zen/core/common/endian.hpp>

namespace zen::crypto {

ZEN_THREAD_LOCAL ObjectPool<SHA_CTX> Sha1::ctx_pool_{};

Sha1::Sha1() { init(); }

Sha1::~Sha1() {
    if (ctx_) {
        ctx_pool_.add(ctx_.release());
    }
}
Sha1::Sha1(ByteView initial_data) : Sha1() { update(initial_data); }
Sha1::Sha1(std::string_view initial_data) : Sha1(string_view_to_byte_view(initial_data)) {}

void Sha1::init() noexcept {
    if (!ctx_) {
        ctx_.reset(ctx_pool_.acquire());
        if (!ctx_) ctx_ = std::make_unique<SHA_CTX>();
        ZEN_ASSERT(ctx_.get() != nullptr);
    }
    SHA1_Init(ctx_.get());
    bytes_ = 0;
    buffer_offset_ = 0;
}

void Sha1::update(ByteView data) noexcept {
    // If some room left in buffer fill it
    if (buffer_offset_ != 0) {
        const size_t room_size{std::min(buffer_.size() - buffer_offset_, data.size())};
        memcpy(&buffer_[buffer_offset_], data.data(), room_size);
        data.remove_prefix(room_size);  // Already consumed
        buffer_offset_ += room_size;
        bytes_ += room_size;
        if (buffer_offset_ == buffer_.size()) {
            SHA1_Transform(ctx_.get(), buffer_.data());
            buffer_offset_ = 0;
        }
    }

    // Process remaining data in chunks
    while (data.size() >= SHA_CBLOCK) {
        bytes_ += SHA_CBLOCK;
        SHA1_Transform(ctx_.get(), data.data());
        data.remove_prefix(SHA_CBLOCK);
    }

    // Accumulate leftover in buffer
    if (!data.empty()) {
        memcpy(&buffer_[0], data.data(), data.size());
        buffer_offset_ = data.size();
        bytes_ += data.size();
    }
}

void Sha1::update(std::string_view data) noexcept { update(string_view_to_byte_view(data)); }

Bytes Sha1::finalize() noexcept {
    static const std::array<uint8_t, SHA_CBLOCK> pad{0x80};

    Bytes sizedesc(8, '\0');
    endian::store_big_u64(&sizedesc[0], bytes_ << 3);
    update({&pad[0], 1 + ((119 - (bytes_ % SHA_CBLOCK)) % SHA_CBLOCK)});
    update({&sizedesc[0], sizedesc.size()});

    Bytes ret(SHA_DIGEST_LENGTH, '\0');
    endian::store_big_u32(&ret[0], ctx_->h0);
    endian::store_big_u32(&ret[4], ctx_->h1);
    endian::store_big_u32(&ret[8], ctx_->h2);
    endian::store_big_u32(&ret[12], ctx_->h3);
    endian::store_big_u32(&ret[16], ctx_->h4);
    return ret;
}

}  // namespace zen::crypto
