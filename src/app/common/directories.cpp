/*
   Copyright 2022 The Silkworm Authors
   Copyright 2023 Horizen Labs
   Distributed under the MIT software license, see the accompanying
   file COPYING or http://www.opensource.org/licenses/mit-license.php.
*/

#include <app/common/directories.hpp>

#if defined(_MSC_VER)
// clang-format off
#include <windows.h>  // Needed to set target architecture - Keep the order
#include <libloaderapi.h>
// clang-format on
#endif
#include <array>
#include <fstream>

#include <boost/process/environment.hpp>

#include <core/common/misc.hpp>

namespace zen {

std::filesystem::path get_os_temporary_path() { return std::filesystem::temp_directory_path(); }

std::filesystem::path get_process_absolute_full_path() {
#if defined(_MSC_VER)
    std::array<wchar_t, FILENAME_MAX> path{0};
    GetModuleFileNameW(nullptr, path.data(), FILENAME_MAX);
    return {path.data()};
#else
    std::array<char, FILENAME_MAX> path{0};
    ssize_t count = readlink("/proc/self/exe", path.data(), FILENAME_MAX);
    return std::filesystem::path(std::string(path.data(), (count > 0U) ? static_cast<size_t>(count) : 0U));
#endif
}

std::filesystem::path get_unique_temporary_path(std::optional<std::filesystem::path> base_path) {
    if (!base_path) base_path.emplace(get_os_temporary_path());
    if (!base_path->is_absolute()) base_path = std::filesystem::absolute(*base_path);
    if (!std::filesystem::exists(*base_path) || !std::filesystem::is_directory(*base_path)) {
        throw std::invalid_argument("Path " + base_path->string() + " does not exist or is not a directory");
    }

    // Build random paths appending random strings of fixed length to base path
    // If 1000 attempts fail we throw
    for (int i{0}; i < 1000; ++i) {
        auto temp_generated_absolute_path{*base_path / get_random_alpha_string(10)};
        if (!std::filesystem::exists(temp_generated_absolute_path)) {
            return temp_generated_absolute_path;
        }
    }

    // We were unable to find a valid unique non-existent path
    throw std::filesystem::filesystem_error("Unable to find a valid unique non-existent name in " + base_path->string(),
                                            *base_path, std::make_error_code(std::errc::file_exists));
}
std::filesystem::path get_os_default_storage_path() {
    std::string base_path_str{};
    auto environment{boost::this_process::environment()};

    if (auto env_value{environment["XDG_DATA_HOME"]}; !env_value.empty()) {
        // Got storage path from docker
        base_path_str.assign(env_value.to_string());
    } else {
#ifdef _WIN32
        std::string env_name{"LOCALAPPDATA"};
#else
        std::string env_name{"HOME"};
#endif
        env_value = environment[env_name];
        if (env_value.empty()) {
            // We don't actually know where to store data
            // fallback to current directory
            base_path_str.assign(std::filesystem::current_path().string());
        } else {
            base_path_str.assign(env_value.to_string());
        }
    }

    std::filesystem::path base_dir_path{base_path_str};
    std::string project_name{get_buildinfo()->project_name};
    project_name.insert(0, 1, '.');
#ifdef _WIN32
    base_dir_path /= project_name;
#elif __APPLE__
    base_dir_path /= "Library";
    base_dir_path /= "Application Support";
    base_dir_path /= project_name;
#else
    base_dir_path /= ".local";
    base_dir_path /= "share";
    base_dir_path /= project_name;
#endif

    return base_dir_path;
}

Directory::Directory(const std::filesystem::path& path) : path_(path) {
    if (path_.empty()) path_ = std::filesystem::current_path();
    if (!path.is_absolute()) path_ = std::filesystem::absolute(path_);
    if (!path_.has_filename()) {
        throw std::invalid_argument("Invalid path " + path_.string());
    }
    if (std::filesystem::exists(path_) && !std::filesystem::is_directory(path_)) {
        throw std::invalid_argument("Invalid path " + path_.string() + " not a directory");
    }
    create();
}

bool Directory::is_pristine() const { return std::filesystem::is_empty(path_); }

const std::filesystem::path& Directory::path() const noexcept { return path_; }

void Directory::clear(bool recurse) {
    for (const auto& entry : std::filesystem::directory_iterator(path_)) {
        if (std::filesystem::is_directory(entry) && !recurse) continue;
        std::filesystem::remove_all(entry.path());
    }
}

size_t Directory::size(bool recurse) const {
    size_t ret{0};
    for (auto it{std::filesystem::recursive_directory_iterator(path_)};
         it != std::filesystem::recursive_directory_iterator{}; ++it) {
        if (std::filesystem::is_directory(it->path())) {
            if (!recurse) it.disable_recursion_pending();
        } else if (std::filesystem::is_regular_file(it->path())) {
            ret += std::filesystem::file_size(it->path());
        }
    }
    return ret;
}
bool Directory::exists() const { return std::filesystem::exists(path_); }

void Directory::create() const {
    if (exists()) return;
    if (!std::filesystem::create_directories(path_)) {
        throw std::filesystem::filesystem_error("Unable to create directory " + path_.string(), path_,
                                                std::make_error_code(std::errc::io_error));
    }
}
Directory Directory::operator[](const std::filesystem::path& path) const {
    if (path.empty() || path.is_absolute() || !path.has_filename()) throw std::invalid_argument("Invalid Path");
    const auto target{path_ / path};
    if (!std::filesystem::exists(target) && !std::filesystem::create_directories(target)) {
        throw std::filesystem::filesystem_error("Unable to create directory " + target.string(), target,
                                                std::make_error_code(std::errc::io_error));
    }
    return Directory(target);
}
bool Directory::is_writable() const noexcept {
    std::filesystem::path test_file_name{get_random_alpha_string(8)};
    while (std::filesystem::exists(path_ / test_file_name)) {
        test_file_name = get_random_alpha_string(8);
    }
    std::filesystem::path test_file_path{path_ / test_file_name};
    std::ofstream test_file{test_file_path.string()};
    if (!test_file.is_open()) return false;
    test_file << "test";
    test_file.close();
    std::filesystem::remove(test_file_path);
    return true;
}
}  // namespace zen
