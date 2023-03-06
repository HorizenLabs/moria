/*
   Copyright 2014 The Bitcoin Core Developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <zen/core/common/cast.hpp>
#include <zen/core/crypto/hmac_sha_256.hpp>

#include "common_test.hpp"

namespace zen::crypto {

std::string Test_Hmac_Sha256(const std::string_view init_key, const std::string_view input) {
    static crypto::HmacSha256 hmacSha256{};
    hmacSha256.init(*zen::from_hex(init_key));
    return zen::to_hex(RunHasher(hmacSha256, byte_view_to_string_view(*zen::from_hex(input))));
}

TEST_CASE("Hmac Sha256 test vectors", "[crypto]") {
    // See https://www.rfc-editor.org/rfc/rfc4231

    // Test case 1
    CHECK(Test_Hmac_Sha256("0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b", "4869205468657265") ==
          "b0344c61d8db38535ca8afceaf0bf12b"
          "881dc200c9833da726e9376c2e32cff7");

    // Test case 2
    CHECK(Test_Hmac_Sha256("4a656665", "7768617420646f2079612077616e7420666f72206e6f7468696e673f") ==
          "5bdcc146bf60754e6a042426089575c7"
          "5a003f089d2739839dec58b964ec3843");

    // Test case 3
    CHECK(Test_Hmac_Sha256(
              "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa",
              "dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd") ==
          "773ea91e36800e46854db8ebd09181a7"
          "2959098b3ef8c122d9635514ced565fe");

    // Test case 4
    CHECK(Test_Hmac_Sha256(
              "0102030405060708090a0b0c0d0e0f10111213141516171819",
              "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd") ==
          "82558a389a443c0ea4cc819899f2083a"
          "85f0faa3e578f8077a2e3ff46729665b");

    // Test case 5
    CHECK(Test_Hmac_Sha256("0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c0c", "546573742057697468205472756e636174696f6e")
              .starts_with("a3b6167473100ee06e0c796c2955552b"));

    // Test case 6
    CHECK(Test_Hmac_Sha256(std::string(262, 'a'),
                           "54657374205573696e67204c6172676572205468616e20426c6f636b2d53697a65204b6579202d2048617368204"
                           "b6579204669727374") ==
          "60e431591ee0b67f0d8a26aacbf5b77f"
          "8e0bc6213728c5140546040f0ee37f54");

    // Test case 7
    CHECK(Test_Hmac_Sha256(std::string(262, 'a'),
                           "54686973206973206120746573742075"
                           "73696e672061206c6172676572207468"
                           "616e20626c6f636b2d73697a65206b65"
                           "7920616e642061206c61726765722074"
                           "68616e20626c6f636b2d73697a652064"
                           "6174612e20546865206b6579206e6565"
                           "647320746f2062652068617368656420"
                           "6265666f7265206265696e6720757365"
                           "642062792074686520484d414320616c"
                           "676f726974686d2e") ==
          "9b09ffa71b942fcb27635fbcd5b0e944"
          "bfdc63644f0713938a7f51535c3a35e2");
}

}  // namespace zen::crypto
