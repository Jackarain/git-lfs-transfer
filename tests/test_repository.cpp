#include "git-lfs-transfer/repository.h"

#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using namespace git_lfs_transfer;

struct TempRepo {
    fs::path path;
    bool     bare;

    TempRepo(bool is_bare) : bare(is_bare) {
        path = fs::temp_directory_path() / "test_repo_XXXXXX";
        // mkdtemp
        std::string tmpl = path.string();
        std::uint64_t id = static_cast<std::uint64_t>(std::time(nullptr));
        tmpl += std::to_string(id);
        path = tmpl;
        fs::create_directories(path);

        fs::path config_dir = bare ? path : (path / ".git");
        fs::create_directories(config_dir);

        std::ofstream cfg(config_dir / "config");
        cfg << "[core]\n"
            << "\trepositoryformatversion = 0\n"
            << "\tbare = " << (bare ? "true" : "false") << "\n"
            << "\tfilemode = true\n";
        cfg.close();
    }

    ~TempRepo() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
};

// ─────────────────────────────────────────────────────────
// is_bare_repo
// ─────────────────────────────────────────────────────────
TEST(RepositoryTest, DetectsBareRepo) {
    TempRepo repo(true);
    EXPECT_TRUE(is_bare_repo(repo.path));
}

TEST(RepositoryTest, DetectsNonBareRepo) {
    TempRepo repo(false);
    EXPECT_FALSE(is_bare_repo(repo.path));
}

TEST(RepositoryTest, NonexistentPath) {
    EXPECT_FALSE(is_bare_repo("/nonexistent/path/12345"));
}

// ─────────────────────────────────────────────────────────
// get_objects_dir
// ─────────────────────────────────────────────────────────
TEST(RepositoryTest, ObjectsDirBare) {
    TempRepo repo(true);
    std::string dir = get_objects_dir(repo.path.string());
    EXPECT_EQ(dir, repo.path.string() + "/lfs/objects");
}

TEST(RepositoryTest, ObjectsDirNonBare) {
    TempRepo repo(false);
    std::string dir = get_objects_dir(repo.path.string());
    EXPECT_EQ(dir, repo.path.string() + "/.git/lfs/objects");
}
