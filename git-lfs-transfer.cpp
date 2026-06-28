#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace fs = std::filesystem;

constexpr size_t MAX_PKT = 65520;

#ifdef ENABLE_LOG
std::ofstream log_file;
#endif

void log(const std::string& msg) {
#ifdef ENABLE_LOG
    if (!log_file.is_open())
        return;
    std::time_t now = std::time(nullptr);
    char time_buf[20];
    std::strftime(time_buf, sizeof(time_buf), "%H:%M:%S", std::localtime(&now));
    log_file << "[" << time_buf << " PID=" << getpid() << "] " << msg << std::endl;
    log_file.flush();
#else
    (void)msg;
#endif
}

void pkt_write(const std::string& str) {
    if (str.empty())
        return;
    size_t total_len = str.length() + 4;
    char header[5];
    std::snprintf(header, sizeof(header), "%04zx", total_len);
    std::cout.write(header, 4);
    std::cout.write(str.data(), str.length());
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
    int len = static_cast<int>(strtol(header, nullptr, 16));
    if (len == 0)
        return 0;
    if (len == 1) {
        buf.clear();
        return 1;
    }
    if (len < 4 || len > static_cast<int>(MAX_PKT + 4))
        return -1;
    size_t data_len = len - 4;
    buf.resize(data_len);
    if (std::fread(buf.data(), 1, data_len, stdin) != data_len)
        return -1;
    return static_cast<int>(data_len);
}

void trim_line(std::string& s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' '))
        s.pop_back();
    size_t start = s.find_first_not_of(" \r\n");
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

struct LfsObject {
    std::string oid;
    size_t size = 0;
};

bool parse_object_line(const std::string& s, LfsObject& obj) {
    size_t space = s.find(' ');
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

std::string lfs_object_path(const std::string& base_dir, const std::string& oid) {
    if (oid.length() < 4)
        return "";
    std::string subdir = oid.substr(0, 2) + "/" + oid.substr(2, 2);
    return base_dir + "/" + subdir + "/" + oid;
}

bool object_exists(const std::string& objects_dir, const std::string& oid) {
    std::error_code ec;
    return fs::exists(lfs_object_path(objects_dir, oid), ec);
}

bool is_bare_repo(const fs::path& repo_path) {
    std::error_code ec;
    fs::path config_path;

    if (fs::exists(repo_path / "config", ec)) {
        config_path = repo_path / "config";
    } else if (fs::exists(repo_path / ".git" / "config", ec)) {
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

int main(int argc, char** argv) {
#ifdef ENABLE_LOG
    log_file.open("/tmp/git-lfs-transfer.log", std::ios::app);
    if (!log_file.is_open()) {
        fs::create_directories("/tmp");
        log_file.open("/tmp/git-lfs-transfer.log", std::ios::app);
    }
#endif

    log("=== git-lfs-transfer STARTED ===");
    if (argc < 3) {
        log("Usage incorrect");
        return 1;
    }

    const std::string repo = argv[1];
    const std::string op = argv[2];
    log("repo=" + repo + ", op=" + op);

    std::string objects_dir;
    if (is_bare_repo(repo)) {
        objects_dir = repo + "/lfs/objects";
        log("Detected bare repository, objects dir: " + objects_dir);
    } else {
        objects_dir = repo + "/.git/lfs/objects";
        log("Detected non-bare repository, objects dir: " + objects_dir);
    }

    std::error_code ec;
    fs::create_directories(objects_dir, ec);

    // Handshake
    pkt_write("version=1\n");
    pkt_flush();

    std::string line;
    int n = pkt_read(line);
    if (n <= 0) {
        log("No version request");
        return 1;
    }
    trim_line(line);
    if (line != "version 1") {
        pkt_write("status 400\n");
        pkt_flush();
        return 1;
    }
    n = pkt_read(line);  // flush
    pkt_write("status 200\n");
    pkt_flush();
    log("Handshake complete");

    bool running = true;
    try {
        while (running) {
            n = pkt_read(line);
            if (n < 0) {
                log("Connection closed");
                break;
            }
            if (n == 0 || n == 1)
                continue;
            trim_line(line);
            std::string cmd = line;
            log("RECV command: " + cmd);

            // ---- list-lock / lock / unlock ----
            if (cmd == "list-lock" || cmd == "lock" || cmd == "unlock") {
                while (true) {
                    n = pkt_read(line);
                    if (n < 0 || n == 0)
                        break;
                }
                if (cmd == "list-lock") {
                    pkt_write("status 200\n");
                    pkt_delim();
                    pkt_flush();
                } else if (cmd == "lock") {
                    pkt_write("status 201\n");
                    pkt_flush();
                } else {
                    pkt_write("status 200\n");
                    pkt_flush();
                }
            }
            // ---- batch ----
            else if (cmd == "batch") {
                std::vector<std::string> args;
                while (true) {
                    n = pkt_read(line);
                    if (n == 1)
                        break;
                    if (n <= 0) {
                        running = false;
                        break;
                    }
                    trim_line(line);
                    args.push_back(line);
                }
                if (!running)
                    break;

                std::vector<LfsObject> objs;
                while (true) {
                    n = pkt_read(line);
                    if (n == 0)
                        break;
                    if (n <= 0) {
                        running = false;
                        break;
                    }
                    if (n == 1)
                        continue;
                    trim_line(line);
                    LfsObject o;
                    if (parse_object_line(line, o))
                        objs.push_back(o);
                }
                if (!running)
                    break;

                pkt_write("status 200\n");
                pkt_write("hash-algo=sha256\n");
                pkt_delim();
                for (auto& o : objs) {
                    std::string action = object_exists(objects_dir, o.oid) ? "noop" : "upload";
                    pkt_write(o.oid + " " + std::to_string(o.size) + " " + action + "\n");
                }
                pkt_flush();
                log("Batch response sent");
            }
            // ---- put-object ----
            else if (cmd.rfind("put-object ", 0) == 0) {
                std::string oid = cmd.substr(11);
                trim_line(oid);
                size_t expected = 0;
                while (true) {
                    n = pkt_read(line);
                    if (n == 1)
                        break;
                    if (n <= 0) {
                        running = false;
                        break;
                    }
                    trim_line(line);
                    if (line.rfind("size=", 0) == 0) {
                        try {
                            expected = std::stoull(line.substr(5));
                        } catch (...) {
                        }
                    }
                }
                if (!running)
                    break;

                std::string path = lfs_object_path(objects_dir, oid);
                log("put-object oid=" + oid + " size=" + std::to_string(expected) +
                    " -> " + path);
                pkt_write("status 200\n");
                pkt_flush();

                fs::create_directories(fs::path(path).parent_path(), ec);
                if (ec) {
                    log("DIR CREATE FAIL: " + ec.message());
                    pkt_write("status 500\n");
                    pkt_flush();
                    continue;
                }

                std::ofstream ofs(path, std::ios::binary);
                if (!ofs) {
                    log("FILE OPEN FAIL: " + path);
                    pkt_write("status 500\n");
                    pkt_flush();
                    continue;
                }

                size_t received = 0;
                while (true) {
                    n = pkt_read(line);
                    if (n == 0)
                        break;
                    if (n < 0) {
                        running = false;
                        break;
                    }
                    if (n == 1)
                        continue;
                    ofs.write(line.data(), line.size());
                    received += line.size();
                }
                ofs.close();
                if (!running)
                    break;

                log("Stored object: " + path + " (" + std::to_string(received) + " bytes)");
                pkt_write("status 200\n");
                pkt_delim();
                pkt_flush();
            }
            // ---- verify-object ----
            else if (cmd.rfind("verify-object ", 0) == 0) {
                while (true) {
                    n = pkt_read(line);
                    if (n == 0)
                        break;
                    if (n < 0) {
                        running = false;
                        break;
                    }
                }
                if (!running)
                    break;
                log("verify-object " + cmd.substr(14));
                pkt_write("status 200\n");
                pkt_flush();
            }
            // ---- quit ----
            else if (cmd == "quit") {
                n = pkt_read(line);  // flush
                pkt_write("status 200\n");
                pkt_flush();
                log("Received quit");
                running = false;
            }
            // ---- unknown ----
            else {
                log("Unknown cmd: " + cmd);
                pkt_write("status 200\n");
                pkt_flush();
            }
        }
    } catch (const std::exception& e) {
        log(std::string("EXCEPTION: ") + e.what());
        return 1;
    } catch (...) {
        log("UNKNOWN EXCEPTION");
        return 1;
    }

    log("=== git-lfs-transfer TERMINATED ===");
    return 0;
}
