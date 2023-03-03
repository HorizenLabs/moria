/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "stage.hpp"

#include <zen/node/database/stages.hpp>

namespace zen::stages {

Stage::Stage(SyncContext* sync_context, const char* stage_name, NodeSettings* node_settings)
    : sync_context_{sync_context}, stage_name_{stage_name}, node_settings_{node_settings} {}

BlockNum Stage::get_progress(db::RWTxn& txn) { return db::stages::read_stage_progress(*txn, stage_name_); }

BlockNum Stage::get_prune_progress(db::RWTxn& txn) { return db::stages::read_stage_prune_progress(*txn, stage_name_); }

void Stage::update_progress(db::RWTxn& txn, BlockNum progress) {
    db::stages::write_stage_progress(*txn, stage_name_, progress);
}
}  // namespace zen::stages
