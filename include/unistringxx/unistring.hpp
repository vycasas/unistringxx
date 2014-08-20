#if !defined(UNISTRINGXX_USTRING_HPP)
#define UNISTRINGXX_USTRING_HPP

#include <algorithm>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <memory>
#include <vector>

#include "unichar.hpp"
#include "utils.hpp"

#if !(UNISTRINGXX_TEST)
#error Remove these headers
#else
#include <iomanip>
#include <iostream>
#endif

namespace unistringxx
{
    // TODO: test propagating allocators...
    // Notes:
    // Implementation Design Pattern : Many of the fuctions are forwarding calls to other versions of the functions. In
    // essence, keep in mind that, unless there is a useful default implementation, all overloaded functions must end up
    // calling this iterator version, e.g. function(inputIterT first, inputIterT last). The reason for this is that it
    // allows for easy data passing to the underlying container implementation. Of course, it is still necessary to
    // perform exceptional cases whenever necessary. If a useful default implementation is provided, then all other
    // overloaded functions must end up calling that function (whether that useful default implementation is the
    // iterator version or not). Constructors are exempted from this requirement. Some functions may also be exempt from
    // this requirement if it makes sense.
    template<typename allocatorT>
    class generic_unistring
    {
    public:
        // I decided not to use std::deque because in some implementations (e.g. clang/libc++) it pre-allocates 4096
        // objects upon creation. std::vector has much more acceptable default allocation techniques.
        typedef std::vector<unichar_t_traits::char_type, allocatorT> impl_type;

        typedef unichar_t_traits traits_type;
        typedef typename traits_type::char_type char_type;
        typedef typename impl_type::value_type value_type;
        static_assert(sizeof (char_type) == sizeof (value_type), "char_type is not equal to value_type.");

        typedef typename impl_type::reference reference;
        typedef typename impl_type::const_reference const_reference;
        typedef typename impl_type::pointer pointer;
        typedef typename impl_type::const_pointer const_pointer;

        typedef typename impl_type::allocator_type allocator_type;
        typedef typename impl_type::size_type size_type;
        typedef typename impl_type::difference_type difference_type;
        typedef typename impl_type::iterator iterator;
        typedef typename impl_type::const_iterator const_iterator;
        typedef typename impl_type::reverse_iterator reverse_iterator;
        typedef typename impl_type::const_reverse_iterator const_reverse_iterator;

        static constexpr size_type npos = std::numeric_limits<size_type>::max();

        generic_unistring(void) :
            generic_unistring{allocatorT()}
        { return; }

        explicit generic_unistring(const allocator_type& alloc) :
            generic_unistring{0, char_type::null_char(), alloc}
        { return; }

        generic_unistring(const generic_unistring& other) noexcept :
            generic_unistring{other, other.get_allocator()}
        { return; }

        generic_unistring(const generic_unistring& other, const allocator_type& alloc) :
            _impl{other._impl, alloc}
        { return; }

        generic_unistring(generic_unistring&& other) noexcept :
            generic_unistring{other, other.get_allocator()}
        { return; }

        generic_unistring(generic_unistring&& other, const allocator_type& alloc) :
            _impl{other._impl, alloc}
        { return; }

        generic_unistring(
            const generic_unistring& other,
            size_type index,
            size_type count = generic_unistring::npos,
            const allocator_type& alloc = allocator_type()
        )
        {
            if (index > other.size()) {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
                UNISTRINGXX_THROW(std::out_of_range, "The 'index' argument is out of range.");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
                _impl = impl_type{1, char_type::null_char(), alloc};
                return;
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
            }

            const size_type actual_count = std::min(count, other.size() - index);

            _impl = impl_type{actual_count + 1, char_type::null_char(), alloc};
            if (actual_count > 0)
                std::copy_n(other._impl.cbegin() + index, actual_count, _impl.begin());
            return;
        }

        generic_unistring(const char_type* cstr, size_type count, const allocator_type& alloc = allocator_type())
        {
            if (count > _impl.max_size()) {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
                UNISTRINGXX_THROW(std::length_error, "The 'count' argument would have exceeded max_size().");
#else // (UNISTRINGXX_WITH_EXCEPTIONS)
                _impl = impl_type{1, char_type::null_char(), alloc};
                return;
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)
            }

            // note if cstr does not have at least count char_type elements, then it is an undefined behavior.
            _impl = impl_type{count + 1, char_type::null_char(), alloc};
            if (count > 0)
                traits_type::copy(_impl.data(), cstr, count);
            return;
        }

        generic_unistring(const char_type* cstr, const allocator_type& alloc = allocator_type()) :
            generic_unistring{cstr, traits_type::length(cstr), alloc}
        { return; }

        generic_unistring(size_type count, char_type ch, const allocator_type& alloc = allocator_type()) :
            _impl{(count + 1), ch, alloc}
        {
            if (ch != char_type::null_char())
                _impl[_impl.size() - 1] = char_type::null_char();
            return;
        }

        template<typename inputIterT>
        generic_unistring(inputIterT first, inputIterT last, const allocator_type& alloc = allocator_type()) :
            _impl{first, last, alloc}
        {
            _impl.push_back(char_type::null_char());
            _impl.shrink_to_fit();
            return;
        }

        generic_unistring(std::initializer_list<char_type> init_list, const allocator_type& alloc = allocator_type()) :
            _impl{init_list, alloc}
        { 
            _impl.push_back(char_type::null_char());
            _impl.shrink_to_fit();
            return;
        }

        ~generic_unistring(void) = default;

        // UNISTRINGXX_UNISTRING_BASIC_PROPERTIES

        allocator_type get_allocator(void) const
        { return (_impl.get_allocator()); }

        const_pointer data(void) const noexcept
        { return (_impl.data()); }

        const_pointer c_str(void) const noexcept
        { return (this->data()); }

        generic_unistring substr(size_type index = 0, size_type count = npos) const
        { return (generic_unistring{*this, index, count, this->get_allocator()}); }

        // UNISTRINGXX_UNISTRING_COMPARE

        int compare(const generic_unistring& str) const noexcept
        {
            const size_type compare_count = std::min(this->size(), str.size());
            // Note: The compare_count + 1 is done so that we compare the null characters. This provides the
            // needed comparison if compare returns 0 on two strings differing on length.
            return (traits_type::compare(this->c_str(), str.c_str(), compare_count + 1));
        }

        int compare(size_type index, size_type count, const generic_unistring& str) const
        { return (this->substr(index, count).compare(str)); }

        int compare(
            size_type index1, size_type count1,
            const generic_unistring& str, size_type index2, size_type count2 = npos
        ) const
        { return (this->substr(index1, count1).compare(str.substr(index2, count2))); }

        int compare(const char_type* cstr) const noexcept
        { return (this->compare(generic_unistring{cstr})); }

        int compare(size_type index, size_type count, const char_type* cstr) const
        { return (this->substr(index, count).compare(cstr)); }

        int compare(size_type index, size_type count, const char_type* cstr, size_type cstr_count) const
        { return (this->substr(index, count).compare(generic_unistring{cstr, cstr_count})); }

        // UNISTRINGXX_UNISTRING_ASSIGNMENT_OPERATIONS

        generic_unistring& operator=(const generic_unistring& str) = default;

        generic_unistring& operator=(generic_unistring&& str) /*noexcept*/ = default;

        generic_unistring& operator=(const char_type* cstr)
        {
            // Note:
            // I have profiled this code vs. the following version:
            //      _impl.clear();
            //      _impl.insert(...); // assuming arguments are calculated from cstr (or other).
            // and using std::move like below invokes 2000 instruction reads less than above.
            // memory wise, there are no difference. - Vincent
            // calls generic_unistring::operator=(generic_unistring&&)
            return ((*this) = std::move(generic_unistring{cstr, this->get_allocator()}));
        }

        generic_unistring& operator=(char_type ch)
        {
            // calls generic_unistring::operator=(generic_unistring&&)
            return ((*this) = std::move(generic_unistring{1, ch, this->get_allocator()}));
        }

        generic_unistring& operator=(std::initializer_list<char_type> init_list)
        {
            // calls generic_unistring::operator=(generic_unistring&&)
            return ((*this) = std::move(generic_unistring{init_list, this->get_allocator()}));
        }

        generic_unistring& assign(const generic_unistring& str)
        {
            // calls generic_unistring::operator=(generic_unistring&)
            return ((*this) = str);
        }

        generic_unistring& assign(generic_unistring&& str)
        {
            // calls generic_unistring::operator=(generic_unistring&&)
            return ((*this) = std::move(str));
        }

        generic_unistring& assign(const generic_unistring& str, size_type index, size_type count = npos)
        {
            // calls generic_unistring::assign(const generic_unistring&)
            return (this->assign(str.substr(index, count)));
        }

        generic_unistring& assign(const char_type* cstr, size_type count)
        {
            // calls generic_unistring::assign(const generic_unistring&)
            return (this->assign(generic_unistring{cstr, count, this->get_allocator()}));
        }

        generic_unistring& assign(const char_type* cstr)
        {
            // calls generic_unistring::assign(const char_type*, size_type)
            return (this->assign(cstr, traits_type::length(cstr)));
        }

        generic_unistring& assign(size_type count, char_type ch)
        {
            // calls generic_unistring::assign(const generic_unistring&)
            return (this->assign(generic_unistring{count, ch, this->get_allocator()}));
        }

        template<typename inputIterT>
        generic_unistring& assign(inputIterT first, inputIterT last)
        {
            // calls generic_unistring::assign(const generic_unistring&)
            return (this->assign(generic_unistring{first, last}));
        }

        generic_unistring& assign(std::initializer_list<char_type> init_list)
        {
            // calls generic_unistring::assign(inputIterT, inputIterT)
            return (this->assign(init_list.begin(), init_list.end()));
        }

        // UNISTRINGXX_UNISTRING_ITERATORS

        iterator begin(void) noexcept
        { return (_impl.begin()); }

        const_iterator begin(void) const noexcept
        { return (_impl.begin()); }

        const_iterator cbegin(void) const noexcept
        { return (_impl.cbegin()); }

        // Note: The operator-- is needed to hide the null pointer.
        iterator end(void) noexcept
        { return (--_impl.end()); }

        // Note: The operator-- is needed to hide the null pointer.
        const_iterator end(void) const noexcept
        { return (--_impl.end()); }

        // Note: The operator-- is needed to hide the null pointer.
        const_iterator cend(void) const noexcept
        { return (--_impl.cend()); }

        // Note: The operator++ is needed to hide the null pointer.
        reverse_iterator rbegin(void) noexcept
        { return (++_impl.rbegin()); }

        // Note: The operator++ is needed to hide the null pointer.
        const_reverse_iterator rbegin(void) const noexcept
        { return (++_impl.rbegin()); }

        // Note: The operator++ is needed to hide the null pointer.
        const_reverse_iterator crbegin(void) const noexcept
        { return (++_impl.crbegin()); }

        reverse_iterator rend(void) noexcept
        { return (_impl.rend()); }

        const_reverse_iterator rend(void) const noexcept
        { return (_impl.rend()); }

        const_reverse_iterator crend(void) const noexcept
        { return (_impl.crend()); }

        // UNISTRINGXX_UNISTRING_SIZES

        size_type size(void) const noexcept
        {
            // This does not work because a string can contain null characters anywhere in it.
            // traits_type::length counts the number of characters until the 1st null character.
            // return (traits_type::length(this->data()));
            return (_impl.size() - 1); // Don't count the null character.
        }

        size_type length(void) const noexcept
        { return (this->size()); }

        size_type max_size(void) const noexcept
        { return (_impl.max_size()); }

        void resize(size_type count, char_type ch)
        {
            if (count == this->size()) {
                return;
            }

            if (count < this->size()) {
                // calls generic_unistring::operator=(generic_unistring&&)
                (*this) = std::move(generic_unistring{*this, 0, count, this->get_allocator()});
            }
            else {
                // Note: null character is at the end of _impl. "this->cend()" returns the pos of this
                // null character.
                _impl.insert(this->cend(), (count - this->size()), ch);
            }
            return;
        }

        void resize(size_type count)
        {
            this->resize(count, char_type::null_char());
            return;
        }

        size_type capacity(void) const noexcept
        { return (_impl.capacity()); }

        void reserve(size_type reserve_amount = 0)
        {
            _impl.reserve(reserve_amount);
            return;
        }

        void shrink_to_fit(void)
        {
            _impl.shrink_to_fit();
            return;
        }

        void clear(void) noexcept
        {
            _impl.clear();
            _impl.push_back(char_type::null_char());
            return;
        }

        bool empty(void) const noexcept
        { return (this->size() == 0); }

        // UNISTRINGXX_UNISTRING_ELEMENT_ACCESS

        // Note:
        // I initially thought about following Effective C++ advice regarding const and non-const overload while
        // reducing code duplication, but I would rather not use const_cast. (libc++ does not follow this).
        // - Vincent

        // If index == size(), then this returns the null character as per standards.
        const_reference operator[](size_type index) const
        { return (_impl.operator[](index)); }

        // If index == size(), then this returns the null character as per standards.
        // Modifying the returned reference to null character, however, is an undefined behavior.
        reference operator[](size_type index)
        { return (_impl.operator[](index)); }

        const_reference at(size_type index) const
        { 
            if (index >= this->size()) {
                // See comments for the non-const version.
                return (_impl.at(index + 1));
            }
            return (_impl.at(index));
        }

        reference at(size_type index)
        {
            if (index >= this->size()) {
                // As per std::basic_string implementation, this should throw an exception, but because
                // _impl includes null character, we are just making sure that it throws an exception, thus index + 1 is
                // passed as the forwarding argument. Otherwise, _impl might return the null character.
                return (_impl.at(index + 1)); // what if index is SIZE_MAX?
            }
            return (_impl.at(index));
        }

        // TODO: Deal with (empty() == true) situations.
        const char_type& front(void) const
        { return (_impl.front()); }

        // TODO: Deal with (empty() == true) situations.
        char_type& front(void)
        { return (_impl.front()); }

        const char_type& back(void) const
        {
            if (this->empty())
                return (_impl.back());
            // return (_impl.at(_impl.size() - 2));
            return (this->operator[](this->size() - 1));
        }

        char_type& back(void)
        {
            if (this->empty())
                return (_impl.back());
            // return (_impl.at(_impl.size() - 2));
            return (this->operator[](this->size() - 1));
        }

        // UNISTRINGXX_UNISTRING_INSERTIONS

        generic_unistring& insert(size_type index, const generic_unistring& str)
        {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if (index > this->size())
                UNISTRINGXX_THROW(std::out_of_range, "The 'index' argument is out of range.");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            // calls generic_unistring::insert(const_iterator, inputIterT, inputIterT)
            this->insert(this->cbegin() + index, str.cbegin(), str.cend());
            return (*this);
        }

        generic_unistring& insert(
            size_type index,
            const generic_unistring& str,
            size_type str_index,
            size_type str_count = npos
        )
        {
            // calls generic_unistring::insert(size_type, const generic_unistring&)
            return (this->insert(index, str.substr(str_index, str_count)));
        }

        generic_unistring& insert(size_type index, const char_type* cstr, size_type count)
        {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if ((this->size() + count) > this->max_size())
                UNISTRINGXX_THROW(std::length_error, "The 'count' argument would have exceeded max_size().");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            // calls generic_unistring::insert(const generic_unistring&)
            this->insert(index, generic_unistring{cstr, count, this->get_allocator()});
            return (*this);
        }

        generic_unistring& insert(size_type index, const char_type* cstr)
        {
            // calls generic_unistring::insert(size_type, const char_type*, size_type)
            return (this->insert(index, cstr, traits_type::length(cstr)));
        }

        generic_unistring& insert(size_type index, size_type count, char_type ch)
        {
            // calls generic_unistring::insert(size_type, const generic_unistring&)
            return (this->insert(index, generic_unistring{count, ch, this->get_allocator()}));
        }

        iterator insert(const_iterator pos, char_type ch)
        {
            // calls generic_unistring::insert(const_iterator, size_type, char_type)
            return (this->insert(pos, 1, ch));
        }

        iterator insert(const_iterator pos, size_type count, char_type ch)
        {
            generic_unistring ustr{count, ch, this->get_allocator()};
            // calls generic_unistring::insert(const_iterator, inputIterT, inputIterT)
            return (this->insert(pos, ustr.cbegin(), ustr.cend()));
        }

        template<typename inputIterT>
        iterator insert(const_iterator pos, inputIterT first, inputIterT last)
        {
            // Prevent inserting after the null character.
            const_iterator actual_pos = std::min(pos, this->cend());
            return (_impl.insert(actual_pos, first, last));
        }

        iterator insert(const_iterator pos, std::initializer_list<char_type> init_list)
        {
            // calls generic_unistring::insert(const_iterator, inputIterT, inputIterT)
            return (this->insert(pos, init_list.begin(), init_list.end()));
        }

        // UNISTRINGXX_UNISTRING_APPENDING

        generic_unistring& operator+=(const generic_unistring& str)
        {
            // calls generic_unistring::append(const generic_unistring&)
            return (this->append(str));
        }

        generic_unistring& operator+=(const char_type* cstr)
        {
            // calls generic_unistring::append(const char_type*)
            return (this->append(cstr));
        }

        generic_unistring& operator+=(char_type ch)
        {
            this->push_back(ch);
            return (*this);
        }

        generic_unistring& operator+=(std::initializer_list<char_type> init_list)
        {
            // calls generic_unistring::append(std::initializer_list<char_type>)
            return (this->append(init_list));
        }

        generic_unistring& append(const generic_unistring& str)
        {
            // calls generic_unistring::append(inputIterT, inputIterT)
            return (this->append(str.cbegin(), str.cend()));
        }

        generic_unistring& append(const generic_unistring& str, size_type index, size_type count = npos)
        {
            // calls generic_unistring::append(const generic_unistring&)
            return (this->append(str.substr(index, count)));
        }

        generic_unistring& append(const char_type* cstr, size_type count)
        {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if ((this->size() + count) > this->max_size())
                UNISTRINGXX_THROW(std::length_error, "The 'count' argument would have exceeded max_size().");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            // calls generic_unistring::append(const generic_unistring&)
            return (this->append(generic_unistring{cstr, count, this->get_allocator()}));
        }

        generic_unistring& append(const char_type* cstr)
        {
            // calls generic_unistring::append(const char_type*, size_type)
            return (this->append(cstr, traits_type::length(cstr)));
        }

        generic_unistring& append(size_type count, char_type ch)
        {
            // calls generic_unistring::append(const generic_unistring&)
            return (this->append(generic_unistring{count, ch, this->get_allocator()}));
        }

        template<typename inputIterT>
        generic_unistring& append(inputIterT first, inputIterT last)
        {
            // calls generic_unistring::insert(const_iterator, inputIterT, inputIterT)
            this->insert(this->cend(), first, last);
            return (*this);
        }

        generic_unistring& append(std::initializer_list<char_type> init_list)
        {
            // calls generic_unistring::append(inputIterT, inputIterT)
            return (this->append(init_list.begin(), init_list.end()));
        }

        void push_back(char_type ch)
        {
            // Keep in mind the null character in _impl.
            // calls generic_unistring::insert(const_iterator, char_type)
            this->insert(this->cend(), ch);
            return;
        }

        // UNISTRINGXX_UNISTRING_ERASE

        generic_unistring& erase(size_type index = 0, size_type count = npos)
        {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if (index > this->size())
                UNISTRINGXX_THROW(std::out_of_range, "The 'index' argument is out of range.");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            const size_type erase_count = std::min(count, this->size() - index);
            const_iterator first = this->cbegin() + index;
            const_iterator last = first + erase_count;
            // calls generic_unistring::erase(const_iterator, const_iterator)
            this->erase(first, last);
            return (*this);
        }

        iterator erase(const_iterator pos)
        {
            // Technically, cend() cannot be used as pos, but generic_unistring's cend points to 
            // the null character (which is a valid position for _impl).
            if (pos == this->cend())
                return (this->end()); // return unchanged.
            return (_impl.erase(pos));
        }

        iterator erase(const_iterator first, const_iterator last)
        {
            const_iterator actual_first = std::min(first, this->cend());
            const_iterator actual_last = std::min(last, this->cend());
            return (_impl.erase(actual_first, actual_last));
        }

        void pop_back(void)
        {
            if (!this->empty()) {
                // calls generic_unistring::erase(size_type, size_type)
                this->erase((this->size() - 1), 1); // Keep in mind the null character in _impl.
            }
            return;
        }

        // UNISTRINGXX_UNISTRING_REPLACE

        generic_unistring& replace(
            size_type index, size_type count,
            const generic_unistring& str
        )
        {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if (index > this->size())
                UNISTRINGXX_THROW(std::out_of_range, "The 'index' argument is out of range.");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            const_iterator pos_at_index = this->cbegin() + index;
            // calls generic_unistring:::replace(const_iterator, const_iterator, const generic_unistring&)
            return (this->replace(pos_at_index, pos_at_index + count, str));
        }

        generic_unistring& replace(
            size_type index, size_type count,
            const generic_unistring& str, size_type str_index, size_type str_count = npos
        )
        {
            // calls generic_unistring:::replace(size_type, size_type, const generic_unistring&)
            return (this->replace(index, count, str.substr(str_index, str_count)));
        }

        generic_unistring& replace(
            size_type index, size_type count,
            const char_type* cstr, size_type cstr_count
        )
        {
            const size_type replace_count = std::min(count, this->size() - index);

#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if ((this->size() - replace_count) >= (this->max_size() - cstr_count))
                UNISTRINGXX_THROW(std::length_error, "The result would have exceeded max_size().");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            // calls generic_unistring:::replace(size_type, size_type, const generic_unistring&)
            return (this->replace(index, count, generic_unistring{cstr, cstr_count, this->get_allocator()}));
        }

        generic_unistring& replace(
            size_type index, size_type count,
            const char_type* cstr
        )
        {
            // calls generic_unistring:::replace(size_type, size_type, const char_type*, size_type)
            return (this->replace(index, count, cstr, traits_type::length(cstr)));
        }

        generic_unistring& replace(
            size_type index, size_type count,
            size_type char_count, char_type ch
        )
        {
            // calls generic_unistring:::replace(size_type, size_type, const generic_unistring&)
            return (this->replace(index, count, generic_unistring{char_count, ch, this->get_allocator()}));
        }

        generic_unistring& replace(
            const_iterator first, const_iterator last,
            const generic_unistring& str
        )
        {
            // calls generic_unistring::replace(const_iterator, const_iterator, inputIterT, inputIterT)
            return (this->replace(first, last, str.cbegin(), str.cend()));
        }

        generic_unistring& replace(
            const_iterator first, const_iterator last,
            const char_type* cstr, size_type count
        )
        {
            // calls generic_unistring:::replace(const_iterator, const_iterator, const generic_unistring&)
            return (this->replace(first, last, generic_unistring{cstr, count, this->get_allocator()}));
        }

        generic_unistring& replace(
            const_iterator first, const_iterator last,
            const char_type* cstr
        )
        {
            // calls generic::unistring(const_iterator, const_iterator, const char_type*, size_type)
            return (this->replace(first, last, cstr, traits_type::length(cstr)));
        }

        generic_unistring& replace(
            const_iterator first, const_iterator last,
            size_type count, char_type ch
        )
        {
            // calls generic::unistring(const_iterator, const_iterator, const generic_unistring&)
            return (this->replace(first, last, generic_unistring{count, ch, this->get_allocator()}));
        }

        template<typename inputIterT>
        generic_unistring& replace(
            const_iterator first, const_iterator last,
            inputIterT repl_first, inputIterT repl_last
        )
        {
            const_iterator actual_first = std::min(first, this->cend());
            const_iterator actual_last = std::min(last, this->cend());

            // TODO: improve implementation without any construction and/or temporaries
            generic_unistring result{this->cbegin(), actual_first, this->get_allocator()};
            result.append(repl_first, repl_last).append(actual_last, this->cend());
            return ((*this) = std::move(result));
        }

        generic_unistring& replace(
            const_iterator first, const_iterator last,
            std::initializer_list<char_type> init_list
        )
        {
            // calls generic_unistring::replace(const_iterator, const_iterator, inputIterT, inputIterT)
            return (this->replace(first, last, init_list.begin(), init_list.end()));
        }

        size_type copy(char_type* dest_cstr, size_type count, size_type index = 0) const
        {
#if (UNISTRINGXX_WITH_EXCEPTIONS)
            if (index > this->size())
                UNISTRINGXX_THROW(std::out_of_range, "The 'index' argument is out of range.");
#endif // (UNISTRINGXX_WITH_EXCEPTIONS)

            size_type num_copied = 0;

            const size_type copy_count = std::min(count, this->size() - index);
            for (size_type ctr = index; ctr < (index + copy_count); ctr++)
                dest_cstr[num_copied++] = this->operator[](ctr);

            return (num_copied);
        }

        void swap(generic_unistring& other)
        {
            _impl.swap(other._impl);
            return;
        }

        size_type find(const generic_unistring& str, size_type index = 0) const noexcept
        {
            if (index > this->size())
                return (npos);

            if (this->size() < str.size())
                return (npos);

            for (size_type ctr = index; ctr < this->size(); ctr++) {
                if ((this->size() - ctr) < str.size())
                    return (npos);

                bool found = true;
                auto itr1 = this->begin() + ctr;
                auto itr2 = str.begin();
                while (itr2 != str.end()) {
                    if (traits_type::eq(*itr1++, *itr2++))
                        continue;
                    found = false;
                    break;
                }
                if (found)
                    return (ctr);
            }

            return (npos);
        }

        size_type find(const char_type* cstr, size_type index, size_type count) const
        {
            // calls generic_unistring::find(const generic_unistring&, size_type)
            return (this->find(generic_unistring{cstr, count, this->get_allocator()}, index));
        }

        size_type find(const char_type* cstr, size_type index = 0) const /*noexcept*/
        {
            // calls generic_unistring::find(const char_type*, size_type, size_type)
            return (this->find(cstr, index, traits_type::length(cstr)));
        }

        size_type find(char_type ch, size_type index = 0) const /*noexcept*/
        {
            // calls generic_unistring::find(const generic_unistring&, size_type)
            return (this->find(generic_unistring{1, ch, this->get_allocator()}, index));
        }

        size_type rfind(const generic_unistring& str, size_type index = npos) const noexcept
        {
            if (this->size() < str.size())
                return (npos);

            size_type start_index = std::min(index, this->size());
            start_index = start_index - str.size();

            while (start_index > 0){
                bool found = true;
                auto itr1 = this->begin() + start_index;
                auto itr2 = str.begin();
                while (itr2 != str.end()) {
                    if (traits_type::eq(*itr1++, *itr2++))
                        continue;
                    found = false;
                    break;
                }
                if (found)
                    return (start_index);

                start_index--;
            }

            return (npos);
        }

        size_type rfind(const char_type* cstr, size_type index, size_type count) const
        {
            // calls generic_unistring::rfind(const generic_unistring&, size_type)
            return (this->rfind(generic_unistring{cstr, count, this->get_allocator()}, index));
        }

        size_type rfind(const char_type* cstr, size_type index = npos) const
        {
            // calls generic_unistring::rfind(const char_type*, size_type, size_type)
            return (this->rfind(cstr, index, traits_type::length(cstr)));
        }

        size_type rfind(char_type ch, size_type index = npos) const /*noexcept*/
        {
            // calls generic_unistring::rfind(const generic_unistring&, size_type)
            return (this->rfind(generic_unistring{1, ch, this->get_allocator()}, index));
        }

        size_type find_first_of(const generic_unistring& str, size_type index = 0) const noexcept
        {
            if (index > this->size())
                return (npos);

            for (size_type ctr = 0; ctr < this->size(); ctr++) {
                if (
                    std::any_of(
                        str.cbegin(), str.cend(),
                        [ctr, this] (char_type ch) -> bool {
                            return (traits_type::eq(ch, this->operator[](ctr)));
                        }
                    )
                ) { return (ctr); }
            }

            return (npos);
        }

        size_type find_first_of(const char_type* cstr, size_type index, size_type count) const
        {
            // calls generic_unistring::find_first_of(const generic_unistring&, size_type)
            return (this->find_first_of(generic_unistring{cstr, count, this->get_allocator()}, index));
        }

        size_type find_first_of(const char_type* cstr, size_type index = 0) const
        {
            // calls generic_unistring::find_first_of(const char_type*, size_type, size_type)
            return (this->find_first_of(cstr, index, traits_type::length(cstr)));
        }

        size_type find_first_of(char_type ch, size_type index = 0) const /*noexcept*/
        {
            // calls generic_unistring::find_first_of(const generic_unistring&, size_type)
            return (this->find_first_of(generic_unistring{1, ch, this->get_allocator()}, index));
        }

        size_type find_last_of(const generic_unistring& str, size_type index = npos) const noexcept
        {
            const size_type start_index = std::min(index, this->size() - 1);
            for (size_type ctr = start_index; ctr >= 0; ctr--) {
                if (
                    std::any_of(
                        str.cbegin(), str.cend(),
                        [ctr, this] (char_type ch) -> bool {
                            return (traits_type::eq(ch, this->operator[](ctr)));
                        }
                    )
                ) { return (ctr); }
            }

            return (npos);
        }

        size_type find_last_of(const char_type* cstr, size_type index, size_type count) const
        {
            // calls generic_unistring::find_last_of(const generic_unistring&, size_type)
            return (this->find_last_of(generic_unistring{cstr, count, this->get_allocator()}, index));
        }

        size_type find_last_of(const char_type* cstr, size_type index = npos) const
        {
            // calls generic_unistring::find_last_of(const char_type*, size_type, size_type)
            return (this->find_last_of(cstr, index, traits_type::length(cstr)));
        }

        size_type find_last_of(char_type ch, size_type index = npos) const /*noexcept*/
        {
            // calls generic_unistring::find_last_of(const generic_unistring&, size_type)
            return (this->find_last_of(generic_unistring{1, ch, this->get_allocator()}, index));
        }

        size_type find_first_not_of(const generic_unistring& str, size_type index = 0) const noexcept
        {
            if (index > this->size())
                return (npos);

            for (size_type ctr = 0; ctr < this->size(); ctr++) {
                if (
                    std::none_of(
                        str.cbegin(), str.cend(),
                        [ctr, this] (char_type ch) -> bool {
                            return (traits_type::eq(ch, this->operator[](ctr)));
                        }
                    )
                ) { return (ctr); }
            }

            return (npos);
        }

        size_type find_first_not_of(const char_type* cstr, size_type index, size_type count) const
        {
            // calls generic_unistring::find_first_not_of(const generic_unistring&, size_type)
            return (this->find_first_not_of(generic_unistring{cstr, count, this->get_allocator()}, index));
        }

        size_type find_first_not_of(const char_type* cstr, size_type index = 0) const
        {
            // calls generic_unistring::find_first_not_of(const char_type*, size_type, size_type)
            return (this->find_first_not_of(cstr, index, traits_type::length(cstr)));
        }

        size_type find_first_not_of(char_type ch, size_type index = 0) const /*noexcept*/
        {
            // calls generic_unistring::find_first_not_of(const generic_unistring&, size_type)
            return (this->find_first_not_of(generic_unistring{1, ch, this->get_allocator()}, index));
        }

        size_type find_last_not_of(const generic_unistring& str, size_type index = npos) const noexcept
        {
            const size_type start_index = std::min(index, this->size() - 1);
            for (size_type ctr = start_index; ctr >= 0; ctr--) {
                if (
                    std::none_of(
                        str.cbegin(), str.cend(),
                        [ctr, this] (char_type ch) -> bool {
                            return (traits_type::eq(ch, this->operator[](ctr)));
                        }
                    )
                ) { return (ctr); }
            }

            return (npos);
        }

        size_type find_last_not_of(const char_type* cstr, size_type index, size_type count) const
        {
            // calls generic_unistring::find_last_not_of(const generic_unistring&, size_type)
            return (this->find_last_not_of(generic_unistring{cstr, count, this->get_allocator()}, index));
        }

        size_type find_last_not_of(const char_type* cstr, size_type index = npos) const
        {
            // calls generic_unistring::find_last_not_of(const char_type*, size_type, size_type)
            return (this->find_last_not_of(cstr, index, traits_type::length(cstr)));
        }

        size_type find_last_not_of(char_type ch, size_type index = npos) const /*noexcept*/
        {
            // calls generic_unistring::find_last_not_of(const generic_unistring&, size_type)
            return (this->find_last_not_of(generic_unistring{1, ch, this->get_allocator()}, index));
        }

        std::string to_u8string(size_type index = 0, size_type count = npos) const
        {
            return (
                (*this).template to_stringT<std::string>(
                    index, count,
                    [] (std::string& dest, const char_type& uc) -> void {
                        auto vec = uc.to_utf8();
                        dest.append(vec.begin(), vec.end());
                        return;
                    }
                )
            );
        }

        std::u16string to_u16string(size_type index = 0, size_type count = npos) const
        {
            return (
                this->to_stringT<std::u16string>(
                    index, count,
                    [] (std::u16string& dest, const char_type& uc) -> void {
                        auto vec = uc.to_utf16();
                        dest.append(vec.begin(), vec.end());
                        return;
                    }
                )
            );
        }

        std::u32string to_u32string(size_type index = 0, size_type count = npos) const
        {
            return (
                this->to_stringT<std::u32string>(
                    index, count,
                    [] (std::u32string& dest, const char_type& uc) -> void {
                        dest.push_back(uc.to_utf32());
                        return;
                    }
                )
            );
        }

        static generic_unistring from_u8string(
            const std::string& str,
            std::string::size_type index = 0, std::string::size_type count = std::string::npos
        )
        {
            const std::string actual_str = str.substr(index, count);
            generic_unistring result;
            for (std::string::size_type ctr = 0; ctr < actual_str.size(); ctr++) {
                // determine how much octets needed to pass to from_utf8 function
                const char8_t& ch = static_cast<char8_t>(actual_str[ctr]);
                if (is_bit_set(ch, 0x07)) {
                    std::size_t num_seq = 0;
                    std::size_t bit_num = 0x07;
                    while ((bit_num != 0) && (is_bit_set(ch, bit_num))) {
                        num_seq++;
                        bit_num--;
                    }
                    result.push_back(
                        char_type::from_utf8(
                            ch,
                            num_seq > 1 ? actual_str[++ctr] : '\0',
                            num_seq > 2 ? actual_str[++ctr] : '\0',
                            num_seq > 3 ? actual_str[++ctr] : '\0'
                        )
                    );
                }
                else {
                    result.push_back(char_type::from_utf8(ch));
                }
            }
            return (result);
        }

        static generic_unistring from_u16string(
            const std::u16string& str,
            std::u16string::size_type index = 0, std::u16string::size_type count = std::u16string::npos
        )
        {
            const std::u16string actual_str = str.substr(index, count);
            generic_unistring result;
            for (std::u16string::size_type ctr = 0; ctr < actual_str.size(); ctr++) {
                const auto& ch = actual_str[ctr];
                if (ch >= 0xD800 && ch <= 0xDBFF) {
                    ctr++;
                    const auto& ch2 = actual_str[ctr];
                    result.push_back(char_type::from_utf16(ch, ch2));
                }
                else {
                    result.push_back(char_type::from_utf16(ch));
                }
            }
            return (result);
        }

        static generic_unistring from_u32string(
            const std::u32string& str,
            std::u32string::size_type index = 0, std::u32string::size_type count = std::u32string::npos
        )
        {
            const std::u32string actual_str = str.substr(index, count);
            generic_unistring result;
            for (const auto& ch : actual_str) {
                result.push_back(char_type::from_utf32(ch));
            }
            return (result);
        }

        // Note: Because these are a template friend functions, the compiler will generate a free functions that
        // are specialized (i.e. these are not member functions).
        friend bool operator==(const generic_unistring& left, const generic_unistring& right)
        { return (left._impl == right._impl); }

        friend bool operator!=(const generic_unistring& left, const generic_unistring& right)
        { return (left._impl != right._impl); }

        friend bool operator<(const generic_unistring& left, const generic_unistring& right)
        { return (left._impl < right._impl); }

        friend bool operator<=(const generic_unistring& left, const generic_unistring& right)
        { return (left._impl <= right._impl); }

        friend bool operator>(const generic_unistring& left, const generic_unistring& right)
        { return (left._impl > right._impl); }

        friend bool operator>=(const generic_unistring& left, const generic_unistring& right)
        { return (left._impl >= right._impl); }

        friend bool operator==(const char_type* cstr, const generic_unistring& str)
        {
            if (str.size() == 0 && !cstr[0].is_null())
                return (false);
            auto compare_count = std::min(traits_type::length(cstr), str.size());
            return (traits_type::compare(cstr, str.c_str(), compare_count) == 0);
        }

        friend bool operator==(const generic_unistring& str, const char_type* cstr)
        { return (cstr == str); }

        friend bool operator!=(const char_type* cstr, const generic_unistring& str)
        { return (!(cstr == str)); }

        friend bool operator!=(const generic_unistring& str, const char_type* cstr)
        { return (cstr != str); }

        friend bool operator<(const char_type* cstr, const generic_unistring& str)
        {
            if (str.size() == 0 && !cstr[0].is_null())
                return (false);
            auto compare_count = std::min(traits_type::length(cstr), str.size());
            return (traits_type::compare(cstr, str.c_str(), compare_count) < 0);
        }

        friend bool operator<(const generic_unistring& str, const char_type* cstr)
        { return (cstr > str); }

        friend bool operator>(const char_type* cstr, const generic_unistring& str)
        {
            if (str.size() == 0 && !cstr[0].is_null())
                return (true);
            auto compare_count = std::min(traits_type::length(cstr), str.size());
            return (traits_type::compare(cstr, str.c_str(), compare_count) > 0);
        }

        friend bool operator>(const generic_unistring& str, const char_type* cstr)
        { return (cstr < str); }

        friend bool operator<=(const char_type* cstr, const generic_unistring& str)
        { return (!(cstr > str)); }

        friend bool operator<=(const generic_unistring& str, const char_type* cstr)
        { return (!(str > cstr)); }

        friend bool operator>=(const char_type* cstr, const generic_unistring& str)
        { return (!(cstr < str)); }

        friend bool operator>=(const generic_unistring& str, const char_type* cstr)
        { return (!(str < cstr)); }

#if (UNISTRINGXX_TEST)
        const impl_type& get_impl(void) const
        { return (_impl); }
#endif // (UNISTRINGXX_TEST)

    private:
        impl_type _impl;

        size_type _actual_size(void) const
        { return (_impl.size()); }

        template<class stringT>
        stringT to_stringT(
            size_type index, size_type count,
            std::function<void(stringT& dest, const char_type&)>&& convert
        ) const
        {
            auto str = this->substr(index, count);
            stringT result;
            for (const auto& uc : str)
                convert(result, uc);
            return (result);
        }
    }; // class generic_unistring

    typedef generic_unistring<std::allocator<unichar_t_traits::char_type>> unistring;

    // Literal operators.
    inline unistring operator "" UNISTRINGXX_UNISTRING_LITERALOP(const char* cstr, std::size_t size)
    {
        return (unistring::from_u8string(std::string(cstr, size)));
    }

    inline unistring operator "" UNISTRINGXX_UNISTRING_LITERALOP(const char16_t* cstr, std::size_t size)
    {
        return (unistring::from_u16string(std::u16string(cstr, size)));
    }

    inline unistring operator "" UNISTRINGXX_UNISTRING_LITERALOP(const char32_t* cstr, std::size_t size)
    {
        return (unistring::from_u32string(std::u32string(cstr, size)));
    }

    // Helper for using literal operators.
    #define UNISTRINGXX_UNISTRING_LITERAL(str) UNISTRINGXX_CONCAT(str, UNISTRINGXX_UNISTRING_LITERALOP)
    // Shorthand for above
    #define USXX_STR(str) UNISTRINGXX_UNISTRING_LITERAL(str)

} // namespace unistringxx

namespace std
{
    // specialization of std::swap
    template<typename allocatorT>
    void swap(unistringxx::generic_unistring<allocatorT>& left, unistringxx::generic_unistring<allocatorT>& right)
    {
        left.swap(right);
        return;
    }
} // namespace std

#endif // !defined(UNISTRINGXX_USTRING_HPP)
