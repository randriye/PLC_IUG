//
// Created by sylvain on 20/05/2021.
//


#include "ei_widget.h"
#include "ei_types.h"
#include "stdlib.h"
#include "string.h"
#include "ei_widget_button.h"
#include "ei_draw.h"
#include "relief.h"
#include "ei_draw_polygon.h"
#include "picker.h"
#include "ei_application.h"
#include "pixel.h"
#include "widget_text.h"
#include "ei_event.h"

/**
 *  \brief                  A function that update the default button and return a pointer to it
 * @param new_button_data   new default button
 * @param reset             reset default button to stating values
 * @return
 */
static ei_button_t *ei_default_button_handler(ei_button_t *new_button_data, uint8_t reset)
{
        static ei_button_t default_button = {0};

        // if first function call
        if ((default_button.widget.user_data == 0) || (reset == 1)) {
                default_button.widget.user_data = (void *) 1;

                default_button.color = ei_default_background_color;

                default_button.border_with = k_default_button_border_width;

                default_button.text = NULL;
                default_button.text_color = ei_font_default_color;
                default_button.text_font = ei_default_font;
                default_button.text_anchor = ei_anc_center;

                default_button.img_rect = NULL;
                default_button.img_anchor = ei_anc_center;

                default_button.relief = ei_relief_none;

                default_button.corner_radius = k_default_button_corner_radius;

                default_button.callback = NULL;
                default_button.user_param = NULL;
        }

        if (new_button_data != NULL) {
                memcpy(&default_button, new_button_data, sizeof(ei_button_t));
                default_button.widget.user_data = (void *) 1;
        }

        return &default_button;
}

/**
 * \brief   allocate memory for a new button
 * @return  pointer to allocated button
 */
ei_widget_t *ei_button_allocfunc(void)
{
        struct ei_button_t *ptr = malloc(sizeof(struct ei_button_t));
        memset(ptr, 0, sizeof(struct ei_button_t));
        return (ei_widget_t *) ptr;
}

/**
 * \brief       release button
 * @param widget button pointer
 */
void ei_button_releasefunc (struct ei_widget_t* widget)
{
        // remove placer_param
        if (widget->placer_params != NULL) {
                free(widget->placer_params);
        }

        ei_button_t *button = (ei_button_t *) widget;

        // remove img rect
        if (button->img_rect != NULL) {
                free(button->img_rect);
        }

        // remove img
        if (button->img != NULL) {
                free(button->img);
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
 * \brief   draw image inside a button
 * @param surface where to draw
 * @param screen_location screen location of the button
 * @param button_polygon polygon describing the button
 * @param img  image to draw
 * @param img_rect part of the image used
 * @param img_anchor anchor of the image
 * @param clipper button clipper
 */
static void draw_image_button(ei_surface_t surface, ei_rect_t screen_location, ei_linked_point_t *button_polygon, ei_surface_t img, ei_rect_t *img_rect, ei_anchor_t img_anchor, ei_rect_t *clipper)
{

        // resize img_rect if larger than screen_location
        ei_rect_t real_img_rect = *img_rect;
        if (real_img_rect.size.width > screen_location.size.width) {
                real_img_rect.size.width = screen_location.size.width;
        }
        if (real_img_rect.size.height > screen_location.size.height) {
                real_img_rect.size.height = screen_location.size.height;
        }

        ei_rect_t src_rect = real_img_rect;
        ei_rect_t dst_rect = screen_location;

        // place img considering anchor
        switch (img_anchor) {
                case ei_anc_none:
                case ei_anc_northwest:
                        break;
                case ei_anc_center:
                        dst_rect.top_left.x += (screen_location.size.width/2 - real_img_rect.size.width/2);
                        dst_rect.top_left.y += (screen_location.size.height/2 - real_img_rect.size.height/2);
                        break;
                case ei_anc_east:
                        dst_rect.top_left.x += (screen_location.size.width - real_img_rect.size.width);
                        dst_rect.top_left.y += (screen_location.size.height/2 - real_img_rect.size.height/2);
                        break;
                case ei_anc_north:
                        dst_rect.top_left.x += (screen_location.size.width/2 - real_img_rect.size.width/2);
                        break;
                case ei_anc_northeast:
                        dst_rect.top_left.x += (screen_location.size.width - real_img_rect.size.width);
                        break;
                case ei_anc_south:
                        dst_rect.top_left.x += (screen_location.size.width/2 - real_img_rect.size.width/2);
                        dst_rect.top_left.y += (screen_location.size.height - real_img_rect.size.height);
                        break;
                case ei_anc_southeast:
                        dst_rect.top_left.x += (screen_location.size.width - real_img_rect.size.width);
                        dst_rect.top_left.y += (screen_location.size.height - real_img_rect.size.height);
                        break;
                case ei_anc_southwest:
                        dst_rect.top_left.y += (screen_location.size.height - real_img_rect.size.height);
                        break;
                case ei_anc_west:
                        dst_rect.top_left.y += (screen_location.size.height/2 - real_img_rect.size.height/2);
                        break;
                default:
                        break;
        }

        ei_rect_t super_clipper;
        if (clipper == NULL){
                super_clipper = hw_surface_get_rect(surface);
        }else{
                super_clipper = *clipper;
        }

        if (dst_rect.top_left.x < super_clipper.top_left.x) {
                src_rect.size.width-= super_clipper.top_left.x - dst_rect.top_left.x;
                src_rect.top_left.x+= super_clipper.top_left.x - dst_rect.top_left.x;
                dst_rect.size.width-= super_clipper.top_left.x - dst_rect.top_left.x;
                dst_rect.top_left.x = super_clipper.top_left.x;
        }
        int offset_x = dst_rect.top_left.x + dst_rect.size.width - super_clipper.top_left.x - super_clipper.size.width;
        if (offset_x > 0) {
                src_rect.size.width -= offset_x;
                dst_rect.size.width -= offset_x;
        }

        if (dst_rect.top_left.y < super_clipper.top_left.y) {
                src_rect.top_left.y += super_clipper.top_left.y - dst_rect.top_left.y;
                src_rect.size.height-= super_clipper.top_left.y - dst_rect.top_left.y;
                dst_rect.size.height-= super_clipper.top_left.y - dst_rect.top_left.y;
                dst_rect.top_left.y = super_clipper.top_left.y;
        }
        int offset_y = dst_rect.top_left.y + dst_rect.size.height - super_clipper.top_left.y - super_clipper.size.height;
        if (offset_y > 0) {
                src_rect.size.height -= offset_y;
                dst_rect.size.height -= offset_y;
        }

        // if no image to display
        if ((dst_rect.size.width <= 0) || (dst_rect.size.height <= 0)) {
                return;
        }

        // new temporary surface
        ei_surface_t img_mask = hw_surface_create(ei_app_root_surface(), dst_rect.size, EI_FALSE);
        hw_surface_set_origin(img_mask, dst_rect.top_left);
        ei_color_t black = {0x00, 0x00, 0x00, 0x00};
        ei_color_t white = {0xff, 0xff, 0xff, 0xff};
        ei_fill(img_mask, &black, &dst_rect);
        ei_draw_polygon(img_mask, button_polygon, white, &dst_rect);

        // draw img
        uint32_t *msk_ptr = (uint32_t *) hw_surface_get_buffer(img_mask);
        uint32_t *src_ptr = (uint32_t *) hw_surface_get_buffer(img);
        uint32_t *dst_ptr = (uint32_t *) hw_surface_get_buffer(surface);
        uint8_t alpha = 255;
        if (hw_surface_has_alpha(img)) {
                int tmp;
                int alpha_channel;
                hw_surface_get_channel_indices(img, &tmp, &tmp, &tmp, &alpha_channel);
                alpha = src_ptr[0] >> 8*(3-alpha_channel);
        }
        // offset buffers
        ei_size_t dst_size = hw_surface_get_size(surface);
        ei_size_t src_size = hw_surface_get_size(img);
        ei_size_t msk_size = hw_surface_get_size(img_mask);
        dst_ptr += dst_rect.top_left.y*dst_size.width + dst_rect.top_left.x;
        src_ptr += src_rect.top_left.y*src_size.width + src_rect.top_left.x;
        msk_ptr += dst_rect.top_left.y*msk_size.width + dst_rect.top_left.x;
        for (int i = 0; i < dst_rect.size.height; i++) {
                for (int j = 0; j < dst_rect.size.width; j++) {
                        if (msk_ptr[j] != 0) {
                                uint32_t img_pixel = src_ptr[j];
                                draw_point(&dst_ptr[j], img_pixel, alpha);
                        }
                }
                dst_ptr += dst_size.width;
                src_ptr += src_size.width;
                msk_ptr += msk_size.width;
        }

        // free temporary surface
        hw_surface_free(img_mask);
}

/**
 * \brief draw a button
 * @param widget pointer to the widget/button memory block
 * @param surface where to draw
 * @param pick_surface offscreen picking surface
 * @param clipper parent clipper
 */
void ei_button_drawfunc (struct ei_widget_t*	widget,
                        ei_surface_t		surface,
                        ei_surface_t		pick_surface,
                        ei_rect_t*		clipper)
{
        ei_button_t *button = (ei_button_t *) widget;

        // draw relief
        ei_rect_t button_size_without_relief;
        if (button->relief != ei_relief_none) {
                button_size_without_relief.top_left.x = button->widget.screen_location.top_left.x + button->border_with;
                button_size_without_relief.top_left.y = button->widget.screen_location.top_left.y + button->border_with;
                button_size_without_relief.size.width = button->widget.screen_location.size.width - (2*button->border_with);
                button_size_without_relief.size.height = button->widget.screen_location.size.height - (2*button->border_with);

                if ((button_size_without_relief.size.width> 0) && (button_size_without_relief.size.height > 0)) {
                        ei_color_t color_up;
                        ei_color_t color_bottom;
                        if (button->relief == ei_relief_raised) {
                                color_up = get_relief_color(button->color, ei_relief_light);
                                color_bottom = get_relief_color(button->color, ei_relief_dark);
                        }
                        else {
                                color_up = get_relief_color(button->color, ei_relief_dark);
                                color_bottom = get_relief_color(button->color, ei_relief_light);
                        }
                        ei_rounded_frame_free* rounded_frame_up_free = rounded_frame(button_size_without_relief, button->border_with, button->corner_radius, ei_relief_up);
                        ei_rounded_frame_free* rounded_frame_bottom_free = rounded_frame(button_size_without_relief, button->border_with, button->corner_radius, ei_relief_bottom);
                        ei_linked_point_t *relief_up = rounded_frame_up_free->rounded_frame;
                        ei_linked_point_t *relief_bottom = rounded_frame_bottom_free->rounded_frame;
                        ei_draw_polygon(surface, relief_up, color_up, clipper);
                        ei_draw_polygon(surface, relief_bottom, color_bottom, clipper);

                        // free_polygon(relief_up);
                        for (int i = 0; i<7 ; i++){
                                free((rounded_frame_up_free -> free_pointer)[i]);
                        }
                        free(rounded_frame_up_free -> free_pointer);
                        free(rounded_frame_up_free);

                        //free_polygon(relief_bottom);
                        for (int i = 0; i<7 ; i++){
                                free((rounded_frame_bottom_free -> free_pointer)[i]);
                        }
                        free(rounded_frame_bottom_free -> free_pointer);
                        free(rounded_frame_bottom_free);
                }
        }
        else {
                button_size_without_relief = button->widget.screen_location;
        }

        // create button polygon
        ei_linked_point_t *button_polygon;
        int corner_radius = button->corner_radius;
        ei_rounded_frame_free* rounded_frame_button_polygon_free = rounded_rect (button_size_without_relief, corner_radius);
        button_polygon = rounded_frame_button_polygon_free -> rounded_frame;

        // draw button background
        ei_color_t background_color = button->color;
        ei_draw_polygon(surface, button_polygon, background_color, clipper);

        // draw in picking surface ?
        if (surface == ei_app_root_surface()) {
                ei_surface_t picking_surface = get_picking_surface();
                ei_color_t *picking_color = button->widget.pick_color;
                ei_draw_polygon(picking_surface, button_polygon, *picking_color, clipper);
        }

        // draw image
        if ((button->img != NULL) && (button->img_rect != NULL)) {
                draw_image_button(surface, button_size_without_relief, button_polygon, button->img, button->img_rect, button->img_anchor, clipper);
        }

        // draw text
        if (button->text != NULL) {
                ei_rect_t text_clipper;
                text_clipper.top_left.x = button->widget.screen_location.top_left.x + button->border_with;
                text_clipper.top_left.y = button->widget.screen_location.top_left.y;
                text_clipper.size.width =  button->widget.screen_location.size.width - 3*button->border_with;
                text_clipper.size.height =  button->widget.screen_location.size.height - 3*button->border_with + 6 ;
                if (text_clipper.top_left.x < clipper->top_left.x) {
                        text_clipper.size.width -= clipper->top_left.x - text_clipper.top_left.x;
                        text_clipper.top_left.x = clipper->top_left.x;
                }
                if (text_clipper.top_left.y < clipper->top_left.y) {
                        text_clipper.size.height -= clipper->top_left.y - text_clipper.top_left.y;
                        text_clipper.top_left.y = clipper->top_left.y;
                }
                int offset_x = text_clipper.top_left.x + text_clipper.size.width - clipper->top_left.x - clipper->size.width;
                if (offset_x > 0) {
                        text_clipper.size.width -= offset_x;
                }
                int offset_y = text_clipper.top_left.y + text_clipper.size.height - clipper->top_left.y - clipper->size.height;
                if (offset_y > 0) {
                        text_clipper.size.height -= offset_y;
                }
                widget_draw_text(surface, button->widget.screen_location,  button->text_anchor, button->text, button->text_font, button->text_color, &text_clipper);
        }

        //free_polygon(button_polygon);
        for (int i = 0; i<8 ; i++){
                free((rounded_frame_button_polygon_free -> free_pointer)[i]);
        }
        free((rounded_frame_button_polygon_free -> free_pointer));
        free(rounded_frame_button_polygon_free);

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
 * \brief put default button values inside this one
 * @param widget button
 */
void ei_button_setdefaultsfunc (struct ei_widget_t* widget)
{
        ei_button_t *default_button_ptr = ei_default_button_handler(NULL, 0);

        ei_button_t *button_widget = (ei_button_t *) widget;

        memcpy(&button_widget->color, &default_button_ptr->color, sizeof(struct ei_button_t) - sizeof(struct ei_widget_t));
}

/**
 * \brief function to call when the placer modify the location of the button
 * @param widget button
 * @param rect new location
 */
void ei_button_geomnotifyfunc (struct ei_widget_t*	widget,
                              ei_rect_t		rect)
{
        ei_button_t *button_ptr = (ei_button_t *) widget;

        button_ptr->widget.screen_location = rect;
}




/**
 * \brief press or raise the button
 *        set it active or not
 *
 * @param  *widget          the button widget
 * @param  *event  	    the event
 *
 * @return a boolean that says if we call the function call back or not
 */
ei_bool_t ei_button_handlefunc ( ei_widget_t*	widget,
                                 ei_event_t* event)
{
        ei_button_t *button = (ei_button_t *) widget;

        // something happens only if there is something on the left button of the mouse
        if (event->param.mouse.button == ei_mouse_button_left) {

                // if we press down the button
                if (event->type == ei_ev_mouse_buttondown) {
                        button->relief = ei_relief_sunken;
                        ei_event_set_active_widget(widget);

                        // if we release the button
                } else if (event->type == ei_ev_mouse_buttonup) {
                        button->relief = ei_relief_raised;
                        // if we are on the button and the button is active
                        if (ei_event_get_active_widget() == widget) {
                                if (get_picking_id(event->param.mouse.where) == widget->pick_id){
                                        ei_event_set_active_widget(NULL);
                                        if (button->callback != NULL) {
                                                button->callback(widget, event, button->user_param);}
                                        return EI_FALSE;}
                                ei_event_set_active_widget(NULL);
                        }
                }

                ei_app_invalidate_rect(widget->parent->content_rect);

                ei_placer_run(widget->parent);
        }
        return EI_TRUE;

}

