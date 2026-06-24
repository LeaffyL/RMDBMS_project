/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include <algorithm>
#include <memory>
#include <vector>

#include "execution_defs.h"
#include "execution_manager.h"
#include "executor_abstract.h"
#include "executor_utils.h"
#include "index/ix.h"
#include "system/sm.h"

class SortExecutor : public AbstractExecutor {
   private:
    std::unique_ptr<AbstractExecutor> prev_;
    std::vector<SortKey> sort_keys_;
    std::vector<ColMeta> sort_cols_;
    std::vector<std::unique_ptr<RmRecord>> records_;
    size_t cursor_ = 0;
    int limit_;

   public:
    SortExecutor(std::unique_ptr<AbstractExecutor> prev, std::vector<SortKey> sort_keys, int limit) {
        prev_ = std::move(prev);
        sort_keys_ = std::move(sort_keys);
        limit_ = limit;
        sort_cols_.reserve(sort_keys_.size());
        for (const auto &key : sort_keys_) {
            sort_cols_.push_back(prev_->get_col_offset(key.col));
        }
    }

    void beginTuple() override {
        records_.clear();
        cursor_ = 0;

        prev_->beginTuple();
        for (; !prev_->is_end(); prev_->nextTuple()) {
            records_.push_back(prev_->Next());
        }

        if (!sort_keys_.empty()) {
            std::stable_sort(records_.begin(), records_.end(),
                             [&](const std::unique_ptr<RmRecord> &lhs, const std::unique_ptr<RmRecord> &rhs) {
                                 for (size_t i = 0; i < sort_keys_.size(); ++i) {
                                     const auto &col = sort_cols_[i];
                                     int cmp = compare_raw_data(lhs->data + col.offset, col.type, col.len,
                                                                rhs->data + col.offset, col.type, col.len);
                                     if (cmp == 0) {
                                         continue;
                                     }
                                     return sort_keys_[i].is_desc ? (cmp > 0) : (cmp < 0);
                                 }
                                 return false;
                             });
        }

        if (limit_ >= 0 && static_cast<size_t>(limit_) < records_.size()) {
            records_.resize(limit_);
        }
    }

    void nextTuple() override {
        if (cursor_ < records_.size()) {
            ++cursor_;
        }
    }

    std::unique_ptr<RmRecord> Next() override {
        if (is_end()) {
            return nullptr;
        }
        return std::make_unique<RmRecord>(*records_[cursor_]);
    }

    bool is_end() const override { return cursor_ >= records_.size(); }

    size_t tupleLen() const override { return prev_->tupleLen(); }

    const std::vector<ColMeta> &cols() const override { return prev_->cols(); }

    std::string getType() override { return "SortExecutor"; }

    ColMeta get_col_offset(const TabCol &target) override { return prev_->get_col_offset(target); }

    Rid &rid() override { return _abstract_rid; }
};
