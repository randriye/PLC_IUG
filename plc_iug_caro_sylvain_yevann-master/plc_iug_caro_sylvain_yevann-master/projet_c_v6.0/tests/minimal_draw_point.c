#include <stdlib.h>

#include "ei_types.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "hw_interface.h"

#include "stdint.h"
#if defined(_MSC_VER)
/* Microsoft C/C++-compatible compiler */
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
/* GCC-compatible compiler, targeting x86/x86-64 */
#include <x86intrin.h>
#endif

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

/**
 *
 * @param old_pixel     value of the old pixel
 * @param new_color     new color to apply, mapped of the required structure
 * @param alpha         transparency
 * @return              new value to store at the pixel
 */
static inline uint32_t v_compute_pixel(uint32_t old_pixel, uint32_t new_color, uint8_t alpha)
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

int main(int argc, char* argv[])
{
	ei_surface_t			main_window		= NULL;
	ei_size_t			main_window_size	= ei_size(640, 480);
	ei_event_t			event;
	uint32_t			white			= 0xffffffff;
	uint32_t            blue            = 0x000000ff;
	uint32_t*			pixel_ptr;
	int				i;

	// Init acces to hardware.
	hw_init();

	// Create the main window.
	main_window = hw_create_window(main_window_size, EI_FALSE);

	// Lock the surface for drawing, fill in white, unlock, update screen.
	hw_surface_lock(main_window);

	pixel_ptr = (uint32_t*)hw_surface_get_buffer(main_window);
	for (i = 0; i < (main_window_size.width * main_window_size.height); i++) {
	        uint32_t old_pixel = *pixel_ptr;
	        *pixel_ptr++ = v_compute_pixel(old_pixel, blue, 0xff);
	}
//		*pixel_ptr++ = white;

	
	hw_surface_unlock(main_window);
	hw_surface_update_rects(main_window, NULL);

	// Wait for a key press.
	event.type = ei_ev_none;
	while (event.type != ei_ev_keydown)
		hw_event_wait_next(&event);

	// Free hardware resources.
	hw_quit();

	// Terminate program with no error code.
	return 0;
}
