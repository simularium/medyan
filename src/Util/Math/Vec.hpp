#ifndef MEDYAN_Util_Math_Vec_Hpp
#define MEDYAN_Util_Math_Vec_Hpp

#include <algorithm> // copy
#include <array>
#include <cmath>
#include <cstddef> // ptrdiff_t
#include <functional> // reference_wrapper
#include <iterator> // tag
#include <ostream>
#include <type_traits> // common_type, conditional, enable_if, is_pointer, is_same, remove_reference
#include "utility.h"

namespace mathfunc {

//-----------------------------------------------------------------------------
// Vec is a simple coordinate type that makes operations easier
// Can be replaced by linalg/tensor libraries in the future
//-----------------------------------------------------------------------------
template< size_t dim, typename Float = double > struct Vec {

    static constexpr size_t vec_size = dim;
    using float_type = Float;
    using value_type = float_type; // For STL compatibility

    using storage_type = std::array< Float, dim >;
    using size_type = typename storage_type::size_type;
    using iterator       = typename storage_type::iterator;
    using const_iterator = typename storage_type::const_iterator;
    using reference       = typename storage_type::reference;
    using const_reference = typename storage_type::const_reference;

    storage_type value;

    Vec& operator=(const Vec& v) = default;
    template< typename VecType, std::enable_if_t< dim == VecType::vec_size > * = nullptr >
    Vec& operator=(const VecType& v) {
        for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }

    // Conversion operator to another Vec
    template< typename FloatOut >
    explicit operator Vec< vec_size, FloatOut >() const {
        Vec< vec_size, FloatOut > res;
        for(size_t i = 0; i < vec_size; ++i) res[i] = (*this)[i];
        return res;
    }

    constexpr size_type size() const noexcept { return dim; }

    constexpr iterator       begin()       noexcept { return value.begin(); }
    constexpr const_iterator begin() const noexcept { return value.begin(); }
    constexpr iterator       end()       noexcept { return value.end(); }
    constexpr const_iterator end() const noexcept { return value.end(); }

    constexpr       reference operator[](size_type pos)       { return value[pos]; }
    constexpr const_reference operator[](size_type pos) const { return value[pos]; }
};

// Frequently used type alias
using Vec3  = Vec< 3 >;
using Vec3d = Vec< 3, double >;
using Vec3f = Vec< 3, float >;

namespace internal {

    // Helper functions and types to choose implementation according to whether RefVecBase of is_raw_ptr_container
    //-------------------------------------------------------------------------
    template< typename RefVecType, std::enable_if_t< std::remove_reference_t<RefVecType>::is_raw_ptr_container >* = nullptr >
    constexpr decltype(auto) refVecBaseGetBegin(RefVecType&& v) { return v.ref; }
    template< typename RefVecType, std::enable_if_t< !std::remove_reference_t<RefVecType>::is_raw_ptr_container >* = nullptr >
    constexpr decltype(auto) refVecBaseGetBegin(RefVecType&& v) { return v.ref.begin(); }

    template< bool is_raw_ptr_container, typename Container > struct RefVecBaseFloatType;
    template< typename Container > struct RefVecBaseFloatType< false, Container > {
        using type = typename Container::value_type;
    };
    template< typename Container > struct RefVecBaseFloatType< true, Container > {
        using type = std::remove_pointer_t< Container >;
    };
    template< bool is_raw_ptr_container, typename Container >
    using RefVecBaseFloatType_t = typename RefVecBaseFloatType< is_raw_ptr_container, Container >::type;

    template< bool is_const, bool is_raw_ptr_container, typename Float, typename Container >
    struct RefVecBaseIterator { using type = std::conditional_t< is_const, const Float *, Float * >; };
    template< bool is_const, typename Float, typename Container >
    struct RefVecBaseIterator< is_const, false, Float, Container > {
        using type = std::conditional_t< is_const, typename Container::const_iterator, typename Container::iterator >;
    };
    template< bool is_const, bool is_raw_ptr_container, typename Float, typename Container >
    using RefVecBaseIterator_t = typename RefVecBaseIterator< is_const, is_raw_ptr_container, Float, Container >::type;

    template< bool is_const, bool is_raw_ptr_container, typename Float, typename Container >
    struct RefVecBaseReference { using type = std::conditional_t< is_const, const Float &, Float & >; };
    template< bool is_const, typename Float, typename Container >
    struct RefVecBaseReference< is_const, false, Float, Container > {
        using type = std::conditional_t< is_const, typename Container::const_reference, typename Container::reference >;
    };
    template< bool is_const, bool is_raw_ptr_container, typename Float, typename Container >
    using RefVecBaseReference_t = typename RefVecBaseReference< is_const, is_raw_ptr_container, Float, Container >::type;

    // RefVecBase is the base impl for RefVec and ConstRefVec
    // Note:
    //   - There must be a Container::iterator that is a RandomAccessIterator
    //     unless the Container is a Float* raw pointer
    template<
        size_t dim,
        typename Container,
        bool is_const,
        typename Concrete
    > struct RefVecBase {
        static constexpr size_t vec_size = dim;

        using container_type = std::decay_t< Container >;
        static constexpr bool is_raw_ptr_container = std::is_pointer< container_type >::value;
        using float_type = RefVecBaseFloatType_t< is_raw_ptr_container, container_type >;
        using value_type = float_type; // for STL compatibility

        using container_ref_type = std::conditional_t<
            is_raw_ptr_container,
            std::conditional_t< is_const, const float_type * const, float_type * const >,
            std::conditional_t< is_const, const container_type &, container_type & >
        >;
        // The type for use in constructors to prevent unwanted dangling const references
        using container_ref_arg_type = std::conditional_t<
            is_raw_ptr_container,
            std::conditional_t< is_const, const float_type * const, float_type * const >,
            std::conditional_t< is_const, std::reference_wrapper< const container_type >, container_type & >
        >;

        using iterator       = RefVecBaseIterator_t< is_const, is_raw_ptr_container, float_type, container_type >;
        using const_iterator = RefVecBaseIterator_t< true,     is_raw_ptr_container, float_type, container_type >;
        using reference       = RefVecBaseReference_t< is_const, is_raw_ptr_container, float_type, container_type >;
        using const_reference = RefVecBaseReference_t< true,     is_raw_ptr_container, float_type, container_type >;

        container_ref_type ref;
        const size_t pos; // index of first Float

        // Conversion operator to normal Vec
        template< typename FloatOut >
        explicit operator Vec< vec_size, FloatOut >() const {
            Vec< vec_size, FloatOut > res;
            for(size_t i = 0; i < vec_size; ++i) res[i] = (*this)[i];
            return res;
        }

        // Copy/move assignment operators are deleted
        RefVecBase& operator=(const RefVecBase & ) = delete;
        RefVecBase& operator=(      RefVecBase &&) = delete;

        constexpr auto size() const noexcept { return dim; }

        // Iterators
        constexpr iterator begin() const noexcept { return refVecBaseGetBegin(*this) + pos; }
        constexpr iterator end()   const noexcept { return refVecBaseGetBegin(*this) + pos + dim; }

        // sub_pos must be within [0, dim)
        reference operator[](size_t sub_pos) const { return ref[pos + sub_pos]; }

        // also works like pointer
        Concrete*       operator->()       noexcept { return static_cast<      Concrete*>(this); }
        const Concrete* operator->() const noexcept { return static_cast<const Concrete*>(this); }
    };

} // namespace internal


// VecSpan treats a contiguous segment of memory as a Vec.
template< size_t dim, typename ValueType >
struct VecMap {
    using value_type = ValueType;                         // may be const
    using float_type = std::remove_cv_t<value_type>;      // cannot be const

    using size_type = std::size_t;

    using iterator        = value_type*;
    using reference       = value_type&;
    using const_reference = const value_type&;

    static constexpr size_t vec_size = dim;

    // Actual data
    value_type* ptr;

    // Conversion operator to normal Vec
    template< typename FloatOut >
    explicit operator Vec< vec_size, FloatOut >() const {
        Vec< vec_size, FloatOut > res;
        for(size_t i = 0; i < vec_size; ++i) res[i] = (*this)[i];
        return res;
    }

    // copy from Vec
    template< typename Float >
    VecMap& operator=(const Vec<dim, Float>& v) const {
        for(size_type i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }

    // copy from VecMap
    template< typename FloatRaw >
    VecMap& operator=(VecMap<dim, FloatRaw> v) const {
        for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }

    constexpr auto size() const noexcept { return dim; }

    // Iterators
    constexpr iterator begin() const noexcept { return ptr; }
    constexpr iterator end()   const noexcept { return ptr + dim; }

    // idx must be within [0, dim)
    reference operator[](size_t idx) const { return ptr[idx]; }
};

//-----------------------------------------------------------------------------
// RefVec and ConstRefVec refer to the Vec-like data in an array. They contain
// pointers to the container and relative positions in the array, but do not
// contain data themselves.
//
// They can perform most of the arithmetics just like a normal Vec does. A
// RefVec can also change the data it refers to. If an arithmetic generates
// a new object, then the object should NOT be a RefVec.
//
// Template parameters:
//   - dim: Size of vec
//   - Container: The type of the original container, either raw pointer or any
//     container that has member types "value_type", "reference",
//     "const_reference", "iterator", "const_iterator". An object "c" of this
//     type must support c[n] for element accessing.
//
// Note:
//   - The copy/move constructors here have different semantics from copy/move
//     assignment operators. Both RefVec and ConstRefVec have copy/move
//     constructors to copy the pointer information from another RefVec. On the
//     other hand, assignment operators deal directly with the data they refer
//     to, just like a normal Vec.
//-----------------------------------------------------------------------------
template< size_t dim, typename Container >
struct RefVec : internal::RefVecBase< dim, Container, false, RefVec< dim, Container > > {
    using base_type = internal::RefVecBase< dim, Container, false, RefVec >;

    constexpr RefVec(typename base_type::container_ref_arg_type ref, size_t pos) : base_type{ref, pos} {}

    // Copy/move constructors
    constexpr RefVec(const RefVec&  rv) : RefVec(rv.ref, rv.pos) {}
    constexpr RefVec(      RefVec&& rv) : RefVec(rv.ref, rv.pos) {}

    // Copy assignment operator must be explicity defined.
    // Otherwise it is implicitly defined as deleted.
    RefVec& operator=(const RefVec& v) {
        for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }

    template< typename VecType, std::enable_if_t< dim == VecType::vec_size > * = nullptr >
    RefVec& operator=(const VecType& v) {
        for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }
    template< typename VecType, std::enable_if_t< dim == VecType::vec_size > * = nullptr >
    RefVec& operator=(VecType&& v) {
        for(size_t i = 0; i < dim; ++i) (*this)[i] = v[i];
        return *this;
    }
};
template< size_t dim, typename Container >
struct ConstRefVec : internal::RefVecBase< dim, Container, true, ConstRefVec< dim, Container > > {
    using base_type = internal::RefVecBase< dim, Container, true, ConstRefVec >;

    constexpr ConstRefVec(typename base_type::container_ref_arg_type ref, size_t pos) : base_type{ref, pos} {}

    constexpr ConstRefVec(const RefVec< dim, Container >&  rv) : ConstRefVec(rv.ref, rv.pos) {}
    constexpr ConstRefVec(      RefVec< dim, Container >&& rv) : ConstRefVec(rv.ref, rv.pos) {}

    // Copy/move constructors
    constexpr ConstRefVec(const ConstRefVec&  crv) : ConstRefVec(crv.ref, crv.pos) {}
    constexpr ConstRefVec(      ConstRefVec&& crv) : ConstRefVec(crv.ref, crv.pos) {}
};

//-----------------------------------------------------------------------------
// The VecArray type is like an array of Vec's but has flattened storage.
//-----------------------------------------------------------------------------
template<
    size_t dim,
    typename Float = double,
    typename Container = std::vector< Float >
> struct VecArray {

    static constexpr size_t element_vec_size = dim;
    using float_type = Float;
    using container_type = Container;

    using size_type = typename container_type::size_type;
    static_assert(std::is_same<typename container_type::iterator::iterator_category, std::random_access_iterator_tag>::value,
        "The iterator of the VecArray container must be random access iterator.");

    using reference       = RefVec     < dim, Container >;
    using const_reference = ConstRefVec< dim, Container >;

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
        using pointer = std::conditional_t< is_const, VecArray::const_reference, VecArray::reference >; // The pointer type is the reference itself
        using reference = std::conditional_t< is_const, VecArray::const_reference, VecArray::reference >;

    private:
        container_type* _ptr;
        size_type _index;

    public:
        VecIterator() = default;
        VecIterator(container_type* ptr, size_type index) : _ptr(ptr), _index(index) {}
        VecIterator(const VecIterator& rhs) = default;
        template< bool rhs_const >
        VecIterator(const VecIterator<rhs_const>& rhs) : _ptr(rhs._ptr), _index(rhs._index) {}
        VecIterator& operator=(const VecIterator& rhs) = default;
        template< bool rhs_const >
        VecIterator& operator=(const VecIterator<rhs_const>& rhs) { _ptr = rhs._ptr; _index = rhs._index; return *this; }

        reference operator*() const { return reference(*_ptr, _index * dim); }
        pointer operator->() const { return pointer(*_ptr, _index * dim); }
        reference operator[](difference_type rhs) const { return reference(*_ptr, (_index + rhs) * dim); }

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

    reference       operator[](size_type index)       { return       reference(value, index * dim); }
    const_reference operator[](size_type index) const { return const_reference(value, index * dim); }

    reference       back()       { return       reference(value, size_raw() - dim); }
    const_reference back() const { return const_reference(value, size_raw() - dim); }

    template< typename VecType, std::enable_if_t<dim == VecType::vec_size>* = nullptr >
    void push_back(const VecType& v) {
        value.insert(value.end(), v.begin(), v.end());
    }
    void pop_back() {
        value.resize(value.size() - dim);
    }
    void resize(size_type count) {
        value.resize(dim * count);
    }
};

//-----------------------------------------------------------------------------
// Factory functions
//-----------------------------------------------------------------------------
template< size_t dim, typename Float = double >
constexpr auto makeRefVec     (      Float* source) { return VecMap< dim, Float > { source }; }
template< size_t dim, typename Float = double >
constexpr auto makeConstRefVec(const Float* source) { return VecMap< dim, const Float > { source }; }

template< size_t dim, typename Float = double >
inline auto makeVec(const Float* source) {
    Vec< dim, Float > res;
    std::copy(source, source + dim, res.begin());
    return res;
}

//-----------------------------------------------------------------------------
// Traits
//-----------------------------------------------------------------------------

// Properties
//-------------------------------------

// Vec
template< typename VecType > struct IsVec : std::false_type {};
template< size_t dim, typename Float > struct IsVec< Vec< dim, Float > > : std::true_type {};

// Vec mutable lref
template< typename VecType > struct IsVecMlref : std::integral_constant< bool,
    std::is_lvalue_reference< VecType >::value                       // is lvalue reference
    && !std::is_const< std::remove_reference_t<VecType> >::value     // not const
    && IsVec< std::decay_t<VecType> >::value                         // is a Vec
> {};

// VecMap of mutable
template< typename VecType > struct IsVecMapMutable : std::false_type {};
template< size_t dim, typename Float > struct IsVecMapMutable< VecMap< dim, Float > > : std::integral_constant< bool,
    !std::is_const< typename VecMap< dim, Float >::value_type >::value
> {};

// VecMap of const
template< typename VecType > struct IsVecMapConst : std::false_type {};
template< size_t dim, typename Float > struct IsVecMapConst< VecMap< dim, Float > > : std::integral_constant< bool,
    std::is_const< typename VecMap< dim, Float >::value_type >::value
> {};

// VecMap
template< typename VecType > struct IsVecMap : std::integral_constant< bool,
    IsVecMapMutable<VecType>::value || IsVecMapConst<VecType>::value
> {};

// any Vec-like object which can be passed to functions with modifiable data.
template< typename VecType > struct IsVecLikeMut : std::integral_constant< bool,
    IsVecMlref< VecType >::value
    || IsVecMapMutable< std::decay_t<VecType> >::value
> {};

// any Vec-like object
template< typename VecType > struct IsVecLike : std::integral_constant< bool,
    IsVec< std::decay_t<VecType> >::value
    || IsVecMap< std::decay_t<VecType> >::value
> {};


// Operations
//-------------------------------------

// Get float type from Vec-like object
template< typename VecType >
using VecLikeFloatType = typename std::decay_t<VecType>::float_type;

// Get Vec type from Vec-like object
template< typename VecType >
using VecLikeVecType = Vec< std::decay_t<VecType>::vec_size, VecLikeFloatType<VecType> >;




//-----------------------------------------------------------------------------
// Formatting
//-----------------------------------------------------------------------------
template< typename VecType, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr > inline
std::ostream& operator<<(std::ostream& os, const VecType& v) {
    os << '(';
    if(std::decay_t<VecType>::vec_size >= 1) {
        for(std::size_t i = 0; i < std::decay_t<VecType>::vec_size - 1; ++i) os << v[i] << ", ";
        os << v[std::decay_t<VecType>::vec_size - 1];
    }
    os << ')';
    return os;
}

//-----------------------------------------------------------------------------
// Fixed size vector arithmetics
//-----------------------------------------------------------------------------

// Magnitude, distance and normalization
//-------------------------------------

// Find square magnitude of a vector.
template< typename VecType, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto magnitude2(VecType&& v) {
    VecLikeFloatType<VecType> mag2 {};
    for(size_t i = 0; i < std::decay_t<VecType>::vec_size; ++i) mag2 += v[i] * v[i];
    return mag2;
}
// Find magnitude of a vector.
template< typename VecType, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto magnitude(VecType&& v) {
    return std::sqrt(magnitude2(v));
}
// Normalize the vector in place.
template< typename VecType, std::enable_if_t< IsVecLikeMut<VecType>::value >* = nullptr >
inline void normalize(VecType&& v) {
    VecLikeFloatType<VecType> norm = magnitude(v);
    for(size_t i = 0; i < std::decay_t<VecType>::vec_size; ++i) v[i] /= norm;
}
// Find the normalized vector.
template< typename VecType, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto normalizedVector(VecType&& v) {
    VecLikeVecType<VecType> res = v;
    normalize(res);
    return res;
}

// Find the squared distance between two vectors.
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size>* = nullptr >
inline auto distance2(VT1&& v1, VT2&& v2) {
    std::common_type_t<VecLikeFloatType<VT1>, VecLikeFloatType<VT2>> res {};
    for(size_t idx = 0; idx < std::decay_t<VT1>::vec_size; ++idx) {
        res += (v2[idx] - v1[idx]) * (v2[idx] - v1[idx]);
    }
    return res;
}
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size>* = nullptr >
inline auto distance(VT1&& v1, VT2&& v2) {
    return std::sqrt(distance2(v1, v2));
}

// Vector addition, subtraction, multiplication, division
//-------------------------------------

// vector negation
template< typename VecType, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto operator-(VecType&& v){
    VecLikeVecType<VecType> res;
    for(size_t idx = 0; idx < std::decay_t<VecType>::vec_size; ++idx){
        res[idx] = -v[idx];
    }
    return res;
}
// vector sum
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size>* = nullptr >
inline auto operator+(VT1&& v1, VT2&& v2) {
    Vec< std::decay_t<VT1>::vec_size, std::common_type_t<VecLikeFloatType<VT1>, VecLikeFloatType<VT2>> > res;
    for(size_t idx1 = 0; idx1 < std::decay_t<VT1>::vec_size; ++idx1) {
        res[idx1] = v1[idx1] + v2[idx1];
    }
    return res;
}
// vector increment
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size && IsVecLikeMut<VT1>::value>* = nullptr >
inline auto& operator+=(VT1&& v1, VT2&& v2) {
    for(size_t idx = 0; idx < std::decay_t<VT1>::vec_size; ++idx) {
        v1[idx] += v2[idx];
    }
    return v1;
}
// vector subtraction
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size>* = nullptr >
inline auto operator-(VT1&& v1, VT2&& v2) {
    Vec< std::decay_t<VT1>::vec_size, std::common_type_t<VecLikeFloatType<VT1>, VecLikeFloatType<VT2>> > res;
    for(size_t idx1 = 0; idx1 < std::decay_t<VT1>::vec_size; ++idx1) {
        res[idx1] = v1[idx1] - v2[idx1];
    }
    return res;
}
// vector decrement
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size && IsVecLikeMut<VT1>::value>* = nullptr >
inline auto& operator-=(VT1&& v1, VT2&& v2) {
    for(size_t idx = 0; idx < std::decay_t<VT1>::vec_size; ++idx) {
        v1[idx] -= v2[idx];
    }
    return v1;
}
// vector multiplication
template< typename VecType, typename Float, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto operator*(VecType&& v, Float k) {
    Vec< std::decay_t<VecType>::vec_size, std::common_type_t<VecLikeFloatType<VecType>, Float> > res;
    for(size_t idx = 0; idx < std::decay_t<VecType>::vec_size; ++idx){
        res[idx] = v[idx] * k;
    }
    return res;
}
// vector multiplication
template< typename VecType, typename Float, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto operator*(Float k, VecType&& v) {
    return v * k;
}
// vector in-place multiplication
template< typename VecType, typename Float, std::enable_if_t< IsVecLike<VecType>::value && IsVecLikeMut<VecType>::value >* = nullptr >
inline auto& operator*=(VecType&& v, Float k) {
    for(size_t i = 0; i < std::decay_t<VecType>::vec_size; ++i) v[i] *= k;
    return v;
}
// vector division
template< typename VecType, typename Float, std::enable_if_t< IsVecLike<VecType>::value >* = nullptr >
inline auto operator/(VecType&& v, Float k) {
    return v * (static_cast< std::common_type_t<VecLikeFloatType<VecType>, Float> >(1.0) / k);
}
// vector in-place division
template< typename VecType, typename Float, std::enable_if_t< IsVecLike<VecType>::value && IsVecLikeMut<VecType>::value >* = nullptr >
inline auto& operator/=(VecType&& v, Float k) {
    return v *= (static_cast< std::common_type_t<VecLikeFloatType<VecType>, Float> >(1.0) / k);
}

// dot product, cross product
//-------------------------------------

// dot product
template< typename VT1, typename VT2, std::enable_if_t<std::decay_t<VT1>::vec_size == std::decay_t<VT2>::vec_size>* = nullptr >
inline auto dot(VT1&& v1, VT2&& v2) {
    std::common_type_t<VecLikeFloatType<VT1>, VecLikeFloatType<VT2>> res {};
    for(size_t idx = 0; idx < std::decay_t<VT1>::vec_size; ++idx)
        res += v1[idx] * v2[idx];
    return res;
}
// cross product
template<
    typename VT1, typename VT2,
    std::enable_if_t<std::decay_t<VT1>::vec_size == 3 && std::decay_t<VT2>::vec_size == 3>* = nullptr
>
inline auto cross(VT1&& v1, VT2&& v2) {
    return Vec< 3, std::common_type_t<VecLikeFloatType<VT1>, VecLikeFloatType<VT2>> > {
        v1[1]*v2[2] - v1[2]*v2[1],
        v1[2]*v2[0] - v1[0]*v2[2],
        v1[0]*v2[1] - v1[1]*v2[0]
    };
}

//-----------------------------------------------------------------------------
// VecArray arithmetics
//-----------------------------------------------------------------------------

// Dot product
// Always using the size of the 1st operand.
// Doing dot product on arrays with different sizes leads to undefined behavior.
template<
    typename VA1, typename VA2,
    std::enable_if_t<VA1::element_vec_size == VA2::element_vec_size>* = nullptr >
inline auto dot(const VA1& v1, const VA2& v2) {
    using res_type = std::common_type_t< typename VA1::float_type, typename VA2::float_type >;
    res_type res {};

    const size_t num = v1.value.size();
    for(size_t i = 0; i < num; ++i) {
        res += v1.value[i] * v2.value[i];
    }
    return res;
}

// Increment
// Always using the size of the 1st operand.
// If 2 operands have different sizes, the behavior is undefined
template< typename VA1, typename VA2, std::enable_if_t<VA1::element_vec_size == VA2::element_vec_size>* = nullptr >
inline auto& operator+=(VA1& v1, const VA2& v2) {
    const size_t num = v1.value.size();
    for(size_t i = 0; i < num; ++i) {
        v1.value[i] += v2.value[i];
    }
    return v1;
}

// Decrement
// Always using the size of the 1st operand.
// If 2 operands have different sizes, the behavior is undefined
template< typename VA1, typename VA2, std::enable_if_t<VA1::element_vec_size == VA2::element_vec_size>* = nullptr >
inline auto& operator-=(VA1& v1, const VA2& v2) {
    const size_t num = v1.value.size();
    for(size_t i = 0; i < num; ++i) {
        v1.value[i] -= v2.value[i];
    }
    return v1;
}

} // namespace mathfunc

#endif
