/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/types/amounts.hpp>

namespace zen {

TEST_CASE("Amounts", "[types]") {
    Amount a1;
    CHECK_FALSE(a1);
    CHECK(a1.to_string() == "0.00000000 ZEN");

    a1 += kMaxMoney + 1;
    CHECK(a1);
    CHECK_FALSE(a1.valid_money());

    a1 = 10;
    CHECK(a1);
    CHECK(a1.valid_money());
    a1 = -2;
    CHECK(a1);
    CHECK_FALSE(a1.valid_money());

    Amount a2{a1};
    CHECK(a1 == a2);
    CHECK_FALSE(a1 > a2);
    CHECK_FALSE(a1 < a2);

    ++a1;
    CHECK(a1 >= a2);
    --a1;
    CHECK(a1 == a2);
    auto a3{a1 + a2};
    CHECK_FALSE(a3.valid_money());
    a3 *= -1;
    CHECK(a3.to_string() == "0.00000004 ZEN");
}

TEST_CASE("FeeRates", "[types]") {
    FeeRate fr1{10};
    CHECK(fr1.to_string() == "0.00000010 ZEN/K");

    FeeRate fr2{1 * kCoin};
    CHECK(fr2.to_string() == "1.00000000 ZEN/K");
    CHECK(fr1 != fr2);

    // Nominal fee
    auto fee{fr2.fee()};
    CHECK(fee == 1 * kCoin);

    // Fee for specific size
    fee = fr2.fee(100);
    CHECK(fee == 1 * kCoin / 10);
}
}  // namespace zen
