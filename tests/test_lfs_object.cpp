#include "git-lfs-transfer/lfs_object.h"

#include <gtest/gtest.h>

using namespace git_lfs_transfer;

// ─────────────────────────────────────────────────────────
// parse_object_line
// ─────────────────────────────────────────────────────────
TEST(LfsObjectTest, ParseValidLine) {
    LfsObject obj;
    ASSERT_TRUE(parse_object_line(
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 12345", obj));
    EXPECT_EQ(obj.oid, "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    EXPECT_EQ(obj.size, 12345);
}

TEST(LfsObjectTest, ParseLineMissingSpace) {
    LfsObject obj;
    EXPECT_FALSE(parse_object_line("justanoidwithoutspace", obj));
}

TEST(LfsObjectTest, ParseLineOidTooShort) {
    LfsObject obj;
    EXPECT_FALSE(parse_object_line("short 42", obj));
}

TEST(LfsObjectTest, ParseLineInvalidSize) {
    LfsObject obj;
    EXPECT_FALSE(parse_object_line(
        "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855 nan", obj));
}

// ─────────────────────────────────────────────────────────
// lfs_object_path
// ─────────────────────────────────────────────────────────
TEST(LfsObjectTest, ObjectPathFormat) {
    std::string oid = "abcdef1234567890abcdef1234567890abcdef12";
    std::string path = lfs_object_path("/objects", oid);
    EXPECT_EQ(path, "/objects/ab/cd/abcdef1234567890abcdef1234567890abcdef12");
}

TEST(LfsObjectTest, ObjectPathOidTooShort) {
    EXPECT_EQ(lfs_object_path("/objects", "abc"), "");
}
