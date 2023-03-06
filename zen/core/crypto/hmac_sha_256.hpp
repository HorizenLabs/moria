/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <boost/noncopyable.hpp>

#include <zen/core/crypto/sha_256.hpp>

namespace zen::crypto {

class HmacSha256 : private boost::noncopyable {
  public:
    HmacSha256() = default;
    explicit HmacSha256(const ByteView initial_data);
    explicit HmacSha256(const std::string_view initial_data);

    void init(const ByteView initial_data);
    void init(const std::string_view initial_data);

    void update(ByteView data) noexcept;
    void update(std::string_view data) noexcept;

    [[nodiscard]] Bytes finalize() noexcept;

    static constexpr size_t kDigestLength{SHA512_DIGEST_LENGTH};

  private:
    Sha256 outer{};
    Sha256 inner{};
};

}  // namespace zen::crypto
