# git-lfs-transfer

Standalone **Git LFS** custom transfer agent implementing the [pkt-line protocol](https://github.com/git-lfs/git-lfs/blob/main/docs/proposals/ssh_adapter.md).  Designed to be invoked by `git lfs` for repository management operations.

## Features

- **Git LFS Batch API** — `batch`, `put-object`, `verify-object`
- **Lock stubs** — `list-lock`, `lock`, `unlock` (return success, no persistent state)
- **Bare & non-bare repo auto-detection** — inspects `config` for `core.bare`
- **pkt-line wire protocol** — hex-encoded length prefix framing, same encoding used by Git
- **Statically linked** — `libgcc` and `libstdc++` are linked statically; no runtime `.so` dependency

## Quick Start

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/bin/git-lfs-transfer --help
```

To enable debug logging (writes to `/tmp/git-lfs-transfer.log`):

```bash
cmake -B build -DENABLE_LOG=ON
cmake --build build
```

## Usage

This binary is a **Git LFS custom transfer agent** — it speaks the pkt-line protocol on stdin/stdout and is not meant to be run interactively.  `git lfs` invokes it automatically when configured as a custom transfer agent:

```bash
# Configure git-lfs to use this agent (example):
git config lfs.customtransfer.git-lfs-transfer.path /path/to/build/bin/git-lfs-transfer

# Then normal git-lfs operations will use it:
git lfs pull
git lfs push
```

The two positional arguments — repository path and operation (`upload` / `download`) — are passed by `git lfs`; the operation is logged but does not change the behavior (all command routing is handled via pkt-line).

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_LOG` | `OFF` | Enable debug logging to `/tmp/git-lfs-transfer.log` |
| `BUILD_TESTS` | `ON`  | Build unit tests (requires Google Test) |

## Build & Run Tests

```bash
cmake -B build -DBUILD_TESTS=ON
cmake --build build
cd build && ctest --output-on-failure
```

## Requirements

- **CMake** ≥ 3.16
- **C++17** compiler (GCC ≥ 8, Clang ≥ 7, MSVC ≥ 19.14)
- **Google Test** (optional, tests are skipped if not found)

## License

[Boost Software License 1.0](LICENSE)
