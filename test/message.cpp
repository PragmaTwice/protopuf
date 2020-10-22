#include <gtest/gtest.h>

#include <protopuf/message.h>

using namespace pp;
using namespace std;

GTEST_TEST(message, function) {
    message<integer_field<1, int>, float_field<3, float>> m{12, 1.23};
    static_assert(m.size == 2);

    EXPECT_EQ(m.get<1>(), 12);
    EXPECT_FLOAT_EQ(m.get<3>().value(), 1.23);

    auto m2 = m;
    EXPECT_EQ(m2, m);
}
