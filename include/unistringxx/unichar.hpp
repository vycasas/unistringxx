/// @file
#if !defined(UNISTRINGXX_UNICHAR_HPP)
#define UNISTRINGXX_UNICHAR_HPP

#include <cstdint>
#include <ios>
#include <iterator>
#include <functional>
#include <limits>
#include <stdexcept>
#include <vector>

#include "common.hpp"
#include "unicore.hpp"
#include "utils.hpp"

namespace unistringxx
{
    // For convenience
    typedef std::uint8_t char8_t;

    ///
    /// Represents a single Unicode code point. This type does not tie itself to any Unicode encoding. Each instance of
    /// this type is supposed to be an actual Unicode code point (valid or not). When using outside this file
    /// definition, the type must be referred to as "unichar_t" to ensure implementation compatibility in future
    /// releases.
    ///
    struct unichar
    {
#if !(UNISTRINGXX_TEST)
        typedef uint24_t impl_type;
#else // !(UNISTRINGXX_TEST)
        // If we are testing, we do not want to depend on uint24_t as it may not have been properly tested.
        typedef std::int_least32_t impl_type;
#endif // !(UNISTRINGXX_TEST)

        typedef std::int_least32_t int_type;

        // using
        static const int_type invalid_value = std::numeric_limits<int_type>::max();

        unichar(void) : _impl{0x00}
        { return; }

        // does not error check, validate with is_valid function
        unichar(int_type value) : _impl{static_cast<impl_type>(value)}
        { return; }

        static unichar from_utf8(
            char8_t utf8_ch, char8_t utf8_ch1 = '\0', char8_t utf8_ch2 = '\0', char8_t utf8_ch3 = '\0'
        ) UNISTRINGXX_MAY_THROW_EXCEPTIONS
        {
            std::size_t index = 0x07;
            int_type value = unichar::invalid_value;

            if (!is_bit_set(utf8_ch, index)) {
                // ASCII
                value = utf8_ch;
            }
            else {
                // Decode the head sequence.
                // Number of sequences for this Unicode code point.
                std::size_t num_seq = 0, total_num_seq = 0; // total_num_seq is used for later validation

                while ((index != 0) && (is_bit_set(utf8_ch, index))) {
                    num_seq++;
                    index--;
                }

                // The minimum must be 2 including the head sequence octet, while the maximum (as per RFC 3629) is 4.
                if (num_seq < 2 || num_seq > 4) {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
                    UNISTRINGXX_THROW(std::range_error, "Invalid head sequence found while decoding UTF-8.");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
                    _impl = impl_type{unichar::invalid_value};
                    return;
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
                }

                total_num_seq = num_seq;

                // Process the head sequence.
                index--;
                // "index" is now pointing to the first valid bit of the head sequence octet which maps to the actual
                // Unicode code point.

                value = (utf8_ch & (0xFF >> (0x07 - index))) << (((num_seq - 1) * 8) - ((num_seq - 1) * 2));
                num_seq--;


                bool is_valid = (num_seq == 0);

                if (!is_valid) {
                    // The lamba below is just a nice syntactic sugar to reduce repeated code that will be applied to
                    // utf8_ch1, utf8_ch2, and utf8_ch3. The idea is to process these three parameters as a container
                    // and iterate through the items in this container.
                    is_valid = (
                        [&value, &num_seq] (const std::initializer_list<char8_t>& init_list) -> bool {
                            // Note:
                            // 1. There was already a check performed that num_seq will be 1 <= num_seq <= 3.
                            // 2. Guaranteed valid arguments passed to this function.
                            // With #1 & #2, we do not need to check if begin() != end().
                            auto iter = init_list.begin();
                            while (num_seq != 0) {
                                if (((*iter) & 0xC0) != 0x80)
                                    return (false);
                                const auto seq = *iter;
                                bool is_valid_header_bits = (is_bit_set(seq, 0x07) && !is_bit_set(seq, 0x06));
                                if (!is_valid_header_bits) {
                                    return (false);
                                }
                                value |= ((seq & (0xFF >> 0x02)) << (((num_seq - 1) * 8) - ((num_seq - 1) * 2)));
                                num_seq--;
                                iter++;
                            }
                            return (true);
                        }
                    )({ utf8_ch1, utf8_ch2, utf8_ch3 });
                }

                // make sure value was encoded with the right number of sequences
                if (value <= 0x007F) {
                    is_valid = is_valid && (total_num_seq == 1);
                }
                else if ((value >= 0x0080) && (value <= 0x07FF)) {
                    is_valid = is_valid && (total_num_seq == 2);
                }
                else if (value >= 0x0800 && value <= 0xFFFF) {
                    is_valid = is_valid && (total_num_seq == 3);
                }
                else if ((value >= 0x10000) && (value <= 0x1FFFF)) {
                    is_valid = is_valid && (total_num_seq == 4);
                }

                // make sure value is not within UTF-16 surrogates (as per RFC 3629)
                is_valid = is_valid && (!((value >= 0xD800) && (value <= 0xDFFF)));
                // make sure value does not go beyond U+10FFFF (as per RFC 3629)
                is_valid = is_valid && (value <= 0x10FFFF);

                if (!is_valid) {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
                    UNISTRINGXX_THROW(std::range_error, "Invalid octets detected while decoding UTF-8.");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
                    value = unichar::invalid_value;
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
                }
            }

            return (unichar{value});
        }

        static unichar from_utf16(char16_t utf16_ch, char16_t utf16_ls = u'\0') UNISTRINGXX_MAY_THROW_EXCEPTIONS
        {
            int_type value = unichar::invalid_value;

            if (utf16_ch < 0xD800 || utf16_ch > 0xDFFF) {
                value = utf16_ch;
            }
            else {
                if ((utf16_ch >= 0xD800 && utf16_ch <= 0xDBFF) && (utf16_ls >= 0xDC00 && utf16_ls <= 0xDFFF)) {
                    // decode surrogate pairs
                    value = (((utf16_ch - 0xD800) << 0x0A) | (utf16_ls - 0xDC00)) + 0x10000;
                    // since the result of surrogate pair decoding will be in the range:
                    // 0x010000 - 0x10FFFF
                    // there is no need to explicitly test whether this goes beyond these ranges.
                }
                else {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
                    UNISTRINGXX_THROW(std::range_error, "Invalid surrogate pairs.");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
                    value = unichar::invalid_value;
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
                }
            }

            return (unichar{value});
        }

        static unichar from_utf32(char32_t utf32_ch) UNISTRINGXX_MAY_THROW_EXCEPTIONS
        {
            bool is_valid = true;
            int_type value = static_cast<int_type>(utf32_ch);

            if (value != unichar::invalid_value) {
                // Make sure value is not within UTF-16 surrogates (as per RFC 3629).
                is_valid = !((value >= 0xD800) && (value <= 0xDFFF));

                // Make sure value does not go beyond U+10FFFF (as per RFC 3629).
                is_valid = (is_valid && (value <= 0x10FFFF));
            }

            if (!is_valid) {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
                UNISTRINGXX_THROW(std::range_error, "Invalid Unicode code points detected while decoding UTF-32.");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
                value = unichar::invalid_value;
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
            }

            return (unichar{value});
        }

        bool is_valid(void) const
        {
            bool is_valid = this->code_point() != unichar::invalid_value;
            if (is_valid) {
                // Further validation of correct code points.
                int_type value = this->code_point();

                // Make sure value is not within UTF-16 surrogates (as per RFC 3629).
                is_valid = !((value >= 0xD800) && (value <= 0xDFFF));

                // Make sure value does not go beyond U+10FFFF (as per RFC 3629).
                is_valid = (is_valid && (value <= 0x10FFFF));
            }
            return (is_valid);
        }

        operator bool(void) const
        {
            // returns whether the unicode is valid or not
            return (is_valid());
        }

        int_type code_point(void) const
        {
            return (static_cast<int_type>(_impl));
        }

        // note: relies on RVO
        std::vector<char8_t> to_utf8(void) const
        {
            typedef std::vector<char8_t> return_type;
            auto cp_value = this->code_point();

            if (!this->is_valid()) {
                return (return_type());
            }

            if (cp_value < 0x80) {
                return (return_type{static_cast<char8_t>(cp_value)});
            }

            return_type result;
            std::size_t num_seq = 2;
            char8_t head_seq;

            if (cp_value < 0x0800) { num_seq = 2; head_seq = 0xC0; }
            else if (cp_value < 0x10000) { num_seq = 3; head_seq = 0xE0; }
            else if (cp_value < 0x110000) { num_seq = 4; head_seq = 0xF0; }

            bool is_head_seq = true;

            do {
                char8_t seq = static_cast<char8_t>((cp_value >> ((num_seq - 1) * 6)) & 0x3F);

                if (is_head_seq) {
                    // this is the head octet sequence
                    seq = head_seq | seq;
                    is_head_seq = false;
                }
                else { seq = seq | 0x80; }

                result.push_back(seq);

                num_seq--;
            }
            while (num_seq > 0);

            return (result);
        }

        // note: relies on RVO
        std::vector<char16_t> to_utf16(void) const
        {
            typedef std::vector<char16_t> return_type;
            auto cp_value = this->code_point();

            if (!this->is_valid()) {
                return (return_type());
            }

            if (cp_value <= 0xFFFF) {
                return (return_type{static_cast<char16_t>(cp_value)});
            }

            cp_value -= 0x10000;

            return (return_type{
                static_cast<char16_t>(0xD800 | ((cp_value >> 0x0A) & 0x03FF)),
                static_cast<char16_t>(0xDC00 | ((cp_value >> 0x00) & 0x03FF))
            });
        }

        char32_t to_utf32(void) const
        {
            if (!this->is_valid()) {
                return (std::numeric_limits<char32_t>::max());
            }
            return (static_cast<char32_t>(this->code_point()));
        }

        // unichar should not be treated like native types where implicit casting can happen, thus these overload
        // operators must be explicitly defined
        bool operator==(const unichar& other) const
        { return (this->code_point() == other.code_point()); }

        bool operator!=(const unichar& other) const
        { return (!this->operator==(other)); }

        bool operator<(const unichar& other) const
        { return (this->code_point() < other.code_point()); }

        bool operator>(const unichar& other) const
        { return (this->code_point() > other.code_point()); }

        bool operator<=(const unichar& other) const
        { return (!(this->code_point() > other.code_point())); }

        bool operator>=(const unichar& other) const
        { return (!(this->code_point() < other.code_point())); }

        impl_type _impl;
    }; // struct unicode

    typedef unichar unichar_t;

    // Literal operators.
    // Overloaded in case future implementations/standard will force compilers to treat char16_t and char32_t as
    // native types like wchar_t.
    unichar_t operator "" _uc(char ch)
    {
        return (unichar_t::from_utf8(ch));
    }

    unichar_t operator "" _uc(char16_t ch16)
    {
        return (unichar_t::from_utf16(ch16));
    }

    unichar_t operator "" _uc(char32_t ch32)
    {
        return (unichar_t::from_utf32(ch32));
    }

    struct unichar_traits
    {
        typedef unichar_t char_type;
        typedef char_type::int_type int_type;
        typedef std::streamoff off_type;
        typedef std::streampos pos_type;
        typedef std::nullptr_t state_type;

        static void assign(char_type& dest, const char_type& src) noexcept
        {
            dest = src;
            return;
        }

        static char_type* assign(char_type* dest, std::size_t count, char_type ch)
        {
            std::fill_n(dest, count, ch);
            return (dest);
        }

        static /*constexpr*/ bool eq(char_type ch_a, char_type ch_b) noexcept
        {
            return (ch_a == ch_b);
        }

        static /*constexpr*/ bool lt(char_type ch_a, char_type ch_b)
        {
            return (ch_a < ch_b);
        }

        static char_type* move(char_type* dest, const char_type* src, std::size_t count)
        {
            std::copy(src, (src + count), dest);
            return (dest);
        }

        static char_type* copy(char_type* dest, const char_type* src, std::size_t count)
        {
            std::copy(src, (src + count), dest);
            return (dest);
        }

        static int compare(const char_type* ch_str_a, const char_type* ch_str_b, std::size_t count)
        {
            std::size_t itr = 0;
            for (itr = 0; itr < count; itr++) {
                int result = (*(ch_str_a + itr)).code_point() - (*(ch_str_b + itr)).code_point();
                if (result != 0)
                    return (result);
            }
            return (0);
        }

        static std::size_t length(const char_type* ch_str)
        {
            std::size_t itr = 0;
            unichar null_char;
            while (*(ch_str + itr) !=  null_char)
                itr++;
            return (itr);
        }

        static const char_type* find(const char_type* ch_str, std::size_t count, const char_type& ch)
        {
            std::size_t itr = 0;
            for (itr = 0; itr < count; itr++) {
                if (*(ch_str + itr) == ch)
                    return (ch_str + itr);
            }
            return (nullptr);
        }

        static /*constexpr*/ char_type to_char_type(int_type value) noexcept
        {
            return (char_type(static_cast<char32_t>(value)));
        }

        static /*constexpr*/ int_type to_int_type(char_type ch) noexcept
        {
            return (ch.code_point());
        }

        static /*constexpr*/ bool eq_int_type(int_type value_a, int_type value_b) noexcept
        {
            return (value_a == value_b);
        }

        static /*constexpr*/ int_type eof(void) noexcept
        {
            return (char_type::invalid_value);
        }

        static /*constexpr*/ int_type not_eof(int_type value) noexcept
        {
            // if value != eof, then return value
            // if value == eof, then return 0 (can't return eof because it will be equivalent to the previous predicate)
            return (value != unichar_traits::eof() ? value : 0);
        }
    }; // struct unichar_traits

    typedef unichar_traits unichar_t_traits;

} // namespace unistringxx

#endif // !defined(UNISTRINGXX_UNICHAR_HPP)
