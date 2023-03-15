/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <iostream>

#include <catch2/catch.hpp>

#include <zen/core/types/block.hpp>

namespace zen {

TEST_CASE("Boost serialize block", "[types]") {
    BlockHeader bh{};
    std::ostream& os = std::cout;

    {
        boost::archive::text_oarchive oa(os, 50);
        oa << bh;
    }
}
}  // namespace zen
