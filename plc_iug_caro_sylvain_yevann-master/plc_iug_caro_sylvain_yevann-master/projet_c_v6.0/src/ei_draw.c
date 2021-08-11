#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "ei_types.h"
#include "ei_draw.h"
#include "hw_interface.h"
#include "string.h"
#include "pixel.h"
#include "ei_draw_polygon.h"
#include "relief.h"
#include "clipper.h"
#include "ei_application.h"

#include "stdio.h"

/**
 * @brief Delete the head of a list of clipped linked points
 * @param cpl
 */
static void del_head_cpt(ei_linked_clipped_point_t **cpl){
        ei_linked_clipped_point_t *suiv = (*cpl)->next;
        free(*cpl);
        *cpl = suiv;
}

/**
 * @brief free entirely a list of linked clipped points
 * @param to_free
 */
static void free_clipped_point_list(ei_linked_clipped_point_t **to_free){
        while(*to_free != NULL){
                del_head_cpt(to_free);
        }
        free(*to_free);
}


/**
 * \brief   Application of Bresenham for the case |delta_x|>|delta_y| to change the value of E
 *          and y
 * @param   delta_x        the delta x of the algorithm of Bresenham, an integer
 * @param   delta_y        the delta x of the algorithm of Bresenham, an integer
 * @param   *y             a pointer of an int, a coordinate of the structure ei_point_t
 * @param   move_y         an integer in {-1,1} that tells how do we move the y
 * @param   *E             a pointer of E an int
 */
static inline void			Bresenham		(int delta_x,
                                                                 int delta_y,
                                                                 int *y,
                                                                 int move_y,
                                                                 int *E){
        *E += abs(delta_y);
        if (abs(2* (*E)) > abs(delta_x)){
                *y += move_y;
                *E -= abs(delta_x);
        }
}



/**
 * \brief   Draw a line segments using 2 endpoints
 *
 * @param   buffer          Pointer to the first pixel of the screen
 * @param   surface_size    couple of integers that indicates the width and the length of the
 *                          screen
 * @param   p1              a couple of int representing the first point, using the structure
 *                          of ei_point_t
 * @param   p2              a couple of int representing the second point, using the structure
 *                          of ei_point_t
 * @param   raw_color       value corresponding to the color of the pixel without before
 *                          transparency managing
 * @param   alpha_mask      mask used to quickly compute final pixel value
 */

static void			draw_line              (uint32_t*               buffer,
                                                        ei_size_t               surface_size,
                                                        ei_point_t              p1,
                                                        ei_point_t              p2,
                                                        uint32_t                raw_color,
                                                        uint8_t                 alpha)
{
        // The case where p1 is next to p2
        if (abs(p1.x-p2.x)<2 && abs(p1.y-p2.y)<2) {
                draw_point(buffer + p1.y * surface_size.width + p1.x, raw_color, alpha);
                draw_point(buffer + p2.y * surface_size.width + p2.x, raw_color, alpha);
                return;
        }

        // find the point with the smallest address and put in p1
        ei_point_t p_prime ;
        if (p1.x + p1.y*surface_size.width >  p2.x + p2.y*surface_size.width){
                p_prime = p2;
                p2 = p1;
                p1 = p_prime;
        }

        // find delta_x and delta_y of the algorithm of Bresenham
        int delta_x = p2.x - p1.x;
        int delta_y = p2.y - p1.y;
        int E = 0;

        // after putting p1 and p2 in the right position, we can only browse the y-coordinate in the ascending order
        int8_t move_y = 1;

        // the case |delta_x| < |delta_y|: the line is x-directed
        if  (abs(delta_x) > abs(delta_y)) {
                // the sign of delta_x gives the sign of our x-step
                int8_t move_x = delta_x < 0 ? -1 : 1;

                // browse the x-coordinate until p1.x equals p2.x
                for (++p1.x  ;          p1.x - p2.x != 0;            p1.x += move_x){
                        Bresenham(delta_x, delta_y, &(p1.y), move_y, &E);
                        draw_point(buffer + p1.y * surface_size.width + p1.x,
                                   raw_color,
                                   alpha);
                }
        } else {
        // the case |delta_x| > |delta_y| : the line is y-directed (we do the same thing by exchanging all the "x" and "y")
                // the sign of delta_x gives the sign of our x-step
                int8_t move_x = delta_x<0 ? -1 : 1;

                // browse the y-coordinate until p1.y equals p2.y
                for (p1.y;  p1.y - p2.y != 0; p1.y += move_y) {
                        Bresenham(delta_y, delta_x, &(p1.x), move_x, &E);
                        draw_point(
                                buffer + p1.y * surface_size.width + p1.x,
                                raw_color,
                                alpha);
                }
        }
}



/**
 * \brief	Converts the red, green, blue and alpha components of a color into a 32 bits integer
 * 		than can be written directly in the memory returned by \ref hw_surface_get_buffer.
 * 		The surface parameter provides the channel order.
 *
 * @param	surface		The surface where to store this pixel, provides the channels order.
 * @param	color		The color to convert.
 *
 * @return 			The 32 bit integer corresponding to the color. The alpha component
 *				of the color is ignored in the case of surfaces that don't have an
 *				alpha channel.
 */
uint32_t		ei_map_rgba		(ei_surface_t surface, ei_color_t color)
{
        // index table of color channels
        int rgba[4];
        hw_surface_get_channel_indices(surface, &rgba[0], &rgba[1], &rgba[2], &rgba[3]);

        // build of rgb
        uint32_t color_value = (color.red << 8*(rgba[0])) | (color.green << 8*(rgba[1])) | (color.blue << 8*(rgba[2]));

        // add alpha if needed
        if (rgba[3] >= 0) {
                color_value = color_value | ((uint32_t) color.alpha << 8*(rgba[3]));
        }

        return color_value;
}



/**
 * \brief	Draws a line that can be made of many line segments.
 *
 * @param	surface 	Where to draw the line. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	first_point 	The head of a linked list of the points of the polyline. It can be NULL
 *				(i.e. draws nothing), can have a single point, or more.
 *				If the last point is the same as the first point, then this pixel is
 *				drawn only once.
 * @param	color		The color used to draw the line. The alpha channel is managed.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void			ei_draw_polyline	(ei_surface_t			surface,
                                     const ei_linked_point_t*	first_point,
                                     ei_color_t			color,
                                     const ei_rect_t*		clipper)
{
        if (first_point != NULL) {
                ei_rect_t super_clipper = hw_surface_get_rect(surface);
                if (clipper == NULL){
                        super_clipper = hw_surface_get_rect(surface);
//                fill_polygon(surface, first_point, color);
                }
                else {
                        super_clipper = *clipper;
                }
                ei_linked_point_t *clipped_first_point = (ei_linked_point_t *) ei_polygone_clipper_SH(first_point, &super_clipper);

                // get color to write in the memory
                uint32_t raw_color = ei_map_rgba(surface, color);

                // create alpha mask if needed
                uint8_t alpha = color.alpha;
                // do not use alpha if the surface doesn't support it and it isn't the root surface
                if ((hw_surface_has_alpha(surface) == EI_FALSE) && (surface != ei_app_root_surface())) {
                        // draw_point will no try to compute new pixel
                        alpha = 255;
                }

                // create often used variables
                ei_linked_point_t *next_point = clipped_first_point->next;
                uint8_t *buffer = hw_surface_get_buffer(surface);
                ei_size_t surface_size = hw_surface_get_size(surface);

                // draw first point
                uint8_t first_point_offset = (clipped_first_point->point.y*surface_size.height + clipped_first_point->point.x) << 2;
                draw_point((uint32_t *) buffer + first_point_offset, raw_color, alpha);

                // draw the other stuff
                ei_point_t p1 = clipped_first_point->point;
                while ((next_point != NULL) && (next_point != clipped_first_point)) {
                        ei_point_t p2 = next_point->point;
                        draw_line((uint32_t *) buffer, surface_size, p1, p2, raw_color, alpha);
                        p1 = p2;
                        next_point = next_point->next;
                }

                // free
                free_clipped_point_list((ei_linked_clipped_point_t **) &clipped_first_point);
        }
}

/**
 * \brief	Draws a filled polygon.
 *
 * @param	surface 	Where to draw the polygon. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	first_point 	The head of a linked list of the points of the line. It is either
 *				NULL (i.e. draws nothing), or has more than 2 points. The last point
 *				is implicitly connected to the first point, i.e. polygons are
 *				closed, it is not necessary to repeat the first point.
 * @param	color		The color used to draw the polygon. The alpha channel is managed.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */




void			ei_draw_polygon		(ei_surface_t			surface,
                                        const ei_linked_point_t*	first_point,
                                        ei_color_t			color,
                                        const ei_rect_t*		clipper)
{
        ei_rect_t super_clipper;
        if (clipper == NULL){
                super_clipper = hw_surface_get_rect(surface);
        }
        else
        {
                super_clipper = *clipper;
        }
        ei_linked_clipped_point_t *clipped_first_point = ei_polygone_clipper_SH(first_point, &super_clipper);
        if (clipped_first_point->next != NULL) {
                fill_polygon(surface, (ei_linked_point_t *) clipped_first_point, color);
        }
        free_clipped_point_list(&clipped_first_point);
        //fill_polygon(surface, first_point, color, clipper);
}







/**
 * \brief	Draws text by calling \ref hw_text_create_surface.
 *
 * @param	surface 	Where to draw the text. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	where		Coordinates, in the surface, where to anchor the top-left corner of
 *				the rendered text.
 * @param	text		The string of the text. Can't be NULL.
 * @param	font		The font used to render the text. If NULL, the \ref ei_default_font
 *				is used.
 * @param	color		The text color. Can't be NULL. The alpha parameter is not used.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void			ei_draw_text		(ei_surface_t		surface,
                                     const ei_point_t*	where,
                                     const char*		text,
                                     ei_font_t		font,
                                     ei_color_t		color,
                                     const ei_rect_t*	clipper)
{
        if (text[0] != '\0') {
                if (font == NULL) {
                        font = ei_default_font;
                }
                // create new text surface
                ei_surface_t text_surface = hw_text_create_surface(text, font, color);

                // copy text surface to main surface
                ei_size_t src_size = hw_surface_get_size(text_surface);

                ei_rect_t dest_rect;
                dest_rect.top_left.x = where->x;
                dest_rect.top_left.y = where->y;

                dest_rect.size.width = src_size.width;
                dest_rect.size.height = src_size.height;


                if (clipper != NULL) {
                        if (clipper->top_left.x > dest_rect.top_left.x) {
                                dest_rect.top_left.x = clipper->top_left.x;
                        }
                        if (clipper->top_left.y > dest_rect.top_left.y) {
                                dest_rect.top_left.y = clipper->top_left.y;
                        }
                        int offset_x = dest_rect.top_left.x + dest_rect.size.width - clipper->top_left.x - clipper->size.width;
                        if (offset_x > 0) {
                                dest_rect.size.width-= offset_x;
                                if (dest_rect.size.width < 0) {
                                        return;
                                }
                        }
                        int offset_y = dest_rect.top_left.y + dest_rect.size.height - clipper->top_left.y - clipper->size.height;
                        if (offset_y > 0) {
                                dest_rect.size.height-= dest_rect.size.height;
                                if (dest_rect.size.height < 0) {
                                        return;
                                }
                        }
                        src_size.width = dest_rect.size.width;
                        src_size.height = dest_rect.size.height;
                }

                ei_rect_t src_rect;
                src_rect.top_left.x = 0;
                src_rect.top_left.y = 0;
                src_rect.size = src_size;

                ei_copy_surface(surface, &dest_rect, (ei_surface_t) text_surface, &src_rect, EI_TRUE);

                hw_surface_free(text_surface);
        }
}

/**
 * \brief	Fills the surface with the specified color.
 *
 * @param	surface		The surface to be filled. The surface must be *locked* by
 *				\ref hw_surface_lock.
 * @param	color		The color used to fill the surface. If NULL, it means that the
 *				caller want it painted black (opaque).
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle.
 */
void			ei_fill			(ei_surface_t		surface,
                                    const ei_color_t*	color,
                                    const ei_rect_t*	clipper)
{
        uint32_t *buffer = (uint32_t *) hw_surface_get_buffer(surface);
        ei_size_t buffer_size = hw_surface_get_size(surface);

        // compute color value
        uint32_t  new_color = 0;
        if (color != NULL) {
                new_color = ei_map_rgba(surface, *color);
        }

        // do not use alpha if the surface doesn't support it and it isn't the root surface
        uint8_t alpha;
        if ((hw_surface_has_alpha(surface) == EI_FALSE) && (surface != ei_app_root_surface())) {
                // draw_point will no try to compute new pixel
                alpha = 255;
        }
        else if (color != NULL) {
                alpha = color->alpha;
        }
        else {
                alpha = 255;
        }

        // if alpha management
        if ((alpha != 255) && new_color) {
                if (clipper == NULL) {
                        for (uint32_t i = 0; i < (uint32_t) (buffer_size.width*buffer_size.height); i++) {
                                draw_point(buffer, new_color, alpha);
                                buffer++;
                        }
                }
                else {
                        buffer = buffer + buffer_size.width*clipper->top_left.y + clipper->top_left.x;
                        uint32_t *saved_buffer = buffer;
                        for (uint32_t i = clipper->top_left.y; i < (uint32_t) clipper->top_left.y + clipper->size.height; i++) {
                                for (uint32_t j = clipper->top_left.x; j < (uint32_t) clipper->top_left.x + clipper->size.width; j++) {
                                        draw_point(buffer, new_color, alpha);
                                        buffer++;
                                }
                                saved_buffer = saved_buffer + buffer_size.width;
                                buffer = saved_buffer;
                        }
                }
        }
        // if no alpha management
        else {
                if (clipper == NULL) {
                        for (uint32_t i = 0; i < (uint32_t) (buffer_size.width*buffer_size.height); i++) {
                                *buffer++ = new_color;
                        }
                }
                else {
                        buffer = buffer + buffer_size.width*clipper->top_left.y + clipper->top_left.x;
                        uint32_t *saved_buffer = buffer;
                        for (uint32_t i = clipper->top_left.y; i < (uint32_t) clipper->top_left.y + clipper->size.height; i++) {
                                for (uint32_t j = clipper->top_left.x; j < (uint32_t) clipper->top_left.x + clipper->size.width; j++) {
                                        *buffer++ = new_color;
                                }
                                saved_buffer = saved_buffer + buffer_size.width;
                                buffer = saved_buffer;
                        }
                }

        }

}


/**
 * \brief	Copies pixels from a source surface to a destination surface.
 *		The source and destination areas of the copy (either the entire surfaces, or
 *		subparts) must have the same size before considering clipping.
 *		Both surfaces must be *locked* by \ref hw_surface_lock.
 *
 * @param	destination	The surface on which to copy pixels.
 * @param	dst_rect	If NULL, the entire destination surface is used. If not NULL,
 *				defines the rectangle on the destination surface where to copy
 *				the pixels.
 * @param	source		The surface from which to copy pixels.
 * @param	src_rect	If NULL, the entire source surface is used. If not NULL, defines the
 *				rectangle on the source surface from which to copy the pixels.
 * @param	alpha		If true, the final pixels are a combination of source and
 *				destination pixels weighted by the source alpha channel and
 *				the transparency of the final pixels is set to opaque.
 *				If false, the final pixels are an exact copy of the source pixels,
 				including the alpha channel.
 *
 * @return			Returns 0 on success, 1 on failure (different sizes between source and destination).
 */
int			ei_copy_surface		(ei_surface_t		destination,
                                       const ei_rect_t*	dst_rect,
                                       ei_surface_t		source,
                                       const ei_rect_t*	src_rect,
                                       ei_bool_t		alpha)
{
        uint8_t *buffer_dst = hw_surface_get_buffer(destination);
        ei_size_t dst_surface_size = hw_surface_get_size(destination);
        ei_size_t src_surface_size = hw_surface_get_size(source);
        uint8_t *buffer_src = hw_surface_get_buffer(source);
        ei_rect_t final_dest_rect;
        ei_rect_t final_source_rect;

        if (dst_rect) {
                final_dest_rect.size = dst_rect->size;
                final_dest_rect.top_left = dst_rect->top_left;
        }
        else {
                final_dest_rect.size = hw_surface_get_size(destination);
                final_dest_rect.top_left.x = 0;
                final_dest_rect.top_left.y = 0;
        }

        if (src_rect) {
                final_source_rect.size = src_rect->size;
                final_source_rect.top_left = src_rect->top_left;
        }
        else {
                final_source_rect.size = hw_surface_get_size(source);
                final_source_rect.top_left.x = 0;
                final_source_rect.top_left.y = 0;
        }

        if ((final_source_rect.size.height == final_dest_rect.size.height) &
           (final_source_rect.size.width == final_dest_rect.size.width)) {
                if (alpha == EI_FALSE) {
                        uint8_t *ptr_dest = buffer_dst;
                        uint8_t *ptr_src  = buffer_src;
                        ptr_dest += 4*dst_surface_size.width*final_dest_rect.top_left.y;
                        ptr_src  += 4*src_surface_size.width*final_source_rect.top_left.y;
                        ptr_dest += 4*final_dest_rect.top_left.x;
                        ptr_src  += 4*final_source_rect.top_left.x;
                        for (uint32_t i = 0; i < (uint32_t) final_dest_rect.size.height; i++) {
                                uint8_t *saved_ptr_src = ptr_src;
                                uint8_t *saved_ptr_dest = ptr_dest;
                                memmove(ptr_dest, ptr_src, 4*final_dest_rect.size.width);
                                ptr_dest = saved_ptr_dest + 4*dst_surface_size.width;
                                ptr_src  = saved_ptr_src  + 4*src_surface_size.width;
                        }
                }
                else {
                        uint32_t *ptr_dest = (uint32_t *) buffer_dst;
                        uint32_t *ptr_src  = (uint32_t *) buffer_src;
                        int alpha_channel;
                        int tmp;
                        hw_surface_get_channel_indices(source, &tmp, &tmp, &tmp, &alpha_channel);
                        alpha_channel = 8*alpha_channel;

                        ptr_dest += dst_surface_size.width*final_dest_rect.top_left.y;
                        ptr_src  += src_surface_size.width*final_source_rect.top_left.y;
                        ptr_dest += final_dest_rect.top_left.x;
                        ptr_src  += final_source_rect.top_left.x;
                        for (uint32_t i = 0; i < (uint32_t) final_dest_rect.size.height; i++) {
                                uint32_t *saved_ptr_src = ptr_src;
                                uint32_t *saved_ptr_dest = ptr_dest;
                                for (uint32_t j = 0; j < (uint32_t) final_dest_rect.size.width; j++) {
                                        uint32_t old_pixel = *ptr_dest;
                                        uint32_t alpha_value = (*ptr_src) >> alpha_channel;
                                        uint32_t new_pixel = compute_pixel(old_pixel, *ptr_src, (uint8_t) alpha_value);
                                        *ptr_dest++ = new_pixel;
                                        ptr_src++;
                                }
                                ptr_dest = saved_ptr_dest + dst_surface_size.width;
                                ptr_src  = saved_ptr_src  + src_surface_size.width;
                        }
                }
                return 1;
        }

        // error
        return 0;
}




