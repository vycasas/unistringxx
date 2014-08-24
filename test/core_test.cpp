#include <cstdint>

#include <gtest/gtest.h>

#include <unistringxx/core.hpp>

class uint24_test : public ::testing::Test
{
protected:
    virtual void SetUp(void)
    {
        int_value = 0xFFEEDDCC;
        ui24_init = unistringxx::uint24_t(int_value);
        return;
    }

    unistringxx::uint24_t ui24_default; // default constructed uint24_t
    std::int_least32_t int_value;
    unistringxx::uint24_t ui24_init;
};

TEST_F(uint24_test, default_construction)
{
    EXPECT_EQ(0, ui24_default.data[0]);
    EXPECT_EQ(0, ui24_default.data[1]);
    EXPECT_EQ(0, ui24_default.data[2]);
    return;
}

TEST_F(uint24_test, initialized_construction)
{
    EXPECT_EQ(0xCC, ui24_init.data[0]);
    EXPECT_EQ(0xDD, ui24_init.data[1]);
    EXPECT_EQ(0xEE, ui24_init.data[2]);
    return;
}

TEST_F(uint24_test, literal_construction)
{
    using namespace unistringxx::operators;

    unistringxx::uint24_t ui24a = 0x99887766AABBCCDD_u24;
    EXPECT_EQ(0xDD, ui24a.data[0]);
    EXPECT_EQ(0xCC, ui24a.data[1]);
    EXPECT_EQ(0xBB, ui24a.data[2]);
    return;
}

TEST_F(uint24_test, int_conversion)
{
    EXPECT_EQ(0x00EEDDCC, static_cast<std::int_least32_t>(ui24_init));
    return;
}

TEST_F(uint24_test, basic_arithmethics)
{
    using namespace unistringxx::operators;

    unistringxx::uint24_t ui24a = 0x112233_u24;
    unistringxx::uint24_t ui24b = 0x112233_u24;
    EXPECT_EQ(0x00224466, static_cast<std::int_least32_t>(ui24a + ui24b));
    EXPECT_EQ(0x00, static_cast<std::int_least32_t>(ui24a - ui24b));
    EXPECT_EQ(0x00224466, static_cast<std::int_least32_t>(ui24a * 2));
    EXPECT_EQ(1, static_cast<std::int_least32_t>(ui24a / ui24b));
    return;
}

TEST_F(uint24_test, assignments)
{
    using namespace unistringxx::operators;

    unistringxx::uint24_t ui24a = 0x112233_u24;
    ui24a += 0x112233_u24;
    EXPECT_EQ(0x00224466, static_cast<std::int_least32_t>(ui24a));
    ui24a -= 0x112233_u24;
    EXPECT_EQ(0x00112233, static_cast<std::int_least32_t>(ui24a));

    EXPECT_EQ(0x00112234, static_cast<std::int_least32_t>(++ui24a));
    EXPECT_EQ(0x00112234, static_cast<std::int_least32_t>(ui24a++));
    EXPECT_EQ(0x00112235, static_cast<std::int_least32_t>(ui24a));
    EXPECT_EQ(0x00112234, static_cast<std::int_least32_t>(--ui24a));
    EXPECT_EQ(0x00112234, static_cast<std::int_least32_t>(ui24a--));
    EXPECT_EQ(0x00112233, static_cast<std::int_least32_t>(ui24a));

    return;
}
