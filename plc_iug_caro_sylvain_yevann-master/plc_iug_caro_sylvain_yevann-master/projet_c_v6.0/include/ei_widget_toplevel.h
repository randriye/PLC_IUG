//
// Created by sylvain on 20/05/2021.
//

#ifndef PROJETC_IG_EI_WIDGET_TOPLEVEL_H
#define PROJETC_IG_EI_WIDGET_TOPLEVEL_H

#include "ei_widget.h"
#include "ei_types.h"

typedef enum {
        toplevel_no_action = 0,
        toplevel_resizing,
        toplevel_closing,
        toplevel_moving
} toplevel_action_t;

typedef struct ei_toplevel_t {
        ei_widget_t widget;
        ei_color_t	color;
        int border_width;
        char *title;
        ei_bool_t closable;
        ei_axis_set_t resizable;
        ei_size_t *min_size;
        ei_bool_t already_configured;
        toplevel_action_t active_action;
        ei_rect_t drawn_location;
} ei_toplevel_t;

ei_widget_t *ei_toplevel_allocfunc(void);

void ei_toplevel_releasefunc (struct ei_widget_t* widget);

void ei_toplevel_drawfunc (struct ei_widget_t*	widget,
                        ei_surface_t		surface,
                        ei_surface_t		pick_surface,
                        ei_rect_t*		clipper);

void ei_toplevel_setdefaultsfunc (struct ei_widget_t* widget);

void ei_toplevel_geomnotifyfunc (struct ei_widget_t*	widget,
                              ei_rect_t		rect);

ei_bool_t ei_toplevel_handlefunc (struct ei_widget_t*	widget,
                               struct ei_event_t*	event);

#endif //PROJETC_IG_EI_WIDGET_TOPLEVEL_H
