//
// Created by sylvain on 18/05/2021.
//

#include "ei_types.h"
#include "ei_placer.h"
#include "ei_widget.h"
#include "hw_interface.h"
#include "ei_application.h"
#include "ei_widgetclass.h"
#include "stdlib.h"

/**
 * \brief	Configures the geometry of a widget using the "placer" geometry manager.
 *
 *		The placer computes a widget's geometry relative to its parent *content_rect*.
 *
 * 		If the widget was already managed by the "placer", then this calls simply updates
 *		the placer parameters: arguments that are not NULL replace previous values.
 *
 * 		When the arguments are passed as NULL, the placer uses default values (detailed in
 *		the argument descriptions below). If no size is provided (either absolute or
 *		relative), then either the requested size of the widget is used if one was provided,
 *		or the default size is used.
 *
 * @param	widget		The widget to place.
 * @param	anchor		How to anchor the widget to the position defined by the placer
 *				(defaults to ei_anc_northwest).
 * @param	x		The absolute x position of the widget (defaults to 0).
 * @param	y		The absolute y position of the widget (defaults to 0).
 * @param	width		The absolute width for the widget (defaults to the requested width or
 * 				the default width of the widget if rel_width is NULL, or 0 otherwise).
 * @param	height		The absolute height for the widget (defaults to the requested height or
 *				the default height of the widget if rel_height is NULL, or 0 otherwise).
 * @param	rel_x		The relative x position of the widget: 0.0 corresponds to the left
 *				side of the master, 1.0 to the right side (defaults to 0.0).
 * @param	rel_y		The relative y position of the widget: 0.0 corresponds to the top
 *				side of the master, 1.0 to the bottom side (defaults to 0.0).
 * @param	rel_width	The relative width of the widget: 0.0 corresponds to a width of 0,
 *				1.0 to the width of the master (defaults to 0.0).
 * @param	rel_height	The relative height of the widget: 0.0 corresponds to a height of 0,
 *				1.0 to the height of the master (defaults to 0.0).
 */
void		ei_place	(struct ei_widget_t*	widget,
                         ei_anchor_t*		anchor,
                         int*			x,
                         int*			y,
                         int*			width,
                         int*			height,
                         float*			rel_x,
                         float*			rel_y,
                         float*			rel_width,
                         float*			rel_height)
{

        if (widget ->placer_params == NULL) {
                ei_placer_params_t *pp = malloc(sizeof(ei_placer_params_t));
                widget -> placer_params = pp;
        }
        // anchor
        if (anchor == NULL){
                widget -> placer_params->anchor_data = ei_anc_northwest;
                widget -> placer_params->anchor = NULL;
        }else{
                widget -> placer_params -> anchor_data = *anchor;
                widget -> placer_params->anchor = &(widget -> placer_params->anchor_data);
        }

        // x
        if (x == NULL){
                widget -> placer_params -> x_data = 0;
                widget -> placer_params -> x = NULL;
        }else{
                widget -> placer_params -> x_data = *x;
                widget -> placer_params -> x = &(widget -> placer_params -> x_data);
        }

        // y
        if (y == NULL){
                widget -> placer_params -> y_data = 0;
                widget -> placer_params -> y = NULL;
        }else{
                widget -> placer_params -> y_data = *y;
                widget -> placer_params -> y = &(widget -> placer_params -> y_data);
        }

        // w and rel_width
        if (width == NULL){
                // requested size - we have to consider the rel_width
                if (rel_width == NULL){
                        widget -> placer_params -> w = &(widget -> placer_params -> w_data);
                        widget -> placer_params -> w_data = widget -> requested_size.width;
                        widget -> placer_params -> rw = NULL;
                }else{
                        widget -> placer_params -> rw_data = *rel_width;
                        widget -> placer_params -> rw = &(widget -> placer_params -> rw_data);
                        widget -> placer_params -> w = NULL;
                }
        }else{
                widget -> placer_params -> w_data = *width;
                widget -> placer_params -> w = &(widget -> placer_params -> w_data);
        }

        // h and rel_height
        if (height == NULL){
                // requested size - we have to consider the rel_height
                if (rel_height == NULL){
                        widget -> placer_params -> h_data = widget -> requested_size.height;
                        widget -> placer_params -> h = &(widget -> placer_params -> h_data);
                        widget -> placer_params -> rh = NULL;
                }else{
                        widget -> placer_params -> rh_data = *rel_height;
                        widget -> placer_params -> rh = &(widget -> placer_params -> rh_data);
                        widget -> placer_params -> h = NULL;
                }
        }else{
                widget -> placer_params -> h_data = *height;
                widget -> placer_params -> h = &(widget -> placer_params -> h_data);
        }

        // rx
        if ( rel_x == NULL){
                widget -> placer_params -> rx_data = 0;
                widget -> placer_params -> rx = NULL;
        }else{
                widget -> placer_params -> rx_data = *rel_x;
                widget -> placer_params -> rx = &(widget -> placer_params -> rx_data);
        }

        // ry
        if ( rel_y == NULL){
                widget -> placer_params -> ry_data = 0;
                widget -> placer_params -> ry = NULL;
        }else{
                widget -> placer_params -> ry_data = *rel_y;
                widget -> placer_params -> ry = &(widget -> placer_params -> ry_data);
        }
}




/**
 * \brief	Tells the placer to recompute the geometry of a widget.
 *		The widget must have been previously placed by a call to \ref ei_place.
 *		Geometry re-computation is necessary for example when the text label of
 *		a widget has changed, and thus the widget "natural" size has changed.
 *
 * @param	widget		The widget which geometry must be re-computed.
 */
void ei_placer_run(struct ei_widget_t* widget)
{
        if (widget == NULL) {
                return;
        }
        if (widget->placer_params != NULL) {
                // get parent size
                ei_rect_t parent_rect;
                if (widget->parent != NULL) {
                        parent_rect = *(widget->parent->content_rect);
                }
                else {
                        // root widget
                        parent_rect.top_left.x = 0;
                        parent_rect.top_left.y = 0;
                        parent_rect.size = hw_surface_get_size(ei_app_root_surface());
                }

                ei_anchor_t widget_anchor = widget->placer_params->anchor_data;
                if ((widget->placer_params->anchor == NULL) || (*widget->placer_params->anchor == ei_anc_none)) {
                        widget_anchor = ei_anc_northwest;
                }

                ei_point_t starting_point = parent_rect.top_left;
                if (widget->placer_params->x != NULL) {
                        starting_point.x += widget->placer_params->x_data;
                }
                if (widget->placer_params->y != NULL) {
                        starting_point.y += widget->placer_params->y_data;
                }
                if (widget->placer_params->rx != NULL) {
                        starting_point.x+= (int) (((float) parent_rect.size.width)*widget->placer_params->rx_data);
                }
                if (widget->placer_params->ry != NULL) {
                        starting_point.y+= (int) (((float) parent_rect.size.height)*widget->placer_params->ry_data);
                }

                ei_size_t widget_size_abs = {0, 0};
                ei_size_t widget_size_rel = {0, 0};
                ei_size_t widget_size_req = widget->requested_size;
                if (widget->placer_params->w != NULL) {
                        widget_size_abs.width = widget->placer_params->w_data;
                        widget_size_req.width = 0;
                        widget_size_req.height = 0;
                }
                if (widget->placer_params->h != NULL) {
                        widget_size_abs.height = widget->placer_params->h_data;
                        widget_size_req.width = 0;
                        widget_size_req.height = 0;
                }
                if (widget->placer_params->rw != NULL) {
                        widget_size_rel.width = (int) (((float) parent_rect.size.width)*widget->placer_params->rw_data);
                        widget_size_req.width = 0;
                        widget_size_req.height = 0;
                }
                if (widget->placer_params->rh != NULL) {
                        widget_size_rel.height = (int) (((float) parent_rect.size.height)*widget->placer_params->rh_data);
                        widget_size_req.width = 0;
                        widget_size_req.height = 0;
                }
                ei_size_t widget_size;
                widget_size.width = widget_size_req.width + widget_size_abs.width + widget_size_rel.width;
                widget_size.height = widget_size_req.height + widget_size_abs.height + widget_size_rel.height;

                widget->screen_location.top_left = starting_point;
                switch ((ei_anchor_t) widget_anchor) {
                        case ei_anc_northwest:
                                break;
                        case ei_anc_center:
                                widget->screen_location.top_left.x+= widget_size.width / 2;
                                widget->screen_location.top_left.y+= widget_size.height / 2;
                                break;
                        case ei_anc_north:
                                widget->screen_location.top_left.x+= widget_size.width / 2;
                                break;
                        case ei_anc_south:
                                widget->screen_location.top_left.x+= widget_size.width / 2;
                                widget->screen_location.top_left.y-= widget_size.height;
                                break;
                        case ei_anc_east:
                                widget->screen_location.top_left.x-= widget_size.width;
                                widget->screen_location.top_left.y+= widget_size.height / 2;
                                break;
                        case ei_anc_southeast:
                                widget->screen_location.top_left.x-= widget_size.width;
                                widget->screen_location.top_left.y-= widget_size.height;
                                break;
                        case ei_anc_west:
                                widget->screen_location.top_left.y+= widget_size.height / 2;
                                break;
                        case ei_anc_northeast:
                                widget->screen_location.top_left.x+= widget_size.width;
                                break;
                        case ei_anc_southwest:
                                widget->screen_location.top_left.y-= widget_size.height;
                                break;
                        case ei_anc_none:
                                break;
                }

                widget->screen_location.size = widget_size;

                //ei_app_invalidate_rect(&widget->screen_location);

                widget->wclass->geomnotifyfunc(widget, widget->screen_location);
        }

        ei_widget_t *current = widget->children_head;
        while (current != NULL) {
                ei_placer_run(current);
                if (current->next_sibling != NULL && current != widget->children_tail) {
                        current = current->next_sibling;
                }
                else {
                        current = NULL;
                }
        }
}



/**
 * \brief	Tells the placer to remove a widget from the screen and forget about it.
 *		Note: the widget is not destroyed and still exists in memory.
 *
 * @param	widget		The widget to remove from screen.
 */
void ei_placer_forget(struct ei_widget_t* widget)
{
        free(widget->placer_params);
        widget->placer_params = NULL;
}

