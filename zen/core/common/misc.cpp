/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "misc.hpp"

#include <array>
#include <random>
#include <regex>

#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>  // TODO(C++20/23) Replace with std::format when compiler supports

namespace zen {

std::string abridge(std::string_view input, size_t length) {
    if (!length || input.empty()) return std::string(input);
    if (input.length() <= length) {
        return std::string(input);
    }
    return std::string(input.substr(0, length)) + "...";
}

tl::expected<uint64_t, std::string> parse_binary_size(const std::string& input) {
    if (input.empty()) {
        return 0ULL;
    }

    static const std::regex pattern{R"(^(\d*)(\.\d{1,3})?\ *?(B|KB|MB|GB|TB)?$)", std::regex_constants::icase};
    std::smatch matches;
    if (!std::regex_search(input, matches, pattern, std::regex_constants::match_default)) {
        return tl::unexpected{"Not a recognized numeric input"};
    }

    std::string int_part;
    std::string dec_part;
    std::string suf_part;

    uint64_t multiplier{1};  // Default for bytes (B|b)

    int_part = matches[1].str();
    if (!matches[2].str().empty()) {
        dec_part = matches[2].str().substr(1);
    }
    suf_part = matches[3].str();

    if (!suf_part.empty()) {
        if (boost::iequals(suf_part, "KB")) {
            multiplier = kKibi;
        } else if (boost::iequals(suf_part, "MB")) {
            multiplier = kMebi;
        } else if (boost::iequals(suf_part, "GB")) {
            multiplier = kGibi;
        } else if (boost::iequals(suf_part, "TB")) {
            multiplier = kTebi;
        }
    }

    auto number{std::strtoull(int_part.c_str(), nullptr, 10)};
    number *= multiplier;
    if (!dec_part.empty()) {
        // Use literals, so we don't deal with floats and doubles
        auto base{"1" + std::string(dec_part.size(), '0')};
        auto b{std::strtoul(base.c_str(), nullptr, 10)};
        auto d{std::strtoul(dec_part.c_str(), nullptr, 10)};
        number += multiplier * d / b;
    }
    return number;
}

std::string to_string_binary(const size_t input) {
    static const std::array<const char*, 5> suffix{"B", "KB", "MB", "GB", "TB"};
    static const uint32_t items{sizeof(suffix) / sizeof(suffix[0])};
    uint32_t index{0};
    double value{static_cast<double>(input)};
    while (value >= kKibi) {
        value /= kKibi;
        if (++index == (items - 1)) {
            break;
        }
    }
    return boost::str(boost::format("%.2f %s") % value % suffix[index]);
}

std::string get_random_alpha_string(size_t length) {
    static constexpr char kAlphaNum[]{
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz"};

    static constexpr size_t kNumberOfCharacters{sizeof(kAlphaNum) - 1};  // don't count the null terminator

    std::random_device rd;
    std::default_random_engine engine{rd()};

    // yield random numbers up to and including kNumberOfCharacters - 1
    std::uniform_int_distribution<size_t> uniform_dist{0, kNumberOfCharacters - 1};

    std::string ret(length, '\0');
    for (size_t i{0}; i < length; ++i) {
        size_t random_number{uniform_dist(engine)};
        ret[i] = kAlphaNum[random_number];
    }

    return ret;
}
}  // namespace zen
