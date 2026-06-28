#include "git-lfs-transfer/protocol.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>

namespace git_lfs_transfer {

void pkt_write(const std::string& str) {
    if (str.empty())
        return;
    auto total_len = static_cast<unsigned>(str.length() + 4);
    if (total_len > kMaxPacketSize + 4)
        total_len = kMaxPacketSize + 4;
    char header[5];
    std::snprintf(header, sizeof(header), "%04x", total_len);
    std::cout.write(header, 4);
    std::cout.write(str.data(), static_cast<std::streamsize>(str.length()));
    std::cout.flush();
}

void pkt_flush() {
    std::cout.write("0000", 4);
    std::cout.flush();
}

void pkt_delim() {
    std::cout.write("0001", 4);
    std::cout.flush();
}

int pkt_read(std::string& buf) {
    char header[5] = {0};
    if (std::fread(header, 1, 4, stdin) != 4)
        return -1;
    int len = static_cast<int>(std::strtol(header, nullptr, 16));
    if (len == 0)
        return 0;
    if (len == 1) {
        buf.clear();
        return 1;
    }
    if (len < 4 || len > static_cast<int>(kMaxPacketSize + 4))
        return -1;
    std::size_t data_len = static_cast<std::size_t>(len) - 4;
    buf.resize(data_len);
    if (std::fread(buf.data(), 1, data_len, stdin) != data_len)
        return -1;
    return static_cast<int>(data_len);
}

void trim_line(std::string& s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
        s.pop_back();
    std::size_t start = s.find_first_not_of(" \r\n");
    if (start != std::string::npos && start > 0)
        s = s.substr(start);
}

std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(),
                                  [](unsigned char ch) { return std::isspace(ch); });
    auto end = std::find_if_not(str.rbegin(), str.rend(),
                                [](unsigned char ch) { return std::isspace(ch); }).base();
    return (start < end) ? std::string(start, end) : "";
}

}  // namespace git_lfs_transfer
