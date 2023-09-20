/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include "signals.hpp"

#include <csignal>
#include <cstdlib>
#include <iostream>
#include <utility>

#include <absl/strings/str_cat.h>

namespace zenpp::os {

namespace {
    const char* sig_name(int sig_code) {
        switch (sig_code) {
            case SIGSEGV:
                return "SIGSEGV";
#if defined(__linux__) || defined(__APPLE__)
            case SIGBUS:
                return "SIGBUS";
            case SIGSYS:
                return "SIGSYS";
#endif
            case SIGFPE:
                return "SIGFPE";
            case SIGILL:
                return "SIGILL";
#if defined(__linux__) || defined(__APPLE__)
            case SIGTRAP:
                return "SIGTRAP";
#endif
#if defined(SIGBREAK)
            case SIGBREAK:
                return "SIGBREAK";
#endif
#if defined(__linux__) || defined(__APPLE__)
            case SIGQUIT:
                return "SIGQUIT";
#if defined(SIGSTP)
            case SIGSTP:
                return "SIGSTP";
#endif
            case SIGSTOP:
                return "SIGSTOP";
            case SIGKILL:
                return "SIGKILL";
#endif
            case SIGABRT:
                return "SIGABRT";
#if defined(SIGABRT_COMPAT)
            case SIGABRT_COMPAT:
                return "SIGABRT_COMPAT";
#endif
            case SIGINT:
                return "SIGINT";
            case SIGTERM:
                return "SIGTERM";
#if defined(__linux__) || defined(__APPLE__)
            case SIGVTALRM:
                return "SIGVTALRM";
            case SIGXFSZ:
                return "SIGXFZS";
            case SIGXCPU:
                return "SIGXCPU";
            case SIGHUP:
                return "SIGHUP";
            case SIGALRM:
                return "SIGALRM";
            case SIGUSR1:
                return "SIGUSR1";
            case SIGUSR2:
                return "SIGUSR2";
#endif
            default:
                return "Unknown";
        }
    }
}  // namespace

constexpr int kHandleableCodes[] {  // NOLINT(*-avoid-c-arrays)
#if defined(SIGBREAK)
    SIGBREAK,  // Windows keyboard CTRL+Break
#endif
#if defined(__linux__) || defined(__APPLE__)
        SIGQUIT,  // CTRL+\ (like CTRL+C but also generates a coredump)
        SIGTSTP,  // CTRL+Z to interrupt a process
#endif
        SIGINT,  // Keyboard CTRL+C
        SIGTERM  // Termination request (kill/killall default)
};

std::atomic_uint32_t Signals::sig_count_{0};
std::atomic_int Signals::sig_code_{0};
std::atomic_bool Signals::signalled_{false};
std::function<void(int)> Signals::custom_handler_;

void Signals::init(std::function<void(int)> custom_handler) {
    for (const int sig_code : kHandleableCodes) {
        signal(sig_code, &Signals::handle);
    }
    custom_handler_ = std::move(custom_handler);
}

void Signals::handle(int sig_code) {
    if (bool expected{false}; signalled_.compare_exchange_strong(expected, true)) {
        sig_code_ = sig_code;
        std::cerr << absl::StrCat("\nCaught OS signal ", sig_name(sig_code_), ", shutting down ...\n") << std::endl;
    }
    const uint32_t sig_count = ++sig_count_;
    if (sig_count >= 10) {
        std::abort();
    }
    if (sig_count > 1) {
        std::cerr << absl::StrCat("Already shutting down. Interrupt other ", (10 - sig_count), " times to panic.")
                  << std::endl;
    }

    if (custom_handler_) {
        custom_handler_(sig_code);
    }
    signal(sig_code, &Signals::handle);  // Re-enable the hook
}

void Signals::reset() noexcept {
    signalled_ = false;
    sig_count_ = 0;
}

void Signals::throw_if_signalled() {
    if (signalled()) {
        throw signal_exception(sig_code_);
    }
}

signal_exception::signal_exception(int code)
    : sig_code_{code}, message_{absl::StrCat("Caught OS signal ", sig_name(code))} {}
const char* signal_exception::what() const noexcept { return message_.c_str(); }

}  // namespace zenpp::os