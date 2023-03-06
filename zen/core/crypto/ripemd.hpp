/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <array>

#include <boost/noncopyable.hpp>
#include <openssl/ripemd.h>

#include <zen/core/common/base.hpp>
#include <zen/core/common/object_pool.hpp>

namespace zen::crypto {
//! \brief A wrapper around OpenSSL's RIPEMD160 crypto functions
class Ripemd160 : private boost::noncopyable {
  public:
    Ripemd160();
    ~Ripemd160();

    explicit Ripemd160(ByteView initial_data);
    explicit Ripemd160(std::string_view initial_data);

    void init() noexcept;
    void update(ByteView data) noexcept;
    void update(std::string_view data) noexcept;
    [[nodiscard]] Bytes finalize() noexcept;

    inline constexpr size_t digest_length() { return RIPEMD160_DIGEST_LENGTH; };
    inline constexpr size_t block_size() { return RIPEMD160_CBLOCK; };

  private:
    std::unique_ptr<RIPEMD160_CTX> ctx_{nullptr};
    static ZEN_THREAD_LOCAL ObjectPool<RIPEMD160_CTX> ctx_pool_;
    std::array<uint8_t, RIPEMD160_CBLOCK> buffer_{0x0};
    size_t buffer_offset_{0};
    size_t bytes_{0};
};

}  // namespace zen::crypto
