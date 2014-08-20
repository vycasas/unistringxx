#if !defined(UNISTRINGXX_COMMON_HPP)
#define UNISTRINGXX_COMMON_HPP

#if !defined(__cplusplus)
    #error Non-C++ compiler detected. Cannot continue.
#elif (__cplusplus < 201103L)
    #error Non-C++11 compiler detected. Cannot continue.
#endif

#if defined(NDEBUG) || defined(_NDEBUG)
    #define UNISTRINGXX_DEBUG 0
#else
    #define UNISTRINGXX_DEBUG 1
#endif

#define COMPILER_HAS_UNICODE_SUPPORT 0
#if defined(__STDC_UTF_16__) && defined(__STDC_UTF_32__)
    #if (__STDC_UTF_16__ == 1) && (__STDC_UTF_32__ == 1)
        #undef COMPILER_HAS_UNICODE_SUPPORT
        #define COMPILER_HAS_UNICODE_SUPPORT 1
    #endif
#endif

#if !(COMPILER_HAS_UNICODE_SUPPORT)
#error Compiler does not use UTF-16/UTF-32 encodings for char16_t/char32_t types.
#endif

#if defined(__clang__)
    #if defined(__EXCEPTIONS)
        #define UNISTRINGXX_WITH_EXCEPTIONS 1
    #endif
#elif defined(__GNUC__)
    #if defined(__EXCEPTIONS)
        #define UNISTRINGXX_WITH_EXCEPTIONS 1
    #endif
#elif defined(_MSC_VER)
    #if defined(_CPPUNWIND)
        #define UNISTRINGXX_WITH_EXCEPTIONS 1
    #endif
#else 
    #error Untested/unsupported compiler detected. Cannot continue.
#endif

#if !defined(UNISTRINGXX_WITH_EXCEPTIONS)
    #define UNISTRINGXX_WITH_EXCEPTIONS 0
    #define UNISTRINGXX_MAY_THROW_EXCEPTIONS noexcept
#else
    #define UNISTRINGXX_MAY_THROW_EXCEPTIONS
#endif

#if !defined(UNISTRINGXX_TEST)
    #define UNISTRINGXX_TEST 0
#else
    #undef UNISTRINGXX_TEST
    #define UNISTRINGXX_TEST 1
    #undef UNISTRINGXX_UNICHAR_LITERALOP
    #undef UNISTRINGXX_UNISTRING_LITERALOP
#endif

#if !defined(UNISTRINGXX_UNICHAR_LITERALOP)
    #define UNISTRINGXX_UNICHAR_LITERALOP _uc
#endif

#if !defined(UNISTRINGXX_UNISTRING_LITERALOP)
    #define UNISTRINGXX_UNISTRING_LITERALOP _us
#endif

// Preprocessor helpers used by literal operations
#define UNISTRINGXX_PPCONCAT(A, B) A##B
#define UNISTRINGXX_CONCAT(A, B) UNISTRINGXX_PPCONCAT(A, B)

#endif // !defined(UNISTRINGXX_COMMON_HPP)
