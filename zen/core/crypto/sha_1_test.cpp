/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/crypto/sha_1.hpp>
#include <zen/core/encoding/hex.hpp>

namespace zen {

void Test_Sha1(const std::string& input, const std::string_view expected_output) {
    static crypto::Sha1 sha1{};
    sha1.init();
    sha1.update(input);
    auto hash{sha1.finalize()};
    CHECK(hash.size() == crypto::Sha1::kDigestLength);
    CHECK(zen::to_hex({hash.data(), hash.length()}) == expected_output);
}

TEST_CASE("Sha1 test vectors", "[crypto]") {
    // See https://www.di-mgt.com.au/sha_testvectors.html
    // clang-format off
    Test_Sha1("", "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    Test_Sha1("abc", "a9993e364706816aba3e25717850c26c9cd0d89d");
    Test_Sha1("message digest", "c12252ceda8be8994d5fa0290a47231c1d16aae3");
    Test_Sha1("secure hash algorithm", "d4d6d2f0ebe317513bbd8d967d89bac5819c2f60");
    Test_Sha1("SHA1 is considered to be safe", "f2b6650569ad3a8720348dd6ea6c497dee3a842a");
    Test_Sha1("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq", "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
    Test_Sha1("For this sample, this 63-byte string will be used as input data", "4f0ea5cd0585a23d028abdc1a6684e5a8094dc49");
    Test_Sha1("This is exactly 64 bytes long, not counting the terminating byte", "fb679f23e7d1ce053313e66e127ab1b444397057");
    Test_Sha1( std::string(1'000'000, 'a'), "34aa973cd4c4daa4f61eeb2bdbad27316534016f");
    // clang-format on
}

TEST_CASE("Sha1 with init + update", "[crypto]") {
    {
        crypto::Sha1 sha1("abc");
        auto hash{sha1.finalize()};
        CHECK(hash.size() == crypto::Sha1::kDigestLength);
        CHECK(zen::to_hex({hash.data(), hash.length()}) ==
              "a9993e364706816aba3e25717850c26c9cd0d89d");
    }

    {
        crypto::Sha1 sha1("This is exactly 64 bytes long");
        sha1.update(", not counting the terminating byte");
        auto hash{sha1.finalize()};
        CHECK(hash.size() == crypto::Sha1::kDigestLength);
        CHECK(zen::to_hex({hash.data(), hash.length()}) ==
              "fb679f23e7d1ce053313e66e127ab1b444397057");
    }
}
}  // namespace zen
