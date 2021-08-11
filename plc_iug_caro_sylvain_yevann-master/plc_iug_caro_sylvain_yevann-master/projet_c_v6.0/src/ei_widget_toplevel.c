//
// Created by sylvain on 20/05/2021.
//

#include "stdint.h"
#include "ei_widget.h"
#include "stdlib.h"
#include "string.h"
#include "ei_widget_toplevel.h"
#include "relief.h"
#include "ei_event.h"
#include "picker.h"
#include "ei_application.h"

static int triangle_offset = 12;
static int header_height = 27;
static int cross_offset = 15;
static int cross_radius = 9;

static ei_size_t default_toplevel_size = {160, 120};

static ei_color_t default_toplevel_border_color = {100, 100, 100, 255};

static ei_point_t last_mouse_location;

/**
 * \brief A function that update the default toplevel and return a pointer to it
 * @param new_toplevel_data new default toplevel
 * @param reset reset default toplevel to starting value
 * @return
 */
static ei_toplevel_t *ei_default_toplevel_handler(ei_toplevel_t *new_toplevel_data, uint8_t reset)
{
        static ei_toplevel_t default_toplevel = {0};

        // if first function call
        if ((default_toplevel.widget.user_data == 0) || (reset == 1)) {
                default_toplevel.widget.user_data = (void *) 1;

                default_toplevel.color = ei_default_background_color;

                default_toplevel.border_width = k_default_button_border_width;

                default_toplevel.title = NULL;

                default_toplevel.closable = EI_TRUE;

                default_toplevel.resizable = ei_axis_both;

                default_toplevel.min_size = &default_toplevel_size;
        }

        if (new_toplevel_data != NULL) {
                memcpy(&default_toplevel, new_toplevel_data, sizeof(struct ei_toplevel_t));
                default_toplevel.widget.user_data = (void *) 1;
        }

        return &default_toplevel;
}

/**
 * \brief allocate memory block for a toplevel
 * @return a pointer to the allocated memory block
 */
ei_widget_t *ei_toplevel_allocfunc(void)
{
        struct ei_toplevel_t *ptr = malloc(sizeof(struct ei_toplevel_t));
        memset(ptr, 0, sizeof(struct ei_toplevel_t));
        return (ei_widget_t *) ptr;
}

/**
 * delete and free a toplevel memory block
 * @param widget pointer to the toplevel memory block
 */
void ei_toplevel_releasefunc (struct ei_widget_t* widget)
{
        // remove placer_param
        if (widget->placer_params != NULL) {
                free(widget->placer_params);
        }

        // remove all children
        if (widget->children_head!= NULL) {
                ei_widget_t *current = widget->children_head;
                while (current != NULL) {
                        ei_widget_t *tmp = current->next_sibling;
                        if (current->destructor != NULL) {
                                current->destructor(current);
                        }
                        current->wclass->releasefunc(current);
                        current = tmp;
                }
        }

        free(widget);
}

/**
 * \draw a toplevel
 * @param widget pointer to the widget/toplevel to draw
 * @param surface where to draw
 * @param pick_surface offscreen picking surface
 * @param clipper parent clipper
 */
void ei_toplevel_drawfunc (struct ei_widget_t*	widget,
                           ei_surface_t		surface,
                           ei_surface_t		pick_surface,
                           ei_rect_t*		clipper)
{
        ei_rect_t super_clipper;
        if (clipper == NULL){
                super_clipper = hw_surface_get_rect(surface);
        }
        else
        {
                super_clipper = *clipper;
        }
        ei_toplevel_t *toplevel_widget = (ei_toplevel_t *) widget;
        int offset_x;
        int offset_y;

        // draw content background using ei_fill and a specified clipper
        ei_rect_t content_clipper = widget->screen_location;
        content_clipper.top_left.y -= header_height;
        content_clipper.size.height += header_height;

        if ((&super_clipper)->top_left.x > content_clipper.top_left.x) {
                content_clipper.size.width-= (&super_clipper)->top_left.x - content_clipper.top_left.x;
                content_clipper.top_left.x = (&super_clipper)->top_left.x;
        }
        if ((&super_clipper)->top_left.y > content_clipper.top_left.y) {
                content_clipper.size.height-= (&super_clipper)->top_left.y - content_clipper.top_left.y;
                content_clipper.top_left.y = (&super_clipper)->top_left.y;
        }
        offset_x = (&super_clipper)->top_left.x + (&super_clipper)->size.width - widget->screen_location.top_left.x - widget->screen_location.size.width;
        if (offset_x < 0) {
                content_clipper.size.width = super_clipper.size.width  - content_clipper.top_left.x;
        }
        offset_y = (&super_clipper)->top_left.y + (&super_clipper)->size.height - widget->screen_location.top_left.y - widget->screen_location.size.height;
        if (offset_y < 0) {
                content_clipper.size.height = super_clipper.size.height - content_clipper.top_left.y;;
        }

        toplevel_widget->drawn_location = content_clipper;
//        ei_linked_point_t *toplevel_polygon = malloc(5*sizeof(ei_linked_point_t));
//        toplevel_polygon[0].point = content_clipper.top_left;
//        toplevel_polygon[0].next = &toplevel_polygon[1];
//        toplevel_polygon[1].point.x = content_clipper.top_left.x + content_clipper.size.width;
//        toplevel_polygon[1].point.y = content_clipper.top_left.y;
//        toplevel_polygon[1].next = &toplevel_polygon[2];
//        toplevel_polygon[2].point.x = content_clipper.top_left.x + content_clipper.size.width;
//        toplevel_polygon[2].point.y = content_clipper.top_left.y + content_clipper.size.height;
//        toplevel_polygon[2].next = &toplevel_polygon[3];
//        toplevel_polygon[3].point.x = content_clipper.top_left.x;
//        toplevel_polygon[3].point.y = content_clipper.top_left.y + content_clipper.size.height;
//        toplevel_polygon[3].next = &toplevel_polygon[4];
//        toplevel_polygon[4].point = toplevel_polygon[0].point;
//        toplevel_polygon[4].next = NULL;
//        ei_draw_polygon(surface, toplevel_polygon, toplevel_widget->color, &content_clipper);
//        free(toplevel_polygon);
        ei_fill(surface, &(toplevel_widget->color), &content_clipper);
        // draw on picking surface ?
        if (surface == ei_app_root_surface()) {
                ei_surface_t picking_surface = get_picking_surface();
                ei_color_t *picking_color = toplevel_widget->widget.pick_color;
                ei_fill(picking_surface, picking_color, &content_clipper);
        }

        // draw borders
        ei_linked_point_t *border_polygon = malloc(5*sizeof(ei_linked_point_t));
        border_polygon[0].point.x = content_clipper.top_left.x - toplevel_widget->border_width;
        border_polygon[0].point.y = content_clipper.top_left.y;
        border_polygon[0].next = &border_polygon[1];
        border_polygon[1].point.x = content_clipper.top_left.x - toplevel_widget->border_width;
        border_polygon[1].point.y = content_clipper.top_left.y + content_clipper.size.height + toplevel_widget->border_width;
        border_polygon[1].next = &border_polygon[2];
        border_polygon[2].point.x = content_clipper.top_left.x;
        border_polygon[2].point.y = content_clipper.top_left.y + content_clipper.size.height;
        border_polygon[2].next = &border_polygon[3];
        border_polygon[3].point.x = content_clipper.top_left.x;
        border_polygon[3].point.y = content_clipper.top_left.y;
        border_polygon[3].next = &border_polygon[4];
        border_polygon[4].point = border_polygon[0].point;
        border_polygon[4].next = NULL;
        ei_draw_polygon(surface, border_polygon, default_toplevel_border_color, &super_clipper);

        border_polygon[0].point.x = content_clipper.top_left.x - toplevel_widget->border_width;
        border_polygon[0].point.y = content_clipper.top_left.y + content_clipper.size.height + toplevel_widget->border_width;
        border_polygon[0].next = &border_polygon[1];
        border_polygon[1].point.x = content_clipper.top_left.x + content_clipper.size.width + toplevel_widget->border_width;
        border_polygon[1].point.y = content_clipper.top_left.y + content_clipper.size.height + toplevel_widget->border_width;
        border_polygon[1].next = &border_polygon[2];
        border_polygon[2].point.x = content_clipper.top_left.x + content_clipper.size.width;
        border_polygon[2].point.y = content_clipper.top_left.y + content_clipper.size.height;
        border_polygon[2].next = &border_polygon[3];
        border_polygon[3].point.x = content_clipper.top_left.x;
        border_polygon[3].point.y = content_clipper.top_left.y + content_clipper.size.height;
        border_polygon[3].next = &border_polygon[4];
        border_polygon[4].point = border_polygon[0].point;
        border_polygon[4].next = NULL;
        ei_draw_polygon(surface, border_polygon, default_toplevel_border_color, &super_clipper);

        border_polygon[0].point.x = content_clipper.top_left.x + content_clipper.size.width + toplevel_widget->border_width;
        border_polygon[0].point.y = content_clipper.top_left.y + content_clipper.size.height + toplevel_widget->border_width;
        border_polygon[0].next = &border_polygon[1];
        border_polygon[1].point.x = content_clipper.top_left.x + content_clipper.size.width + toplevel_widget->border_width;
        border_polygon[1].point.y = content_clipper.top_left.y;
        border_polygon[1].next = &border_polygon[2];
        border_polygon[2].point.x = content_clipper.top_left.x + content_clipper.size.width;
        border_polygon[2].point.y = content_clipper.top_left.y;
        border_polygon[2].next = &border_polygon[3];
        border_polygon[3].point.x = content_clipper.top_left.x + content_clipper.size.width;
        border_polygon[3].point.y = content_clipper.top_left.y + content_clipper.size.height;
        border_polygon[3].next = &border_polygon[4];
        border_polygon[4].point = border_polygon[0].point;
        border_polygon[4].next = NULL;
        ei_draw_polygon(surface, border_polygon, default_toplevel_border_color, &super_clipper);

        // draw header
        border_polygon[0].point.x = content_clipper.top_left.x - toplevel_widget->border_width;
        border_polygon[0].point.y = content_clipper.top_left.y;
        border_polygon[1].point.x = content_clipper.top_left.x - toplevel_widget->border_width;
        border_polygon[1].point.y = content_clipper.top_left.y + header_height;
        border_polygon[2].point.x = content_clipper.top_left.x + content_clipper.size.width + toplevel_widget->border_width;
        border_polygon[2].point.y = content_clipper.top_left.y + header_height;
        border_polygon[3].point.x = content_clipper.top_left.x + content_clipper.size.width + toplevel_widget->border_width;
        border_polygon[3].point.y = content_clipper.top_left.y;
        border_polygon[3].next = &border_polygon[4];
        border_polygon[4].point = border_polygon[0].point;
        border_polygon[4].next = NULL;
        ei_draw_polygon(surface, border_polygon, default_toplevel_border_color, &super_clipper);
        free(border_polygon);

        // draw closing cross
        if (toplevel_widget->closable == EI_TRUE) {
                ei_color_t cross_background_color = {200, 0x00, 0x00, 0xff};
                ei_color_t cross_color = {0x00, 0x00, 0x00, 0xff};
                ei_point_t center;
                center.x = widget->screen_location.top_left.x + widget->screen_location.size.width - cross_offset;
                center.y = content_clipper.top_left.y + (header_height / 2);
                ei_linked_point_t *last_point;
                ei_linked_point_t *cross_background = draw_arc(center, cross_radius,0, 7, &last_point);
                ei_linked_point_t *yevann_point = malloc(sizeof(ei_linked_point_t));
                yevann_point->point = cross_background->point;
                yevann_point->next = NULL;
                last_point->next = yevann_point;

                ei_draw_polygon(surface, cross_background, cross_background_color, &super_clipper);
                free(cross_background);
                free(yevann_point);
                ei_linked_point_t *cross = calloc(sizeof(ei_linked_point_t), 4);
                cross[0].point.x = center.x - (cross_radius / 2);
                cross[0].point.y = center.y - (cross_radius / 2);
                cross[0].next = &cross[1];
                cross[1].point.x = center.x + (cross_radius / 2);
                cross[1].point.y = center.y + (cross_radius / 2);
                cross[1].next = NULL;
                cross[2].point.x = center.x - (cross_radius / 2);
                cross[2].point.y = center.y + (cross_radius / 2);
                cross[2].next = &cross[3];
                cross[3].point.x = center.x + (cross_radius / 2);
                cross[3].point.y = center.y - (cross_radius / 2);
                cross[3].next = NULL;
                ei_draw_polyline(surface, &cross[0], cross_color, &super_clipper);
                ei_draw_polyline(surface, &cross[2], cross_color, &super_clipper);
                free(cross);
        }

        // draw title
        if (toplevel_widget->title != NULL) {
                int text_offset = 40;
                if (widget->screen_location.size.width - text_offset > 0) {
                        ei_rect_t title_clipper;
                        title_clipper.top_left.x = widget->screen_location.top_left.x;
                        title_clipper.top_left.y = content_clipper.top_left.y;
                        title_clipper.size.width = content_clipper.size.width;
                        if (widget->screen_location.top_left.x < 0) {
                                title_clipper.size.width -= text_offset;
                        }
                        title_clipper.size.height = header_height;


                        if (title_clipper.top_left.x < (&super_clipper)->top_left.x) {
                                title_clipper.top_left.x = (&super_clipper)->top_left.x;}
                        if (title_clipper.top_left.y < (&super_clipper)->top_left.y) {
                                title_clipper.top_left.y = (&super_clipper)->top_left.y;}


                        offset_x = title_clipper.top_left.x + title_clipper.size.width - widget->screen_location.top_left.x - widget->screen_location.size.width;
                        if (offset_x > 0) {
                                title_clipper.size.width = super_clipper.size.width - content_clipper.top_left.x;
                        }
                        offset_y = title_clipper.top_left.y + title_clipper.size.height - widget->screen_location.top_left.y - widget->screen_location.size.height;
                        if (offset_y > 0) {
                                title_clipper.size.height = super_clipper.size.height - content_clipper.top_left.y;
                        }

                        ei_color_t black;
                        black.blue = 0x00;
                        black.red = 0x00;
                        black.green = 0x00;
                        black.alpha = 0xff;
                        ei_draw_text(surface, &title_clipper.top_left, toplevel_widget->title, NULL, black, &title_clipper);
                }
        }

        // if resizable, draw resizable triangle
        if (toplevel_widget->resizable != ei_axis_none) {
                ei_color_t triangle_color = {100, 100, 100, 255};

                ei_linked_point_t *triangle = malloc(4*sizeof(ei_linked_point_t));

                triangle[0].point.x = content_clipper.top_left.x + content_clipper.size.width;
                triangle[0].point.y = content_clipper.top_left.y + content_clipper.size.height;
                triangle[0].next = &triangle[1];

                triangle[1].point = triangle[0].point;
                triangle[1].point.x -= triangle_offset;
                triangle[1].next = &triangle[2];

                triangle[2].point = triangle[0].point;
                triangle[2].point.y -= triangle_offset;
                triangle[2].next = &triangle[3];

                // Yevann point
                triangle[3].point = triangle[0].point;
                triangle[3].next = NULL;

                ei_draw_polygon(surface, triangle, triangle_color, &super_clipper);

                free(triangle);
        }

        // draw children

        ei_widget_t *current = widget->children_head;
        while (current != NULL) {
                ei_rect_t new_clipper = content_clipper;
                if (clipper->top_left.x > new_clipper.top_left.x) {
                        new_clipper.top_left.x = clipper->top_left.x;
                }
                if (clipper->top_left.y > new_clipper.top_left.y) {
                        new_clipper.top_left.y = clipper->top_left.y;
                }
                offset_x = clipper->top_left.x + clipper->size.width - new_clipper.top_left.x + new_clipper.size.width;
                if (offset_x < 0) {
                        new_clipper.size.width += offset_x;
                }
                offset_y = clipper->top_left.x + clipper->size.height - new_clipper.top_left.x + new_clipper.size.height;
                if (offset_y < 0) {
                        new_clipper.size.height += offset_y;
                }
                if (current->placer_params != NULL) {
                        current->wclass->drawfunc(current, surface, pick_surface, &new_clipper);
                }
                current = current->next_sibling;
        }
}

/**
 * \brief put default values in the toplevel
 * @param widget pointer to a widget/toplevel
 */
void ei_toplevel_setdefaultsfunc (struct ei_widget_t* widget)
{
        struct ei_toplevel_t *default_toplevel_ptr = ei_default_toplevel_handler(NULL, 0);

        struct ei_toplevel_t *toplevel_widget = (ei_toplevel_t *) widget;

        memcpy(&toplevel_widget->color, &default_toplevel_ptr->color, sizeof(struct ei_toplevel_t) - sizeof(struct ei_widget_t));
}

/**
 * \brief a function to call when the location of a toplevel is changed
 * @param widget pointer to a widget/toplevel
 * @param rect new location
 */
void ei_toplevel_geomnotifyfunc (struct ei_widget_t*	widget,
                                 ei_rect_t		rect)
{
        struct ei_toplevel_t *toplevel_widget = (struct ei_toplevel_t *) widget;

        toplevel_widget->widget.screen_location = rect;

        // if size less than min size
        if (toplevel_widget->widget.screen_location.size.width < toplevel_widget->min_size->width) {
                toplevel_widget->widget.screen_location.size.width = toplevel_widget->min_size->width;
        }
        if (toplevel_widget->widget.screen_location.size.height < toplevel_widget->min_size->height) {
                toplevel_widget->widget.screen_location.size.height = toplevel_widget->min_size->height;
        }

        // if on the top border, we don't want to cut the header
        if (toplevel_widget->widget.screen_location.top_left.y < toplevel_widget->widget.parent->content_rect->top_left.y + header_height) {
//                toplevel_widget->widget.screen_location.size.height += toplevel_widget->widget.parent->content_rect->top_left.y + header_height - toplevel_widget->widget.screen_location.top_left.y;
                toplevel_widget->widget.screen_location.top_left.y = toplevel_widget->widget.parent->content_rect->top_left.y + header_height;
        }

}

/**
 * \brief return EI_TRUE if the event is located on the closing cross of a toplevel
 * @param toplevel pointer to a widget/toplevel
 * @param event event
 * @return boolean
 */
static ei_bool_t is_closing_cross(struct ei_toplevel_t*	toplevel,
                                  struct ei_event_t*	event)
{
        ei_point_t cross_center;
        cross_center.x = toplevel->drawn_location.top_left.x + toplevel->drawn_location.size.width - cross_offset;
        cross_center.y = toplevel->drawn_location.top_left.y + (header_height / 2);
        ei_point_t picking_point = event->param.mouse.where;
        float dx = (float) (picking_point.x - cross_center.x);
        float dy = (float) (picking_point.y - cross_center.y);
        int distance2 = (int) (dx*dx + dy*dy);
        int radius2 = cross_radius*cross_radius;
        if (distance2 <= radius2) {
                return EI_TRUE;
        }
        return EI_FALSE;
}

/**
 * \brief return EI_TRUE if the event is located on the header of a toplevel
 * @param toplevel pointer to a widget/toplevel
 * @param event event
 * @return boolean
 */
static ei_bool_t is_header(struct ei_toplevel_t*	toplevel,
                           struct ei_event_t*	event)
{
        ei_rect_t header = toplevel->widget.screen_location;
        header.top_left.y -= header_height;
        if (header.top_left.y < 0) {
                header.top_left.y = 0;
        }
        header.top_left.x -= toplevel->border_width;
        header.size.width += toplevel->border_width;
        header.size.height = header_height;

        ei_point_t picking_point = event->param.mouse.where;

        if ((picking_point.x < header.top_left.x) || (picking_point.y < header.top_left.y)) {
                return EI_FALSE;
        }
        if ((picking_point.x > header.top_left.x + header.size.width) || (picking_point.y > header.top_left.y + header.size.height)) {
                return EI_FALSE;
        }

        return EI_TRUE;
}

/**
 * \brief return EI_TRUE if the event is located on the resizing rectangle of a toplevel
 * @param toplevel pointer to a widget/toplevel
 * @param event event
 * @return boolean
 */
static ei_bool_t is_resize(struct ei_toplevel_t*	toplevel,
                                    struct ei_event_t*	event)
{
        ei_rect_t rect_resize = toplevel->widget.screen_location;
        rect_resize.top_left.x += toplevel->widget.screen_location.size.width - triangle_offset;
        rect_resize.top_left.y += toplevel->widget.screen_location.size.height - triangle_offset;

        ei_point_t picking_point = event->param.mouse.where;

        if (picking_point.x < rect_resize.top_left.x || (picking_point.y < rect_resize.top_left.y)) {
                return EI_FALSE;
        }

        if ((picking_point.x > rect_resize.top_left.x + rect_resize.size.width) || (picking_point.y > rect_resize.top_left.y + rect_resize.size.height)) {
                return EI_FALSE;
        }

        return EI_TRUE;
}

/**
 * \brief function to call when an event is linked or located to a toplevel
 * @param widget pointer to a widget/toplevel
 * @param event event
 * @return Ei_TRUE if the event was consumed, EI_FALSE otherwise
 */
ei_bool_t ei_toplevel_handlefunc (struct ei_widget_t*	widget,
                                  struct ei_event_t*	event)
{
        ei_toplevel_t *toplevel = (ei_toplevel_t *) widget;

        // if mouse click event
        if ((event->type == ei_ev_mouse_buttondown) && (event->param.mouse.button == ei_mouse_button_left)) {
                ei_event_set_active_widget(widget);

                // select which part of the toplevel was clicked
                toplevel->active_action = toplevel_no_action;

                // check if it is the closing cross
                if (is_closing_cross(toplevel, event) == EI_TRUE) {
                        toplevel->active_action = toplevel_closing;
                }

                // check if it is the header
                else if (is_header(toplevel, event) == EI_TRUE) {
                        toplevel->active_action = toplevel_moving;
                }

                // check if it is the resizable triangle
                else if (is_resize(toplevel, event) == EI_TRUE) {
                        toplevel->active_action = toplevel_resizing;
                }

                last_mouse_location = event->param.mouse.where;

                // put on the foreground
                ei_widget_t *parent = widget->parent;
                // remove widget from parent's list of widget
                if (parent->children_head == widget) {
                        parent->children_head = widget->next_sibling;
                        if (parent->children_tail == widget) {
                                parent->children_tail = NULL;
                        }
                }
                else {
                        ei_widget_t *current = parent->children_head;

                        while (current->next_sibling != widget) {
                                current = current->next_sibling;
                        }

                        if (parent->children_tail != widget) {
                                parent->children_tail = widget->next_sibling;
                        }
                        else {
                                parent->children_tail = parent->children_head;
                        }

                        current->next_sibling = widget->next_sibling;
                }
                // put widget at the end of the parent's list
                if (parent->children_tail != NULL) {
                        parent->children_tail->next_sibling = widget;
                }
                widget->next_sibling = NULL;
                parent->children_tail = widget;
                if (parent->children_head == NULL) {
                        parent->children_head = widget;
                }
        }

        // if mouse move
        if ((ei_event_get_active_widget() == widget) && (event->type == ei_ev_mouse_move) && (event->param.mouse.button == ei_mouse_button_left)) {
                int offset_x = event->param.mouse.where.x - last_mouse_location.x;
                int offset_y = event->param.mouse.where.y - last_mouse_location.y;

                if (toplevel->active_action == toplevel_resizing) {
                        if (toplevel->widget.placer_params->w == NULL) {
                                toplevel->widget.placer_params->w_data = 0;
                                toplevel->widget.placer_params->w = &toplevel->widget.placer_params->w_data;
                        }
                        if (toplevel->widget.placer_params->h == NULL) {
                                toplevel->widget.placer_params->h_data = 0;
                                toplevel->widget.placer_params->h = &toplevel->widget.placer_params->h_data;
                        }
                        switch (toplevel->resizable) {
                                case ei_axis_x:
                                        toplevel->widget.placer_params->w_data += offset_x;
                                        break;
                                case ei_axis_y:
                                        toplevel->widget.placer_params->h_data += offset_y;
                                        break;
                                case ei_axis_both:
                                        toplevel->widget.placer_params->w_data += offset_x;
                                        toplevel->widget.placer_params->h_data += offset_y;
                                        break;
                                default:
                                        break;
                        }

                        ei_placer_run(toplevel->widget.parent);
                }
                else if (toplevel->active_action == toplevel_moving) {
                        if (toplevel->widget.placer_params->x == NULL) {
                                toplevel->widget.placer_params->x_data = 0;
                                toplevel->widget.placer_params->x = &toplevel->widget.placer_params->x_data;
                        }
                        if (toplevel->widget.placer_params->y == NULL) {
                                toplevel->widget.placer_params->y_data = 0;
                                toplevel->widget.placer_params->y = &toplevel->widget.placer_params->y_data;
                        }
                        toplevel->widget.placer_params->x_data += offset_x;
                        toplevel->widget.placer_params->y_data += offset_y;

                        ei_placer_run(toplevel->widget.parent);
                }

                last_mouse_location = event->param.mouse.where;
        }

        // if mouse unclick event
        if ((event->type == ei_ev_mouse_buttonup) && (event->param.mouse.button == ei_mouse_button_left)) {
                if (ei_event_get_active_widget() == widget) {
                        if (toplevel->active_action == toplevel_closing) {
                                if (is_closing_cross(toplevel, event) == EI_TRUE) {
                                        ei_widget_t *parent = toplevel->widget.parent;

                                        // si à détruire
                                        ei_widget_destroy(widget);

                                        // si simplement à oublier
//                                        ei_placer_forget(widget);

                                        ei_app_invalidate_rect(parent->content_rect);

                                        ei_placer_run(parent);

                                }
                        }
                }
                ei_event_set_active_widget(NULL);
                toplevel->active_action = toplevel_no_action;
        }

        return EI_TRUE;
}