#if !defined(UNISTRINGXX_UNICORE_HPP)
#define UNISTRINGXX_UNICORE_HPP

#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <limits>
#include <string>

#include <config.h>

#include "common.hpp"

namespace unistringxx
{
    ///
    /// Unistringxx version information.
    ///
    namespace version
    {
        ///
        /// The year segment of the version number.
        ///
        constexpr int year = UNISTRINGXX_VERSION_YEAR;

        ///
        /// The month segment of the version number.
        ///
        constexpr int month = UNISTRINGXX_VERSION_MONTH;

        ///
        /// The day segment of the version number.
        ///
        constexpr int day = UNISTRINGXX_VERSION_DAY;

        ///
        /// The build number for this version.
        ///
        constexpr int build = UNISTRINGXX_VERSION_BUILD;

        ///
        /// Gets the version information.
        /// @returns A string representing the version information.
        ///
        constexpr std::string str(void)
        {
            return (std::string(UNISTRINGXX_VERSION_STRING));
        }
    } // namespace version

    /// @internal
    namespace // ImplementationDetail
    {
        typedef enum _endian_mode
        {
            unknown_endian = 0,
            little_endian = 1,
            big_endian = 2,
            pdp_endian = 3
        }
        endian_mode;

        // endian check
        inline endian_mode get_endian_mode(void)
        {
            const std::uint32_t test = 0x22446688;
            const std::uint8_t* testptr = reinterpret_cast<const std::uint8_t*>(&test);
            return (
                (testptr[0] == 0x88) && (testptr[1] == 0x66) &&
                (testptr[2] == 0x44) && (testptr[3] == 0x22) ? little_endian : (
                    (testptr[0] == 0x22) && (testptr[1] == 0x44) &&
                    (testptr[2] == 0x66) && (testptr[3] == 0x88) ? big_endian : (
                        (testptr[0] == 0x44) && (testptr[1] == 0x22) &&
                        (testptr[2] == 0x88) && (testptr[3] == 0x66) ? pdp_endian :
                            unknown_endian
                    )
                )
            );
        }

        static const endian_mode PLATFORM_ENDIAN = get_endian_mode();

        // table lookup for index matching. only used when transforming a 32-bit data to 24-bit data.
        static const std::size_t _s_32bit_idx_lookup[4][3] =
        {
            { 0, 0, 0 }, // unknown endian
            { 0, 1, 2 }, // little endian
            { 3, 2, 1 }, // big endian
            { 2, 3, 0 } // pdp endian
        };

        static const std::size_t* _s_32bit_idx = _s_32bit_idx_lookup[PLATFORM_ENDIAN];
    } // namespace // ImplementationDetail
    /// @endinternal

    // Print and scan formats: use 32-bit's formats.
    #define PRId24 PRId32
    #define PRIi24 PRIi32
    #define PRIo24 PRIo32
    #define PRIu24 PRIu32
    #define PRIx24 PRIx32
    #define PRIX24 PRIX32
    #define SCNd24 SCNd32
    #define SCNi24 SCNi32
    #define SCNo24 SCNo32
    #define SCNu24 SCNu32
    #define SCNx24 SCNx32

    #define UINT24_MIN 0
    #define UINT24_MAX 16777215U // 0xFFFFFF

    ///
    /// Non-standard 24-bit data type.
    ///
    struct uint24_t
    {
        ///
        /// Creates a 24-bit data with a default value.
        ///
        uint24_t(void)
        {
            std::fill_n(std::begin(data), 3, 0x00);
        }

        ///
        /// Creates a 24-bit data with a provided value.
        /// @param value The value to initialize the 24-bit data to.
        ///
        uint24_t(std::int_least32_t value)
        {
            auto valueItr = reinterpret_cast<std::uint8_t*>(&value);

            // not endian agnostic (code is OK with little endian, but not others)
            // std::copy(valueItr, (valueItr + 3), std::begin(data));

            data[0] = *(valueItr + _s_32bit_idx[0]);
            data[1] = *(valueItr + _s_32bit_idx[1]);
            data[2] = *(valueItr + _s_32bit_idx[2]);

            return;
        }

        ///
        /// Conversion operator to std::int_least32_t.
        /// @returns The std::int_least32_t representation of this data.
        ///
        operator std::int_least32_t(void) const
        {
            // implementation is little endian, so there is no need for the lookup table.
            return ((data[0] << 0x00) | (data[1] << 0x08) | (data[2] << 0x10));
        }

        /// @internal
        // Implemented as array to be contiguous. Internally, the data is represented in little endian fashion
        // regardless of the system's endian mode.
        // i.e. data[0] is the LSB and data[2] is the MSB.
        // Also, we do not need the features provided by std::array so plain old C-style array works.
        // std::bitset<24> is another option, but common implementation for this type stores a pointer to
        // implementation, thus at minimum, memory used by std::bitset<24> is >= (24 + (sizeof (void*) * 8))
        std::uint8_t data[3];
        /// @endinternal
    }; // struct uint24_t

    ///
    /// Adds and assigns the sum.
    /// @param left The 1st value to add. This will also be updated with the sum.
    /// @param right The 2nd value to add.
    /// @returns The sum of the addition (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator+=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left + right));
    }

    ///
    /// Subtracts and assigns the result.
    /// @param left The minuend. This will also be updated with the difference.
    /// @param right The subtrahend.
    /// @returns The difference of the subtraction (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator-=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left - right));
    }

    ///
    /// Multiplies and assigns the result.
    /// @param left The 1st factor. This will also be updated with the product.
    /// @param right The 2nd factor.
    /// @returns The product of the multiplication (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator*=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left * right));
    }

    ///
    /// Divides and assigns the result.
    /// @param left The dividend. This will also be updated with the quotient.
    /// @param right The divisor.
    /// @returns The quotient of the division (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator/=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left / right));
    }

    ///
    /// Performs modulus and assigns the result.
    /// @param left The dividend. This will also be updated with the remainder.
    /// @param right The divisor.
    /// @returns The remainder of the division (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator%=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left % right));
    }

    ///
    /// Performs bit-wise AND operation and assigns the result.
    /// @param left The 1st value to perform bit-wise AND with. This will also be updated with the result.
    /// @param right The 2nd value to perform bit-wise AND with.
    /// @returns The result of the bit-wise AND operation (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator&=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left & right));
    }

    ///
    /// Performs bit-wise OR operation and assigns the result.
    /// @param left The 1st value to perform bit-wise OR with. This will also be updated with the result.
    /// @param right The 2nd value to perform bit-wise OR with.
    /// @returns The result of the bit-wise OR operation (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator|=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left | right));
    }

    ///
    /// Performs bit-wise XOR operation and assigns the result.
    /// @param left The 1st value to perform bit-wise XOR with. This will also be updated with the result.
    /// @param right The 2nd value to perform bit-wise XOR with.
    /// @returns The result of the bit-wise XOR operation (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator^=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left ^ right));
    }

    ///
    /// Performs bit-wise left shift operation and assigns the result.
    /// @param left The value to shift left. This will also be updated with the result.
    /// @param right The amount of shift to perform.
    /// @returns The result of the bit-wise left shift operation (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator<<=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left << right));
    }

    ///
    /// Performs bit-wise right shift operation and assigns the result.
    /// @param left The value to shift right. This will also be updated with the result.
    /// @param right The amount of shift to perform.
    /// @returns The result of the bit-wise right shift operation (which is actually the 'left' parameter).
    ///
    inline uint24_t& operator>>=(uint24_t& left, const uint24_t& right)
    {
        return (left = (left >> right));
    }

/*
    // These operators do not need to be overloaded due to possible implicit promotion of uint24_t to int type.
    inline uint24_t operator+(const uint24_t& left, const uint24_t& right);
    inline uint24_t operator-(const uint24_t& left, const uint24_t& right);
    inline uint24_t operator*(const uint24_t& left, const uint24_t& right);
    inline uint24_t operator/(const uint24_t& left, const uint24_t& right);
    inline uint24_t operator%(const uint24_t& left, const uint24_t& right);

    inline bool operator==(const uint24_t& left, const uint24_t& right)
    inline bool operator!=(const uint24_t& left, const uint24_t& right);
    inline bool operator<(const uint24_t& left, const uint24_t& right);
    inline bool operator<=(const uint24_t& left, const uint24_t& right);
    inline bool operator>(const uint24_t& left, const uint24_t& right);
    inline bool operator>=(const uint24_t& left, const uint24_t& right);
*/

    ///
    /// Literal operator for creating uint24_t type.
    /// @param value The value of the literal.
    /// @returns A uint24_t type based on the literal value.
    ///
    uint24_t operator "" _u24(unsigned long long int value)
    {
        return (uint24_t(static_cast<std::int_least32_t>(value)));
    }

    ///
    /// Literal operator for creating uint24_t type.
    /// @param value The value of the literal.
    /// @returns A uint24_t type based on the literal value.
    ///
    uint24_t operator "" _U24(unsigned long long int value)
    {
        return (uint24_t(static_cast<std::int_least32_t>(value)));
    }

    // uint24_t operators
} // namespace unistringxx

#endif // !defined(UNISTRINGXX_UNICORE_HPP)
