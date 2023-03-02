/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <vector>

#include <catch2/catch.hpp>

#include <zen/core/common/cast.hpp>
#include <zen/core/common/misc.hpp>

namespace zen {

TEST_CASE("parse_binary_size", "[misc]") {
    auto parsed = parse_binary_size("");
    CHECK((parsed && *parsed == 0));

    parsed = parse_binary_size("not a number");
    CHECK_FALSE(parsed);

    static_assert(kKibi == 1024ULL);
    static_assert(kMebi == 1024ULL * 1024ULL);
    static_assert(kGibi == 1024ULL * 1024ULL * 1024ULL);
    static_assert(kTebi == 1024ULL * 1024ULL * 1024ULL * 1024ULL);

    const std::vector<std::pair<std::string, uint64_t>> tests{
        {"128", 128},          //
        {"128B", 128},         //
        {"180", 180},          //
        {"640KB", 640_Kibi},   //
        {"640 KB", 640_Kibi},  //
        {"750 MB", 750_Mebi},  //
        {"400GB", 400_Gibi},   //
        {"2TB", 2_Tebi},       //
        {".5TB", 512_Gibi},    //
        {"0.5 TB", 512_Gibi}   //
    };

    for (const auto& [input, expected] : tests) {
        const auto value = parse_binary_size(input);
        CHECK((value && *value == expected));
    }
}

TEST_CASE("to_string_binary", "[misc]") {
    const std::vector<std::pair<uint64_t, std::string>> tests{
        {1_Tebi, "1.00 TB"},               //
        {1_Tebi + 512_Gibi, "1.50 TB"},    //
        {1_Tebi + 256_Gibi, "1.25 TB"},    //
        {128, "128 B"},                    //
        {46_Mebi, "46.00 MB"},             //
        {46_Mebi + 256_Kibi, "46.25 MB"},  //
        {1_Kibi, "1.00 KB"}                //
    };

    for (const auto& [val, expected] : tests) {
        CHECK(to_string_binary(val) == expected);
    }
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
