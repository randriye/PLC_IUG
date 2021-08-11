//
// Created by randriamora on 23/05/2021.
//

#include "hash_table.h"
#include "picker.h"
#include "ei_types.h"
#include "ei_widget.h"
#include "stdint.h"
#include "picking_widget.h"
#include "stdio.h"
#include "stdlib.h"

struct dir {
        uint32_t  len; // the length of the hash table
        struct picking_widget **tab; // the table
        uint32_t nbw; // number of widget / picking widget
};


/**
 * create a new dir, initialized to NULL
 * @param length length of the hash table. Default size is 10 when we initialize the directory of picking
 * @return a dir of length length, initialized to NULL
 */
struct dir *dir_create(uint32_t length){
        struct dir *dir = malloc(sizeof(struct dir));
        if (dir == NULL){
                return NULL;
        }
        // create the new table - initializing all the first pointers to NULL
        struct picking_widget **new_tab = malloc(sizeof (struct picking_widget) * length);
        for (uint32_t i = 0; i < length; i++){
                new_tab[i] = NULL;
        }
        dir -> len = length;
        dir -> nbw = 0;
        dir -> tab = new_tab;
        return dir;
}


/**
 * @brief print the hash table. useful to debug
 * @param dir to print
 */
void dir_print(struct dir *dir){
        printf(" Table de hashage \n");
        for (uint32_t i = 0; i < dir -> len; i++){
                printf("dir[%u] :",i);
                if ((dir -> tab)[i] != NULL){
                        print_pw((dir->tab)[i]);
                }
                else{
                        printf(" \n");
                }

        }
        printf("end of hash table \n");
}


/**
 * @brief resize up or down the dir
 * @param dir the dir to resize.
 * @param uod up or down. If uod = up, the new len is the old one * 2, if uod = down, the new lew is old one / 2
 * @return the new_tab.
 */
struct picking_widget **resize(struct dir *dir, ei_resize_t uod) {
        uint32_t new_len;
        // size up or size_down
        if (uod == up) {
                // size up
                new_len = (dir -> len) * 2;
        } else {
                // size down
                new_len = (dir -> len) / 2;
        }
        // initializing the new tab - all the first pointers to NULL
        struct picking_widget **new_tab = malloc(sizeof(struct picking_widget) * new_len);
        for (uint32_t j = 0; j < new_len; j++){
                new_tab[j] = NULL;
        }

        // filling the new_hash table by inserting all the picking_widget of the old tab to the new one
        for (uint32_t i = 0; i < dir->len; i++) {
                struct picking_widget *current = (dir -> tab)[i];
                //printf("AHA \n");
                //print_pw((dir->tab)[i]);
                while (current != NULL) {
                        uint32_t new_hash = (current -> widget -> pick_id) % new_len;
                        struct picking_widget *tmp = current -> next;
                        insert_head_pw(&new_tab[new_hash], current);
                        current = tmp;
                }
        }
        return new_tab;
}


/**
 * @brief check if he have to resize or not.
 * Resize up if and only of the number of picking widget is superior to 75% of the length of the table
 * Resize down if and only if the number of picking widget is inferior to 15% of the length of the table and the new
 * length would be superior  to 10, minimal length of table
 * @param dir
 */
void test_resize(struct dir *dir){
        uint32_t nb_pw = dir -> nbw;
        uint32_t length = dir -> len;
        // check if we have to size up
        if ( (float) nb_pw >= 0.75 * (float) length){
                struct picking_widget **new_tab = resize(dir, up);
                free(dir -> tab);
                dir -> tab = new_tab;
                dir -> len *= 2;
        }

        // check if we have to size_down
        else if(((float) nb_pw <= 0.15 * (float) length) && ((length / 2) >= 10)){
                struct picking_widget **new_tab = resize(dir, down);
                free(dir -> tab);
                dir -> tab = new_tab;
                dir -> len /= 2;
        }
}
/**
 * @brief insert a widget by creating his own picking widget
 * @param dir the dir in which we insert
 * @param widget the widget to insert. pointer
 */
void dir_insert(struct dir *dir, ei_widget_t *widget){
        // compute the hash_value
        uint32_t hash_value = (widget -> pick_id) % (dir -> len);
        // create the new picking_widget
        struct picking_widget *new_pw = malloc(sizeof(struct picking_widget));
        new_pw ->widget = widget;
        new_pw -> next = NULL;
        // insert it
        insert_head_pw(&(dir -> tab)[hash_value], new_pw);
        // incrementing the number of picking widgets / widgets
        dir -> nbw ++;
        // we test if we have to resize
        test_resize(dir);
}


/**
 * @brief check if a widget has a certain picking id
 * @param dir the dir to check in
 * @param pid picking id
 * @return pointer to widget if it's in, NULL otherwise
 */
ei_widget_t *dir_lookup_tab(struct dir *dir, uint32_t pid){
        uint32_t hash_value = pid % (dir -> len);
        ei_bool_t is_empty = ((dir -> tab)[hash_value] == NULL);

        if (is_empty == EI_TRUE){
                // no picking_widget assigned to this hash value
                return NULL;
        }else{
                // check if there is a widget with this pick_id
                struct picking_widget *exist = look_up_pw((dir -> tab)[hash_value], pid);
                if (exist == NULL){
                        // widget not found
                        return NULL;
                }else{
                        // widget found
                        return exist -> widget;
                }

        }
}


/**
 * @brief delete a widget form an hash table
 * @param dir the hash table
 * @param widget the widget which has to be deleted. if not in the table, return NULL
 */
void dir_delete(struct dir *dir, ei_widget_t *widget) {
        uint32_t hash_value = (widget -> pick_id) % dir -> len;
        // check if is it existing or not
        ei_widget_t *exist = dir_lookup_tab(dir, widget -> pick_id);
        if (exist != NULL) {
                // delete the widget
                del_pw(&(dir -> tab)[hash_value], widget -> pick_id);

                // reduce by one the number of picking widget
                dir -> nbw--;

                // check if we have to resize or not
                test_resize(dir);
        }
}
/**
 * @brief free entirely an hash table - call to free of picking widget lists
 * @param dir hash table which has to be free
 */
void dir_free(struct dir *dir){
        uint32_t  n = dir -> len;
        for ( uint32_t i = 0; i < n; i++){
                if ((dir -> tab)[i] != NULL){
                        free_pw(&(dir -> tab)[i]);
                }
        }
        free(dir -> tab);
        free(dir);
}
