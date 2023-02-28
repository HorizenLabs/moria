/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "sha_256.hpp"

#include <boost/endian.hpp>

#include <zen/core/common/assert.hpp>
#include <zen/core/common/cast.hpp>

namespace zen::crypto {

thread_local ObjectPool<SHA256_CTX> Sha256::ctx_pool_{};

Sha256::Sha256() { init(); }

Sha256::~Sha256() {
    if (ctx_) {
        ctx_pool_.add(ctx_.release());
    }
}
void Sha256::init() noexcept {
    if (!ctx_) {
        ctx_.reset(ctx_pool_.acquire());
        if (!ctx_) ctx_ = std::make_unique<SHA256_CTX>();
        ZEN_ASSERT(ctx_.get() != nullptr);
    }
    SHA256_Init(ctx_.get());
    buffer_offset_ = 0;
    bytes_ = 0;
}

void Sha256::update(ByteView data) noexcept {
    // Consume data in chunks of 64 bytes
    while (!data.empty()) {
        const size_t chunk_size{std::min(buffer_.size() - buffer_offset_, data.size())};
        memcpy(&buffer_[buffer_offset_], data.data(), chunk_size);
        data.remove_prefix(chunk_size);
        buffer_offset_ += chunk_size;
        bytes_ += chunk_size;
        if (buffer_offset_ == buffer_.size()) {
            //SHA256_Update(ctx_.get(), buffer_.data(), buffer_.size());
            SHA256_Transform(ctx_.get(), buffer_.data());
            buffer_offset_ = 0;
        }
    }

    // SHA256_Update(ctx_.get(), data.data(), data.size());
}
void Sha256::update(std::string_view data) noexcept { update(string_view_to_byte_view(data)); }
Bytes Sha256::finalize() noexcept {
    using namespace boost;

    static Bytes pad(64, 0x80);

    // Pad to up to buffer size
    const size_t chunk_size(buffer_.size() - buffer_offset_);
    endian::store_big_u64(pad.data(), bytes_ << 3);
    update({&pad[0], chunk_size});
    return finalize_nopadding(/*compression=*/false);

    //    Bytes ret(SHA256_DIGEST_LENGTH, '\0');
    //    SHA256_Final(ret.data(), ctx_.get());
    //    return ret;
}
Bytes Sha256::finalize_nopadding(bool compression) noexcept {
    using namespace boost;
    if (compression) ZEN_ASSERT(bytes_ == 64);
    Bytes ret(SHA256_DIGEST_LENGTH, '\0');
    for (int i{0}; i < 8; ++i) {
        endian::store_big_u32(&ret[i * 4], ctx_->h[i]);
    }
    return ret;
}

}  // namespace zen::crypto
