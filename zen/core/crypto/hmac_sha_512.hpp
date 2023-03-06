/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <boost/noncopyable.hpp>

#include <zen/core/crypto/sha_512.hpp>

namespace zen::crypto {

class HmacSha512 : private boost::noncopyable {
  public:
    HmacSha512() = default;
    explicit HmacSha512(const ByteView initial_data);
    explicit HmacSha512(const std::string_view initial_data);

    void init(const ByteView initial_data);
    void init(const std::string_view initial_data);

    void update(ByteView data) noexcept;
    void update(std::string_view data) noexcept;

    [[nodiscard]] Bytes finalize() noexcept;

    static constexpr size_t kDigestLength{SHA512_DIGEST_LENGTH};

  private:
    Sha512 outer{};
    Sha512 inner{};
};

}  // namespace zen::crypto
