#include <cstdint>

#include <gtest/gtest.h>

#include <unistringxx/unicore.hpp>

using namespace std;
using namespace unistringxx;

class uint24_t_test : public ::testing::Test
{
protected:
    virtual void SetUp(void)
    {
        int_value = 0xFFEEDDCC;
        ui24_init = uint24_t(int_value);
        return;
    }

    uint24_t ui24_default; // default constructed uint24_t
    int_least32_t int_value;
    uint24_t ui24_init;
};

TEST_F(uint24_t_test, default_construction)
{
    EXPECT_EQ(0, ui24_default.data[0]);
    EXPECT_EQ(0, ui24_default.data[1]);
    EXPECT_EQ(0, ui24_default.data[2]);
    return;
}

TEST_F(uint24_t_test, initialized_construction)
{
    EXPECT_EQ(0xCC, ui24_init.data[0]);
    EXPECT_EQ(0xDD, ui24_init.data[1]);
    EXPECT_EQ(0xEE, ui24_init.data[2]);
    return;
}

TEST_F(uint24_t_test, literal_construction)
{
    uint24_t ui24a = 0x99887766AABBCCDD_u24;
    EXPECT_EQ(0xDD, ui24a.data[0]);
    EXPECT_EQ(0xCC, ui24a.data[1]);
    EXPECT_EQ(0xBB, ui24a.data[2]);
    return;
}

TEST_F(uint24_t_test, int_conversion)
{
    EXPECT_EQ(0x00EEDDCC, static_cast<int_least32_t>(ui24_init));
    return;
}

TEST_F(uint24_t_test, basic_arithmethics)
{
    uint24_t ui24a = 0x112233_u24;
    uint24_t ui24b = 0x112233_u24;
    EXPECT_EQ(0x00224466, static_cast<int_least32_t>(ui24a + ui24b));
    EXPECT_EQ(0x00, static_cast<int_least32_t>(ui24a - ui24b));
    EXPECT_EQ(0x00224466, static_cast<int_least32_t>(ui24a * 2));
    EXPECT_EQ(1, static_cast<int_least32_t>(ui24a / ui24b));
    return;
}

TEST_F(uint24_t_test, assignments)
{
    uint24_t ui24a = 0x112233_u24;
    ui24a += 0x112233_u24;
    EXPECT_EQ(0x00224466, static_cast<int_least32_t>(ui24a));
    ui24a -= 0x112233_u24;
    EXPECT_EQ(0x00112233, static_cast<int_least32_t>(ui24a));

    EXPECT_EQ(0x00112234, static_cast<int_least32_t>(++ui24a));
    EXPECT_EQ(0x00112234, static_cast<int_least32_t>(ui24a++));
    EXPECT_EQ(0x00112235, static_cast<int_least32_t>(ui24a));
    EXPECT_EQ(0x00112234, static_cast<int_least32_t>(--ui24a));
    EXPECT_EQ(0x00112234, static_cast<int_least32_t>(ui24a--));
    EXPECT_EQ(0x00112233, static_cast<int_least32_t>(ui24a));

    return;
}
