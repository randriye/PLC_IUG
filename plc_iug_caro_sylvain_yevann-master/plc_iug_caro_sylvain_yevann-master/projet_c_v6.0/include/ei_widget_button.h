//
// Created by sylvain on 20/05/2021.
//

#ifndef PROJETC_IG_EI_WIDGET_BUTTON_H
#define PROJETC_IG_EI_WIDGET_BUTTON_H


#include "ei_widget.h"
#include "ei_types.h"

typedef struct ei_button_t {
        ei_widget_t widget;
        ei_color_t	color;
        int border_with;
        int corner_radius;
        ei_relief_t relief;
        char *text;
        ei_font_t text_font;
        ei_color_t text_color;
        ei_anchor_t text_anchor;
        ei_surface_t img;
        ei_rect_t* img_rect;
        ei_anchor_t img_anchor;
        ei_callback_t callback;
        void**user_param;
        ei_bool_t already_configured;
} ei_button_t;

ei_widget_t *ei_button_allocfunc(void);

void ei_button_releasefunc (struct ei_widget_t* widget);

void ei_button_drawfunc (struct ei_widget_t*	widget,
                        ei_surface_t		surface,
                        ei_surface_t		pick_surface,
                        ei_rect_t*		clipper);

void ei_button_setdefaultsfunc (struct ei_widget_t* widget);

void ei_button_geomnotifyfunc (struct ei_widget_t*	widget,
                              ei_rect_t		rect);

ei_bool_t ei_button_handlefunc (struct ei_widget_t*	widget,
                               struct ei_event_t*	event);


#endif //PROJETC_IG_EI_WIDGET_BUTTON_H
