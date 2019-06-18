/*
AUTHOR: G.A. Papoian, Date: Nov 22, 2018

Main code for both AVX and AVX2 calculations where the elementary type is a float.
*/

#ifndef DIST_AVX
#define DIST_AVX

#ifdef __AVX__

#include <iostream>
#include <random>
#include <array>
#include <cmath>
#include <bitset>

#include "dist_moduleV2/dist_simd.h"
#include "dist_moduleV2/dist_avx_aux.h"


namespace dist {

	extern std::array<uint, 256*8> _lut_avx2; 
 
	// Load 8 elements of i and j vectors, and operate on them vectorially. 
	template <uint D, bool SELF>
	inline void dist_simd_ij(dOut<D,SELF> &out, std::vector<int> &c1_indices, uvec8_f &c1_vxi, uvec8_f &c1_vyi, uvec8_f &c1_vzi, Coords &c2, uint i, uint j, tag_simd<simd_avx,float> tag){
		
		uvec8_f c2_vxj(&c2.x[j]);
		uvec8_f c2_vyj(&c2.y[j]);
		uvec8_f c2_vzj(&c2.z[j]);

		uvec8_f vdx = c2_vxj-c1_vxi;
		uvec8_f vdy = c2_vyj-c1_vyi;
		uvec8_f vdz = c2_vzj-c1_vzi;

		uvec8_f vsum = vdx*vdx + vdy*vdy + vdz*vdz;
	
	
		for(uint d=0; d<D; ++d){
			uint &counter(out.counter[d]);
			uvec8_f &vdl = out.v_dt[2*d];
			uvec8_f &vdh = out.v_dt[2*d+1];
		
			// The commented line below is preferable, but DTII gcc 6.1 and UME::SIMD interact badly, slowing down
			// the program by 4 fold (because UMD::SIMD starts using scalar emulation, for some reason).
			// On my Macbook Pro it is fine, even with gcc (8.x)
			
			// auto vcond = (vsum > vdl) && (vsum < vdh);

			__m256i vcond = _mm256_castps_si256(_mm256_and_ps(_mm256_cmp_ps(vsum.mVec,vdl.mVec,_CMP_GT_OQ),
			                                _mm256_cmp_ps(vdh.mVec,vsum.mVec,_CMP_GT_OQ)));
											
	
			int icond = _mm256_movemask_ps(_mm256_castsi256_ps(vcond));

			uvec8_i i_ind(&c1_indices[i]);
			uvec8_i j_ind(&c2.indices[j]);
			
#ifdef __AVX2__
			UME::SIMD::SIMDSwizzle<8> vmask(&_lut_avx2[8*icond]);
			i_ind.swizzlea(vmask);
			j_ind.swizzlea(vmask);
#else
			swizzlea_8x32_avx_impl(i_ind, icond);
			swizzlea_8x32_avx_impl(j_ind, icond);
#endif
			
			i_ind.store(&(out.dout[2*d])[counter]);
			j_ind.store(&(out.dout[2*d+1])[counter]);
			
			counter+= _mm_popcnt_u32(icond);
		}
	}
	
} // end-of-namespace dist

#endif // __AVX2__

#endif // DIST_AVX
