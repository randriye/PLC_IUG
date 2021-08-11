//
// Created by sylva on 23/05/2021.
//

#ifndef PROJETC_IG_RELIEF_H
#define PROJETC_IG_RELIEF_H

#include "ei_types.h"


/**
 * @brief	Identifies if the relief part is the upper or the lower one
 */
typedef enum {
        ei_relief_up		= 0,
        ei_relief_bottom,
} ei_relief_part_t;


/**
 * @brief	Identifies if the color of the relief
 */
typedef enum {
        ei_relief_light		= 0,
        ei_relief_dark,
} ei_relief_color_change_t;



/**
 * @brief	Structure created to free the list of point for the
 *              particular case of rounded frames in general.
 *
 * rounded_frame is the list of points or our rounded frame
 * free_pointer is the list of pointers of every part of the rounded_frame : a segment or an arc
 */
typedef struct {
        ei_linked_point_t**      free_pointer;
        ei_linked_point_t*      rounded_frame;
}ei_rounded_frame_free;



/**
 * \brief   Takes a color and return another colors : a lighter one
 *          or a darker one for the relief.
 *
 * @param   color                     the color taken
 * @param   color change              tells us if the wi return a
 *                                    lighter or a darker color
 */
ei_color_t              get_relief_color                (ei_color_t color,
                                                         ei_relief_color_change_t color_change);



/**
 * \brief   Takes a ei_rect_t and return the list of points of  the relief behind (border).
 *
 * @param   widget_button_size        the size of the button (rectangle)
 * @param   border_width              the width of the border
 * @param   part                      do we draw the upper part or the lower part?
 */
ei_linked_point_t       *rect_frame                     (ei_rect_t widget_size_without_relief,
                                                         int border_width,
                                                         ei_relief_part_t part);



/**
 * \brief   Draw an arc with a start and an end.
 *          The arc is returned with the smallest angle at the beginning and trigonometric
 *          direction.
 *          Warning : it is the trigonometric direction for the screen, so it is clockwise
 *          for the user.
 *
 * @param   center        the center point of the arc
 * @param   radius        the radius of the arc
 * @param   start_angle   the start angle of the arc
 * @param   end_angle     the end angle of the arc
 */
ei_linked_point_t*               draw_arc               (ei_point_t center,
                                                         uint32_t radius,
                                                         float start_angle,
                                                         float end_angle,
                                                         ei_linked_point_t** last);



/**
 * \brief   Takes a ei_rect_t and return the list of points of the relief behind (border) with
 *          rounded frame.
 *
 * @param   widget_button_size        the size of the button (rectangle)
 * @param   border_width              the width of the border
 * @param   corner_radius             the radius or the arc on the border
 * @param   part                      do we draw the upper part or the lower part?
 */
ei_rounded_frame_free           *rounded_frame          (ei_rect_t widget_size_without_border,
                                                         int border_width, int corner_radius,
                                                         ei_relief_part_t part);



/**
 * \brief   Takes a ei_rect_t and returns the rectangle with the corners rounded
 * @param   button_size_without_relief        the rectangle
 * @param   corner_radius             the radius or the arc on the border
 */
ei_rounded_frame_free       *rounded_rect               (ei_rect_t border,
                                                         int corner_radius);

#endif //PROJETC_IG_RELIEF_H
