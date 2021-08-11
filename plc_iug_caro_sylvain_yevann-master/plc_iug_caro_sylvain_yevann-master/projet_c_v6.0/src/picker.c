//
// Created by randriamora on 23/05/2021.
//

#include "picker.h"
#include "ei_types.h"
#include "hw_interface.h"
#include "hash_table.h"
#include "picking_widget.h"
#include "hash_table.h"
#include "ei_application.h"



static struct dir *widget_dir = NULL;
static ei_surface_t picking_surface = NULL;
/**
 * @brief Create a picking surface
 * @return
 */
static ei_surface_t create_picking_surface(void)
{
        if (picking_surface == NULL) {
                ei_size_t picking_surface_size = hw_surface_get_size(ei_app_root_surface());
                picking_surface = hw_surface_create(ei_app_root_surface(), picking_surface_size, EI_FALSE);
        }
        return picking_surface;
}
/**
 * Delete the picking surface
 */
static void delete_picking_surface(void)
{
        hw_surface_free(picking_surface);
        picking_surface = NULL;
}
/**
 * @brief create the picking hash table
 * the initial length is 10
 */
static void init_hash_table(void)
{
        uint32_t length = 10;
        widget_dir = dir_create(length);

}
/**
 * @brief free the picking hash table
 */
static void deinit_hash_table(void)
{
        dir_free(widget_dir);
}
/**
 * @brief get the picking surface
 * @return the picking surface
 */
ei_surface_t get_picking_surface(void)
{
        return picking_surface;
}


/**
 * Get the picking ID corresponding to a point
 * @param point the point with a certain picking ID
 * @return Picking ID associated to the point, so to a color of a picking widget
 */
uint32_t get_picking_id(ei_point_t point)
{
        uint32_t *picking_buffer = (uint32_t *) hw_surface_get_buffer(picking_surface);

        ei_size_t picking_size = hw_surface_get_size(picking_surface);

        uint32_t raw_value = picking_buffer[picking_size.width*point.y + point.x];

        int red_channel;
        int green_channel;
        int blue_channel;
        int alpha_channel;
        hw_surface_get_channel_indices(picking_surface, &red_channel, &green_channel, &blue_channel, &alpha_channel);

        uint8_t red_shift = 8*red_channel;
        uint8_t green_shift = 8*green_channel;
        uint8_t blue_shift = 8*blue_channel;

        ei_color_t picking_color;
        picking_color.alpha = 0;
        picking_color.red = raw_value >> red_shift;
        picking_color.green = raw_value >> green_shift;
        picking_color.blue = raw_value >> blue_shift;

        uint32_t picking_id;
        picking_id = 0;
        picking_id |= (uint32_t) picking_color.red;
        picking_id |= (uint32_t) ((uint32_t) picking_color.green) << 8;
        picking_id |= (uint32_t) ((uint32_t) picking_color.blue) << 16;

        //uint32_t picking_id = ei_map_rgba(picking_surface, picking_color);

        return picking_id;

}
/**
 * @brief generate a picking id for a widget in the picking surface. The first picking id is 0. Incremented by one
 * each time
 * @return a picking id for a widget
 */
static uint32_t generate_picking_id(void)
{
        static uint32_t picking_id_counter = 0;

        return picking_id_counter++;

//        static uint32_t picking_id_counter = 0;
//
//        return picking_id_counter++;
}
/**
 * @ generate a picking color
 * @return
 */
static ei_color_t generate_picking_color(void)
{
        static uint32_t picking_id_counter = 0;

        picking_id_counter += 1;

        uint8_t red   = (uint8_t) (picking_id_counter >> 16);
        uint8_t green = (uint8_t) (picking_id_counter >> 8);
        uint8_t blue  = (uint8_t) picking_id_counter;

        ei_color_t picking_color = {red, green, blue, 0};

        return picking_color;

//        static uint32_t picking_id_counter = 0;
//
//        return picking_id_counter++;
}
/**
 * @brief Initialize resources necessary to the picking. Initialize an hash table and create a picking surface
 * @return the picking surface
 */
ei_surface_t init_picking()
{
        init_hash_table();

        return create_picking_surface();
}
/**
 * @brief Free all the resources necessary to the picking
 * Free the hash table
 * Free the picking surface
 */
void deinit_picking()
{
        deinit_hash_table();

        delete_picking_surface();
}
/**
 * @brief search in the hash table of picking a widget with a certain picking id
 * @param picking_id the picking id of the widget we want
 * @return a pointer to the widget
 */
ei_widget_t *get_widget_from_picking_id(uint32_t picking_id)
{
        ei_widget_t *widget = dir_lookup_tab(widget_dir, picking_id);

        return widget;
}
/**
 * @brief create the picking_widget of a widget
 * generate a picking id and put it into widget
 * insert the widget in the picking hash table
 * @param widget
 */
void set_widget_picking(ei_widget_t *widget)
{
        //uint32_t picking_id = ei_map_rgba(picking_surface, picking_color);

        uint32_t picking_id = generate_picking_id();

        widget->pick_id = picking_id;
        widget->pick_color = (ei_color_t *) &widget->pick_id;

        dir_insert(widget_dir, widget);
}
/**
 * Delete a widget from the picking hash table
 * @param widget widget to delete
 */
void delete_widget_picking(ei_widget_t *widget)
{
        dir_delete(widget_dir, widget);
}
