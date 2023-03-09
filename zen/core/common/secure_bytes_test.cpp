/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <catch2/catch.hpp>

#include <zen/core/common/base.hpp>
#include <zen/core/common/secure_bytes.hpp>

namespace zen {
TEST_CASE("Secure Bytes", "[memory]") {
    uint8_t* ptr{nullptr};
    {
        SecureBytes secure_bytes(4_Kibi, '\0');
        ptr = secure_bytes.data();
        secure_bytes[0] = 'a';
        secure_bytes[1] = 'b';
        secure_bytes[2] = 'c';
        CHECK(ptr[0] == 'a');
        CHECK_FALSE(LockedPagesManager::instance().empty());
    }
    CHECK(LockedPagesManager::instance().empty());
    CHECK(ptr[0] != 'a');
}
}  // namespace zen
