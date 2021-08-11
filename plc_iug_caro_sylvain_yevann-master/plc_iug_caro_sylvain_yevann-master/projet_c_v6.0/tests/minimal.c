#include <stdlib.h>

#include "ei_types.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "hw_interface.h"
#include "ei_draw.h"

int main(int argc, char* argv[]) {
        ei_surface_t main_window = NULL;
        ei_size_t main_window_size = ei_size(640, 480);
        ei_event_t event;
        uint32_t white = 0xffffffff;
        ei_color_t blue;
        blue.blue = 0xff;
        blue.alpha = 0xff;
        blue.red = 0x00;
        blue.green = 0x00;
        uint32_t *pixel_ptr;
        int i;

// Init acces to hardware.
        hw_init();

// Create the main window.
        main_window = hw_create_window(main_window_size, EI_FALSE);

// Lock the surface for drawing, fill in white, unlock, update screen.
        hw_surface_lock(main_window);

        pixel_ptr = (uint32_t *) hw_surface_get_buffer(main_window);
        for (i = 0; i < (main_window_size.width * main_window_size.height); i++)
                *pixel_ptr++ = white;

        ei_rect_t clipper;
        clipper.top_left.x = 100;
        clipper.top_left.y = 100;
        clipper.size.height = 100;
        clipper.size.width = 100;
        ei_fill(main_window, &blue, NULL);
//        ei_fill(main_window, &blue, &clipper);
	
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
