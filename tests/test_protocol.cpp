#include "git-lfs-transfer/protocol.h"

#include <gtest/gtest.h>
#include <string>

using namespace git_lfs_transfer;

// ─────────────────────────────────────────────────────────
// trim / trim_line
// ─────────────────────────────────────────────────────────
TEST(ProtocolTest, TrimRemovesWhitespace) {
    EXPECT_EQ(trim("  hello  "), "hello");
    EXPECT_EQ(trim("\n\r hello \r\n"), "hello");
    EXPECT_EQ(trim(""), "");
    EXPECT_EQ(trim("   "), "");
}

TEST(ProtocolTest, TrimLineInPlace) {
    std::string s = "hello\n\r ";
    trim_line(s);
    EXPECT_EQ(s, "hello");

    s = "  spaced  ";
    trim_line(s);
    EXPECT_EQ(s, "spaced");  // trim_line trims front too
}

// ─────────────────────────────────────────────────────────
// Packet I/O — cannot easily test without mocking stdin/stdout
// but we can at least verify constants.
// ─────────────────────────────────────────────────────────
TEST(ProtocolTest, MaxPacketSizeConstant) {
    EXPECT_EQ(kMaxPacketSize, 65520);
}

TEST(ProtocolTest, TrimLineLeadingSpaces) {
    std::string s = "  \r\n  foo";
    trim_line(s);
    EXPECT_EQ(s, "foo");
}

TEST(ProtocolTest, TrimLineAlreadyClean) {
    std::string s = "clean";
    trim_line(s);
    EXPECT_EQ(s, "clean");
}
