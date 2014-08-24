#include <array>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <unistringxx/unistring.hpp>

using namespace unistringxx;

// Note:
// Some tests are not performed because they are forwarding functions. They are on the TOTEST list.

TEST(unistring_test, construction)
{
    // Putting tests in scoped blocks to clean up as soon as not needed.
    {
        unistring ustr_a;
        const auto& impl_data_a = ustr_a.get_impl();

        // null pointer must be added in the impl
        ASSERT_EQ(static_cast<decltype(impl_data_a.size())>(1), impl_data_a.size());
        auto char_value = impl_data_a[0];
        ASSERT_EQ(static_cast<decltype(char_value.code_point())>(0), char_value.code_point());
    }

    {
        // unistring ustr_b{8, u'\u3042'_uc}; // cannot do this, it becomes an initializer list.
        unistring ustr_b{8, u'\u3042'_uc, unistring::allocator_type()};
        const auto& impl_data_b = ustr_b.get_impl();
        // null pointer must be added in the impl
        ASSERT_EQ(static_cast<decltype(impl_data_b.size())>(9), impl_data_b.size());
        ASSERT_EQ(static_cast<decltype(impl_data_b[0])>(0x3042), impl_data_b[0]);
        ASSERT_EQ(static_cast<decltype(impl_data_b[7])>(0x3042), impl_data_b[7]);
        ASSERT_EQ(static_cast<decltype(impl_data_b[8])>(0x0000), impl_data_b[8]);

        unistring ustr_c{ustr_b, 0};
        const auto& impl_data_c = ustr_c.get_impl();
        ASSERT_EQ(impl_data_b.size(), impl_data_c.size());
        ASSERT_EQ(0, unistring::traits_type::compare(impl_data_c.data(), impl_data_b.data(), impl_data_b.size()));

        unistring ustr_d{ustr_b, 4};
        const auto& impl_data_d = ustr_d.get_impl();
        ASSERT_EQ(static_cast<decltype(impl_data_d.size())>(5), impl_data_d.size());
        ASSERT_EQ(impl_data_b[4], impl_data_d[0]);
        ASSERT_EQ(impl_data_b[8], impl_data_d[4]);

        // count is specific
        unistring ustr_e{ustr_b, 3, 5};
        const auto& impl_data_e = ustr_e.get_impl();
        // null pointer must be added in the impl
        ASSERT_EQ(static_cast<decltype(impl_data_e.size())>(6), impl_data_e.size());
        ASSERT_EQ(impl_data_b[3], impl_data_e[0]);
        ASSERT_EQ(impl_data_b[7], impl_data_e[4]);
        ASSERT_EQ(static_cast<decltype(impl_data_e[5])>(0x0000), impl_data_e[5]);

        // count is greater than size() (must copy size() items only)
        unistring ustr_f{ustr_b, 6, (ustr_b.size() + 10)};
        const auto& impl_data_f = ustr_f.get_impl();
        ASSERT_EQ(static_cast<decltype(impl_data_f.size())>(3), impl_data_f.size());

        // 0 count
        unistring ustr_g{ustr_b, 2, 0};
        const auto& impl_data_g = ustr_g.get_impl();
        ASSERT_EQ(static_cast<decltype(impl_data_g.size())>(1), impl_data_g.size());
    }

    {
        unistring ustr_h{4, U'\U0010FFFF', unistring::allocator_type()};
#if (UNISTRINGXX_WITH_EXCEPTIONS)
        EXPECT_THROW(unistring(ustr_h, (ustr_h.size() + 4)), std::out_of_range);
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
        unistring ustr_i{ustr_h, (ustr_h.size() + 4)};
        EXPECT_EQ(static_cast<decltype(ustr_i.size())>(0), ustr_i.size());
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
    }

    {
        // A C-style array to mimic C-style string.
        const unistring::char_type uc_str_a[] = {
            'A', u'\u3042', 'B'_uc, u'\u3043', '\0'_uc
        };
        const unistring::size_type uc_str_a_len = unistring::traits_type::length(uc_str_a);

        unistring ustr_j{uc_str_a, 0};
        ASSERT_EQ(static_cast<decltype(ustr_j.size())>(0), ustr_j.size());

        unistring ustr_k{uc_str_a, 2};
        ASSERT_EQ(static_cast<decltype(ustr_k.size())>(2), ustr_k.size());
        const auto& impl_data_k = ustr_k.get_impl();
        ASSERT_EQ(uc_str_a[0], impl_data_k[0]);
        ASSERT_EQ(uc_str_a[1], impl_data_k[1]);
        ASSERT_EQ(static_cast<decltype(impl_data_k[2])>(0x0000), impl_data_k[2]);

        unistring ustr_l{uc_str_a};
        ASSERT_EQ(uc_str_a_len, ustr_l.size());
    }

    {
        std::vector<unistring::char_type> uc_vector_a = {
            'A', u'\u3042', 'B'_uc, u'\u3043'
        };

        unistring ustr_m{uc_vector_a.begin(), uc_vector_a.end()};
        ASSERT_EQ(uc_vector_a.size(), ustr_m.size());
        const auto& impl_data_m = ustr_m.get_impl();
        ASSERT_EQ(0, unistring::traits_type::compare(impl_data_m.data(), uc_vector_a.data(), uc_vector_a.size()));
        ASSERT_EQ(static_cast<decltype(impl_data_m[4])>(0x0000), impl_data_m[4]);
    }

    // Copy/Move constructors
    {
        unistring::char_type uc_str_a[] = {
            'A', u'\u3042', 'B'_uc, u'\u3043', '\0'_uc
        };
        unistring ustr_n{uc_str_a};
        unistring ustr_o{ustr_n};
        const auto& impl_data_n = ustr_n.get_impl();
        const auto& impl_data_o = ustr_o.get_impl();
        ASSERT_EQ(0, unistring::traits_type::compare(impl_data_o.data(), impl_data_n.data(), impl_data_n.size()));

        unistring ustr_p{std::move(ustr_o)};
        const auto& impl_data_p = ustr_p.get_impl();
        ASSERT_EQ(0, unistring::traits_type::compare(impl_data_p.data(), impl_data_n.data(), impl_data_n.size()));
    }

    // Initializer list
    {
        unistring ustr_q{{ 'A', u'\u3042', 'B'_uc, u'\u3043' }};
        const auto& impl_data_q = ustr_q.get_impl();
        ASSERT_EQ(static_cast<decltype(impl_data_q.size())>(5), impl_data_q.size());
    }

    return;
}

TEST(unistring_test, custom_allocator)
{
    // A simple test when using a custom allocator. The test simply checks whether the count is reset (i.e. allocate and
    // deallocate functions are called).

    typedef std::allocator<unichar_t> my_allocator_base;
    struct my_allocator : public my_allocator_base
    {
        typedef typename my_allocator_base::size_type size_type;
        typedef typename my_allocator_base::pointer pointer;

        size_type& alloc_count;

        my_allocator(size_type& arg) : my_allocator_base{}, alloc_count{arg}
        { return; }

        pointer allocate(size_type n, std::allocator<void>::const_pointer hint = 0)
        {
            alloc_count++;
            return (my_allocator_base::allocate(n, hint));
        }

        void deallocate(pointer p, size_type n)
        {
            alloc_count--;
            my_allocator_base::deallocate(p, n);
            return;
        }
    };

    typedef generic_unistring<my_allocator> my_unistring;
    my_unistring::size_type count = 0;
    {
        my_unistring ustr_a{8, 'A', my_allocator{count}};
        ASSERT_EQ(static_cast<decltype(count)>(1), count);
    }
    ASSERT_EQ(static_cast<decltype(count)>(0), count);
    return;
}

TEST(unistring_test, properties)
{
    unistring ustr_a{{ 'A', u'\u3042', 'B'_uc, u'\u3043' }};
    std::size_t ustr_a_cstr_len = unistring::traits_type::length(ustr_a.c_str());
    ASSERT_EQ(static_cast<std::size_t>(4), ustr_a_cstr_len);

    unistring ustr_b{ustr_a.substr(0, 2)};
    std::size_t ustr_b_cstr_len = unistring::traits_type::length(ustr_b.c_str());
    ASSERT_EQ(static_cast<std::size_t>(2), ustr_b_cstr_len);
    const auto& impl_data_b = ustr_b.get_impl();
    ASSERT_EQ(static_cast<decltype(impl_data_b[0])>('A'), impl_data_b[0]);
    ASSERT_EQ(static_cast<decltype(impl_data_b[1])>(0x3042), impl_data_b[1]);

    return;
}

TEST(unistring_test, comparison)
{
    // unistring vs. unistring comparisons
    unistring ustr_a{{ 'A', u'\u3042', 'B'_uc, u'\u3043' }};
    unistring ustr_b{ustr_a};
    ASSERT_TRUE(ustr_a == ustr_b);
    ASSERT_TRUE(ustr_a >= ustr_b);

    unistring ustr_c;
    ASSERT_FALSE(ustr_a == ustr_c);
    ASSERT_TRUE(ustr_b > ustr_c);

    unistring ustr_d{{ 'A', u'\u3043', 'B'_uc, u'\u3044' }};
    ASSERT_TRUE(ustr_a != ustr_d);
    ASSERT_TRUE(ustr_a < ustr_d);
    ASSERT_TRUE(ustr_d >= ustr_b);
    ASSERT_TRUE(ustr_b <= ustr_d);

    // unistring vs. char_type* string comparisons
    const unistring::char_type uc_str_a[] = {
        'A', u'\u3042', 'B'_uc, u'\u3043', '\0'_uc
    };
    const unistring::char_type uc_str_b[] = {
        'A', u'\u3043', 'B'_uc, u'\u3044', '\0'_uc
    };
    ASSERT_TRUE(uc_str_a == ustr_a);
    ASSERT_TRUE(uc_str_b != ustr_a);
    ASSERT_FALSE(ustr_d == uc_str_a);
    ASSERT_FALSE(ustr_d != uc_str_b);
    ASSERT_FALSE(uc_str_a == ustr_c);
    ASSERT_TRUE(ustr_c != uc_str_a);

    ASSERT_TRUE(uc_str_a < ustr_d);
    ASSERT_TRUE(uc_str_a <= ustr_d);
    ASSERT_FALSE(ustr_d < uc_str_a);
    ASSERT_TRUE(uc_str_a > ustr_c);
    ASSERT_TRUE(ustr_c < uc_str_a);
    ASSERT_TRUE(ustr_c <= uc_str_a);
    ASSERT_FALSE(uc_str_b < ustr_d);
    ASSERT_FALSE(ustr_d > uc_str_b);
    ASSERT_TRUE(ustr_d >= uc_str_b);

    // using compare function
    ASSERT_EQ(0, ustr_a.compare(ustr_b));
    ASSERT_GT(ustr_a.compare(ustr_c), 0);
    ASSERT_LT(ustr_c.compare(ustr_a), 0);
    ASSERT_GT(ustr_d.compare(ustr_b), 0);

    unistring ustr_e{{ 'A', u'\u3042', 'B'_uc, u'\u3043', 'C'_uc }};
    unistring ustr_f{{ 'A', u'\u3042', 'B'_uc }};
    ASSERT_GT(ustr_e.compare(ustr_f), 0);
    ASSERT_LT(ustr_a.compare(ustr_e), 0);
    ASSERT_GT(ustr_a.compare(ustr_f), 0);

    // TOTEST: Other compare versions are just calling compare function from a substring.

    return;
}

// All tests from this point forward relies on the success of the comparison operators to validate results.

TEST(unistring_test, assignment)
{
    unistring ustr_a{{ 'A', u'\u3042', 'B'_uc, u'\u3043' }};
    unistring ustr_b;
    ASSERT_FALSE(ustr_a == ustr_b);
    ustr_b = ustr_a;
    ASSERT_TRUE(ustr_a == ustr_b);

    unistring ustr_c;
    ustr_c = std::move(ustr_b);
    ASSERT_TRUE(ustr_a == ustr_c);

    const unistring::char_type uc_str_a[] = {
        'A', u'\u3042', 'B'_uc, u'\u3043', '\0'_uc
    };
    unistring ustr_d;
    ustr_d = uc_str_a;
    ASSERT_TRUE(ustr_a == ustr_d);

    ustr_d = U'\U0010FFFF'_uc;
    ASSERT_EQ(static_cast<decltype(ustr_d.size())>(1), ustr_d.size());

    ustr_b = std::initializer_list<unistring::char_type>{ 'A', u'\u3042', 'B'_uc, u'\u3043' };
    ASSERT_TRUE(ustr_a == ustr_b);

    // assign is implemented as forwarding calls to operator= and copy/move ctor. there is no need to extensively test
    // all cases
    ustr_c.assign(4, u'\uFFFF'_uc);
    ASSERT_EQ(static_cast<decltype(ustr_c.size())>(4), ustr_c.size());

    ustr_b.assign(ustr_c);
    ASSERT_TRUE(ustr_b == ustr_c);

    return;
}

TEST(unistring_test, iterators)
{
    unistring ustr_a{{ '0', '2', '4', '6' }};
    size_t iterate_count = 0;
    for (auto itr = ustr_a.cbegin(); itr != ustr_a.cend(); itr++) {
        iterate_count++;
    }
    ASSERT_EQ(static_cast<size_t>(4), iterate_count);

    unistring ustr_b{ustr_a.cbegin(), ustr_a.cend()};;
    ASSERT_EQ(ustr_a, ustr_b);

    unistring ustr_c{{ '6', '4', '2', '0' }};
    unistring ustr_d{ustr_a.crbegin(), ustr_a.crend()};
    ASSERT_EQ(ustr_c, ustr_d);

    return;
}

TEST(unistring_test, sizes)
{
    unistring ustr_a;
    ASSERT_TRUE(ustr_a.empty());

    ustr_a = unistring{{'A'}};
    ASSERT_FALSE(ustr_a.empty());

    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(1), ustr_a.size());
    ustr_a = unistring{{ 'A', 'B', 'C', 'D' }};
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(4), ustr_a.size());

    ustr_a.clear();
    ASSERT_TRUE(ustr_a.empty());


    unistring ustr_b = ustr_a;
    ustr_b.resize(4);
    ASSERT_EQ(static_cast<decltype(ustr_b.length())>(4), ustr_b.length());

    ustr_b.resize(0);
    ASSERT_TRUE(ustr_b.empty());

    ustr_b.resize(4);
    ASSERT_EQ(static_cast<decltype(ustr_b.length())>(4), ustr_b.length());

    ustr_b.resize(8, 'C');
    ASSERT_EQ(static_cast<decltype(ustr_b.length())>(8), ustr_b.length());
    for (unistring::size_type i = 0; i < ustr_b.size(); i++) {
        if (i < 4) {
            ASSERT_EQ('\0'_uc, ustr_b[i]);
        }
        else {
            ASSERT_EQ('C'_uc, ustr_b[i]);
        }
    }

    return;
}

TEST(unistring_test, element_access)
{
    unistring ustr_a{{ 'A'_uc, 'B'_uc, 'C'_uc, 'D'_uc }};
    ASSERT_EQ(('A'_uc).code_point(), ustr_a.at(0).code_point());

    ustr_a.at(1) = 'b'_uc;
    ASSERT_EQ(('b'_uc).code_point(), ustr_a.at(1).code_point());    

    ustr_a.at(0) = ustr_a.at(1);
    ASSERT_TRUE(ustr_a.at(0) == ustr_a.at(1));

    ustr_a[2] = 'X'_uc;
    ASSERT_EQ(('X'_uc).code_point(), ustr_a[2].code_point());

    ASSERT_EQ(('b'_uc).code_point(), ustr_a.front().code_point());

    ustr_a[3] = 'Z'_uc;
    ASSERT_EQ(('Z'_uc).code_point(), ustr_a.back().code_point());
    ustr_a.back() = '1'_uc;
    ASSERT_EQ('1'_uc, ustr_a[3]);

    const unistring::char_type* cstr = ustr_a.c_str();
    ASSERT_EQ(static_cast<decltype(unistring::traits_type::length(cstr))>(4), unistring::traits_type::length(cstr));

    return;
}

TEST(unistring_test, insertions)
{
    unistring ustr_a;
    ustr_a.insert(ustr_a.size(), 4, 'A'_uc);
    ASSERT_EQ(static_cast<decltype(ustr_a.length())>(4), ustr_a.length());
    for (unistring::size_type ctr = 0; ctr < ustr_a.length(); ctr++) {
        ASSERT_EQ('A'_uc, ustr_a[ctr]);
    }

    ustr_a.insert(ustr_a.size(), 4, 'B'_uc);
    ASSERT_EQ(static_cast<decltype(ustr_a.length())>(8), ustr_a.length());
    for (unistring::size_type ctr = 4; ctr < ustr_a.length(); ctr++) {
        ASSERT_EQ('B'_uc, ustr_a[ctr]);
    }

    ustr_a.insert(4, 4, 'C'_uc);
    ASSERT_EQ(static_cast<decltype(ustr_a.length())>(12), ustr_a.length());
    unistring ustr_b{{ 'A', 'A', 'A', 'A', 'C', 'C', 'C', 'C', 'B', 'B', 'B', 'B' }};
    ASSERT_EQ(ustr_b, ustr_a);

    const unistring::char_type uc_str_a[] = {
        'A', 'A', 'A', 'A', 'C', 'C', 'C', 'C', 'B', 'B', 'B', 'B', '\0'
    };
    unistring ustr_c;
    ustr_c.insert(0, uc_str_a, 4);
    ASSERT_EQ(static_cast<decltype(ustr_c.length())>(4), ustr_c.length());
    ustr_c.clear();
    ustr_c.insert(0, uc_str_a);
    ASSERT_EQ(ustr_b, ustr_c);

    const unistring::char_type uc_str_b[] = {
        'A', 'A', 'A', 'A', 'B', 'B', 'B', 'B', '\0'
    };
    ustr_b.clear();
    ustr_b.insert(0, &(uc_str_a[4]), 4);
    ustr_c.clear();
    ustr_c.insert(0, uc_str_b);
    ustr_c.insert(4, ustr_b);
    ASSERT_EQ(ustr_a, ustr_c);

    ustr_c.insert(ustr_c.cend(), '0');
    ASSERT_EQ('0'_uc, ustr_c[ustr_c.size() - 1]);

    return;
}

TEST(unistring_test, appending)
{
    unistring ustr_a{{ 'A', 'A', 'A', 'A' }};
    ustr_a += unistring{{ 'A', 'A', 'A', 'A' }};
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(8), ustr_a.size());

    const unistring::char_type uc_str_a[] = {
        'B'_uc, 'B'_uc, 'B'_uc, 'B'_uc, '\0'_uc
    };

    ustr_a += uc_str_a;
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(12), ustr_a.size());

    ustr_a += 'C'_uc;
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(13), ustr_a.size());

    ustr_a += { 'C'_uc, 'C'_uc, 'C'_uc };
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(16), ustr_a.size());

    for (unistring::size_type i = 0; i < ustr_a.size(); i++) {
        if (i < 8)
            ASSERT_EQ('A'_uc, ustr_a[i]);
        else if (i < 12)
            ASSERT_EQ('B'_uc, ustr_a[i]);
        else
            ASSERT_EQ('C'_uc, ustr_a[i]);
    }

    ustr_a.clear();
    ustr_a.append(unistring{{ '0', '0' }});
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(2), ustr_a.size());

    ustr_a.append(unistring{{ '1', '1', '0', '0', '2', '2' }}, 2, 2);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(4), ustr_a.size());

    const unistring::char_type uc_str_b[] = {
        '0'_uc, '0'_uc, '0'_uc, '1'_uc, '\0'_uc
    };
    const unistring::char_type uc_str_c[] = {
        '0'_uc, '\0'_uc
    };
    ustr_a.append(uc_str_b, 3);
    ustr_a.append(uc_str_c);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(8), ustr_a.size());

    ustr_a.append(4, '0'_uc);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(12), ustr_a.size());

    std::vector<unistring::char_type> uc_vector_a(4, '0'_uc);
    ustr_a.append(uc_vector_a.cbegin(), uc_vector_a.cend());
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(16), ustr_a.size());

    ustr_a.append({ '0'_uc, '0'_uc, '0'_uc, '0'_uc });
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(20), ustr_a.size());

    for (auto itr = ustr_a.cbegin(); itr != ustr_a.cend(); itr++) {
        ASSERT_EQ('0'_uc, *itr);
    }

    ustr_a.push_back('1'_uc);
    ASSERT_EQ('1'_uc, *(--ustr_a.cend()));

    return;
}

TEST(unistring_test, erase)
{
    unistring ustr_a{{ '0', '0', '0', '0', '9', '9', '9', '9' }};
    ustr_a.erase(0, 0);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(8), ustr_a.size());
    
    ustr_a.erase();
    ASSERT_TRUE(ustr_a.empty());

    ustr_a = unistring{{ '9', '9', '9', '9', '0', '0', '0', '0' }};
    ustr_a.erase(4);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(4), ustr_a.size());

#if (UNISTRINGXX_WITH_EXCEPTIONS)
    ASSERT_THROW(ustr_a.erase(1000, unistring::npos), std::out_of_range);
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

    ustr_a[1] = '8'_uc;
    auto itr_a = ustr_a.erase(ustr_a.cbegin());
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(3), ustr_a.size());
    ASSERT_EQ('8'_uc, *itr_a);
    ASSERT_EQ(ustr_a.end(), ustr_a.erase(ustr_a.cend()));

    ustr_a = unistring{{ '9', '9', '9', '9', '0', '0', '0', '0' }};
    itr_a = ustr_a.erase(ustr_a.cbegin(), ustr_a.cbegin() + 4);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(4), ustr_a.size());
    ASSERT_EQ('0'_uc, *itr_a);

    for (auto itr = ustr_a.cbegin(); itr != ustr_a.cend(); itr++) {
        ASSERT_EQ('0'_uc, *itr);
    }
    itr_a = ustr_a.erase(ustr_a.cbegin(), ustr_a.cbegin() + 5);
    ASSERT_EQ(ustr_a.end(), itr_a);
    ASSERT_TRUE(ustr_a.empty());

    ustr_a.pop_back(); // Empty string, should not have any side effects.
    ASSERT_TRUE(ustr_a.empty());
    ustr_a.pop_back();
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(0), ustr_a.size());

    ustr_a = unistring{{ '0', '9' }};
    ustr_a.pop_back();
    ASSERT_EQ('0'_uc, *(--ustr_a.end()));
    ustr_a.pop_back();
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(0), ustr_a.size());
    ustr_a.pop_back();
    ASSERT_TRUE(ustr_a.empty());

    return;
}


TEST(unistring_test, replacing)
{
    unistring ustr_a{{ '0', '1', '2', '3', '4', '5', '6', '7' }};
    unistring ustr_b{{ '8', '8', '8', '8' }};
    
    ustr_a.replace(2, 2, ustr_b);
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(10), ustr_a.size());
    for (unistring::size_type ctr = 2; ctr < 5; ctr++) {
        ASSERT_EQ('8'_uc, ustr_a[ctr]);
    }

    ustr_a.replace(ustr_a.cbegin(), ustr_a.cend(), { 'A', 'A', 'A', 'A' });
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(4), ustr_a.size());
    for (auto& uc : ustr_a) {
        ASSERT_EQ('A'_uc, uc);
    }

    ustr_a.replace(ustr_a.cbegin() + 1, ustr_a.cbegin() + 3, ustr_b.cbegin(), ustr_b.cend());
    ASSERT_EQ(static_cast<decltype(ustr_a.size())>(6), ustr_a.size());
    for (unistring::size_type ctr = 2; ctr < 5; ctr++) {
        ASSERT_EQ('8'_uc, ustr_a[ctr]);
    }

    return;
}

TEST(unistring_test, copying)
{
    unistring ustr_a{{ '0', '1', '2', '3', '4', '5', '6', '7' }};
    const unistring::char_type uc_str_a[] = {
        '2'_uc, '3'_uc, '4'_uc, '5'_uc, '\0'_uc
    };
    unistring::char_type uc_arr[4];

#if (UNISTRINGXX_WITH_EXCEPTIONS)
    EXPECT_THROW(ustr_a.copy(uc_arr, 4, 10), std::out_of_range);
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

    unistring::size_type result = ustr_a.copy(uc_arr, 4, 2);
    ASSERT_EQ(static_cast<unistring::size_type>(4), result);
    ASSERT_EQ(0, unistring::traits_type::compare(uc_arr, uc_str_a, 4));

    return;
}

TEST(unistring_test, swapping)
{
    unistring ustr_a{{ '0', '0', '0', '0' }};
    unistring ustr_b{{ '1', '1', '1', '1' }};
    unistring ustr_c{{ '0', '0', '0', '0' }};
    unistring ustr_d{{ '1', '1', '1', '1' }};

    ustr_c.swap(ustr_d);

    ASSERT_EQ(ustr_a, ustr_d);
    ASSERT_EQ(ustr_b, ustr_c);

    return;
}

TEST(unistring_test, searching)
{
    // find function
    unistring ustr_a{{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }};
    unistring::size_type result = ustr_a.find(unistring{{ '7', '8', '9', 'A' }});
    const unistring::size_type npos_value = unistring::npos;
    ASSERT_EQ(npos_value, result);

    result = ustr_a.find(unistring{{ '6', '7', '8' }});
    ASSERT_EQ(static_cast<unistring::size_type>(6), result);

    ustr_a = unistring{{ '0', '1', '0', '1', '0', '1', '0' }};
    result = ustr_a.find(unistring{{ '1', '0'}});
    ASSERT_EQ(static_cast<unistring::size_type>(1), result);

    result = ustr_a.find('8'_uc);
    ASSERT_EQ(npos_value, result);

    // rfind function
    result = ustr_a.rfind(unistring{{ '1', '0'}});
    ASSERT_EQ(static_cast<unistring::size_type>(5), result);

    result = ustr_a.rfind('8'_uc);
    ASSERT_EQ(npos_value, result);

    ustr_a = unistring{{ '0', '1', '2', '3', '0', '1', '2', '3' }};

    // find_first_of function
    result = ustr_a.find_first_of(unistring{{ '3', '2' }});
    ASSERT_EQ(static_cast<unistring::size_type>(2), result);

    // find_last_of function
    result = ustr_a.find_last_of(unistring{{ '3', '2' }});
    ASSERT_EQ(static_cast<unistring::size_type>(7), result);

    // find_first_not_of function
    result = ustr_a.find_first_not_of(unistring{{ '3', '2' }});
    ASSERT_EQ(static_cast<unistring::size_type>(0), result);

    // find_last_not_of function
    result = ustr_a.find_last_not_of(unistring{{ '3', '2' }});
    ASSERT_EQ(static_cast<unistring::size_type>(5), result);

    return;
}

TEST(unistring_test, conversions)
{
    unistring ustr_a{{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }};
    unistring ustr_b{{ 'A', u'\u3042'_uc, 'B', U'\U0010FFFF'_uc }};
    unistring ustr_c;

    std::string u8str = ustr_a.to_u8string();
    for (std::string::size_type i = 0; i < u8str.size(); i++) {
        ASSERT_EQ(std::to_string(i)[0], u8str[i]);
    }

    u8str = ustr_b.to_u8string();
    ASSERT_EQ(static_cast<decltype(u8str.size())>(9), u8str.size());
    ASSERT_EQ('A', u8str[0]);
    ASSERT_EQ('\xE3', u8str[1]);
    ASSERT_EQ('\x81', u8str[2]);
    ASSERT_EQ('\x82', u8str[3]);
    ASSERT_EQ('B', u8str[4]);
    ASSERT_EQ('\xF4', u8str[5]);
    ASSERT_EQ('\x8F', u8str[6]);
    ASSERT_EQ('\xBF', u8str[7]);
    ASSERT_EQ('\xBF', u8str[8]);

    ustr_c = unistring::from_u8string(u8str);
    ASSERT_EQ(static_cast<decltype(ustr_c.size())>(4), ustr_c.size());
    ASSERT_EQ('A'_uc, ustr_c[0]);
    ASSERT_EQ(u'\u3042'_uc, ustr_c[1]);
    ASSERT_EQ('B'_uc, ustr_c[2]);
    ASSERT_EQ(U'\U0010FFFF'_uc, ustr_c[3]);

    std::u16string u16str = ustr_a.to_u16string();
    for (std::u16string::size_type i = 0; i < u16str.size(); i++) {
        ASSERT_EQ(static_cast<char16_t>(std::to_string(i)[0]), u16str[i]);
    }

    u16str = ustr_b.to_u16string();
    ASSERT_EQ(static_cast<decltype(u16str.size())>(5), u16str.size());
    ASSERT_EQ(u'A', u16str[0]);
    ASSERT_EQ(u'\u3042', u16str[1]);
    ASSERT_EQ(u'B', u16str[2]);
    ASSERT_EQ(u'\xDBFF', u16str[3]);
    ASSERT_EQ(u'\xDFFF', u16str[4]);

    ustr_c = unistring::from_u16string(u16str);
    ASSERT_EQ(static_cast<decltype(ustr_c.size())>(4), ustr_c.size());
    ASSERT_EQ('A'_uc, ustr_c[0]);
    ASSERT_EQ(u'\u3042'_uc, ustr_c[1]);
    ASSERT_EQ('B'_uc, ustr_c[2]);
    ASSERT_EQ(U'\U0010FFFF'_uc, ustr_c[3]);

    std::u32string u32str = ustr_a.to_u32string();
    for (std::u32string::size_type i = 0; i < u32str.size(); i++) {
        ASSERT_EQ(static_cast<char32_t>(std::to_string(i)[0]), u32str[i]);
    }

    u32str = ustr_b.to_u32string();
    ASSERT_EQ(static_cast<decltype(u32str.size())>(4), u32str.size());
    ASSERT_EQ(U'A', u32str[0]);
    ASSERT_EQ(U'\u3042', u32str[1]);
    ASSERT_EQ(U'B', u32str[2]);
    ASSERT_EQ(U'\U0010FFFF', u32str[3]);

    ustr_c = unistring::from_u32string(u32str);
    ASSERT_EQ(static_cast<decltype(ustr_c.size())>(4), ustr_c.size());
    ASSERT_EQ('A'_uc, ustr_c[0]);
    ASSERT_EQ(u'\u3042'_uc, ustr_c[1]);
    ASSERT_EQ('B'_uc, ustr_c[2]);
    ASSERT_EQ(U'\U0010FFFF'_uc, ustr_c[3]);

    ustr_a = unistring{{ '8', '8', 'C', 'C' }};

    int i_expected = 88;
    std::size_t s_end_index = unistring::npos;
    int i_result = unistringxx::stoi(ustr_a, &s_end_index, 10);
    ASSERT_EQ(static_cast<std::size_t>(2), s_end_index);
    ASSERT_EQ(i_expected, i_result);

    i_expected = 0x88CC;
    s_end_index = unistring::npos;
    i_result = unistringxx::stoi(ustr_a, &s_end_index, 16);
    ASSERT_EQ(static_cast<std::size_t>(4), s_end_index);
    ASSERT_EQ(i_expected, i_result);

    unsigned long long ull_expected = 88;
    s_end_index = unistring::npos;
    unsigned long long ull_result = unistringxx::stoull(ustr_a, &s_end_index, 10);
    ASSERT_EQ(static_cast<std::size_t>(2), s_end_index);
    ASSERT_EQ(i_expected, i_result);

    ull_expected = 0x88CC;
    s_end_index = unistring::npos;
    ull_result = unistringxx::stoull(ustr_a, &s_end_index, 16);
    ASSERT_EQ(static_cast<std::size_t>(4), s_end_index);
    ASSERT_EQ(ull_expected, ull_result);

    ustr_a = unistring{{ '8', '8' }};
    ustr_b = unistringxx::to_unistring(88);
    ASSERT_EQ(ustr_a, ustr_b);

    return;
}

TEST(unistring_test, literal_operators)
{
    const unistring ustr_a{{ 'A', 'B', u'\u3042', U'\U0010FFFF' }};
    unistring ustr_b;

    ustr_b = u8"AB\u3042\U0010FFFF"_us;
    ASSERT_EQ(ustr_a, ustr_b);

    ustr_b = u"AB\u3042\U0010FFFF"_us;
    ASSERT_EQ(ustr_a, ustr_b);

    ustr_b = U"AB\u3042\U0010FFFF"_us;
    ASSERT_EQ(ustr_a, ustr_b);

    return;
}

TEST(unistring_test, hashing)
{
    std::string str("ABCDEFGH");
    unistring ustr_a = unistring::from_u8string(str);
    std::hash<std::string> str_hash_function;
    std::hash<unistring> ustr_hash_function;
    const std::size_t
        result_a = str_hash_function(str),
        result_b = ustr_hash_function(ustr_a);
    ASSERT_EQ(result_a, result_b);
    return;
}

