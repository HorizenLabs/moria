/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <zen/core/crypto/sha_512.hpp>

#include "common_test.hpp"

namespace zen::crypto {

std::string Test_Sha512(std::string_view input) {
    static Sha512 sha512{};
    sha512.init();
    return zen::to_hex(RunHasher(sha512, input));
}

TEST_CASE("Sha512 test vectors", "[crypto]") {
    // See https://www.di-mgt.com.au/sha_testvectors.html
    // clang-format off
    CHECK(Test_Sha512("") == "cf83e1357eefb8bdf1542850d66d8007d620e4050b5715dc83f4a921d36ce9ce47d0d13c5d85f2b0ff8318d2877eec2f63b931bd47417a81a538327af927da3e");
    CHECK(Test_Sha512("abc") == "ddaf35a193617abacc417349ae20413112e6fa4e89a97ea20a9eeee64b55d39a2192992a274fc1a836ba3c23a3feebbd454d4423643ce80e2a9ac94fa54ca49f");
    CHECK(Test_Sha512("message digest")== "107dbf389d9e9f71a3a95f6c055b9251bc5268c2be16d6c13492ea45b0199f3309e16455ab1e96118e8a905d5597b72038ddb372a89826046de66687bb420e7c");
    CHECK(Test_Sha512("secure hash algorithm")== "7746d91f3de30c68cec0dd693120a7e8b04d8073cb699bdce1a3f64127bca7a3d5db502e814bb63c063a7a5043b2df87c61133395f4ad1edca7fcf4b30c3236e");
    CHECK(Test_Sha512("SHA512 is considered to be safe") == "099e6468d889e1c79092a89ae925a9499b5408e01b66cb5b0a3bd0dfa51a99646b4a3901caab1318189f74cd8cf2e941829012f2449df52067d3dd5b978456c2");
    CHECK(Test_Sha512("abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq") == "204a8fc6dda82f0a0ced7beb8e08a41657c16ef468b228a8279be331a703c33596fd15c13b1b07f9aa1d3bea57789ca031ad85c7a71dd70354ec631238ca3445");
    CHECK(Test_Sha512("For this sample, this 63-byte string will be used as input data") == "b3de4afbc516d2478fe9b518d063bda6c8dd65fc38402dd81d1eb7364e72fb6e6663cf6d2771c8f5a6da09601712fb3d2a36c6ffea3e28b0818b05b0a8660766");
    CHECK(Test_Sha512("This is exactly 64 bytes long, not counting the terminating byte") == "70aefeaa0e7ac4f8fe17532d7185a289bee3b428d950c14fa8b713ca09814a387d245870e007a80ad97c369d193e41701aa07f3221d15f0e65a1ff970cedf030");
    CHECK(Test_Sha512("abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu") == "8e959b75dae313da8cf4f72814fc143f8f7779c6eb9f7fa17299aeadb6889018501d289e4900f7e4331b99dec4b5433ac7d329eeb6dd26545e96e55b874be909");
    CHECK(Test_Sha512(std::string(1'000'000, 'a')) == "e718483d0ce769644e2e42c7bc15b4638e1f98b13b2044285632a803afa973ebde0ff244877ea60a4cb0432ce577c31beb009c5c2c49aa2e4eadb217ad8cc09b");
    CHECK(Test_Sha512(LongTestString()) == "40cac46c147e6131c5193dd5f34e9d8bb4951395f27b08c558c65ff4ba2de59437de8c3ef5459d76a52cedc02dc499a3c9ed9dedbfb3281afd9653b8a112fafc");
    // clang-format on
}
}  // namespace zen::crypto
