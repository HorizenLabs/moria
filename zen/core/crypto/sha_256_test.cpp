/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/crypto/sha_256.hpp>
#include <zen/core/encoding/hex.hpp>

namespace zen {

TEST_CASE("Sha256 test vectors", "[crypto]") {
    // See https://www.di-mgt.com.au/sha_testvectors.html
    crypto::Sha256 sha256{};
    sha256.update("abc");
    auto hash{sha256.finalize()};
    std::string expected_hash{"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"};
    CHECK(zen::to_hex({hash.data(), hash.length()}) == expected_hash);

    sha256.init();
    sha256.update("");
    hash = sha256.finalize();
    expected_hash = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    CHECK(zen::to_hex({hash.data(), hash.length()}) == expected_hash);

    sha256.init();
    sha256.update(
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrs"
        "tu");
    hash = sha256.finalize();
    expected_hash = "cf5b16a778af8380036ce59e7b0492370b249b11e8f07a51afac45037afee9d1";
    CHECK(zen::to_hex({hash.data(), hash.length()}) == expected_hash);

    sha256.init();
    Bytes long_input(1'000'000, 'a');
    sha256.update({long_input.data(), long_input.size()});
    hash = sha256.finalize();
    expected_hash = "cdc76e5c9914fb9281a1c7e284d73e67f1809a48a497200e046d39ccc7112cd0";
    CHECK(zen::to_hex({hash.data(), hash.length()}) == expected_hash);

}
}  // namespace zen
