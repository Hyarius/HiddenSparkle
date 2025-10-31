#include <gtest/gtest.h>

#include "sparkle.hpp"

TEST(Sparkle_FirstFunction, ReturnsZero)
{
    EXPECT_EQ(spk::firstFunction(), 0);
}