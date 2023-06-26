/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "stopwatch.hpp"

namespace zen {

StopWatch::TimePoint StopWatch::start(bool with_reset) noexcept {
    using namespace std::chrono_literals;
    if (with_reset) {
        reset();
    }

    if (started_) {
        return start_time_;
    }

    started_ = true;
    if (start_time_ == TimePoint()) {
        start_time_ = TimeClock::now();
    }
    if (!laps_.empty()) {
        const auto& [t, d] = laps_.back();
        laps_.emplace_back(start_time_, std::chrono::duration_cast<Duration>(start_time_ - t));
    } else {
        laps_.emplace_back(start_time_, std::chrono::duration_cast<Duration>(0s));
    }
    return start_time_;
}

std::pair<StopWatch::TimePoint, StopWatch::Duration> StopWatch::lap() noexcept {
    if (!started_ || laps_.empty()) {
        return {};
    }
    const auto lap_time{TimeClock::now()};
    const auto& [t, d] = laps_.back();
    laps_.emplace_back(lap_time, std::chrono::duration_cast<Duration>(lap_time - t));
    return laps_.back();
}

StopWatch::Duration StopWatch::since_start(const TimePoint& origin) noexcept {
    if (start_time_ == TimePoint()) {
        return {};
    }
    return Duration(origin - start_time_);
}

StopWatch::Duration StopWatch::since_start() noexcept { return since_start(TimeClock::now()); }

std::pair<StopWatch::TimePoint, StopWatch::Duration> StopWatch::stop() noexcept {
    if (!started_) {
        return {};
    }
    auto ret{lap()};
    started_ = false;
    return ret;
}

void StopWatch::reset() noexcept {
    (void)stop();
    start_time_ = TimePoint();
    if (!laps_.empty()) {
        std::vector<std::pair<TimePoint, Duration>>().swap(laps_);
    }
}

std::string StopWatch::format(Duration duration) noexcept {
    using namespace std::chrono_literals;
    using days = std::chrono::duration<int, std::ratio<86400>>;

    std::ostringstream os;
    char fill = os.fill('0');

    if (duration >= 60s) {
        bool need_space{false};
        if (auto d = std::chrono::duration_cast<days>(duration); d.count()) {
            os << d.count() << "d";
            duration -= d;
            need_space = true;
        }
        if (auto h = std::chrono::duration_cast<std::chrono::hours>(duration); h.count()) {
            os << (need_space ? " " : "") << h.count() << "h";
            duration -= h;
            need_space = true;
        }
        if (auto m = std::chrono::duration_cast<std::chrono::minutes>(duration); m.count()) {
            os << (need_space ? " " : "") << m.count() << "m";
            duration -= m;
            need_space = true;
        }
        if (auto s = std::chrono::duration_cast<std::chrono::seconds>(duration); s.count()) {
            os << (need_space ? " " : "") << s.count() << "s";
        }
    } else {
        if (duration >= 1s) {
            auto s = std::chrono::duration_cast<std::chrono::seconds>(duration);
            duration -= s;
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            os << s.count();
            if (ms.count()) {
                os << "." << std::setw(3) << ms.count();
            }
            os << "s";
        } else if (duration >= 1ms) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
            duration -= ms;
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            os << ms.count();
            if (us.count()) {
                os << "." << std::setw(3) << us.count();
            }
            os << "ms";
        } else if (duration >= 1us) {
            auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
            os << us.count() << "us";
        }
    }

    os.fill(fill);
    return os.str();
}

}  // namespace zen
