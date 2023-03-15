/*
   Copyright 2009-2010 Satoshi Nakamoto
   Copyright 2009-2013 The Bitcoin Core developers
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#pragma once
#include <zen/core/common/base.hpp>
#include <zen/core/types/hash.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

namespace zen {

struct BlockHeader {
    friend class boost::serialization::access;
    int32_t version;
    H160 parent_hash;
    H160 merkle_root;
    H160 sidechains_commitment_root;
    uint32_t time;
    uint32_t bits;

    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) {
        ar & version;
        ar & time;
        ar & bits;
    }
};

}  // namespace zen
