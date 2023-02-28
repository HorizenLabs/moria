/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#ifndef ZEN_CORE_CRYPTO_SHA_256_HPP_
#define ZEN_CORE_CRYPTO_SHA_256_HPP_

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

    void init() noexcept;
    void update(ByteView data) noexcept;
    void update(std::string_view data) noexcept;
    Bytes finalize() noexcept;
    Bytes finalize_nopadding(bool compression) noexcept;

  private:
    std::unique_ptr<SHA256_CTX> ctx_{nullptr};
    static thread_local ObjectPool<SHA256_CTX> ctx_pool_;

    std::array<uint8_t, 64> buffer_{0};  // Accumulation buffer
    size_t buffer_offset_{0};            // Last populated buffer offset
    size_t bytes_{0};                    // Total accrued bytes
};
}  // namespace zen::crypto

#endif  // ZEN_CORE_CRYPTO_SHA_256_HPP_
