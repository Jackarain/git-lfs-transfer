#ifndef GIT_LFS_TRANSFER_PROTOCOL_H
#define GIT_LFS_TRANSFER_PROTOCOL_H

#include <cstddef>
#include <string>

namespace git_lfs_transfer {

/// Maximum packet size (pkt-line encoding).
constexpr size_t kMaxPacketSize = 65520;

/// @brief Write a pkt-line formatted string to stdout.
///        Prefixes the payload with a 4-byte hex length.
void pkt_write(const std::string& str);

/// @brief Write a pkt-line flush packet ("0000").
void pkt_flush();

/// @brief Write a pkt-line delimiter ("0001").
void pkt_delim();

/// @brief Read one pkt-line from stdin.
/// @param buf  Output buffer for the payload (excluding length prefix).
/// @return     Payload length on success, 0 for flush, 1 for delimiter,
///             -1 on error or EOF.
int pkt_read(std::string& buf);

/// @brief Remove trailing whitespace / newlines from a string in-place.
void trim_line(std::string& s);

/// @brief Return a trimmed copy of the input string.
std::string trim(const std::string& str);

}  // namespace git_lfs_transfer

#endif  // GIT_LFS_TRANSFER_PROTOCOL_H
