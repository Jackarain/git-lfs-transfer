#ifndef GIT_LFS_TRANSFER_REPOSITORY_H
#define GIT_LFS_TRANSFER_REPOSITORY_H

#include <filesystem>
#include <string>

namespace git_lfs_transfer {

/// @brief Determine whether the given path points to a bare Git repository
///        by inspecting its config file.
bool is_bare_repo(const std::filesystem::path& repo_path);

/// @brief Compute the LFS objects directory for a repository.
/// @param repo  Filesystem path to the repository root.
/// @return      The path to the LFS objects directory.
std::string get_objects_dir(const std::string& repo);

}  // namespace git_lfs_transfer

#endif  // GIT_LFS_TRANSFER_REPOSITORY_H
