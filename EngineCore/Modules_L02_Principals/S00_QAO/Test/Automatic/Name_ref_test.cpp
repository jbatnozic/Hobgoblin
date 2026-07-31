// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#define HOBGOBLIN_SHORT_NAMESPACE
#include <Hobgoblin/QAO.hpp>

#include <gtest/gtest.h>

#include <cstring>

using namespace hg::qao;

TEST(QAO_NameRefTest, FromCString) {
    const char* string = "ChunkNorris";
    auto        r      = QAO_NameRef(string);

    EXPECT_EQ(r.string, string);
    EXPECT_EQ(r.stringLength, std::strlen(string));
    EXPECT_FALSE(r.stringIsStatic);
}

TEST(QAO_NameRefTest, FromStdString) {
    std::string string = "ChunkNorris";
    auto        r      = QAO_NameRef(string);

    EXPECT_EQ(r.string, string.c_str());
    EXPECT_EQ(r.stringLength, string.length());
    EXPECT_FALSE(r.stringIsStatic);
}

TEST(QAO_NameRefTest, FromStringView) {
    std::string string = "ChunkNorris";
    auto        r      = QAO_NameRef(string);

    EXPECT_EQ(r.string, string.c_str());
    EXPECT_EQ(r.stringLength, string.length());
    EXPECT_FALSE(r.stringIsStatic);
}

TEST(QAO_NameRefTest, StaticName) {
    auto r = QAO_STATIC_NAME("MyStaticName");

    EXPECT_EQ(r.stringLength, std::strlen("MyStaticName"));
    EXPECT_TRUE(std::strncmp(r.string, "MyStaticName", r.stringLength) == 0);
    EXPECT_TRUE(r.stringIsStatic);
}
