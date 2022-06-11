#include <intrin.h>
#include <stdio.h>
#include <stdint.h>

int main()
{
    #ifdef _WIN32
    printf("%d %d\n", _tzcnt_u32(64), _tzcnt_u32(100));
    #else
    printf("%d %d\n", __builtin_ctz(64), __builtin_ctz(100));
    #endif

    __m128i a = *(__m128i*)(uint8_t[16]){1, 3, 6, 14, 20, 21, 31};
    __m128i c0_15 =  *(__m128i*)(uint8_t[]){0,1,2,3, 4,5,6,7, 8,9,10,11, 12,13,14,15};
    __m128i c16_31 = *(__m128i*)(uint8_t[]){16,17,18,19, 20,21,22,23, 24,25,26,27, 28,29,30,31};
    __m128i res = _mm_cmpestrm(a, 7, c0_15, 16, _SIDD_UBYTE_OPS | _SIDD_BIT_MASK);
    // 0b0100'0000'0100'1010
    __m128i res2 = _mm_cmpestrm(a, 7, c16_31, 16, _SIDD_UBYTE_OPS | _SIDD_BIT_MASK);
    // 0b1000'0000'0011'0000

    __m128i v = *(__m128i*)(int[4]){-7,-1,0,-1};
    __m128i v2 = _mm_set_epi32(0,0,0,0);

    res = _mm_setr_epi8(1, 3, 6, 14, 20, 21, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    printf("%x %x %x %x\n", *((uint32_t*)(&res)), *((uint32_t*)(&res)+1), *((uint32_t*)(&res)+2), *((uint32_t*)(&res)+3));

    res = _mm_cmplt_epi32(v, v2);
    int res3 = _mm_cmpestri(_mm_set1_epi16(0), 1, res, 7, _SIDD_SWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT);
    printf("%x %x %x %x => %d\n", *((uint32_t*)(&res)), *((uint32_t*)(&res)+1), *((uint32_t*)(&res)+2), *((uint32_t*)(&res)+3), res3);
    
    res = _mm_cmpgt_epi32(v, v2);
    res3 = _mm_cmpestri(_mm_set1_epi16(-1), 1, res, 7, _SIDD_SWORD_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_LEAST_SIGNIFICANT);
    printf("%x %x %x %x => %d\n", *((uint32_t*)(&res)), *((uint32_t*)(&res)+1), *((uint32_t*)(&res)+2), *((uint32_t*)(&res)+3), res3);
    
    /*printf("%c%c%c%c'%c%c%c%c'%c%c%c%c'%c%c%c%c\n",/* "%I64x %I64x %I64x %I64x\n", res2.m128i_u64[1], res2.m128i_u64[0], res.m128i_u64[1], res.m128i_u64[0]* /
        (res.m128i_u32[0] & 0b1000'0000'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0100'0000'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0010'0000'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0001'0000'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'1000'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0100'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0010'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0001'0000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'1000'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0100'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0010'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0001'0000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0000'1000) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0000'0100) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0000'0010) ? '1' : '0',
        (res.m128i_u32[0] & 0b0000'0000'0000'0001) ? '1' : '0'
    );*/
    return 0;
}