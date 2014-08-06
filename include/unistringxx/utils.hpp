#if !defined(UNISTRINGXX_UTILS_HPP)
#define UNISTRINGXX_UTILS_HPP

#if (UNISTRINGXX_WITH_EXCEPTIONS)
#include <sstream>
#include <stdexcept>
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

#include "common.hpp"

namespace unistringxx
{
#if (UNISTRINGXX_WITH_EXCEPTIONS)
    ///
    /// Utility function for throwing exception with details like file, function, and line number.
    /// @param message
    /// @param file_name
    /// @param function_name
    /// @param line_number
    ///
    template<typename EXCEPTION_T>
#if (UNISTRINGXX_DEBUG)
    inline void throw_exception(
        const char* message, const char* file_name, const char* function_name, std::size_t line_number
    )
#else // (UNISTRINGXX_DEBUG)
    inline void throw_exception(const char* message)
#endif // (UNISTRINGXX_DEBUG)
    {
        std::stringstream ss;
        ss << message;
#if (UNISTRINGXX_DEBUG)
        ss << "\n\tFile: " << file_name << 
            "\n\tFunction: " << function_name <<
            "\n\tLine: " << line_number;
#endif // (UNISTRINGXX_DEBUG)
        throw (EXCEPTION_T(ss.str().c_str()));
        return;
    }

#if (UNISTRINGXX_DEBUG)
    #define UNISTRINGXX_THROW(EXCEPTION_T, message) \
        throw_exception<EXCEPTION_T>(message, __FILE__, __FUNCTION__, __LINE__)
#else // (UNISTRINGXX_DEBUG)
    #define UNISTRINGXX_THROW(EXCEPTION_T, message) throw_exception<EXCEPTION_T>(message)
#endif // (UNISTRINGXX_DEBUG)

#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

    // Checks whether a specified bit field of a value is set.
    template<typename T>
    inline bool is_bit_set(T value, std::size_t bit_index) UNISTRINGXX_MAY_THROW_EXCEPTIONS
    {
        if (bit_index > ((sizeof (T) * 8)- 1)) {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            UNISTRINGXX_THROW(std::out_of_range, "Bit index out of range.");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
            return (false);
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
        }
        return (((value >> bit_index) & 0x01) == 0x01);
    }
} // namespace unistringxx

#endif // !defined(UNISTRINGXX_UTILS_HPP)
