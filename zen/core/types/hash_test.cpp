/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "hash.hpp"

#include <catch2/catch.hpp>

namespace zen {

TEST_CASE("Hash", "[types]") {
    Hash hash;
    CHECK_FALSE(hash);  // Is empty

    // Empty hash hex
    std::string expected_hex{"0000000000000000000000000000000000000000000000000000000000000000"};
    std::string out_hex{hash.to_hex()};
    REQUIRE(out_hex == expected_hex);

    // Exact length valid hex
    std::string input_hex{"0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470"};
    auto parsed_hash = Hash::from_hex(input_hex);
    REQUIRE(parsed_hash);
    REQUIRE(parsed_hash->operator[](0) == 0xc5);
    out_hex = parsed_hash->to_hex(/* with_prefix=*/true);
    REQUIRE(input_hex == out_hex);

    // Exact length invalid hex
    input_hex = "0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85zzzz";
    parsed_hash = Hash::from_hex(input_hex);
    REQUIRE((!parsed_hash && parsed_hash.error() == DecodingError::kInvalidHexDigit));

    // Oversize length (Hash loaded but null)
    input_hex = "0xc5d2460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470000000";
    parsed_hash = Hash::from_hex(input_hex);
    REQUIRE(parsed_hash);
    REQUIRE(!(*parsed_hash));  // Is empty

    // Shorter length valid hex
    input_hex = "0x460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470";
    parsed_hash = Hash::from_hex(input_hex);
    REQUIRE(parsed_hash);
    out_hex = parsed_hash->to_hex(/*with_prefix=*/true);
    REQUIRE(input_hex != out_hex);
    expected_hex = "0x0000460186f7233c927e7db2dcc703c0e500b653ca82273b7bfad8045d85a470";
    REQUIRE(out_hex == expected_hex);

    // Comparison
    auto parsed_hash1 = Hash::from_hex("0x01");
    REQUIRE(parsed_hash1);
    auto parsed_hash2 = Hash::from_hex("0x02");
    REQUIRE(parsed_hash2);
    REQUIRE((parsed_hash1 != parsed_hash2));
}

}  // namespace zen
