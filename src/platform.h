// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 whyefkan

#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace ent {

#ifdef _WIN32
using socket_handle = unsigned long long;
inline constexpr socket_handle invalid_socket = static_cast<socket_handle>(~0ull);
#else
using socket_handle = int;
inline constexpr socket_handle invalid_socket = -1;
#endif

struct FontPaths {
    std::string regular;
    std::string bold;
    std::vector<std::string> cjk;
};

void platform_init();
void platform_shutdown();

std::string home_directory();
std::string config_directory();
std::string data_directory();
std::string downloads_directory();

FontPaths system_fonts();

int open_output_file(const std::string& path, bool truncate, std::string& error);
void close_file(int fd);
bool write_at_offset(int fd, const char* data, std::size_t size,
                     unsigned long long offset, std::string& error);
void make_executable(const std::string& path);

void open_path(const std::string& path);
void open_url(const std::string& url);
void launch_program(const std::string& program, const std::string& argument);
void launch_executable(const std::string& executable);

std::string find_chrome();

double process_rss_mb();

std::string executable_directory();
std::string extension_directory(const std::string& fallback);

socket_handle socket_listen_loopback(int port, std::string& error);
socket_handle socket_accept(socket_handle listener);
long socket_read(socket_handle handle, char* buffer, int length);
long socket_write(socket_handle handle, const char* buffer, int length);
void socket_close(socket_handle handle);

}  // namespace ent
