//
// Created by randriamora on 19/05/2021.
//

#ifndef PROJETC_IG_WIDGET_TEXT_H
#define PROJETC_IG_WIDGET_TEXT_H

#include "hw_interface.h"
#include "ei_draw.h"

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
                                                             const ei_rect_t*	clipper);

#endif //PROJETC_IG_WIDGET_TEXT_H
