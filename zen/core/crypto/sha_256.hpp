/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <array>
#include <cstdint>

#include <boost/noncopyable.hpp>
#include <openssl/sha.h>

#include <zen/core/common/base.hpp>
#include <zen/core/common/object_pool.hpp>

namespace zen::crypto {
//! \brief A wrapper around OpenSSL's SHA256 crypto functions
class Sha256 : private boost::noncopyable {
  public:
    Sha256();
    ~Sha256();

    explicit Sha256(ByteView initial_data);
    explicit Sha256(std::string_view initial_data);

    void init() noexcept;
    void update(ByteView data) noexcept;
    void update(std::string_view data) noexcept;
    [[nodiscard]] Bytes finalize() noexcept;
    [[nodiscard]] Bytes finalize_nopadding(bool compression) const noexcept;

    static constexpr size_t kDigestLength{SHA256_DIGEST_LENGTH};

  private:
    std::unique_ptr<SHA256_CTX> ctx_{nullptr};
    static thread_local ObjectPool<SHA256_CTX> ctx_pool_;
    std::array<uint8_t, SHA256_CBLOCK> buffer_{0x0};
    size_t buffer_offset_{0};
    size_t bytes_{0};
};
}  // namespace zen::crypto
