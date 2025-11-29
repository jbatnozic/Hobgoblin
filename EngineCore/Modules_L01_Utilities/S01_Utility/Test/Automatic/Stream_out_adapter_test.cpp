// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/Utility/Stream_out_adapter.hpp>
#include <Hobgoblin/Utility/Stream_view.hpp>

#include <gtest/gtest.h>

#include <list>
#include <string>
#include <vector>

namespace jbatnozic {
namespace hobgoblin {
namespace util {

TEST(OutputStreamContainerAdapterTest, InsertCharsIntoString) {
    std::string                  str;
    OutputStreamContainerAdapter ostream{str};

    ostream << (char)'c' << (char)'h' << (char)'u' << (char)'c' << (char)'k';

    EXPECT_EQ(str, "chuck");
}

TEST(OutputStreamContainerAdapterTest, InsertIntIntoVector) {
    std::vector<unsigned char>   vec;
    OutputStreamContainerAdapter ostream{vec};

    ostream << (int)0xFFFFFFFF;

    ASSERT_EQ(vec.size(), 4);
    EXPECT_EQ(vec[0], 0xFF);
    EXPECT_EQ(vec[1], 0xFF);
    EXPECT_EQ(vec[2], 0xFF);
    EXPECT_EQ(vec[3], 0xFF);
}

TEST(OutputStreamContainerAdapterTest, StringRoundTrip) {
    const std::string name = "Marshall(ing)Tito";

    std::string str;

    {
        OutputStreamContainerAdapter ostream{str};
        ostream << name;
    }

    {
        ViewStream istream{str.data(), static_cast<std::int64_t>(str.size())};

        const auto out = istream.extract<std::string>();

        EXPECT_TRUE(istream.isGood());
        EXPECT_EQ(out, name);
    }
}

TEST(OutputStreamContainerAdapterTest, InsertBeforeEndOfString) {
    std::string                  str{"test"};
    OutputStreamContainerAdapter ostream{str, std::begin(str)};

    ostream << (char)'m' << (char)'a' << (char)'g' << (char)'i' << (char)'c';

    EXPECT_EQ(str, "magictest");
}

} // namespace util
} // namespace hobgoblin
} // namespace jbatnozic
