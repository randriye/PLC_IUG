//
// Created by sylvain on 19/05/2021.
//

#ifndef PROJETC_IG_EI_WIDGET_FRAME_H
#define PROJETC_IG_EI_WIDGET_FRAME_H

#include "ei_widget.h"
#include "ei_types.h"
#include "widget_text.h"

typedef struct ei_frame_t {
        ei_widget_t widget;
        ei_color_t	color;
        int border_with;
        ei_relief_t relief;
        char *text;
        ei_font_t text_font;
        ei_color_t text_color;
        ei_anchor_t text_anchor;
        ei_surface_t img;
        ei_rect_t* img_rect;
        ei_anchor_t img_anchor;
        ei_bool_t already_configured;
} ei_frame_t;

/**
 * @brief create/allocate a pointer to a widget frame
 * @return a pointer to the frame cast in a widget pointer
 */
ei_widget_t *ei_frame_allocfunc(void);

/**
 * @brief Insert a point in a linked list of points
 * @param pl the list of points
 * @param to_insert the point to insert
 */
 void point_insert(ei_linked_point_t **pl, ei_point_t to_insert);


/**
* @brief Release the widget frame
* Free the Placer parameters
* Remove all children
* Free the widget
* @param widget widget to release
*/
void ei_frame_releasefunc (struct ei_widget_t* widget);


/**
 * \brief	A function that draws widgets of a class.
 *
 * @param	widget		A pointer to the widget instance to draw.
 * @param	surface		Where to draw the widget. The actual location of the widget in the
 *				surface is stored in its "screen_location" field.
 * @param	pick_surface	The picking offscreen.
 * @param	clipper		If not NULL, the drawing is restricted within this rectangle
 *				(expressed in the surface reference frame).
 */
void ei_frame_drawfunc (struct ei_widget_t*	widget,
                        ei_surface_t		surface,
                        ei_surface_t		pick_surface,
                        ei_rect_t*		clipper);


/**
 * @brief Put default param to a widget
 * @param widget widget to put default fields
 */
void ei_frame_setdefaultsfunc (struct ei_widget_t* widget);


/**
 * @brief Called by the placer when we modified the place of the widget
 * @param widget Widget with the screen location which has been modified
 * @param rect the new place
 */
void ei_frame_geomnotifyfunc (struct ei_widget_t*	widget,
                              ei_rect_t		rect);


/**
 * @brief return if the widget is handled or not
 * @param widget widget to check
 * @param event the event which has arrived
 * @return For a frame, always True
 */
ei_bool_t ei_frame_handlefunc (struct ei_widget_t*	widget,
                               struct ei_event_t*	event);

#endif //PROJETC_IG_EI_WIDGET_FRAME_H
