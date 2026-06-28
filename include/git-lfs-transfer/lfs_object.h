#ifndef GIT_LFS_TRANSFER_LFS_OBJECT_H
#define GIT_LFS_TRANSFER_LFS_OBJECT_H

#include <cstddef>
#include <string>
#include <vector>

namespace git_lfs_transfer {

/// @brief Represents a single LFS object (OID + size).
struct LfsObject {
    std::string oid;
    std::size_t size = 0;
};

/// @brief Parse a line of the form "<oid> <size>" into an LfsObject.
/// @return true on success, false on parse failure.
bool parse_object_line(const std::string& s, LfsObject& obj);

/// @brief Build the on-disk path for an LFS object.
///        Format: <objects_dir>/<first2>/<next2>/<oid>
std::string lfs_object_path(const std::string& objects_dir, const std::string& oid);

/// @brief Check whether an object with the given OID exists on disk.
bool object_exists(const std::string& objects_dir, const std::string& oid);

}  // namespace git_lfs_transfer

#endif  // GIT_LFS_TRANSFER_LFS_OBJECT_H
