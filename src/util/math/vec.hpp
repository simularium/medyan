#ifndef MEDYAN_UTIL_MATH_VEC_HPP
#define MEDYAN_UTIL_MATH_VEC_HPP

#include <array>
#include <cstddef> // ptrdiff_t
#include <iterator> // tag
#include <ostream>
#include <type_traits> // conditional, enable_if, is_same

namespace mathfunc {

// A simple coordinate type that makes operations easier
// Can be replaced by linalg/tensor libraries in the future
template< size_t dim, typename Float = double > struct Vec {

    static constexpr size_t vec_size = dim;
    using storage_type = std::array< Float, dim >;
    using size_type = typename storage_type::size_type;
    using iterator = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;

    storage_type value;

    Vec& operator=(const Vec& v) = default;
    template< typename VecType, std::enable_if_t< dim == VecType::vec_size > * = nullptr >
    Vec& operator=(const VecType& v) {
        for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }

    constexpr size_type size() const noexcept { return dim; }

    constexpr iterator       begin()       noexcept { return value.begin(); }
    constexpr const_iterator begin() const noexcept { return value.begin(); }
    constexpr iterator       end()       noexcept { return value.end(); }
    constexpr const_iterator end() const noexcept { return value.end(); }

    constexpr       Float& operator[](size_type pos)       { return value[pos]; }
    constexpr const Float& operator[](size_type pos) const { return value[pos]; }
};

// Frequently used type alias
using Vec3 = Vec<3>;

// Formatting
// undefined behavior if dim is 0
template< size_t dim, typename Float > inline
std::ostream& operator<<(std::ostream& os, const Vec<dim, Float>& v) {
    os << '(';
    for(size_t i = 0; i < dim - 1; ++i) os << v[i] << ", ";
    os << v[dim - 1] << ')';
    return os;
}

template<
    size_t dim,
    typename Float = double,
    typename Container = std::vector< Float >
> struct VecArray {

    using container_type = Container;
    using size_type = typename container_type::size_type;
    static_assert(std::is_same<typename container_type::iterator::iterator_category, std::random_access_iterator_tag>::value,
        "The iterator of the VecArray container must be random access iterator.");

    template< bool is_const, typename Concrete > struct RefVecBase {
        static constexpr size_t vec_size = dim;
        using vec_array_type = VecArray;
        using size_type = vec_array_type::size_type;
        using iterator = std::conditional_t< is_const, typename container_type::const_iterator, typename container_type::iterator >;
        using reference = std::conditional_t< is_const, typename container_type::const_reference, typename container_type::reference >;

        std::conditional_t< is_const, const vec_array_type::container_type, vec_array_type::container_type >
            * ptr;
        size_type pos; // index of first Float

        constexpr size_type size() const noexcept { return dim; }

        constexpr iterator begin() const noexcept { return ptr->begin() + pos; }
        constexpr iterator end() const noexcept { return ptr->begin() + pos + dim; }

        // sub_pos must be within [0, dim)
        reference operator[](size_type sub_pos) const { return (*ptr)[pos + sub_pos]; }

        // also works like pointer
        Concrete*       operator->()       noexcept { return static_cast<      Concrete*>(this); }
        const Concrete* operator->() const noexcept { return static_cast<const Concrete*>(this); }
    };
    struct RefVec : RefVecBase< false, RefVec > {
        RefVec(container_type* ptr, size_type pos) : RefVecBase< false, RefVec >{ptr, pos} {}

        template< typename VecType, std::enable_if_t< dim == VecType::vec_size > * = nullptr >
        RefVec& operator=(const VecType& v) {
            for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
            return *this;
        }
    };
    struct ConstRefVec : RefVecBase< true, ConstRefVec > {
        ConstRefVec(const container_type* ptr, size_type pos) : RefVecBase< true, ConstRefVec >{ptr, pos} {}
    };

    template< bool is_const > class VecIterator {
        template< bool friend_const > friend class VecIterator;

    public:
        using vec_array_type = VecArray;
        using size_type = vec_array_type::size_type;
        using SolidVec = Vec< dim, Float >;
        using container_type = std::conditional_t< is_const, const vec_array_type::container_type, vec_array_type::container_type >;

        using iterator_category = std::random_access_iterator_tag;
        using value_type = SolidVec; // Not used in class
        using difference_type = std::ptrdiff_t;
        using pointer = std::conditional_t< is_const, ConstRefVec, RefVec >; // The pointer type is the reference itself
        using reference = std::conditional_t< is_const, ConstRefVec, RefVec >;

    private:
        container_type* _ptr;
        size_type _index;

    public:
        VecIterator() = default;
        VecIterator(container_type* ptr, size_type index) : _ptr(ptr), _index(index) {}
        VecIterator(const VecIterator& rhs) = default;
        template< bool rhs_const, std::enable_if_t<!is_const || rhs_const>* = nullptr >
        VecIterator(const VecIterator<rhs_const>& rhs) : _ptr(rhs._ptr), _index(rhs._index) {}
        VecIterator& operator=(const VecIterator& rhs) = default;
        template< bool rhs_const, std::enable_if_t<!is_const || rhs_const>* = nullptr >
        VecIterator& operator=(const VecIterator<rhs_const>& rhs) { _ptr = rhs._ptr; _index = rhs._index; return *this; }

        reference operator*() const { return reference(_ptr, _index * dim); }
        pointer operator->() const { return pointer(_ptr, _index * dim); }
        reference operator[](difference_type rhs) const { return reference(_ptr, (_index + rhs) * dim); }

        VecIterator& operator+=(difference_type rhs) { _index += rhs; return *this; }
        VecIterator& operator-=(difference_type rhs) { _index -= rhs; return *this; }
        VecIterator& operator++() { ++_index; return *this; }
        VecIterator& operator--() { --_index; return *this; }
        VecIterator operator++(int) { VecIterator tmp(*this); ++_index; return tmp; }
        VecIterator operator--(int) { VecIterator tmp(*this); --_index; return tmp; }

        template< bool rhs_const >
        difference_type operator-(const VecIterator<rhs_const>& rhs) const {
            return difference_type(_index) - difference_type(rhs._index);
        }
        VecIterator operator+(difference_type rhs) const { return VecIterator(_ptr, _index + rhs); }
        VecIterator operator-(difference_type rhs) const { return VecIterator(_ptr, _index - rhs); }
        friend auto operator+(difference_type lhs, const VecIterator& rhs) {
            return VecIterator(rhs._ptr, lhs + rhs._index);
        }

        template< bool rhs_const >
        bool operator==(const VecIterator<rhs_const>& rhs) const { return _ptr == rhs._ptr && _index == rhs._index; }
        template< bool rhs_const >
        bool operator!=(const VecIterator<rhs_const>& rhs) const { return _ptr != rhs._ptr || _index != rhs._index; }
        template< bool rhs_const >
        bool operator>(const VecIterator<rhs_const>& rhs) const { return _ptr > rhs._ptr || (_ptr == rhs._ptr && _index > rhs._index); }
        template< bool rhs_const >
        bool operator<(const VecIterator<rhs_const>& rhs) const { return _ptr < rhs._ptr || (_ptr == rhs._ptr && _index < rhs._index); }
        template< bool rhs_const >
        bool operator>=(const VecIterator<rhs_const>& rhs) const { return _ptr > rhs._ptr || (_ptr == rhs._ptr && _index >= rhs._index); }
        template< bool rhs_const >
        bool operator<=(const VecIterator<rhs_const>& rhs) const { return _ptr < rhs._ptr || (_ptr == rhs._ptr && _index <= rhs._index); }
    };

    using iterator = VecIterator<false>;
    using const_iterator = VecIterator<true>;

    container_type value;

    // returns raw data (Float*, with size equal to value.size())
    Float*       data()       noexcept(noexcept(std::declval<      container_type>().data())) { return value.data(); }
    const Float* data() const noexcept(noexcept(std::declval<const container_type>().data())) { return value.data(); }

    size_type size_raw() const noexcept(noexcept(std::declval<container_type>().data())) { return value.size(); }
    bool empty() const noexcept(noexcept(std::declval<container_type>().empty())) { return value.empty(); }

    // The following considers size as *number of vectors*, which is value.size() / dim
    size_type size() const noexcept(noexcept(std::declval<container_type>().data())) { return value.size() / dim; }

    iterator       begin()       noexcept { return       iterator(&value, 0); }
    const_iterator begin() const noexcept { return const_iterator(&value, 0); }
    iterator       end()       noexcept { return       iterator(&value, size()); }
    const_iterator end() const noexcept { return const_iterator(&value, size()); }

    RefVec      operator[](size_type index)       { return      RefVec(&value, index * dim); }
    ConstRefVec operator[](size_type index) const { return ConstRefVec(&value, index * dim); }

    template< typename VecType, std::enable_if_t<dim == VecType::vec_size>* = nullptr >
    void push_back(const VecType& v) {
        value.insert(value.end(), v.begin(), v.end());
    }
    void pop_back() {
        value.resize(value.size() - dim);
    }
};

} // namespace mathfunc

#endif
