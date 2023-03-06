/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "common_test.hpp"

namespace zen::crypto {
std::string LongTestString() {
    static const size_t kStringLen{200'000};
    std::string ret;
    ret.reserve(kStringLen);
    for (int i = 0; i < kStringLen; i++) {
        ret += static_cast<unsigned char>(i);
        ret += static_cast<unsigned char>(i >> 4);
        ret += static_cast<unsigned char>(i >> 8);
        ret += static_cast<unsigned char>(i >> 12);
        ret += static_cast<unsigned char>(i >> 16);
    }
    return ret;
}
}  // namespace zen::crypto
