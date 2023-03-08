/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/encoding/serialize.hpp>

namespace zen {

TEST_CASE("Serialization Sizes", "[encoding]") {
    CHECK(ser::size('a') == 1);
    CHECK(ser::size(uint8_t{0}) == 1);
    CHECK(ser::size(int8_t{0}) == 1);
    CHECK(ser::size(uint16_t{0}) == 2);
    CHECK(ser::size(int16_t{0}) == 2);
    CHECK(ser::size(uint32_t{0}) == 4);
    CHECK(ser::size(int32_t{0}) == 4);
    CHECK(ser::size(uint64_t{0}) == 8);
    CHECK(ser::size(int64_t{0}) == 8);
    CHECK(ser::size(float{0}) == 4);
    CHECK(ser::size(double{0}) == 8);
    CHECK(ser::size(bool{true}) == 1);

    CHECK(ser::compact_size(0x00) == 1);
    CHECK(ser::compact_size(0xfffa) == 3);
    CHECK(ser::compact_size(256) == 3);
    CHECK(ser::compact_size(0x10003) == 5);
    CHECK(ser::compact_size(0xffffffff) == 5);
    CHECK(ser::compact_size(UINT64_MAX) == 9);
}
}  // namespace zen
