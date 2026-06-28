#include "git-lfs-transfer/log.h"
#include "git-lfs-transfer/lfs_object.h"
#include "git-lfs-transfer/protocol.h"
#include "git-lfs-transfer/repository.h"

#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace fs = std::filesystem;
namespace lfs = git_lfs_transfer;

int main(int argc, char** argv) {
    lfs::init_logging();
    lfs::log("=== git-lfs-transfer STARTED ===");

    if (argc < 3) {
        lfs::log("Usage: git-lfs-transfer <repo> <operation>");
        return 1;
    }

    const std::string repo = argv[1];
    const std::string op   = argv[2];
    lfs::log("repo=" + repo + ", op=" + op);

    const std::string objects_dir = lfs::get_objects_dir(repo);
    lfs::log("Objects directory: " + objects_dir);

    {
        std::error_code ec;
        fs::create_directories(objects_dir, ec);
    }

    // ---- Handshake ----
    lfs::pkt_write("version=1\n");
    lfs::pkt_flush();

    std::string line;
    int n = lfs::pkt_read(line);
    if (n <= 0) {
        lfs::log("No version request");
        return 1;
    }
    lfs::trim_line(line);
    if (line != "version 1") {
        lfs::pkt_write("status 400\n");
        lfs::pkt_flush();
        return 1;
    }
    n = lfs::pkt_read(line); // consume flush
    lfs::pkt_write("status 200\n");
    lfs::pkt_flush();
    lfs::log("Handshake complete");

    // ---- Main command loop ----
    bool running = true;
    try {
        while (running) {
            n = lfs::pkt_read(line);
            if (n < 0) {
                lfs::log("Connection closed");
                break;
            }
            if (n == 0 || n == 1)
                continue;

            lfs::trim_line(line);
            const std::string cmd = line;
            lfs::log("RECV command: " + cmd);

            // ---- list-lock / lock / unlock ----
            if (cmd == "list-lock" || cmd == "lock" || cmd == "unlock") {
                while (true) {
                    n = lfs::pkt_read(line);
                    if (n < 0 || n == 0)
                        break;
                }
                if (cmd == "list-lock") {
                    lfs::pkt_write("status 200\n");
                    lfs::pkt_delim();
                    lfs::pkt_flush();
                } else if (cmd == "lock") {
                    lfs::pkt_write("status 201\n");
                    lfs::pkt_flush();
                } else {
                    lfs::pkt_write("status 200\n");
                    lfs::pkt_flush();
                }
            }
            // ---- batch ----
            else if (cmd == "batch") {
                std::vector<std::string> args;
                while (true) {
                    n = lfs::pkt_read(line);
                    if (n == 1)
                        break;
                    if (n <= 0) {
                        running = false;
                        break;
                    }
                    lfs::trim_line(line);
                    args.push_back(line);
                }
                if (!running)
                    break;

                std::vector<lfs::LfsObject> objs;
                while (true) {
                    n = lfs::pkt_read(line);
                    if (n == 0)
                        break;
                    if (n <= 0) {
                        running = false;
                        break;
                    }
                    if (n == 1)
                        continue;
                    lfs::trim_line(line);
                    lfs::LfsObject o;
                    if (lfs::parse_object_line(line, o))
                        objs.push_back(o);
                }
                if (!running)
                    break;

                lfs::pkt_write("status 200\n");
                lfs::pkt_write("hash-algo=sha256\n");
                lfs::pkt_delim();
                for (const auto& o : objs) {
                    const std::string action =
                        lfs::object_exists(objects_dir, o.oid) ? "noop" : "upload";
                    lfs::pkt_write(o.oid + " " + std::to_string(o.size) + " " + action + "\n");
                }
                lfs::pkt_flush();
                lfs::log("Batch response sent");
            }
            // ---- put-object ----
            else if (cmd.rfind("put-object ", 0) == 0) {
                std::string oid = cmd.substr(11);
                lfs::trim_line(oid);
                std::size_t expected = 0;
                while (true) {
                    n = lfs::pkt_read(line);
                    if (n == 1)
                        break;
                    if (n <= 0) {
                        running = false;
                        break;
                    }
                    lfs::trim_line(line);
                    if (line.rfind("size=", 0) == 0) {
                        try {
                            expected = std::stoull(line.substr(5));
                        } catch (...) {
                        }
                    }
                }
                if (!running)
                    break;

                const std::string path = lfs::lfs_object_path(objects_dir, oid);
                lfs::log("put-object oid=" + oid +
                         " size=" + std::to_string(expected) + " -> " + path);

                lfs::pkt_write("status 200\n");
                lfs::pkt_flush();

                std::error_code ec;
                fs::create_directories(fs::path(path).parent_path(), ec);
                if (ec) {
                    lfs::log("DIR CREATE FAIL: " + ec.message());
                    lfs::pkt_write("status 500\n");
                    lfs::pkt_flush();
                    continue;
                }

                std::ofstream ofs(path, std::ios::binary);
                if (!ofs) {
                    lfs::log("FILE OPEN FAIL: " + path);
                    lfs::pkt_write("status 500\n");
                    lfs::pkt_flush();
                    continue;
                }

                std::size_t received = 0;
                while (true) {
                    n = lfs::pkt_read(line);
                    if (n == 0)
                        break;
                    if (n < 0) {
                        running = false;
                        break;
                    }
                    if (n == 1)
                        continue;
                    ofs.write(line.data(), static_cast<std::streamsize>(line.size()));
                    received += line.size();
                }
                ofs.close();
                if (!running)
                    break;

                lfs::log("Stored object: " + path + " (" +
                         std::to_string(received) + " bytes)");
                lfs::pkt_write("status 200\n");
                lfs::pkt_delim();
                lfs::pkt_flush();
            }
            // ---- verify-object ----
            else if (cmd.rfind("verify-object ", 0) == 0) {
                while (true) {
                    n = lfs::pkt_read(line);
                    if (n == 0)
                        break;
                    if (n < 0) {
                        running = false;
                        break;
                    }
                }
                if (!running)
                    break;
                lfs::log("verify-object " + cmd.substr(14));
                lfs::pkt_write("status 200\n");
                lfs::pkt_flush();
            }
            // ---- quit ----
            else if (cmd == "quit") {
                n = lfs::pkt_read(line); // consume flush
                lfs::pkt_write("status 200\n");
                lfs::pkt_flush();
                lfs::log("Received quit");
                running = false;
            }
            // ---- unknown ----
            else {
                lfs::log("Unknown cmd: " + cmd);
                lfs::pkt_write("status 200\n");
                lfs::pkt_flush();
            }
        }
    } catch (const std::exception& e) {
        lfs::log(std::string("EXCEPTION: ") + e.what());
        return 1;
    } catch (...) {
        lfs::log("UNKNOWN EXCEPTION");
        return 1;
    }

    lfs::log("=== git-lfs-transfer TERMINATED ===");
    return 0;
}
