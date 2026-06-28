#include "git-lfs-transfer/log.h"

#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unistd.h>

namespace git_lfs_transfer {
namespace {

#ifdef ENABLE_LOG
std::ofstream s_log_file;
std::once_flag s_log_init_flag;
#endif

}  // anonymous namespace

void init_logging() {
#ifdef ENABLE_LOG
    std::call_once(s_log_init_flag, []() {
        std::error_code ec;
        std::filesystem::create_directories("/tmp", ec);
        s_log_file.open("/tmp/git-lfs-transfer.log", std::ios::app);
    });
#endif
}

void log(const std::string& msg) {
#ifdef ENABLE_LOG
    if (!s_log_file.is_open())
        return;
    std::time_t now = std::time(nullptr);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", std::localtime(&now));
    s_log_file << "[" << time_buf << " PID=" << getpid() << "] " << msg << std::endl;
    s_log_file.flush();
#else
    (void)msg;
#endif
}

}  // namespace git_lfs_transfer
