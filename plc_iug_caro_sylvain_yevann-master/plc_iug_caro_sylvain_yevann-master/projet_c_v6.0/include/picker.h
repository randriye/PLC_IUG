//
// Created by randriamora on 23/05/2021.
//

#ifndef PROJETC_IG_PICKER_H
#define PROJETC_IG_PICKER_H

#include "ei_types.h"
#include "hw_interface.h"
#include "ei_widget.h"

struct picking_widget{
        ei_widget_t *widget;
        struct picking_widget *next;
};


/**
 * @brief get the picking surface
 * @return the picking surface
 */
ei_surface_t get_picking_surface(void);


/**
 * Get the picking ID corresponding to a point
 * @param point the point with a certain picking ID
 * @return Picking ID associated to the point, so to a color of a picking widget
 */
uint32_t get_picking_id(ei_point_t point);


/**
 * @brief Initialize resources necessary to the picking. Initialize an hash table and create a picking surface
 * @return the picking surface
 */
ei_surface_t init_picking();


/**
 * @brief Free all the resources necessary to the picking
 * Free the hash table
 * Free the picking surface
 */
void deinit_picking();


/**
 * @brief search in the hash table of picking a widget with a certain picking id
 * @param picking_id the picking id of the widget we want
 * @return a pointer to the widget
 */
ei_widget_t *get_widget_from_picking_id(uint32_t picking_id);


/**
 * @brief create the picking_widget of a widget
 * generate a picking id and put it into widget
 * insert the widget in the picking hash table
 * @param widget
 */
void set_widget_picking(ei_widget_t *widget);


/**
 * Delete a widget from the picking hash table
 * @param widget widget to delete
 */
void delete_widget_picking(ei_widget_t *widget);


#endif //PROJETC_IG_PICKER_H