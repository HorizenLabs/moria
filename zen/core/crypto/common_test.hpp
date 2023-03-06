/*
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once

#include <random>

#include <catch2/catch.hpp>

#include <zen/core/encoding/hex.hpp>

namespace zen::crypto {

template <typename Hasher>
Bytes RunHasher(Hasher& hasher, std::string_view input) {
    static std::random_device rd;
    static std::mt19937_64 rng(rd());

    // Consume input in pieces to ensure partial updates don't break anything
    while (!input.empty()) {
        std::uniform_int_distribution<size_t> uni(1ULL, (input.size() / 2) + 1);
        const size_t chunk_size{uni(rng)};
        const auto input_chunk{input.substr(0, chunk_size)};
        hasher.update(input_chunk);
        input.remove_prefix(chunk_size);
    }

    const auto hash{hasher.finalize()};
    return hash;
}
}  // namespace zen::crypto
