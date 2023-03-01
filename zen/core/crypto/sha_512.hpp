/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#ifndef ZEN_CORE_CRYPTO_SHA_512_HPP_
#define ZEN_CORE_CRYPTO_SHA_512_HPP_

#include <array>
#include <cstdint>

#include <boost/noncopyable.hpp>
#include <openssl/sha.h>

#include <zen/core/common/base.hpp>
#include <zen/core/common/object_pool.hpp>

namespace zen::crypto {
//! \brief A wrapper around OpenSSL's SHA512 crypto functions
class Sha512 : private boost::noncopyable {
  public:
    Sha512();
    ~Sha512();

    void init() noexcept;
    void update(ByteView data) noexcept;
    void update(std::string_view data) noexcept;
    [[nodiscard]] Bytes finalize() noexcept;
    [[nodiscard]] Bytes finalize_nopadding(bool compression) const noexcept;

    static constexpr size_t kDigestLength{SHA512_DIGEST_LENGTH};

  private:
    std::unique_ptr<SHA512_CTX> ctx_{nullptr};
    static thread_local ObjectPool<SHA512_CTX> ctx_pool_;
    std::array<uint8_t, SHA512_CBLOCK> buffer_{0x0};
    size_t buffer_offset_{0};
    size_t bytes_{0};
};
}  // namespace zen::crypto

#endif  // ZEN_CORE_CRYPTO_SHA_512_HPP_
