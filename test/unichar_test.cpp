#include <array>
#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

#include <unistringxx/common.hpp>
#include <unistringxx/unichar.hpp>

using namespace unistringxx;

TEST(unichar_test, default_construction)
{
    unichar_t uc;
    EXPECT_EQ(0x00, uc.code_point());
    return;
}

TEST(unichar_test, initialized_construction)
{
    // char
    unichar_t uca{'A'};
    EXPECT_EQ(0x41, uca.code_point());

    // char16_t (note: char16_t literal value cannot be more than 16-bit and cannot be in the surrogate ranges)
    unichar_t ucb{u'\u3042'};
    EXPECT_EQ(0x3042, ucb.code_point());

    // char32_t
    unichar_t ucc{U'\U0002F83F'};
    EXPECT_EQ(0x02F83F, ucc.code_point());

    // invalid value
    unichar_t ucd{unichar_t::invalid_value};
    EXPECT_FALSE(ucd.is_valid());

    return;
}

TEST(unichar_test, utf8_decoder)
{
    unichar_t uca;

    // single byte
    // 'A' : U+0041 | UTF-8 : 0x41
    uca = unichar_t::from_utf8('A');
    EXPECT_EQ(0x41, uca.code_point());

    // multi byte
    // 'Ⓐ' : U+24B6 | UTF-8 : 0xE292B6
    uca = unichar_t::from_utf8('\xE2', '\x92', '\xB6');
    EXPECT_EQ(0x24B6, uca.code_point());

#if (UNISTRINGXX_WITH_EXCEPTIONS)

    // invalid head sequence
    EXPECT_THROW(unichar_t::from_utf8('\x8F'), std::range_error);

    // insufficient sequences to decode
    // '周' : U+2F83F | UTF-8 : 0xF0AFA0BF
    EXPECT_THROW(unichar_t::from_utf8('\xF0', '\xAF', '\xA0'), std::range_error); // missing trailing 0xBF

    // encoded using 2 octets but only requires 1
    // "*" : U+002A | UTF-8 : 0x2A
    EXPECT_THROW(unichar_t::from_utf8('\xC0', '\xAA'), std::range_error);

    // encoded a code point within UTF-16 surrogate ranges
    // U+DCBA | UTF-8 : 0xEDB2BA (invalid)
    EXPECT_THROW(unichar_t::from_utf8('\xED', '\xB2', '\xBA'), std::range_error);

    // exceeded U+10FFFF
    // U+110000 | UTF-8 : 0xF4908080 (invalid)
    EXPECT_THROW(unichar_t::from_utf8('\xF4', '\x90', '\x80', '\x80'), std::range_error);

#else // (UNISTRINGXX_WITH_EXCEPTIONS)

    uca = unichar_t::from_utf8('\x8F');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

    uca = unichar_t::from_utf8('\xF0', '\xAF', '\xA0');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

    uca = unichar_t::from_utf8('\xC0', '\xAA');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

    uca = unichar_t::from_utf8('\xED', '\xB2', '\xBA');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

    uca = unichar_t::from_utf8('\xF4', '\x90', '\x80', '\x80');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

    return;
}

TEST(unichar_test, utf16_decoder)
{
    unichar_t uca;

    // "あ" : U+3042 | UTF-16 : 0x3042
    uca = unichar_t::from_utf16(u'\u3042');
    EXPECT_EQ(0x3042, uca.code_point());

    // surrogate pairs
    // "𧋊" : U+272CA | UTF-16 : 0xD85C 0xDECA
    // uca = unichar::from_utf16(u'\uD85C', u'\uDECA');
        // this results in invalid universal character because a char16_t literal
        // escaped by '\u' in the surrogate range is undefined (implementation
        // defined?). to solve, use '\x' escape instead.
    uca = unichar_t::from_utf16(u'\xD85C', u'\xDECA');
    EXPECT_EQ(0x0272CA, uca.code_point());

#if (UNISTRINGXX_WITH_EXCEPTIONS)

    // missing surrogate pair
    EXPECT_THROW(unichar_t::from_utf16(u'\xDE00'), std::range_error);

#else // (UNISTRINGXX_WITH_EXCEPTIONS)

    uca = unichar_t::from_utf16(u'\xDE00');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

    return;
}

TEST(unichar_test, utf32_decoder)
{
    unichar_t uca;

    uca = unichar_t::from_utf32(U'\U00000041');
    EXPECT_EQ(0x41, uca.code_point());

    uca = unichar_t::from_utf32(U'\U00003042');
    EXPECT_EQ(0x3042, uca.code_point());

    uca = unichar_t::from_utf32(U'\U0010FFFF');
    EXPECT_EQ(0x10FFFF, uca.code_point());

#if (UNISTRINGXX_WITH_EXCEPTIONS)

    // UTF-16 surrogate range
    EXPECT_THROW(unichar_t::from_utf32(U'\x0000DBCA'), std::range_error);

    // beyond U+10FFFF
    EXPECT_THROW(unichar_t::from_utf32(U'\x00110000'), std::range_error);

#else // (UNISTRINGXX_WITH_EXCEPTIONS)

    uca = unichar_t::from_utf32(U'\x0000DBCA');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

    uca = unichar_t::from_utf32(U'\x00110000');
    EXPECT_EQ(unichar_t::invalid_value, uca.code_point());

#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

    return;
}

TEST(unichar_test, literal_operators)
{
    unichar_t uca;

    uca = 'A'_uc;
    EXPECT_EQ(0x41, uca.code_point());

    uca = u'\u3042'_uc;
    EXPECT_EQ(0x3042, uca.code_point());

    uca = U'\U0010FFFF'_uc;
    EXPECT_EQ(0x10FFFF, uca.code_point());    

    return;
}

TEST(unichar_test, utf8_encoder)
{
    unichar_t uca;
    decltype(uca.to_utf8()) utf8_value;
    std::uint32_t value;

    // single byte
    // '_' : U+005F | UTF-8 : 0x5F
    uca = '_'_uc;
    utf8_value = uca.to_utf8();
    ASSERT_FALSE(utf8_value.empty());
    ASSERT_EQ(static_cast<std::size_t>(1), utf8_value.size());
    EXPECT_EQ(0x5F, utf8_value[0]);

    // multi byte
    // "⓺" : U+24FA | UTF-8 : 0xE293BA
    uca = u'\u24FA'_uc;
    utf8_value = uca.to_utf8();
    ASSERT_FALSE(utf8_value.empty());
    ASSERT_EQ(static_cast<std::size_t>(3), utf8_value.size());
    value = utf8_value[0] << 0x10 |
            utf8_value[1] << 0x08 |
            utf8_value[2] << 0x00;
    EXPECT_EQ(static_cast<std::uint32_t>(0xE293BA), value);

    // invalid
    uca = unichar_t{unichar_t::invalid_value};
    EXPECT_TRUE(uca.to_utf8().empty());

    return;
}

TEST(unichar_test, utf16_encoder)
{
    unichar_t uca;
    decltype(uca.to_utf16()) utf16_value;
    std::uint32_t value;

    // simple encode
    // "あ" : U+3042 | UTF-16 : 0x3042
    uca = u'\u3042'_uc;
    utf16_value = uca.to_utf16();
    ASSERT_FALSE(utf16_value.empty());
    ASSERT_EQ(static_cast<std::size_t>(1), utf16_value.size());
    EXPECT_EQ(0x3042, utf16_value[0]);

    // encode with surrogate pairs
    // "𧋊" : U+272CA | UTF-16 : 0xD85C 0xDECA
    uca = U'\U0002F83F'_uc;
    utf16_value = uca.to_utf16();
    ASSERT_FALSE(utf16_value.empty());
    ASSERT_EQ(static_cast<std::size_t>(2), utf16_value.size());
    value = utf16_value[0] << 0x10 |
            utf16_value[1] << 0x00;
    EXPECT_EQ(static_cast<std::uint32_t>(0xD87EDC3F), value);    

    // invalid
    uca = unichar_t{unichar_t::invalid_value};
    EXPECT_TRUE(uca.to_utf16().empty());

    return;
}

TEST(unichar_test, utf32_encoder)
{
    const char32_t c32_max = std::numeric_limits<char32_t>::max();

    unichar_t uca{u'\u3042'};

    EXPECT_EQ(U'\U00003042', uca.to_utf32());

    uca = unichar_t{unichar_t::invalid_value};

    EXPECT_EQ(c32_max, uca.to_utf32());

    return;
}

TEST(unichar_test, comparisons)
{
    unichar_t uca{U'\U00003042'};
    unichar_t ucb{u'\u3042'};

    EXPECT_EQ(true, uca == ucb); // note: the operator=='s result is being tested, not uca & ucb.

    unichar_t ucc{U'\U0010FFFF'};

    EXPECT_EQ(true, uca != ucc); // note: the operator!='s result is being tested, not uca & ucb.

    EXPECT_EQ(true, uca < ucc);

    EXPECT_EQ(false, uca > ucc);

    EXPECT_EQ(true, uca <= ucc);

    EXPECT_EQ(false, uca >= ucc);

    return;
}

TEST(unichar_traits_test, properties)
{
    unichar_t_traits::char_type ch_a{u'\u3042'};
    unichar_t_traits::char_type ch_b;
    unichar_t_traits::char_type ch_c{'Z'};

    unichar_t_traits::assign(ch_b, ch_a);
    ASSERT_EQ(ch_a.code_point(), ch_b.code_point());

    ASSERT_TRUE(unichar_t_traits::eq(ch_a, ch_b));
    ASSERT_FALSE(unichar_t_traits::eq(ch_a, ch_c));

    ASSERT_TRUE(unichar_t_traits::lt(ch_c, ch_a));
    ASSERT_FALSE(unichar_t_traits::lt(ch_a, ch_b));

    typedef std::array<unichar_t_traits::char_type, 4> uc_array_t;
    uc_array_t arr1{{ 'A', u'\u3042', U'\U0010FFFF', '\0'_uc }};
    uc_array_t arr2;

    unichar_t_traits::move(arr2.data(), arr1.data(), arr1.size());
    for (std::size_t ctr = 0; ctr < arr1.size(); ctr++) {
        ASSERT_EQ(arr1[ctr], arr2[ctr]);
    }

    uc_array_t arr3;

    unichar_t_traits::copy(arr3.data(), arr2.data(), arr2.size());
    for (std::size_t ctr = 0; ctr < arr2.size(); ctr++) {
        ASSERT_EQ(arr2[ctr], arr3[ctr]);
    }

    uc_array_t arr4{{ 'A', u'\u3043', U'\U0010FFFF', '\0'_uc }};

    int result = unichar_t_traits::compare(arr1.data(), arr3.data(), arr1.size());
    ASSERT_EQ(0, result);

    result = unichar_t_traits::compare(arr1.data(), arr4.data(), arr1.size());
    ASSERT_TRUE(result < 0);

    ASSERT_EQ(static_cast<std::size_t>(3), unichar_t_traits::length(arr4.data()));
    arr4[1] = '\0';
    ASSERT_EQ(static_cast<std::size_t>(1), unichar_t_traits::length(arr4.data()));

    ASSERT_EQ((arr1.data() + 2), unichar_t_traits::find(arr1.data(), arr1.size(), U'\U0010FFFF'));
    ASSERT_EQ(nullptr, unichar_t_traits::find(arr1.data(), arr1.size(), 'B'));

    unichar_t_traits::char_type ch{u'\u3041'};
    ASSERT_EQ(ch, unichar_t_traits::to_char_type(0x3041));
    ASSERT_EQ(static_cast<unichar_t_traits::int_type>(0x3041), unichar_t_traits::to_int_type(ch));

    ASSERT_TRUE(unichar_t_traits::eq_int_type(0x3041, 0x3041));
    unichar_t_traits::int_type uct_it_a = 1000, uct_it_b = 5000;
    ASSERT_FALSE(unichar_t_traits::eq_int_type(uct_it_a, uct_it_b));

    auto eof_value = unichar_t_traits::char_type::invalid_value;
    ASSERT_EQ(eof_value, unichar_t_traits::eof());

    uct_it_a = 0x3041;
    ASSERT_EQ(static_cast<unichar_t_traits::int_type>(0x3041), unichar_t_traits::not_eof(uct_it_a));
    ASSERT_EQ(static_cast<unichar_t_traits::int_type>(0), unichar_t_traits::not_eof(eof_value));

    return;
}
