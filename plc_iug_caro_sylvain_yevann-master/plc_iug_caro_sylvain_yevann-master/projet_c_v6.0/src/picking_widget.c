//
// Created by randriamora on 23/05/2021.
//

#include "picker.h"
#include "hash_table.h"
#include "ei_types.h"
#include "ei_widget.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

/**
 * @brief print a list of picking widgets
 * @param pw
 */
void print_pw(struct picking_widget *pw){
        while (pw != NULL){
                printf("pid = %u ||",  pw->widget->pick_id);
                pw = pw -> next;
        }
        printf("\n");
        printf("\n");
}
/**
 * @brief deleting the picking_widget with a certain picking id
 * @param pw
 * @param pid
 */
void del_pw(struct picking_widget **pw, uint32_t pid){
        if (*pw == NULL){
                return;
        }
        // head has to be delete
        if (pid == (*pw) -> widget -> pick_id){
                struct picking_widget *suiv= (*pw) -> next;
                free(*pw);
                (*pw) = suiv;
                return;
        }
        // general case
        struct picking_widget *prev = (*pw);
        struct picking_widget *current = (*pw) -> next;
        // we browse till we find the picking_widget
        while (prev != NULL){
                if (current -> widget -> pick_id == pid){
                        prev->next = current->next;
                        free(current);
                        return;
                }
                prev = current;
                current = current -> next;
        }
}


/**
 * @brief deleting / free the head of a picking widget list
 * @param pw the picking widget list
 */
void del_head_pw(struct picking_widget **pw){
        struct picking_widget *next = (*pw) -> next;
        free(*pw);
        (*pw) = next;
}
/**
 * @brief free a list of picking widgets by deleting the head till NULL then free the NULL
 * @param pw the list of picking widgets
 */
void free_pw(struct picking_widget **pw){
        while(*pw != NULL){
                del_head_pw(pw);
        }
        free(*pw);
}


/**
 * @brief check if a list of picking widgets has or not a widget with a pid
 * @param pw picking_widget lists
 * @param pid the pid which has to be found
 * @return
 */
struct picking_widget *look_up_pw(struct picking_widget *pw, uint32_t pid){
        struct picking_widget *current = pw;
        while (current != NULL && current -> widget -> pick_id != pid){
                current = current -> next;
        }
        if (current -> widget -> pick_id != pid){
                return NULL;
        }else{
                return current;
        }

}


/**
 * @brief insert in head a new picking widget
 * @param pw pointer to the list of picking widgets
 * @param new_pw pointer to the picking_widget which has to be inserted
 */
void insert_head_pw(struct picking_widget **pw, struct picking_widget *new_pw){
        new_pw -> next = *pw;
        *pw = new_pw;

}
