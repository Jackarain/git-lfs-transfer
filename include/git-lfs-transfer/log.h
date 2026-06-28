#ifndef GIT_LFS_TRANSFER_LOG_H
#define GIT_LFS_TRANSFER_LOG_H

#include <string>

namespace git_lfs_transfer {

/// @brief Initialize logging to a file (e.g., /tmp/git-lfs-transfer.log).
///        Safe to call multiple times; only the first call opens the file.
void init_logging();

/// @brief Write a timestamped log message.
///        If ENABLE_LOG is not defined, this is a no-op.
void log(const std::string& msg);

}  // namespace git_lfs_transfer

#endif  // GIT_LFS_TRANSFER_LOG_H
