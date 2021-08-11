//
// Created by sylvain on 19/05/2021.
//

#include "ei_widget_frame.h"
#include "stdint.h"
#include "stdlib.h"
#include "ei_draw.h"
#include "ei_widgetclass.h"
#include "ei_widget.h"
#include "widget_text.h"
#include "ei_types.h"
#include "hw_interface.h"
#include "relief.h"
#include "ei_draw_polygon.h"
//#include "picker.h"
#include "string.h"
#include "ei_application.h"
#include <stdio.h>

struct vertexes{
        ei_point_t a; // top left point
        ei_point_t b; // top right point
        ei_point_t c; // bottom right point
        ei_point_t d; // bottom left point
};

static ei_rect_t remove_border_from_screen_location(ei_rect_t rect, int border_width)
{
        ei_rect_t new_rect;
                new_rect.top_left.x = rect.top_left.x + border_width;
        new_rect.top_left.y = rect.top_left.y + border_width;
        new_rect.size.width = rect.size.width - 2*border_width;
        new_rect.size.height = rect.size.height - 2*border_width;
        if (new_rect.size.width < 0) {
                new_rect.size.width = 0;
        }
        if (new_rect.size.height < 0) {
                new_rect.size.height = 0;
        }

        return new_rect;
}


/**
 * @brief Create a struct vertexes from a rectangle
 * @param rect the rectangle to take into account
 * @return
 */
struct vertexes create_vertexes(ei_rect_t rect){
        struct vertexes vert;
        // getting parameters
        ei_point_t anchor = rect.top_left;
        int w = rect.size.width;
        int h = rect.size.height;
        // compute points
        ei_point_t  pa = {anchor.x, anchor.y};
        ei_point_t pb = {anchor.x + w, anchor.y};
        ei_point_t pc = { anchor.x + w, anchor.y + h};
        ei_point_t pd = {anchor.x, anchor.y + h};
        // stock in the structure
        vert.a = pa;
        vert.b = pb;
        vert.c = pc;
        vert.d = pd;
        // return the new structure
        return vert;
}

/**
 * @brief Insert a point in a linked list of points
 * @param pl the list of points
 * @param to_insert the point to insert
 */
 void point_insert(ei_linked_point_t **pl, ei_point_t to_insert){
        ei_linked_point_t *new_head = malloc(sizeof(ei_linked_point_t));
        new_head->point = to_insert;
        new_head -> next = *pl;
        *pl = new_head;
}

//struct vertexes{
//                 ei_point_t a;
//                 ei_point_t b;
//                 ei_point_t c;
//                 ei_point_t d;
//         };
//
//struct vertexes create_vertexes(ei_rect_t rect){
//        struct vertexes vert;
//        ei_point_t anchor = rect.top_left;
//        int w = rect.size.width;
//        int h = rect.size.height;
//        ei_point_t  pa = {anchor.x, anchor.y};
//        ei_point_t pb = {anchor.x + w, anchor.y};
//        ei_point_t pc = { anchor.x + w, anchor.y + h};
//        ei_point_t pd = {anchor.x, anchor.y + h};
//        vert.a = pa;
//        vert.b = pb;
//        vert.c = pc;
//        vert.d = pd;
//        return vert;
//}
//
//void point_insert(ei_linked_point_t **pl, ei_point_t to_insert){
//        ei_linked_point_t *new_head = malloc(sizeof(ei_point_t));
//        new_head->point = to_insert;
//        new_head -> next = *pl;
//        *pl = new_head;
//}


/*
 * update default frame values and return a pointer to it, do not update in new_frame_data is NULL
 */
static ei_frame_t *ei_default_frame_handler(ei_frame_t *new_frame_data, uint8_t reset)
{
        static ei_frame_t default_frame = {0};

        // if first function call
        if ((default_frame.widget.user_data == 0) || (reset == 1)) {
                default_frame.widget.user_data = (void *) 1;

                default_frame.color = ei_default_background_color;

                default_frame.border_with = 0;

                default_frame.text = NULL;
                default_frame.text_color = ei_font_default_color;
                default_frame.text_font = ei_default_font;
                default_frame.text_anchor = ei_anc_center;

                default_frame.img_rect = NULL;
                default_frame.img_anchor = ei_anc_center;

                default_frame.relief = ei_relief_none;
        }

        if (new_frame_data != NULL) {
                memcpy(&default_frame, new_frame_data, sizeof(ei_frame_t));
                default_frame.widget.user_data = (void *) 1;
        }

        return &default_frame;
}
/**
 * @brief create/allocate a pointer to a widget frame
 * @return a pointer to the frame cast in a widget pointer
 */
ei_widget_t *ei_frame_allocfunc(void)
{
        ei_frame_t *ptr = malloc(sizeof(ei_frame_t));
        memset(ptr, 0, sizeof(ei_frame_t));
        return (ei_widget_t *) ptr;
}


/**
 * @brief Release the widget frame
 * Free the Placer parameters
 * Remove all children
 * Free the widget
 * @param widget widget to release
 */
void ei_frame_releasefunc (struct ei_widget_t* widget)
{
//        ei_frame_t *ptr_frame = (ei_frame_t *) widget;
//        struct ei_widget_t *ptr_widget = malloc(sizeof(struct ei_widget_t));
//        memcpy(ptr_widget, ptr_frame, sizeof(struct ei_widget_t));

        // remove placer_param
        if (widget->placer_params != NULL) {
                free(widget->placer_params);
        }

        ei_frame_t *frame = (ei_frame_t *) widget;

        // remove img rect
        if (frame->img_rect != NULL) {
                free(frame->img_rect);
        }

        // remove img
        if (frame->img != NULL) {
                free(frame->img);
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
                                             ei_rect_t*		clipper)
{
        // recover widget fields useful to this function
        // 1 - common widget fields
        ei_rect_t sl = widget -> screen_location;
        // 2 - specific widget frame fields
        ei_color_t col = ((ei_frame_t *) widget )-> color;
        ei_relief_t rel = ((ei_frame_t *) widget )-> relief;
        int bw = ((ei_frame_t *) widget )-> border_with;
        char *txt = ((ei_frame_t *) widget )-> text;
        ei_font_t tf = ((ei_frame_t *) widget )-> text_font;
        ei_color_t tc = ((ei_frame_t *) widget )-> text_color;
        ei_anchor_t ta = ((ei_frame_t *) widget )-> text_anchor;
        ei_surface_t image = ((ei_frame_t *) widget )-> img;
        ei_rect_t *ir = ((ei_frame_t *) widget )-> img_rect;

        ei_rect_t super_clipper;
        if (clipper == NULL){
                super_clipper = hw_surface_get_rect(surface);
        }else{
                super_clipper = *clipper;
        }



        // fill the surface of the frame
        ei_rect_t screen_location_without_border = remove_border_from_screen_location(sl, bw);
        struct vertexes widget_vert = create_vertexes(screen_location_without_border);
        ei_linked_point_t *first_point = malloc(sizeof(ei_linked_point_t));
        first_point -> point = widget_vert.a;
        first_point -> next = NULL;
        point_insert(&first_point, widget_vert.d);
        point_insert(&first_point, widget_vert.c);
        point_insert(&first_point, widget_vert.b);
        point_insert(&first_point, widget_vert.a);

        if (rel != ei_relief_none){
                // draw of the relief
                ei_linked_point_t *top_part = rect_frame(screen_location_without_border, bw, ei_relief_up);
                ei_linked_point_t *bottom_part = rect_frame(screen_location_without_border, bw, ei_relief_bottom);
                ei_color_t col_top;
                ei_color_t col_bottom;
                if (rel == ei_relief_raised){
                        col_top = get_relief_color(col, ei_relief_light);
                        col_bottom = get_relief_color(col, ei_relief_dark);
                }else{
                        col_top = get_relief_color(col, ei_relief_dark);
                        col_bottom = get_relief_color(col, ei_relief_light);
                }
                ei_draw_polygon(surface, top_part, col_top, clipper);
                ei_draw_polygon(surface, bottom_part, col_bottom, clipper);
                free_polygon(&top_part);
                free_polygon(&bottom_part);

        }

        // draw frame
        ei_draw_polygon(surface, first_point, col, &super_clipper);

        // draw the frame in the piking surface
        if (surface == ei_app_root_surface()) {
                ei_draw_polygon(pick_surface, first_point,*(widget -> pick_color) , &super_clipper);
        }

        // write the text in the frame
        if (txt != NULL) {
                ei_rect_t *text_clipper = malloc(sizeof(ei_rect_t));
                text_clipper->top_left.x = sl.top_left.x + bw;
                text_clipper->top_left.y = sl.top_left.y + bw;
                text_clipper->size.height = sl.size.height - 2*bw;
                text_clipper->size.width = sl.size.width - 2*bw;
                // check and / or modify the text_clipper considering
                if (text_clipper->top_left.x < super_clipper.top_left.x ){
                        text_clipper->top_left.x = super_clipper.top_left.x;
                }
                if (text_clipper->top_left.y < super_clipper.top_left.y){
                        text_clipper->top_left.y = super_clipper.top_left.y;
                }
                int offset_x = text_clipper->top_left.x + text_clipper->size.width - super_clipper.top_left.x - super_clipper.size.width;
                if (offset_x > 0){
                        text_clipper->size.width -= offset_x;
                }
                int offset_y = text_clipper->top_left.y + text_clipper->size.height - super_clipper.top_left.y - super_clipper.size.height;
                if (offset_y > 0){
                        text_clipper->size.height -= offset_y;
                }
                widget_draw_text(surface,sl, ta, txt, tf, tc, text_clipper);
                free(text_clipper);
        }

        // display the image in the frame
        else if (image  != NULL) {
                ei_point_t anchor;
                anchor.x = sl.top_left.x + bw;
                anchor.y = sl.top_left.y + bw;
                ei_size_t global_size = sl.size;
                global_size.height -= 2*bw;
                global_size.width -= 2*bw;
                ei_rect_t  img_rect;
                if (ir == NULL){
                        img_rect = hw_surface_get_rect(image);
                }else{
                        img_rect = *ir;
                }
                // anchor of the image
                ei_anchor_t indice = ((ei_frame_t *) widget )-> img_anchor;
                switch (indice) {
                        case ei_anc_none:
                                anchor.x += 0;
                                anchor.y += 0;
                                break;

                        case ei_anc_north:
                                anchor.x += (global_size.width - img_rect.size.width) / 2;
                                anchor.y += 0;
                                break;

                        case ei_anc_northeast:
                                anchor.x += (global_size.width - img_rect.size.width);
                                anchor.y += 0;
                                break;

                        case ei_anc_east:
                                anchor.x += (global_size.width - img_rect.size.width);
                                anchor.y += (global_size.height - img_rect.size.height) / 2;
                                break;

                        case ei_anc_southeast:
                                anchor.x += (global_size.width  - img_rect.size.width);
                                anchor.y += (global_size.height - img_rect.size.height);
                                break;

                        case ei_anc_south:
                                anchor.x += (global_size.width  - img_rect.size.width) / 2;
                                anchor.y += (global_size.height - img_rect.size.height);
                                break;

                        case ei_anc_southwest:
                                anchor.x += 0;
                                anchor.y += (global_size.height - img_rect.size.height);
                                break;

                        case ei_anc_west:
                                anchor.x += 0;
                                anchor.y += (global_size.height - img_rect.size.height) / 2;
                                break;
                        case ei_anc_center:
                                anchor.x += (global_size.width  - img_rect.size.width) / 2;
                                anchor.y += (global_size.height - img_rect.size.height) / 2;
                                break;

                        case ei_anc_northwest:
                                anchor.x += 0;
                                anchor.y += 0;
                                break;



                        default:
                                anchor.x += (global_size.width  - img_rect.size.width) / 2;
                                anchor.y += (global_size.height - img_rect.size.height) / 2;
                                break;

                }
                // compute of the dest_rect & src_rect (considering clipper)
                ei_rect_t dest_rect;
                ei_rect_t src_rect = img_rect;
                dest_rect = sl;
                dest_rect.top_left = anchor;
                // check the dest rect compared to the super clipper and reduce the size if necessary
                if (dest_rect.top_left.x < super_clipper.top_left.x){
                        src_rect.size.width -= ( super_clipper.top_left.x - src_rect.top_left.x);
                        dest_rect.size.width -= (super_clipper.top_left.x - dest_rect.top_left.x);
                        dest_rect.top_left.x = super_clipper.top_left.x;
                }
                if (dest_rect.top_left.y < super_clipper.top_left.y){
                        src_rect.size.height -= ( super_clipper.top_left.y - src_rect.top_left.y);
                        dest_rect.size.height -= (super_clipper.top_left.y - dest_rect.top_left.y);
                        dest_rect.top_left.y = super_clipper.top_left.y;
                }

                if (dest_rect.size.width > src_rect.size.width){
                        dest_rect.size.width = src_rect.size.width;
                }
                if (dest_rect.size.height > src_rect.size.height){
                        dest_rect.size.height = src_rect.size.height;
                }

                if ((dest_rect.size.width <= 0) || (dest_rect.size.height <= 0)){
                        printf("caca\n");
                        return;
                }
                ei_copy_surface(surface, &dest_rect, image, &src_rect, EI_TRUE);
        }
        free_polygon(&first_point);

        // draw children
        ei_widget_t *current = widget->children_head;
        while (current != NULL) {
                ei_rect_t new_clipper = *widget->content_rect;
                if (clipper->top_left.x > new_clipper.top_left.x) {
                        new_clipper.top_left.x = clipper->top_left.x;
                }
                if (clipper->top_left.y > new_clipper.top_left.y) {
                        new_clipper.top_left.y = clipper->top_left.y;
                }
                int offset_x = clipper->top_left.x + clipper->size.width - new_clipper.top_left.x + new_clipper.size.width;
                if (offset_x < 0) {
                        new_clipper.size.width += offset_x;
                }
                int offset_y = clipper->top_left.x + clipper->size.height - new_clipper.top_left.x + new_clipper.size.height;
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
 * @brief Put default param to a widget
 * @param widget widget to put default fields
 */
void ei_frame_setdefaultsfunc (struct ei_widget_t* widget)
{
        ei_frame_t *default_frame_ptr = ei_default_frame_handler(NULL, 0);

        ei_frame_t *frame_widget = (ei_frame_t *) widget;

        memcpy(&frame_widget->color, &default_frame_ptr->color, sizeof(struct ei_frame_t) - sizeof(struct ei_widget_t));
}

/**
 * @brief Called by the placer when we modified the place of the widget
 * @param widget Widget with the screen location which has been modified
 * @param rect the new place
 */
void ei_frame_geomnotifyfunc (struct ei_widget_t*	widget,
                                               ei_rect_t		rect)
{
        ei_frame_t *frame_ptr = (ei_frame_t *) widget;


        frame_ptr->widget.screen_location = rect;

//        if (frame_ptr->img != NULL) {
//                int dx = rect.top_left.x - frame_ptr->widget.screen_location.top_left.x;
//                int dy = rect.top_left.y - frame_ptr->widget.screen_location.top_left.y;
//                frame_ptr->img_rect->top_left.x += dx;
//                frame_ptr->img_rect->top_left.y += dy;
//                frame_ptr->img_rect->size = rect.size;
//        }
}

/**
 * @brief return if the widget is handled or not
 * @param widget widget to check
 * @param event the event which has arrived
 * @return For a frame, always True
 */
ei_bool_t ei_frame_handlefunc (struct ei_widget_t*	widget,
                                         struct ei_event_t*	event)
{
        return EI_FALSE;
}