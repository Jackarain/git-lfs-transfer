#include "git-lfs-transfer/repository.h"
#include "git-lfs-transfer/protocol.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace git_lfs_transfer {

bool is_bare_repo(const std::filesystem::path& repo_path) {
    std::error_code ec;
    std::filesystem::path config_path;

    if (std::filesystem::exists(repo_path / "config", ec)) {
        config_path = repo_path / "config";
    } else if (std::filesystem::exists(repo_path / ".git" / "config", ec)) {
        config_path = repo_path / ".git" / "config";
    } else {
        return false;
    }

    std::ifstream file(config_path);
    if (!file.is_open())
        return false;

    std::string line;
    bool in_core_section = false;

    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;

        if (line[0] == '[' && line.back() == ']') {
            std::string section = line.substr(1, line.size() - 2);
            section = trim(section);
            in_core_section = (section == "core");
            continue;
        }

        if (in_core_section) {
            auto delim_pos = line.find('=');
            if (delim_pos != std::string::npos) {
                std::string key = trim(line.substr(0, delim_pos));
                std::string value = trim(line.substr(delim_pos + 1));
                if (key == "bare") {
                    return (value == "true" || value == "yes" || value == "on" || value == "1");
                }
            }
        }
    }
    return false;
}

std::string get_objects_dir(const std::string& repo) {
    if (is_bare_repo(repo)) {
        return repo + "/lfs/objects";
    } else {
        return repo + "/.git/lfs/objects";
    }
}

}  // namespace git_lfs_transfer
