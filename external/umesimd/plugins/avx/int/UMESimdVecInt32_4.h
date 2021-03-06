// The MIT License (MIT)
//
// Copyright (c) 2015-2017 CERN
//
// Author: Przemyslaw Karpinski
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//
//  This piece of code was developed as part of ICE-DIP project at CERN.
//  "ICE-DIP is a European Industrial Doctorate project funded by the European Community's 
//  7th Framework programme Marie Curie Actions under grant PITN-GA-2012-316596".
//

#ifndef UME_SIMD_VEC_INT32_4_H_
#define UME_SIMD_VEC_INT32_4_H_

#include <type_traits>
#include <immintrin.h>

#include "../../../UMESimdInterface.h"

namespace UME {
namespace SIMD {

    template<>
    class SIMDVec_i<int32_t, 4> :
        public SIMDVecSignedInterface<
            SIMDVec_i<int32_t, 4>,
            SIMDVec_u<uint32_t, 4>,
            int32_t,
            4,
            uint32_t,
            SIMDVecMask<4>,
            SIMDSwizzle<4>> ,
        public SIMDVecPackableInterface<
            SIMDVec_i<int32_t, 4>,
            SIMDVec_i<int32_t, 2 >>
    {
        friend class SIMDVec_u<uint32_t, 4>;
        friend class SIMDVec_f<float, 4>;
        friend class SIMDVec_f<double, 4>;

        friend class SIMDVec_i<int32_t, 8>;
    public: // private:
        __m128i mVec;

        UME_FORCE_INLINE explicit SIMDVec_i(__m128i & x) { mVec = x; }
        UME_FORCE_INLINE explicit SIMDVec_i(const __m128i & x) { mVec = x; }
    public:

        constexpr static uint32_t length() { return 4; }
        constexpr static uint32_t alignment() { return 16; }

        // ZERO-CONSTR
        UME_FORCE_INLINE SIMDVec_i() {};

        // SET-CONSTR
        UME_FORCE_INLINE SIMDVec_i(int32_t i) {
            mVec = _mm_set1_epi32(i);
        }
        // This constructor is used to force types other than SCALAR_TYPES
        // to be promoted to SCALAR_TYPE instead of SCALAR_TYPE*. This prevents
        // ambiguity between SET-CONSTR and LOAD-CONSTR.
        template<typename T>
        UME_FORCE_INLINE SIMDVec_i(
            T i, 
            typename std::enable_if<std::is_fundamental<T>::value &&
                                    !std::is_same<T, int32_t>::value,
                                    void*>::type = nullptr)
        : SIMDVec_i(static_cast<int32_t>(i)) {}
        // LOAD-CONSTR
        UME_FORCE_INLINE explicit SIMDVec_i(int32_t const *p) { this->load(p); };
        // FULL-CONSTR
        UME_FORCE_INLINE SIMDVec_i(int32_t i0, int32_t i1, int32_t i2, int32_t i3)
        {
            mVec = _mm_setr_epi32(i0, i1, i2, i3);
        }
        // EXTRACT
        UME_FORCE_INLINE int32_t extract(uint32_t index) const {
            //return _mm256_extract_epi32(mVec, index); // TODO: this can be implemented in ICC
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i *)raw, mVec);
            return raw[index];
        }
        UME_FORCE_INLINE int32_t operator[] (uint32_t index) const {
            return extract(index);
        }

        // INSERT
        UME_FORCE_INLINE SIMDVec_i & insert(uint32_t index, int32_t value) {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            raw[index] = value;
            mVec = _mm_load_si128((__m128i*)raw);
            return *this;
        }
        UME_FORCE_INLINE IntermediateIndex<SIMDVec_i, int32_t> operator[] (uint32_t index) {
            return IntermediateIndex<SIMDVec_i, int32_t>(index, static_cast<SIMDVec_i &>(*this));
        }

        // Override Mask Access operators
#if defined(USE_PARENTHESES_IN_MASK_ASSIGNMENT)
        UME_FORCE_INLINE IntermediateMask<SIMDVec_i, int32_t, SIMDVecMask<4>> operator() (SIMDVecMask<4> const & mask) {
            return IntermediateMask<SIMDVec_i, int32_t, SIMDVecMask<4>>(mask, static_cast<SIMDVec_i &>(*this));
        }
#else
        UME_FORCE_INLINE IntermediateMask<SIMDVec_i, int32_t, SIMDVecMask<4>> operator[] (SIMDVecMask<4> const & mask) {
            return IntermediateMask<SIMDVec_i, int32_t, SIMDVecMask<4>>(mask, static_cast<SIMDVec_i &>(*this));
        }
#endif

        // ****************************************************************************************
        // Overloading Interface functions starts here!
        // ****************************************************************************************

        // ASSIGNV
        UME_FORCE_INLINE SIMDVec_i & assign(SIMDVec_i const & b) {
            mVec = b.mVec;
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator=(SIMDVec_i const & b) {
            return assign(b);
        }
        // MASSIGNV
        UME_FORCE_INLINE SIMDVec_i & assign(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            mVec = _mm_blendv_epi8(mVec, b.mVec, mask.mMask);
            return *this;
        }
        // ASSIGNS
        UME_FORCE_INLINE SIMDVec_i & assign(int32_t b) {
            mVec = _mm_set1_epi32(b);
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator= (int32_t b) {
            return assign(b);
        }
        // MASSIGNS
        UME_FORCE_INLINE SIMDVec_i & assign(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }

        // PREFETCH0
        // PREFETCH1
        // PREFETCH2

        // LOAD
        UME_FORCE_INLINE SIMDVec_i & load(int32_t const * p) {
            mVec = _mm_loadu_si128((__m128i*)p);
            return *this;
        }
        // MLOAD
        UME_FORCE_INLINE SIMDVec_i & load(SIMDVecMask<4> const & mask, int32_t const * p) {
            __m128i t0 = _mm_loadu_si128((__m128i*)p);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // LOADA
        UME_FORCE_INLINE SIMDVec_i & loada(int32_t const * p) {
            mVec = _mm_load_si128((__m128i*)p);
            return *this;
        }
        // MLOADA
        UME_FORCE_INLINE SIMDVec_i & loada(SIMDVecMask<4> const & mask, int32_t const * p) {
            __m128i t0 = _mm_load_si128((__m128i*)p);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // STORE
        UME_FORCE_INLINE int32_t * store(int32_t * p) const {
            _mm_storeu_si128((__m128i*) p, mVec);
            return p;
        }
        // MSTORE
        UME_FORCE_INLINE int32_t * store(SIMDVecMask<4> const & mask, int32_t * p) const {
            __m128i t0 = _mm_loadu_si128((__m128i*)p);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_storeu_si128((__m128i*) p, t1);
            return p;
        }
        // STOREA
        UME_FORCE_INLINE int32_t * storea(int32_t * p) const {
            _mm_store_si128((__m128i *)p, mVec);
            return p;
        }
        // MSTOREA
        UME_FORCE_INLINE int32_t * storea(SIMDVecMask<4> const & mask, int32_t * p) const {
            __m128i t0 = _mm_load_si128((__m128i*)p);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*) p, t1);
            return p;
        }

        // BLENDV
        UME_FORCE_INLINE SIMDVec_i blend(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_blendv_epi8(mVec, b.mVec, mask.mMask);
            return SIMDVec_i(t0);
        }
        // BLENDS
        UME_FORCE_INLINE SIMDVec_i blend(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // SWIZZLE
        // SWIZZLEA

        // SORTA
        UME_FORCE_INLINE SIMDVec_i sorta() {
            __m128i t0 = _mm_shuffle_epi32(mVec, 0xB1); // permute BADC
            __m128i t1 = _mm_min_epi32(mVec, t0);
            __m128i t2 = _mm_max_epi32(mVec, t0);
            __m128i t3 = _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(t1), _mm_castsi128_ps(t2), 0x06));
            __m128i t4 = _mm_shuffle_epi32(t3, 0x4E);   // permute CDAB
            __m128i t5 = _mm_min_epi32(t3, t4);
            __m128i t6 = _mm_max_epi32(t3, t4);
            __m128i t7 = _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(t5), _mm_castsi128_ps(t6), 0x0C));
            __m128i t8 = _mm_shuffle_epi32(t7, 0xB1); // permute BADC
            __m128i t9 = _mm_min_epi32(t7, t8);
            __m128i t10 = _mm_max_epi32(t7, t8);
            __m128i t11 = _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(t9), _mm_castsi128_ps(t10), 0x0A));
            return SIMDVec_i(t11);
        }
        // SORTD
        UME_FORCE_INLINE SIMDVec_i sortd() {
            __m128i t0 = _mm_shuffle_epi32(mVec, 0xB1); // permute BADC
            __m128i t1 = _mm_min_epi32(mVec, t0);
            __m128i t2 = _mm_max_epi32(mVec, t0);
            __m128i t3 = _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(t2), _mm_castsi128_ps(t1), 0x06));
            __m128i t4 = _mm_shuffle_epi32(t3, 0x4E);   // permute CDAB
            __m128i t5 = _mm_min_epi32(t3, t4);
            __m128i t6 = _mm_max_epi32(t3, t4);
            __m128i t7 = _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(t6), _mm_castsi128_ps(t5), 0x0C));
            __m128i t8 = _mm_shuffle_epi32(t7, 0xB1); // permute BADC
            __m128i t9 = _mm_min_epi32(t7, t8);
            __m128i t10 = _mm_max_epi32(t7, t8);
            __m128i t11 = _mm_castps_si128(_mm_blend_ps(_mm_castsi128_ps(t10), _mm_castsi128_ps(t9), 0x0A));
            return SIMDVec_i(t11);
        }

        // ADDV
        UME_FORCE_INLINE SIMDVec_i add(SIMDVec_i const & b) const {
            __m128i t0 = _mm_add_epi32(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        UME_FORCE_INLINE SIMDVec_i operator+ (SIMDVec_i const & b) const {
            return add(b);
        }
        // MADDV
        UME_FORCE_INLINE SIMDVec_i add(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_add_epi32(mVec, b.mVec);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // ADDS
        UME_FORCE_INLINE SIMDVec_i add(int32_t b) const {
            __m128i t0 = _mm_add_epi32(mVec, _mm_set1_epi32(b));
            return SIMDVec_i(t0);
        }
        UME_FORCE_INLINE SIMDVec_i operator+ (int32_t b) const {
            return add(b);
        }
        // MADDS
        UME_FORCE_INLINE SIMDVec_i add(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_add_epi32(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // ADDVA
        UME_FORCE_INLINE SIMDVec_i & adda(SIMDVec_i const & b) {
            mVec = _mm_add_epi32(mVec, b.mVec);
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator+= (SIMDVec_i const & b) {
            return adda(b);
        }
        // MADDVA
        UME_FORCE_INLINE SIMDVec_i & adda(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_add_epi32(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // ADDSA
        UME_FORCE_INLINE SIMDVec_i & adda(int32_t b) {
            mVec = _mm_add_epi32(mVec, _mm_set1_epi32(b));
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator+= (int32_t b) {
            return adda(b);
        }
        // MADDSA
        UME_FORCE_INLINE SIMDVec_i & adda(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_add_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // SADDV
        // MSADDV
        // SADDS
        // MSADDS
        // SADDVA
        // MSADDVA
        // SADDSA
        // MSADDSA
        // POSTINC
        UME_FORCE_INLINE SIMDVec_i postinc() {
            __m128i t0 = _mm_set1_epi32(1);
            __m128i t1 = mVec;
            mVec = _mm_add_epi32(mVec, t0);
            return SIMDVec_i(t1);
        }
        UME_FORCE_INLINE SIMDVec_i operator++ (int) {
            return postinc();
        }
        // MPOSTINC
        UME_FORCE_INLINE SIMDVec_i postinc(SIMDVecMask<4> const & mask) {
            __m128i t0 = _mm_set1_epi32(1);
            __m128i t1 = mVec;
            __m128i t2 = _mm_add_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t2, mask.mMask);
            return SIMDVec_i(t1);
        }
        // PREFINC
        UME_FORCE_INLINE SIMDVec_i & prefinc() {
            __m128i t0 = _mm_set1_epi32(1);
            mVec = _mm_add_epi32(mVec, t0);
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator++ () {
            return prefinc();
        }
        // MPREFINC
        UME_FORCE_INLINE SIMDVec_i & prefinc(SIMDVecMask<4> const & mask) {
            __m128i t0 = _mm_set1_epi32(1);
            __m128i t1 = _mm_add_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // SUBV
        UME_FORCE_INLINE SIMDVec_i sub(SIMDVec_i const & b) const {
            __m128i t0 = _mm_sub_epi32(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        UME_FORCE_INLINE SIMDVec_i operator- (SIMDVec_i const & b) const {
            return sub(b);
        }
        // MSUBV
        UME_FORCE_INLINE SIMDVec_i sub(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_sub_epi32(mVec, b.mVec);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // SUBS
        UME_FORCE_INLINE SIMDVec_i sub(int32_t b) const {
            __m128i t0 = _mm_sub_epi32(mVec, _mm_set1_epi32(b));
            return SIMDVec_i(t0);
        }
        UME_FORCE_INLINE SIMDVec_i operator- (int32_t b) const {
            return sub(b);
        }
        // MSUBS
        UME_FORCE_INLINE SIMDVec_i sub(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_sub_epi32(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // SUBVA
        UME_FORCE_INLINE SIMDVec_i & suba(SIMDVec_i const & b) {
            mVec = _mm_sub_epi32(mVec, b.mVec);
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator-= (SIMDVec_i const & b) {
            return suba(b);
        }
        // MSUBVA
        UME_FORCE_INLINE SIMDVec_i & suba(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_sub_epi32(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // SUBSA
        UME_FORCE_INLINE SIMDVec_i & suba(int32_t b) {
            mVec = _mm_sub_epi32(mVec, _mm_set1_epi32(b));
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator-= (int32_t b) {
            return suba(b);
        }
        // MSUBSA
        UME_FORCE_INLINE SIMDVec_i & suba(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_sub_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // SSUBV
        // MSSUBV
        // SSUBS
        // MSSUBS
        // SSUBVA
        // MSSUBVA
        // SSUBSA
        // MSSUBSA
        // SUBFROMV
        UME_FORCE_INLINE SIMDVec_i subfrom(SIMDVec_i const & b) const {
            __m128i t0 = _mm_sub_epi32(b.mVec, mVec);
            return SIMDVec_i(t0);
        }
        // MSUBFROMV
        UME_FORCE_INLINE SIMDVec_i subfrom(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t1 = _mm_sub_epi32(b.mVec, mVec);
            __m128i t0 = _mm_blendv_epi8(b.mVec, t1, mask.mMask);
            return SIMDVec_i(t0);
        }
        // SUBFROMS
        UME_FORCE_INLINE SIMDVec_i subfrom(int32_t b) const {
            __m128i t0 = _mm_sub_epi32(_mm_set1_epi32(b), mVec);
            return SIMDVec_i(t0);
        }
        // MSUBFROMS
        UME_FORCE_INLINE SIMDVec_i subfrom(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t2 = _mm_sub_epi32(t0, mVec);
            __m128i t1 = _mm_blendv_epi8(t0, t2, mask.mMask);
            return SIMDVec_i(t1);
        }
        // SUBFROMVA
        UME_FORCE_INLINE SIMDVec_i & subfroma(SIMDVec_i const & b) {
            mVec = _mm_sub_epi32(b.mVec, mVec);
            return *this;
        }
        // MSUBFROMVA
        UME_FORCE_INLINE SIMDVec_i & subfroma(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t1 = _mm_sub_epi32(b.mVec, mVec);
            mVec = _mm_blendv_epi8(b.mVec, t1, mask.mMask);
            return *this;
        }
        // SUBFROMSA
        UME_FORCE_INLINE SIMDVec_i & subfroma(int32_t b) {
            mVec = _mm_sub_epi32(_mm_set1_epi32(b), mVec);
            return *this;
        }
        // MSUBFROMSA
        UME_FORCE_INLINE SIMDVec_i subfroma(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t2 = _mm_sub_epi32(t0, mVec);
            mVec = _mm_blendv_epi8(t0, t2, mask.mMask);
            return *this;
        }
        // POSTDEC
        UME_FORCE_INLINE SIMDVec_i postdec() {
            __m128i t0 = _mm_set1_epi32(1);
            __m128i t1 = mVec;
            mVec = _mm_sub_epi32(mVec, t0);
            return SIMDVec_i(t1);
        }
        UME_FORCE_INLINE SIMDVec_i operator-- (int) {
            return postdec();
        }
        // MPOSTDEC
        UME_FORCE_INLINE SIMDVec_i postdec(SIMDVecMask<4> const & mask) {
            __m128i t0 = _mm_set1_epi32(1);
            __m128i t1 = mVec;
            __m128i t2 = _mm_sub_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t2, mask.mMask);
            return SIMDVec_i(t1);
        }
        // PREFDEC
        UME_FORCE_INLINE SIMDVec_i & prefdec() {
            __m128i t0 = _mm_set1_epi32(1);
            mVec = _mm_sub_epi32(mVec, t0);
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator-- () {
            return prefdec();
        }
        // MPREFDEC
        UME_FORCE_INLINE SIMDVec_i & prefdec(SIMDVecMask<4> const & mask) {
            __m128i t0 = _mm_set1_epi32(1);
            __m128i t2 = _mm_sub_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t2, mask.mMask);
            return *this;
        }
        // MULV
        UME_FORCE_INLINE SIMDVec_i mul(SIMDVec_i const & b) const {
            __m128i t0 = _mm_mullo_epi32(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        UME_FORCE_INLINE SIMDVec_i operator* (SIMDVec_i const & b) const {
            return mul(b);
        }
        // MMULV
        UME_FORCE_INLINE SIMDVec_i mul(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t1 = _mm_mullo_epi32(mVec, b.mVec);
            __m128i t0 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t0);
        }
        // MULS
        UME_FORCE_INLINE SIMDVec_i mul(int32_t b) const {
            __m128i t0 = _mm_mullo_epi32(mVec, _mm_set1_epi32(b));
            return SIMDVec_i(t0);
        }
        UME_FORCE_INLINE SIMDVec_i operator* (int32_t b) const {
            return mul(b);
        }
        // MMULS
        UME_FORCE_INLINE SIMDVec_i mul(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t2 = _mm_mullo_epi32(mVec, t0);
            __m128i t1 = _mm_blendv_epi8(mVec, t2, mask.mMask);
            return SIMDVec_i(t1);
        }
        // MULVA
        UME_FORCE_INLINE SIMDVec_i & mula(SIMDVec_i const & b) {
            mVec = _mm_mullo_epi32(mVec, b.mVec);
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator*= (SIMDVec_i const & b) {
            return mula(b);
        }
        // MMULVA
        UME_FORCE_INLINE SIMDVec_i & mula(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_mullo_epi32(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // MULSA
        UME_FORCE_INLINE SIMDVec_i & mula(int32_t b) {
            mVec = _mm_mullo_epi32(mVec, _mm_set1_epi32(b));
            return *this;
        }
        UME_FORCE_INLINE SIMDVec_i & operator*= (int32_t b) {
            return mula(b);
        }
        // MMULSA
        UME_FORCE_INLINE SIMDVec_i & mula(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_mullo_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // DIVV
        // MDIVV
        // DIVS
        // MDIVS
        // DIVVA
        // MDIVVA
        // DIVSA
        // MDIVSA
        // RCP
        // MRCP
        // RCPS
        // MRCPS
        // RCPA
        // MRCPA
        // RCPSA
        // MRCPSA

        // CMPEQV
        UME_FORCE_INLINE SIMDVecMask<4> cmpeq(SIMDVec_i const & b) const {
            __m128i t0 = _mm_cmpeq_epi32(mVec, b.mVec);
            return SIMDVecMask<4>(t0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator==(SIMDVec_i const & b) const {
            return cmpeq(b);
        }
        // CMPEQS
        UME_FORCE_INLINE SIMDVecMask<4> cmpeq(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_cmpeq_epi32(mVec, t0);
            return SIMDVecMask<4>(t1);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator== (int32_t b) const {
            return cmpeq(b);
        }
        // CMPNEV
        UME_FORCE_INLINE SIMDVecMask<4> cmpne(SIMDVec_i const & b) const {
            __m128i t0 = _mm_cmpeq_epi32(mVec, b.mVec);
            __m128i m0 = _mm_set1_epi32(SIMDVecMask<4>::TRUE_VAL());
            __m128i t1 = _mm_xor_si128(t0, m0);
            return SIMDVecMask<4>(t1);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator!= (SIMDVec_i const & b) const {
            return cmpne(b);
        }
        // CMPNES
        UME_FORCE_INLINE SIMDVecMask<4> cmpne(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_cmpeq_epi32(mVec, t0);
            __m128i m0 = _mm_set1_epi32(SIMDVecMask<4>::TRUE_VAL());
            __m128i t2 = _mm_xor_si128(t1, m0);
            return SIMDVecMask<4>(t2);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator!= (int32_t b) const {
            return cmpne(b);
        }
        // CMPGTV
        UME_FORCE_INLINE SIMDVecMask<4> cmpgt(SIMDVec_i const & b) const {
            __m128i m0 = _mm_cmpgt_epi32(mVec, b.mVec);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator> (SIMDVec_i const & b) const {
            return cmpgt(b);
        }
        // CMPGTS
        UME_FORCE_INLINE SIMDVecMask<4> cmpgt(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i m0 = _mm_cmpgt_epi32(mVec, t0);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator> (int32_t b) const {
            return cmpgt(b);
        }
        // CMPLTV
        UME_FORCE_INLINE SIMDVecMask<4> cmplt(SIMDVec_i const & b) const {
            __m128i m0 = _mm_cmplt_epi32(mVec, b.mVec);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator< (SIMDVec_i const & b) const {
            return cmplt(b);
        }
        // CMPLTS
        UME_FORCE_INLINE SIMDVecMask<4> cmplt(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i m0 = _mm_cmplt_epi32(mVec, t0);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator< (int32_t b) const {
            return cmplt(b);
        }
        // CMPGEV
        UME_FORCE_INLINE SIMDVecMask<4> cmpge(SIMDVec_i const & b) const {
            __m128i t0 = _mm_max_epi32(mVec, b.mVec);
            __m128i m0 = _mm_cmpeq_epi32(mVec, t0);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator>= (SIMDVec_i const & b) const {
            return cmpge(b);
        }
        // CMPGES
        UME_FORCE_INLINE SIMDVecMask<4> cmpge(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_max_epi32(mVec, t0);
            __m128i m0 = _mm_cmpeq_epi32(mVec, t1);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator>= (int32_t b) const {
            return cmpge(b);
        }
        // CMPLEV
        UME_FORCE_INLINE SIMDVecMask<4> cmple(SIMDVec_i const & b) const {
            __m128i t0 = _mm_max_epi32(mVec, b.mVec);
            __m128i m0 = _mm_cmpeq_epi32(b.mVec, t0);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator<= (SIMDVec_i const & b) const {
            return cmple(b);
        }
        // CMPLES
        UME_FORCE_INLINE SIMDVecMask<4> cmple(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_max_epi32(mVec, t0);
            __m128i m0 = _mm_cmpeq_epi32(t0, t1);
            return SIMDVecMask<4>(m0);
        }
        UME_FORCE_INLINE SIMDVecMask<4> operator<= (int32_t b) const {
            return cmple(b);
        }
        // CMPEV
        UME_FORCE_INLINE bool cmpe(SIMDVec_i const & b) const {
            alignas(16) int32_t raw[4];
            __m128i m0 = _mm_cmpeq_epi32(mVec, b.mVec);
            _mm_store_si128((__m128i*)raw, m0);
            return (raw[0] != 0) && (raw[1] != 0) && (raw[2] != 0) && (raw[3] !=0);
        }
        // CMPES
        UME_FORCE_INLINE bool cmpe(int32_t b) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(b);
            __m128i m0 = _mm_cmpeq_epi32(mVec, t0);
            _mm_store_si128((__m128i*)raw, m0);
            return (raw[0] != 0) && (raw[1] != 0) && (raw[2] != 0) && (raw[3] !=0);
        }
        // UNIQUE
        UME_FORCE_INLINE bool unique() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            for (unsigned int i = 0; i < 3; i++) {
                for (unsigned int j = i + 1; j < 4; j++) {
                    if (raw[i] == raw[j]) return false;
                }
            }
            return true;
        }
        // HADD
        UME_FORCE_INLINE int32_t hadd() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] + raw[1] + raw[2] + raw[3];
        }
        // MHADD
        UME_FORCE_INLINE int32_t hadd(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_blendv_epi8(_mm_set1_epi32(0), mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t0);
            return raw[0] + raw[1] + raw[2] + raw[3];
        }
        // HADDS
        UME_FORCE_INLINE int32_t hadd(int32_t b) const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] + raw[1] + raw[2] + raw[3] + b;
        }
        // MHADDS
        UME_FORCE_INLINE int32_t hadd(SIMDVecMask<4> const & mask, int32_t b) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_blendv_epi8(_mm_set1_epi32(0), mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t0);
            return raw[0] + raw[1] + raw[2] + raw[3] + b;
        }
        // HMUL
        UME_FORCE_INLINE int32_t hmul() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] * raw[1] * raw[2] * raw[3];
        }
        // MHMUL
        UME_FORCE_INLINE int32_t hmul(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_blendv_epi8(_mm_set1_epi32(1), mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t0);
            return raw[0] * raw[1] * raw[2] * raw[3];
        }
        // HMULS
        UME_FORCE_INLINE int32_t hmul(int32_t b) const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] * raw[1] * raw[2] * raw[3] * b;
        }
        // MHMULS
        UME_FORCE_INLINE int32_t hmul(SIMDVecMask<4> const & mask, int32_t b) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_blendv_epi8(_mm_set1_epi32(1), mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t0);
            return raw[0] * raw[1] * raw[2] * raw[3] * b;
        }
        // FMULADDV
        UME_FORCE_INLINE SIMDVec_i fmuladd(SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_mullo_epi32(mVec, b.mVec);
            __m128i t1 = _mm_add_epi32(t0, c.mVec);
            return SIMDVec_i(t1);
        }
        // MFMULADDV
        UME_FORCE_INLINE SIMDVec_i fmuladd(SIMDVecMask<4> const & mask, SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_mullo_epi32(mVec, b.mVec);
            __m128i t1 = _mm_add_epi32(t0, c.mVec);
            t1 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t1);
        }
        // FMULSUBV
        UME_FORCE_INLINE SIMDVec_i fmulsub(SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_mullo_epi32(mVec, b.mVec);
            __m128i t1 = _mm_sub_epi32(t0, c.mVec);
            return SIMDVec_i(t1);
        }
        // MFMULSUBV
        UME_FORCE_INLINE SIMDVec_i fmulsub(SIMDVecMask<4> const & mask, SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_mullo_epi32(mVec, b.mVec);
            __m128i t1 = _mm_sub_epi32(t0, c.mVec);
            t1 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t1);
        }
        // FADDMULV
        UME_FORCE_INLINE SIMDVec_i faddmul(SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_add_epi32(mVec, b.mVec);
            __m128i t1 = _mm_mullo_epi32(t0, c.mVec);
            return SIMDVec_i(t1);
        }
        // MFADDMULV
        UME_FORCE_INLINE SIMDVec_i faddmul(SIMDVecMask<4> const & mask, SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_add_epi32(mVec, b.mVec);
            __m128i t1 = _mm_mullo_epi32(t0, c.mVec);
            t1 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t1);
        }
        // FSUBMULV
        UME_FORCE_INLINE SIMDVec_i fsubmul(SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_sub_epi32(mVec, b.mVec);
            __m128i t1 = _mm_mullo_epi32(t0, c.mVec);
            return SIMDVec_i(t1);
        }
        // MFSUBMULV
        UME_FORCE_INLINE SIMDVec_i fsubmul(SIMDVecMask<4> const & mask, SIMDVec_i const & b, SIMDVec_i const & c) const {
            __m128i t0 = _mm_sub_epi32(mVec, b.mVec);
            __m128i t1 = _mm_mullo_epi32(t0, c.mVec);
            t1 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t1);
        }
        // MAXV
        UME_FORCE_INLINE SIMDVec_i max(SIMDVec_i const & b) const {
            __m128i t0 = _mm_max_epi32(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        // MMAXV
        UME_FORCE_INLINE SIMDVec_i max(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t1 = _mm_max_epi32(mVec, b.mVec);
            __m128i t0 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t0);
        }
        // MAXS
        UME_FORCE_INLINE SIMDVec_i max(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_max_epi32(mVec, t0);
            return SIMDVec_i(t1);
        }
        // MMAXS
        UME_FORCE_INLINE SIMDVec_i max(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t2 = _mm_max_epi32(mVec, t0);
            __m128i t1 = _mm_blendv_epi8(mVec, t2, mask.mMask);
            return SIMDVec_i(t1);
        }
        // MAXVA
        UME_FORCE_INLINE SIMDVec_i & maxa(SIMDVec_i const & b) {
            mVec = _mm_max_epi32(mVec, b.mVec);
            return *this;
        }
        // MMAXVA
        UME_FORCE_INLINE SIMDVec_i & maxa(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t1 = _mm_max_epi32(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // MAXSA
        UME_FORCE_INLINE SIMDVec_i & maxa(int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            mVec = _mm_max_epi32(mVec, t0);
            return *this;
        }
        // MMAXSA
        UME_FORCE_INLINE SIMDVec_i & maxa(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_max_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // MINV
        UME_FORCE_INLINE SIMDVec_i min(SIMDVec_i const & b) const {
            __m128i t0 = _mm_min_epi32(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        // MMINV
        UME_FORCE_INLINE SIMDVec_i min(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_min_epi32(mVec, b.mVec);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // MINS
        UME_FORCE_INLINE SIMDVec_i min(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_min_epi32(mVec, t0);
            return SIMDVec_i(t1);
        }
        // MMINS
        UME_FORCE_INLINE SIMDVec_i min(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_min_epi32(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // MINVA
        UME_FORCE_INLINE SIMDVec_i & mina(SIMDVec_i const & b) {
            mVec = _mm_min_epi32(mVec, b.mVec);
            return *this;
        }
        // MMINVA
        UME_FORCE_INLINE SIMDVec_i & mina(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_min_epi32(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // MINSA
        UME_FORCE_INLINE SIMDVec_i & mina(int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            mVec = _mm_min_epi32(mVec, t0);
            return *this;
        }
        // MMINSA
        UME_FORCE_INLINE SIMDVec_i & mina(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_min_epi32(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // HMAX
        UME_FORCE_INLINE int32_t hmax() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            int32_t t0 = (raw[0] > raw[1]) ? raw[0] : raw[1];
            int32_t t1 = (raw[2] > raw[3]) ? raw[2] : raw[3];
            return t0 > t1 ? t0 : t1;
        }
        // MHMAX
        UME_FORCE_INLINE int32_t hmax(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            int32_t t2 = (raw[0] > raw[1]) ? raw[0] : raw[1];
            int32_t t3 = (raw[2] > raw[3]) ? raw[2] : raw[3];
            return t2 > t3 ? t2 : t3;
        }
        // IMAX
        // MIMAX
        // HMIN
        UME_FORCE_INLINE int32_t hmin() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            int32_t t0 = (raw[0] < raw[1]) ? raw[0] : raw[1];
            int32_t t1 = (raw[2] < raw[3]) ? raw[2] : raw[3];
            return t0 < t1 ? t0 : t1;
        }
        // MHMIN
        UME_FORCE_INLINE int32_t hmin(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            int32_t t2 = (raw[0] < raw[1]) ? raw[0] : raw[1];
            int32_t t3 = (raw[2] < raw[3]) ? raw[2] : raw[3];
            return t2 < t3 ? t2 : t3;
        }
        // IMIN
        // MIMIN

        // BANDV
        UME_FORCE_INLINE SIMDVec_i band(SIMDVec_i const & b) const {
            __m128i t0 = _mm_and_si128(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        // MBANDV
        UME_FORCE_INLINE SIMDVec_i band(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_and_si128(mVec, b.mVec);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // BANDS
        UME_FORCE_INLINE SIMDVec_i band(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_and_si128(mVec, t0);
            return SIMDVec_i(t1);
        }
        // MBANDS
        UME_FORCE_INLINE SIMDVec_i band(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_and_si128(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // BANDVA
        UME_FORCE_INLINE SIMDVec_i & banda(SIMDVec_i const & b) {
            mVec = _mm_and_si128(mVec, b.mVec);
            return *this;
        }
        // MBANDVA
        UME_FORCE_INLINE SIMDVec_i & banda(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_and_si128(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // BANDSA
        UME_FORCE_INLINE SIMDVec_i & banda(int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            mVec = _mm_and_si128(mVec, t0);
            return *this;
        }
        // MBANDSA
        UME_FORCE_INLINE SIMDVec_i & banda(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_and_si128(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // BORV
        UME_FORCE_INLINE SIMDVec_i bor(SIMDVec_i const & b) const {
            __m128i t0 = _mm_or_si128(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        // MBORV
        UME_FORCE_INLINE SIMDVec_i bor(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_or_si128(mVec, b.mVec);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // BORS
        UME_FORCE_INLINE SIMDVec_i bor(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_or_si128(mVec, t0);
            return SIMDVec_i(t1);
        }
        // MBORS
        UME_FORCE_INLINE SIMDVec_i bor(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_or_si128(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // BORVA
        UME_FORCE_INLINE SIMDVec_i & bora(SIMDVec_i const & b) {
            mVec = _mm_or_si128(mVec, b.mVec);
            return *this;
        }
        // MBORVA
        UME_FORCE_INLINE SIMDVec_i & bora(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_or_si128(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // BORSA
        UME_FORCE_INLINE SIMDVec_i & bora(int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            mVec = _mm_or_si128(mVec, t0);
            return *this;
        }
        // MBORSA
        UME_FORCE_INLINE SIMDVec_i & bora(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_or_si128(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // BXORV
        UME_FORCE_INLINE SIMDVec_i bxor(SIMDVec_i const & b) const {
            __m128i t0 = _mm_xor_si128(mVec, b.mVec);
            return SIMDVec_i(t0);
        }
        // MBXORV
        UME_FORCE_INLINE SIMDVec_i bxor(SIMDVecMask<4> const & mask, SIMDVec_i const & b) const {
            __m128i t0 = _mm_xor_si128(mVec, b.mVec);
            __m128i t1 = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return SIMDVec_i(t1);
        }
        // BXORS
        UME_FORCE_INLINE SIMDVec_i bxor(int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_xor_si128(mVec, t0);
            return SIMDVec_i(t1);
        }
        // MBXORS
        UME_FORCE_INLINE SIMDVec_i bxor(SIMDVecMask<4> const & mask, int32_t b) const {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_xor_si128(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // BXORVA
        UME_FORCE_INLINE SIMDVec_i & bxora(SIMDVec_i const & b) {
            mVec = _mm_xor_si128(mVec, b.mVec);
            return *this;
        }
        // MBXORVA
        UME_FORCE_INLINE SIMDVec_i & bxora(SIMDVecMask<4> const & mask, SIMDVec_i const & b) {
            __m128i t0 = _mm_xor_si128(mVec, b.mVec);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // BXORSA
        UME_FORCE_INLINE SIMDVec_i & bxora(int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            mVec = _mm_xor_si128(mVec, t0);
            return *this;
        }
        // MBXORSA
        UME_FORCE_INLINE SIMDVec_i & bxora(SIMDVecMask<4> const & mask, int32_t b) {
            __m128i t0 = _mm_set1_epi32(b);
            __m128i t1 = _mm_xor_si128(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // BNOT
        UME_FORCE_INLINE SIMDVec_i bnot() const {
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            __m128i t1 = _mm_xor_si128(mVec, t0);
            return SIMDVec_i(t1);
        }
        // MBNOT
        UME_FORCE_INLINE SIMDVec_i bnot(SIMDVecMask<4> const & mask) const {
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            __m128i t1 = _mm_xor_si128(mVec, t0);
            __m128i t2 = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return SIMDVec_i(t2);
        }
        // BNOTA
        UME_FORCE_INLINE SIMDVec_i & bnota() {
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            mVec = _mm_xor_si128(mVec, t0);
            return *this;
        }
        // MBNOTA
        UME_FORCE_INLINE SIMDVec_i bnota(SIMDVecMask<4> const & mask) {
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            __m128i t1 = _mm_xor_si128(mVec, t0);
            mVec = _mm_blendv_epi8(mVec, t1, mask.mMask);
            return *this;
        }
        // HBAND
        UME_FORCE_INLINE int32_t hband() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] & raw[1] & raw[2] & raw[3];
        }
        // MHBAND
        UME_FORCE_INLINE int32_t hband(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            return raw[0] & raw[1] & raw[2] & raw[3];
        }
        // HBANDS
        UME_FORCE_INLINE int32_t hband(int32_t b) const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] & raw[1] & raw[2] & raw[3] & b;
        }
        // MHBANDS
        UME_FORCE_INLINE int32_t hband(SIMDVecMask<4> const & mask, int32_t b) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0xFFFFFFFF);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            return raw[0] & raw[1] & raw[2] & raw[3] & b;
        }
        // HBOR
        UME_FORCE_INLINE int32_t hbor() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] | raw[1] | raw[2] | raw[3];
        }
        // MHBOR
        UME_FORCE_INLINE int32_t hbor(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            return raw[0] | raw[1] | raw[2] | raw[3];
        }
        // HBORS
        UME_FORCE_INLINE int32_t hbor(int32_t b) const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] | raw[1] | raw[2] | raw[3] | b;
        }
        // MHBORS
        UME_FORCE_INLINE int32_t hbor(SIMDVecMask<4> const & mask, int32_t b) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            return raw[0] | raw[1] | raw[2] | raw[3] | b;
        }
        // HBXOR
        UME_FORCE_INLINE int32_t hbxor() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] ^ raw[1] ^ raw[2] ^ raw[3];
        }
        // MHBXOR
        UME_FORCE_INLINE int32_t hbxor(SIMDVecMask<4> const & mask) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            return raw[0] ^ raw[1] ^ raw[2] ^ raw[3];
        }
        // HBXORS
        UME_FORCE_INLINE int32_t hbxor(int32_t b) const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*)raw, mVec);
            return raw[0] ^ raw[1] ^ raw[2] ^ raw[3] ^ b;
        }
        // MHBXORS
        UME_FORCE_INLINE int32_t hbxor(SIMDVecMask<4> const & mask, int32_t b) const {
            alignas(16) int32_t raw[4];
            __m128i t0 = _mm_set1_epi32(0);
            __m128i t1 = _mm_blendv_epi8(t0, mVec, mask.mMask);
            _mm_store_si128((__m128i*)raw, t1);
            return raw[0] ^ raw[1] ^ raw[2] ^ raw[3] ^ b;
        }

        // GATHERS
        UME_FORCE_INLINE SIMDVec_i & gather(int32_t const * baseAddr, uint32_t const * indices) {
            alignas(16) int32_t raw[4] = { baseAddr[indices[0]], baseAddr[indices[1]], baseAddr[indices[2]], baseAddr[indices[3]] };
            mVec = _mm_load_si128((__m128i*)raw);
            return *this;
        }
        // MGATHERS
        UME_FORCE_INLINE SIMDVec_i & gather(SIMDVecMask<4> const & mask, int32_t const * baseAddr, uint32_t const * indices) {
            alignas(16) int32_t raw[4] = { baseAddr[indices[0]], baseAddr[indices[1]], baseAddr[indices[2]], baseAddr[indices[3]] };
            __m128i t0 = _mm_load_si128((__m128i*)raw);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // GATHERV
        UME_FORCE_INLINE SIMDVec_i & gather(int32_t const * baseAddr, SIMDVec_i const & indices) {
            alignas(16) int32_t rawInd[4];
            alignas(16) int32_t raw[4];

            _mm_store_si128((__m128i*) rawInd, indices.mVec);
            for (int i = 0; i < 4; i++) { raw[i] = baseAddr[rawInd[i]]; }
            mVec = _mm_load_si128((__m128i*)raw);
            return *this;
        }
        // MGATHERV
        UME_FORCE_INLINE SIMDVec_i & gather(SIMDVecMask<4> const & mask, int32_t const * baseAddr, SIMDVec_i const & indices) {
            alignas(16) int32_t rawInd[4];
            alignas(16) int32_t raw[4];

            _mm_store_si128((__m128i*) rawInd, indices.mVec);
            for (int i = 0; i < 4; i++) { raw[i] = baseAddr[rawInd[i]]; }
            __m128i t0 = _mm_load_si128((__m128i*)&raw[0]);
            mVec = _mm_blendv_epi8(mVec, t0, mask.mMask);
            return *this;
        }
        // SCATTERS
        UME_FORCE_INLINE int32_t* scatter(int32_t* baseAddr, uint32_t* indices) {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i*) raw, mVec);
            for (int i = 0; i < 4; i++) { baseAddr[indices[i]] = raw[i]; };
            return baseAddr;
        }
        // MSCATTERS
        UME_FORCE_INLINE int32_t* scatter(SIMDVecMask<4> const & mask, int32_t* baseAddr, uint32_t* indices) {
            alignas(16) int32_t raw[4];
            alignas(16) uint32_t rawMask[4];
            _mm_store_si128((__m128i*) raw, mVec);
            _mm_store_si128((__m128i*) rawMask, mask.mMask);
            for (int i = 0; i < 4; i++) {
                if (rawMask[i] == SIMDVecMask<4>::TRUE_VAL()) baseAddr[indices[i]] = raw[i]; 
            };
            return baseAddr;
        }
        // SCATTERV
        UME_FORCE_INLINE int32_t* scatter(int32_t* baseAddr, SIMDVec_i const & indices) {
            alignas(16) int32_t raw[4];
            alignas(16) int32_t rawIndices[4];
            _mm_store_si128((__m128i*) raw, mVec);
            _mm_store_si128((__m128i*) rawIndices, indices.mVec);
            for (int i = 0; i < 4; i++) { baseAddr[rawIndices[i]] = raw[i]; };
            return baseAddr;
        }
        // MSCATTERV
        UME_FORCE_INLINE int32_t* scatter(SIMDVecMask<4> const & mask, int32_t* baseAddr, SIMDVec_i const & indices) {
            alignas(16) int32_t raw[4];
            alignas(16) int32_t rawIndices[4];
            alignas(16) uint32_t rawMask[4];
            _mm_store_si128((__m128i*) raw, mVec);
            _mm_store_si128((__m128i*) rawIndices, indices.mVec);
            _mm_store_si128((__m128i*) rawMask, mask.mMask);
            for (int i = 0; i < 4; i++) {
                if (rawMask[i] == SIMDVecMask<4>::TRUE_VAL())
                    baseAddr[rawIndices[i]] = raw[i];
            };
            return baseAddr;
        }

        // LSHV
        // MLSHV
        // LSHS
        // MLSHS
        // LSHVA
        // MLSHVA
        // LSHSA
        // MLSHSA
        // RSHV
        // MRSHV
        // RSHS
        // MRSHS
        // RSHVA
        // MRSHVA
        // RSHSA
        // MRSHSA
        // ROLV
        // MROLV
        // ROLS
        // MROLS
        // ROLVA
        // MROLVA
        // ROLSA
        // MROLSA
        // RORV
        // MRORV
        // RORS
        // MRORS
        // RORVA
        // MRORVA
        // RORSA
        // MRORSA

        // NEG
        UME_FORCE_INLINE SIMDVec_i operator- () const {
            return neg();
        }
        // MNEG
        // NEGA
        // MNEGA
        // ABS
        // MABS
        // ABSA
        // MABSA

        // PACK
        UME_FORCE_INLINE SIMDVec_i & pack(SIMDVec_i<int32_t, 2> const & a, SIMDVec_i<int32_t, 2> const & b) {
            alignas(16) int32_t raw[4] = { a.mVec[0], a.mVec[1], b.mVec[0], b.mVec[1] };
            mVec = _mm_load_si128((__m128i*)raw);
            return *this;
        }
        // PACKLO
        UME_FORCE_INLINE SIMDVec_i & packlo(SIMDVec_i<int32_t, 2> const & a) {
            alignas(16) int32_t raw[4] = { a.mVec[0], a.mVec[1], 0, 0};
            alignas(16) uint32_t mask[4] = { 0xFFFFFFFF, 0xFFFFFFFF, 0, 0 };
            __m128i t0 = _mm_load_si128((__m128i*)raw);
            __m128i m0 = _mm_load_si128((__m128i*)mask);
            mVec = _mm_blendv_epi8(mVec, t0, m0);
            return *this;
        }
        // PACKHI
        UME_FORCE_INLINE SIMDVec_i & packhi(SIMDVec_i<int32_t, 2> const & b) {
            alignas(16) int32_t raw[4] = { 0, 0, b.mVec[0], b.mVec[1] };
            alignas(16) uint32_t mask[4] = { 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
            __m128i t0 = _mm_load_si128((__m128i*)raw);
            __m128i m0 = _mm_load_si128((__m128i*)mask);
            mVec = _mm_blendv_epi8(mVec, t0, m0);
            return *this;
        }
        // UNPACK
        UME_FORCE_INLINE void unpack(SIMDVec_i<int32_t, 2> & a, SIMDVec_i<int32_t, 2> & b) const {
            UME_PERFORMANCE_UNOPTIMAL_WARNING(); // This routine can be optimized
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i *)raw, mVec);
            a.mVec[0] = raw[0];
            a.mVec[1] = raw[1];
            b.mVec[0] = raw[2];
            b.mVec[1] = raw[3];
        }
        // UNPACKLO
        UME_FORCE_INLINE SIMDVec_i<int32_t, 2> unpacklo() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i *)raw, mVec);
            return SIMDVec_i<int32_t, 2>(raw[0], raw[1]);
        }
        // UNPACKHI
        UME_FORCE_INLINE SIMDVec_i<int32_t, 2> unpackhi() const {
            alignas(16) int32_t raw[4];
            _mm_store_si128((__m128i *)raw, mVec);
            return SIMDVec_i<int32_t, 2>(raw[2], raw[3]);
        }

        // PROMOTE
        UME_FORCE_INLINE operator SIMDVec_i<int64_t, 4>() const;
        // DEGRADE
        UME_FORCE_INLINE operator SIMDVec_i<int16_t, 4>() const;

        // ITOU
        UME_FORCE_INLINE operator SIMDVec_u<uint32_t, 4>() const;
        // ITOF
        UME_FORCE_INLINE operator SIMDVec_f<float, 4>() const;
    };

}
}

#endif
