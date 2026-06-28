#include "git-lfs-transfer/lfs_object.h"

#include <filesystem>
#include <system_error>

namespace git_lfs_transfer {

bool parse_object_line(const std::string& s, LfsObject& obj) {
    std::size_t space = s.find(' ');
    if (space == std::string::npos || space < 40)
        return false;
    obj.oid = s.substr(0, space);
    try {
        obj.size = std::stoull(s.substr(space + 1));
    } catch (...) {
        return false;
    }
    return true;
}

std::string lfs_object_path(const std::string& objects_dir, const std::string& oid) {
    if (oid.length() < 4)
        return {};
    std::string subdir = oid.substr(0, 2) + "/" + oid.substr(2, 2);
    return objects_dir + "/" + subdir + "/" + oid;
}

bool object_exists(const std::string& objects_dir, const std::string& oid) {
    std::error_code ec;
    return std::filesystem::exists(lfs_object_path(objects_dir, oid), ec);
}

}  // namespace git_lfs_transfer
