/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/common/misc.hpp>

namespace zen {

TEST_CASE("parse_binary_size", "[misc]") {
    auto parsed = parse_binary_size("");
    CHECK((parsed && *parsed == 0));

    static_assert(kKibi == 1024ULL);
    static_assert(kMebi == 1024ULL * 1024ULL);
    static_assert(kGibi == 1024ULL * 1024ULL * 1024ULL);
    static_assert(kTebi == 1024ULL * 1024ULL * 1024ULL * 1024ULL);

    parsed = parse_binary_size("128");
    CHECK((parsed && *parsed == 128));
    parsed = parse_binary_size("128B");
    CHECK((parsed && *parsed == 128));
    parsed = parse_binary_size("180");
    CHECK((parsed && *parsed == 180));
    parsed = parse_binary_size("640KB");
    CHECK((parsed && *parsed == 640 * kKibi));
    parsed = parse_binary_size("75MB");
    CHECK((parsed && *parsed == 75 * kMebi));
    parsed = parse_binary_size("400GB");
    CHECK((parsed && *parsed == 400 * kGibi));
    parsed = parse_binary_size("2TB");
    CHECK((parsed && *parsed == 2 * kTebi));
    parsed = parse_binary_size(".5TB");
    CHECK((parsed && *parsed == 0.5 * kTebi));
    parsed = parse_binary_size("0.5TB");
    CHECK((parsed && *parsed == 0.5 * kTebi));
    parsed = parse_binary_size("0.5  TB");
    CHECK((parsed && *parsed == 0.5 * kTebi));

    parsed = parse_binary_size("not a number");
    CHECK_FALSE(parsed);
}

TEST_CASE("to_string_binary", "[misc]") {
    uint64_t val{1 * kTebi};
    CHECK(to_string_binary(val) == "1.00 TB");

    val += 512 * kGibi;
    CHECK(to_string_binary(val) == "1.50 TB");

    val = 128;
    CHECK(to_string_binary(val) == "128.00 B");

    val = kKibi;
    CHECK(to_string_binary(val) == "1.00 KB");
}

TEST_CASE("abridge", "[misc]") {
    std::string input = "01234567890";
    std::string abridged = abridge(input, 50);
    CHECK(input == abridged);
    abridged = abridge(input, 3);
    CHECK(abridged == "012...");

    CHECK(abridge("", 0).empty());
    CHECK(abridge("0123", 0) == "0123");
}

}  // namespace zen
