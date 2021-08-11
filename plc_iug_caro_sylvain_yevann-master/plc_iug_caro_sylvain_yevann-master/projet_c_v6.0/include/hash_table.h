//
// Created by randriamora on 23/05/2021.
//

#ifndef PROJETC_IG_HASH_TABLE_H
#define PROJETC_IG_HASH_TABLE_H

#include "stdint.h"
#include "ei_widget.h"
#include "ei_types.h"

struct dir;

typedef enum{
        up = 0,
        down
}ei_resize_t;

/**
 * create a new dir, initialized to NULL
 * @param length length of the hash table. Default size is 10 when we initialize the directory of picking
 * @return a dir of length length, initialized to NULL
 */
struct dir *dir_create(uint32_t length);


/**
 * @brief print the hash table. useful to debug
 * @param dir to print
 */
void dir_print(struct dir *dir);


/**
 * @brief resize up or down the dir
 * @param dir the dir to resize.
 * @param uod up or down. If uod = up, the new len is the old one * 2, if uod = down, the new lew is old one / 2
 * @return the new_tab.
 */
struct picking_widget **resize(struct dir *dir, ei_resize_t uod);


/**
 * @brief check if he have to resize or not.
 * Resize up if and only of the number of picking widget is superior to 75% of the length of the table
 * Resize down if and only if the number of picking widget is inferior to 15% of the length of the table and the new
 * length would be superior  to 10, minimal length of table
 * @param dir
 */
void test_resize(struct dir *dir);


/**
 * @brief insert a widget by creating his own picking widget
 * @param dir the dir in which we insert
 * @param widget the widget to insert. pointer
 */
void dir_insert(struct dir *dir, ei_widget_t *widget);


/**
 * @brief check if a widget has a certain picking id
 * @param dir the dir to check in
 * @param pid picking id
 * @return pointer to widget if it's in, NULL otherwise
 */
ei_widget_t *dir_lookup_tab(struct dir *dir, uint32_t pid);


/**
 * @brief delete a widget form an hash table
 * @param dir the hash table
 * @param widget the widget which has to be deleted. if not in the table, return NULL
 */
void dir_delete(struct dir *dir, ei_widget_t *widget);


/**
 * @brief free entirely an hash table - call to free of picking widget lists
 * @param dir hash table which has to be free
 */
void dir_free(struct dir *dir);



#endif //PROJETC_IG_HASH_TABLE_H
