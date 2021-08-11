//
// Created by randriamora on 19/05/2021.
//
#include <stdlib.h>
//#include <stdint.h>

#include "widget_text.h"
#include "hw_interface.h"
#include "ei_draw.h"
#include "ei_types.h"
#include "string.h"
//#include "clipper.h"
//#include "ei_draw_polygon.h"




/**
 * \brief	Draws text by calling \ref ei_draw_text.
 *
 * @param	surface 	Where to draw the text. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	screen_location		ei_rect_t, the screen_location of the widget with the text.
 * @param	text		The string of the text. Can't be NULL.
 * @param	font		The font used to render the text. If NULL, the \ref ei_default_font
 *				is used.
 * @param	color		The text color. Can't be NULL. The alpha parameter is not used.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void			widget_draw_text		(ei_surface_t		surface,
                                                         ei_rect_t screen_location,
                                                         ei_anchor_t  text_anchor,
                                                         const char*		text,
                                                         ei_font_t		font,
                                                         ei_color_t		color,
                                                         const ei_rect_t*	clipper)
{
        // text_anchor : the anchor relative to the screen_location (aka the widget rect)
        // initializing the anchor
        if (font == NULL) {
                font = ei_default_font;
        }

        // computing the real text surface without taking into account the clipper
        // 1 - creating text rectangle without considering the clipper
        int *wwc = malloc(sizeof(int)); // width without clipping
        int *hwc = malloc(sizeof(int)); // height without clipping
        hw_text_compute_size(text, font, wwc, hwc);
        // 1.1 - computing the anchor
        ei_point_t anchor;
        anchor.x = screen_location.top_left.x;
        anchor.y = screen_location.top_left.y;
        int w = screen_location.size.width;
        int h = screen_location.size.height;

        int wt = *wwc;
        int ht = *hwc;

        switch (text_anchor){
                case ei_anc_none:
                        anchor.x += (w - wt) / 2;
                        anchor.y += (h - ht) / 2;
                        break;



                case ei_anc_north:
                        anchor.x += (w - wt) / 2;
                        anchor.y += 0;
                        break;

                case ei_anc_northeast:
                        anchor.x += w - wt;
                        anchor.y += 0;
                        break;

                case ei_anc_east:
                        anchor.x += w - wt;
                        anchor.y += (h - ht) / 2;
                        break;

                case ei_anc_southeast:
                        anchor.x += w - wt;
                        anchor.y += h - ht;
                        break;

                case ei_anc_south:
                        anchor.x += (w - wt) / 2;
                        anchor.y += (h - ht);
                        break;

                case ei_anc_southwest:
                        anchor.x += 0;
                        anchor.y += h - ht;
                        break;

                case ei_anc_west:
                        anchor.x += 0;
                        anchor.y += (h - ht) / 2;
                        break;

                case ei_anc_center:
                        anchor.x += (w - wt) / 2;
                        anchor.y += (h - ht) / 2;
                        break;
                case ei_anc_northwest:
                        anchor.x += 0;
                        anchor.y += 0;
                        break;
                default:
                        anchor.x += (w - wt) / 2;
                        anchor.y += (h - ht) / 2;
                        break;

        }
        // call to drawtext - considering the clipper
        ei_draw_text(surface, &anchor, text, font, color, clipper);
        free(wwc);
        free(hwc);
}