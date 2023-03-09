/*
   Copyright 2009-2010 Satoshi Nakamoto
   Copyright 2009-2013 The Bitcoin Core developers
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <boost/format.hpp>

#include <zen/core/types/amounts.hpp>

namespace zen {

bool Amount::valid_money() const noexcept { return amount_ >= 0 && amount_ <= kMaxMoney; }

Amount::operator bool() const noexcept { return amount_ != 0; }

bool Amount::operator==(int64_t value) const noexcept { return amount_ == value; }

bool Amount::operator>(int64_t value) const noexcept { return amount_ > value; }

bool Amount::operator<(int64_t value) const noexcept { return amount_ < value; }

int64_t Amount::operator*() const noexcept { return amount_; }

Amount& Amount::operator=(int64_t value) noexcept {
    amount_ = value;
    return *this;
}

Amount& Amount::operator=(const Amount& rhs) noexcept {
    amount_ = *rhs;
    return *this;
}

std::string Amount::to_string() const {
    const auto div{std::div(amount_, kCoin)};
    return boost::str(boost::format("%i.%08i %s") % div.quot % div.rem % kCurrency);
}

void Amount::operator+=(int64_t value) noexcept { amount_ += value; }

void Amount::operator*=(int64_t value) noexcept { amount_ *= value; }

void Amount::operator-=(int64_t value) noexcept { amount_ -= value; }

void Amount::operator++() noexcept { ++amount_; }

void Amount::operator--() noexcept { --amount_; }

FeeRate::FeeRate(const Amount paid, size_t size) {
    satoshis_per_K_ = size ? static_cast<int64_t>(*paid * 1'000 / size) : 0;
}

std::string FeeRate::to_string() const { return satoshis_per_K_.to_string() + "/K"; }
Amount FeeRate::fee(size_t bytes_size) const {
    Amount ret(*satoshis_per_K_ * static_cast<int64_t>(bytes_size) / 1'000);
    if (!ret && satoshis_per_K_) ret = satoshis_per_K_;
    return ret;
}
}  // namespace zen
