//
// Created by sylvain on 18/05/2021.
//

#ifndef PROJETC_IG_PIXEL_H
#define PROJETC_IG_PIXEL_H

#include "stdint.h"

#if defined(_MSC_VER)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
/* GCC-compatible compiler, targeting x86/x86-64 */
#include <x86intrin.h>
#endif

#if (defined(__x86_64__) || defined(__i386__) || defined(_MSC_VER))

/**
 * \brief compute a new pixel value using x86 SSE extension
 * @param old_pixel     value of the old pixel
 * @param new_color     new color to apply, mapped of the required structure
 * @param alpha         transparency
 * @return              new value to store at the pixel using SSE x86 extension
 */
static inline uint32_t compute_pixel(uint32_t old_pixel, uint32_t new_color, uint8_t alpha)
{
        uint64_t new_pixel;
        uint64_t e_old_pixel = (uint64_t) old_pixel;
        uint64_t e_new_color = (uint64_t) new_color;
        uint16_t e_alpha = (uint16_t) alpha + 1;
        uint16_t e_alpha_comp = (uint16_t) 255 - alpha;

        // load data int vectors
        __m128i v_old_pixel = _mm_loadu_si64(&e_old_pixel);
        __m128i v_new_color = _mm_loadu_si64(&e_new_color);
        __m128i v_alpha = _mm_set1_epi16((short) e_alpha);
        __m128i v_alpha_comp = _mm_set1_epi16((short) e_alpha_comp);

        // extend uint8_t into uint16_t
        v_old_pixel = _mm_cvtepu8_epi16(v_old_pixel);
        v_new_color = _mm_cvtepu8_epi16(v_new_color);

        // compute
        v_old_pixel = _mm_mullo_epi16(v_old_pixel, v_alpha_comp);
        v_new_color = _mm_mullo_epi16(v_new_color, v_alpha);
        __m128i v_new_pixel = _mm_add_epi16(v_old_pixel, v_new_color);

        // keep only MSB uint8_t of result
        v_new_pixel = _mm_srli_epi16 (v_new_pixel, 8);
        v_new_pixel = _mm_packus_epi16 (v_new_pixel, v_new_pixel);

        // store result
        _mm_storeu_si64(&new_pixel, v_new_pixel);

        return (uint32_t) new_pixel;
}

#else

/**
 * \brief compute a new pixel in a scalar way
 * @param old_pixel     value of the old pixel
 * @param new_color     new color to apply, mapped of the required structure
 * @param alpha         transparency
 * @return              new value to store at the pixel
 */
static inline uint32_t compute_pixel(uint32_t old_pixel, uint32_t new_color, uint8_t alpha)
{
        uint32_t new_pixel = 0;
        for (uint8_t i = 0; i < 4; i++) {
                // select a byte of consumed variables
                uint8_t old_sub_pixel = (uint8_t) old_pixel;
                uint8_t new_sub_color = (uint8_t) new_color;

                // compute new sub pixel
                uint8_t new_sub_pixel = ((uint32_t) alpha*new_sub_color + (255 - alpha)*old_sub_pixel) / 255;
                // uint8_t new_sub_pixel = ((uint32_t) alpha*new_sub_color + (255 - alpha)*old_sub_pixel) >> 8;

                // aggregate result
                new_pixel = new_pixel | (new_sub_pixel << 8*i);

                // update consumed variables
                old_pixel >>= 8;
                new_color >>= 8;
        }
        return new_pixel;
}

#endif

/**
 * \brief   Draw a point, with management of alpha
 * @param   ptr         Pointer to the pixel which must be written
 * @param   raw_color   value corresponding to the color of the pixel without before transparency managing
 * @param   alpha       alpha value
 */
static inline void draw_point(uint32_t *ptr, uint32_t raw_color, uint8_t alpha)
{
        if (alpha != 255) {
                raw_color = compute_pixel(*ptr, raw_color, alpha);
        }
        *ptr = raw_color;
}

#endif //PROJETC_IG_PIXEL_H
