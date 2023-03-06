/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <zen/core/crypto/sha_256.hpp>

#include "common_test.hpp"

namespace zen::crypto {

std::string Test_Sha256(std::string_view input) {
    static Sha256 sha256{};
    sha256.init();
    return zen::to_hex(RunHasher(sha256, input));
}

TEST_CASE("Sha256 test vectors", "[crypto]") {
    // See https://www.di-mgt.com.au/sha_testvectors.html
    // clang-format off
    CHECK(Test_Sha256("") == "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    CHECK(Test_Sha256("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    CHECK(Test_Sha256("message digest") == "f7846f55cf23e14eebeab5b4e1550cad5b509e3348fbc4efa3a1413d393cb650");
    CHECK(Test_Sha256("secure hash algorithm") == "f30ceb2bb2829e79e4ca9753d35a8ecc00262d164cc077080295381cbd643f0d");
    CHECK(Test_Sha256("SHA256 is considered to be safe") == "6819d915c73f4d1e77e4e1b52d1fa0f9cf9beaead3939f15874bd988e2a23630");
    CHECK(Test_Sha256("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") == "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    CHECK(Test_Sha256("For this sample, this 63-byte string will be used as input data") == "f08a78cbbaee082b052ae0708f32fa1e50c5c421aa772ba5dbb406a2ea6be342");
    CHECK(Test_Sha256("This is exactly 64 bytes long, not counting the terminating byte") == "ab64eff7e88e2e46165e29f2bce41826bd4c7b3552f6b382a9e7d3af47c245f8");
    CHECK(Test_Sha256("As Bitcoin relies on 80 byte header hashes, we want to have an example for that.") == "7406e8de7d6e4fffc573daef05aefb8806e7790f55eab5576f31349743cca743");
    CHECK(Test_Sha256("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu") == "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1");
    CHECK(Test_Sha256( std::string(1'000'000, 'a')) == "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0");
    CHECK(Test_Sha256( LongTestString()) == "a316d55510b49662420f49d145d42fb83f31ef8dc016aa4e32df049991a91e26");
    // clang-format on
}
}  // namespace zen::crypto
