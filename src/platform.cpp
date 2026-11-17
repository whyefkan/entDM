// SPDX-License-Identifier: Apache-2.0
// Copyright 2026 whyefkan

#include "platform.h"

#include <algorithm>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <mutex>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <shellapi.h>
#include <psapi.h>
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace ent {

namespace fs = std::filesystem;

namespace {

std::string env_or(const char* name, const std::string& fallback) {
    const char* value = std::getenv(name);
    return value != nullptr && value[0] != '\0' ? std::string(value) : fallback;
}

void ensure_directory(const fs::path& path) {
    std::error_code ec;
    fs::create_directories(path, ec);
}

std::string quote_argument(const std::string& value) {
    std::string out = "\"";
    for (char c : value) {
        if (c == '"') {
            out += "\\\"";
        } else {
            out += c;
        }
    }
    out += "\"";
    return out;
}

}  // namespace

void platform_init() {
#ifdef _WIN32
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
#endif
}

void platform_shutdown() {
#ifdef _WIN32
    WSACleanup();
#endif
}

std::string home_directory() {
#ifdef _WIN32
    return env_or("USERPROFILE", env_or("HOME", "."));
#else
    return env_or("HOME", ".");
#endif
}

std::string config_directory() {
#ifdef _WIN32
    const fs::path dir = fs::path(env_or("APPDATA", home_directory())) / "entDM";
#else
    const fs::path dir = fs::path(home_directory()) / ".config" / "entdm";
#endif
    ensure_directory(dir);
    return dir.string();
}

std::string data_directory() {
#ifdef _WIN32
    const fs::path dir = fs::path(env_or("LOCALAPPDATA", home_directory())) / "entDM";
#else
    const fs::path dir = fs::path(home_directory()) / ".local" / "share" / "entdm";
#endif
    ensure_directory(dir);
    return dir.string();
}

std::string downloads_directory() {
    const fs::path dir = fs::path(home_directory()) / "Downloads";
    ensure_directory(dir);
    return dir.string();
}

FontPaths system_fonts() {
    FontPaths fonts;
#ifdef _WIN32
    const fs::path font_dir = fs::path(env_or("WINDIR", "C:\\Windows")) / "Fonts";
    const fs::path regular = font_dir / "segoeui.ttf";
    const fs::path bold = font_dir / "segoeuib.ttf";
    const fs::path arial = font_dir / "arial.ttf";
    const fs::path arial_bold = font_dir / "arialbd.ttf";
    std::error_code ec;
    if (fs::exists(regular, ec)) {
        fonts.regular = regular.string();
    } else if (fs::exists(arial, ec)) {
        fonts.regular = arial.string();
    }
    if (fs::exists(bold, ec)) {
        fonts.bold = bold.string();
    } else if (fs::exists(arial_bold, ec)) {
        fonts.bold = arial_bold.string();
    }
    for (const char* name : {"msyh.ttc", "malgun.ttf", "YuGothM.ttc", "meiryo.ttc"}) {
        const fs::path path = font_dir / name;
        if (fs::exists(path, ec)) {
            fonts.cjk.push_back(path.string());
        }
    }
#else
    std::error_code ec;
    const char* regular_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
    };
    const char* bold_paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf",
    };
    const char* cjk_paths[] = {
        "/usr/share/fonts/truetype/droid/DroidSansFallbackFull.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
    };
    for (const char* path : regular_paths) {
        if (fs::exists(path, ec)) {
            fonts.regular = path;
            break;
        }
    }
    for (const char* path : bold_paths) {
        if (fs::exists(path, ec)) {
            fonts.bold = path;
            break;
        }
    }
    for (const char* path : cjk_paths) {
        if (fs::exists(path, ec)) {
            fonts.cjk.push_back(path);
        }
    }
#endif
    return fonts;
}

int open_output_file(const std::string& path, bool truncate, std::string& error) {
#ifdef _WIN32
    int flags = _O_WRONLY | _O_CREAT | _O_BINARY;
    if (truncate) {
        flags |= _O_TRUNC;
    }
    const int fd = _open(path.c_str(), flags, _S_IREAD | _S_IWRITE);
    if (fd < 0) {
        error = std::string("cannot open output file: ") + std::strerror(errno);
    }
    return fd;
#else
    int flags = O_WRONLY | O_CREAT;
    if (truncate) {
        flags |= O_TRUNC;
    }
    const int fd = ::open(path.c_str(), flags, 0644);
    if (fd < 0) {
        error = std::string("cannot open output file: ") + std::strerror(errno);
    }
    return fd;
#endif
}

void close_file(int fd) {
    if (fd < 0) {
        return;
    }
#ifdef _WIN32
    _close(fd);
#else
    ::close(fd);
#endif
}

bool write_at_offset(int fd, const char* data, std::size_t size,
                     unsigned long long offset, std::string& error) {
#ifdef _WIN32
    static std::mutex write_mutex;
    std::lock_guard<std::mutex> lock(write_mutex);
    if (_lseeki64(fd, static_cast<__int64>(offset), SEEK_SET) < 0) {
        error = std::string("seek failed: ") + std::strerror(errno);
        return false;
    }
    std::size_t total = 0;
    while (total < size) {
        const std::size_t chunk = std::min<std::size_t>(size - total, 1u << 30);
        const int written = _write(fd, data + total, static_cast<unsigned int>(chunk));
        if (written <= 0) {
            error = std::string("write failed: ") + std::strerror(errno);
            return false;
        }
        total += static_cast<std::size_t>(written);
    }
    return true;
#else
    std::size_t total = 0;
    while (total < size) {
        const ssize_t written = ::pwrite(fd, data + total, size - total,
                                         static_cast<off_t>(offset + total));
        if (written < 0) {
            error = std::string("write failed: ") + std::strerror(errno);
            return false;
        }
        if (written == 0) {
            error = "short write";
            return false;
        }
        total += static_cast<std::size_t>(written);
    }
    return true;
#endif
}

void make_executable(const std::string& path) {
#ifndef _WIN32
    ::chmod(path.c_str(), 0755);
#else
    (void)path;
#endif
}

void open_path(const std::string& path) {
#ifdef _WIN32
    ShellExecuteA(nullptr, "open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#else
    const pid_t pid = ::fork();
    if (pid == 0) {
        ::execlp("xdg-open", "xdg-open", path.c_str(), static_cast<char*>(nullptr));
        ::_exit(127);
    }
#endif
}

void open_url(const std::string& url) {
    open_path(url);
}

void launch_program(const std::string& program, const std::string& argument) {
#ifdef _WIN32
    std::string command = quote_argument(program) + " " + quote_argument(argument);
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    CreateProcessA(program.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr,
                   nullptr, &startup, &process);
    if (process.hProcess != nullptr) {
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    }
#else
    const pid_t pid = ::fork();
    if (pid == 0) {
        ::execlp(program.c_str(), program.c_str(), argument.c_str(),
                 static_cast<char*>(nullptr));
        ::_exit(127);
    }
#endif
}

void launch_executable(const std::string& executable) {
    if (executable.empty()) {
        return;
    }
#ifdef _WIN32
    const std::string directory = fs::path(executable).parent_path().string();
    std::string command = quote_argument(executable);
    STARTUPINFOA startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    CreateProcessA(executable.c_str(), command.data(), nullptr, nullptr, FALSE, 0, nullptr,
                   directory.empty() ? nullptr : directory.c_str(), &startup, &process);
    if (process.hProcess != nullptr) {
        CloseHandle(process.hThread);
        CloseHandle(process.hProcess);
    }
#else
    const pid_t pid = ::fork();
    if (pid == 0) {
        const fs::path parent = fs::path(executable).parent_path();
        if (!parent.empty()) {
            ::chdir(parent.c_str());
        }
        ::execl(executable.c_str(), executable.c_str(), static_cast<char*>(nullptr));
        ::_exit(127);
    }
#endif
}

std::string find_chrome() {
#ifdef _WIN32
    const fs::path program_files = env_or("ProgramFiles", "C:\\Program Files");
    const fs::path program_files_x86 = env_or("ProgramFiles(x86)", "C:\\Program Files (x86)");
    const fs::path local_app_data = env_or("LOCALAPPDATA", home_directory());
    const fs::path candidates[] = {
        program_files / "Google/Chrome/Application/chrome.exe",
        program_files_x86 / "Google/Chrome/Application/chrome.exe",
        local_app_data / "Google/Chrome/Application/chrome.exe",
        program_files_x86 / "Microsoft/Edge/Application/msedge.exe",
        program_files / "Microsoft/Edge/Application/msedge.exe",
    };
    std::error_code ec;
    for (const fs::path& path : candidates) {
        if (fs::exists(path, ec)) {
            return path.string();
        }
    }
    return "";
#else
    const char* candidates[] = {
        "/usr/bin/google-chrome",
        "/usr/bin/google-chrome-stable",
        "/usr/bin/chromium",
        "/usr/bin/chromium-browser",
        "/usr/bin/brave-browser",
        "/snap/bin/chromium",
    };
    for (const char* candidate : candidates) {
        if (::access(candidate, X_OK) == 0) {
            return candidate;
        }
    }
    return "";
#endif
}

double process_rss_mb() {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS counters{};
    if (GetProcessMemoryInfo(GetCurrentProcess(), &counters, sizeof(counters)) != 0) {
        return static_cast<double>(counters.WorkingSetSize) / (1024.0 * 1024.0);
    }
    return 0.0;
#else
    std::ifstream file("/proc/self/statm");
    long total_pages = 0;
    long resident_pages = 0;
    if (file >> total_pages >> resident_pages) {
        return static_cast<double>(resident_pages) * 4096.0 / (1024.0 * 1024.0);
    }
    return 0.0;
#endif
}

std::string executable_directory() {
#ifdef _WIN32
    char buffer[MAX_PATH] = {0};
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length > 0) {
        return fs::path(std::string(buffer, length)).parent_path().string();
    }
#else
    char buffer[4096] = {0};
    const ssize_t length = ::readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length > 0) {
        return fs::path(std::string(buffer, static_cast<std::size_t>(length)))
            .parent_path()
            .string();
    }
#endif
    return fs::current_path().string();
}

std::string extension_directory(const std::string& fallback) {
    const fs::path base = executable_directory();
    const fs::path candidates[] = {
        base / "extension",
        base / ".." / "share" / "entdm" / "extension",
        base / ".." / "extension",
    };
    std::error_code ec;
    for (const fs::path& candidate : candidates) {
        if (fs::is_directory(candidate, ec)) {
            return fs::weakly_canonical(candidate, ec).string();
        }
    }
    return fallback;
}

socket_handle socket_listen_loopback(int port, std::string& error) {
    const socket_handle listener = static_cast<socket_handle>(
        ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listener == invalid_socket) {
        error = "socket creation failed";
        return invalid_socket;
    }

    int reuse = 1;
    ::setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(static_cast<unsigned short>(port));

    if (::bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        socket_close(listener);
        error = "bind failed";
        return invalid_socket;
    }
    if (::listen(listener, 8) != 0) {
        socket_close(listener);
        error = "listen failed";
        return invalid_socket;
    }
    return listener;
}

socket_handle socket_accept(socket_handle listener) {
    return static_cast<socket_handle>(::accept(listener, nullptr, nullptr));
}

long socket_read(socket_handle handle, char* buffer, int length) {
    return static_cast<long>(::recv(handle, buffer, length, 0));
}

long socket_write(socket_handle handle, const char* buffer, int length) {
    return static_cast<long>(::send(handle, buffer, length, 0));
}

void socket_close(socket_handle handle) {
    if (handle == invalid_socket) {
        return;
    }
#ifdef _WIN32
    ::closesocket(handle);
#else
    ::close(handle);
#endif
}

}  // namespace ent
