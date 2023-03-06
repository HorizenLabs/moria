/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <zen/core/common/assert.hpp>
#include <zen/core/common/cast.hpp>
#include <zen/core/common/endian.hpp>
#include <zen/core/crypto/ripemd.hpp>

namespace zen::crypto {

ZEN_THREAD_LOCAL ObjectPool<RIPEMD160_CTX> Ripemd160::ctx_pool_{};

Ripemd160::Ripemd160() { init(); }

Ripemd160::~Ripemd160() {
    if (ctx_) {
        ctx_pool_.add(ctx_.release());
    }
}
Ripemd160::Ripemd160(ByteView initial_data) : Ripemd160() { update(initial_data); }
Ripemd160::Ripemd160(std::string_view initial_data) : Ripemd160(string_view_to_byte_view(initial_data)) {}

void Ripemd160::init() noexcept {
    if (!ctx_) {
        ctx_.reset(ctx_pool_.acquire());
        if (!ctx_) ctx_ = std::make_unique<RIPEMD160_CTX>();
        ZEN_ASSERT(ctx_.get() != nullptr);
    }
    RIPEMD160_Init(ctx_.get());
    bytes_ = 0;
    buffer_offset_ = 0;
}

void Ripemd160::update(ByteView data) noexcept {
    // If some room left in buffer fill it
    if (buffer_offset_ != 0) {
        const size_t room_size{std::min(buffer_.size() - buffer_offset_, data.size())};
        memcpy(&buffer_[buffer_offset_], data.data(), room_size);
        data.remove_prefix(room_size);  // Already consumed
        buffer_offset_ += room_size;
        bytes_ += room_size;
        if (buffer_offset_ == buffer_.size()) {
            RIPEMD160_Transform(ctx_.get(), buffer_.data());
            buffer_offset_ = 0;
        }
    }

    // Process remaining data in chunks
    while (data.size() >= RIPEMD160_CBLOCK) {
        bytes_ += RIPEMD160_CBLOCK;
        RIPEMD160_Transform(ctx_.get(), data.data());
        data.remove_prefix(RIPEMD160_CBLOCK);
    }

    // Accumulate leftover in buffer
    if (!data.empty()) {
        memcpy(&buffer_[0], data.data(), data.size());
        buffer_offset_ = data.size();
        bytes_ += data.size();
    }
}

void Ripemd160::update(std::string_view data) noexcept { update(string_view_to_byte_view(data)); }

Bytes Ripemd160::finalize() noexcept {
    static const std::array<uint8_t, RIPEMD160_CBLOCK> pad{0x80};

    Bytes sizedesc(8, '\0');
    endian::store_little_u64(&sizedesc[0], bytes_ << 3);
    update({&pad[0], 1 + ((119 - (bytes_ % RIPEMD160_CBLOCK)) % RIPEMD160_CBLOCK)});
    update({&sizedesc[0], sizedesc.size()});

    Bytes ret(RIPEMD160_DIGEST_LENGTH, '\0');
    endian::store_little_u32(&ret[0], ctx_->A);
    endian::store_little_u32(&ret[4], ctx_->B);
    endian::store_little_u32(&ret[8], ctx_->C);
    endian::store_little_u32(&ret[12], ctx_->D);
    endian::store_little_u32(&ret[16], ctx_->E);
    return ret;
}

}  // namespace zen::crypto
