//
// Created by randriamora on 18/05/2021.
//
#include <stdint.h>
#include <stdlib.h>
#include "ei_types.h"
#include "ei_draw.h"
#include "hw_interface.h"
#include "string.h"
#include "ei_draw_polygon.h"
#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "ei_application.h"
#include "pixel.h"

struct side{

        int ymax; // number of the scanline from which there is no intersection
        float xymin; // abs of the side with the first scanline which intersects the side.
        // updated at each new scanline
        float rs; // reciprocal of the slope. (1 / m). Added to x when y++
        struct side *next;
};

struct segment{
        ei_point_t *p1; // pointer to the first point of the segment
        ei_point_t *p2; // pointer to the second point of the segment
};



// MERGE FUNCTIONS
/**
 * @brief Recursive Function. Merge Side List
 * @param ls1 First Side List
 * @param ls2 Second Side List
 * @return a sorted list
 */
struct side * MSL(struct side* ls1, struct side* ls2) {

        struct side *res = NULL;
        // basic cases: one list is NULL so we just have to return the other
        if (ls1 == NULL){
                return ls2;
        }
        else if (ls2 == NULL){
                return ls1;
        }
        // general case : recursive call to the function while lists contain more than 2 sides
        if (ls1 -> xymin <= ls2 -> xymin){
                res = ls1;
                res -> next = MSL(ls1 -> next, ls2);
        }
        else {
                res = ls2;
                res -> next = MSL(ls1, ls2 -> next);
        }
        return res;
}

/**
 * @brief Split in Two Halfs a Side of List
 * @param ls Side List
 * @param front The Front list
 * @param back The Back List
 */
void split_ls (struct side * ls, struct side ** front, struct side ** back){
        struct side *ptr1;
        struct side *ptr2;
        ptr2 = ls;
        ptr1 = ls -> next;
        while( ptr1 != NULL){
                ptr1 = ptr1 -> next;
                if (ptr1 != NULL){
                        ptr2 = ptr2 -> next;
                        ptr1 = ptr1 -> next;
                }
        }
        *front = ls;
        *back = ptr2 -> next;
        ptr2 -> next = NULL;
}
/**
 * @brief Merge Sort a Side List
 * @param ls The Side List to sort
 */
void MS(struct side **ls){
        struct side *head = (*ls);
        struct side *ptr1;
        struct side *ptr2;
        if ((head == NULL) || ((head -> next) == NULL)){
                return;
        }
        // splitting the list in 2 halfs
        split_ls(head, &ptr1, &ptr2);
        // recursive call to the function to split again and again to the 0-length or 1-length side list
        MS(&ptr1);
        MS(&ptr2);
        // merging all the elementary side lists
        (*ls) = MSL(ptr1, ptr2);
}


// LINKED POINTS FUNCTIONS



/**
 * @brief Find the y_min of a polygon
 * @param first_point : linked points
 * @return the y_min of a polygon
 */
int           find_y_min             (const ei_linked_point_t*    first_point)
{
        struct ei_linked_point_t* current = first_point -> next;
        int  y_min = first_point -> point.y;
        // we browse trough all the list to find the y min
        while (current != NULL){
                if ( current -> point. y < y_min){
                        y_min = current -> point.y;
                }
                current = current -> next;
        }
        return y_min;
}


/**
 * @brief Find the y_max of a polygon
 * @param first_point : linked points
 * @return the y_max of a polygon
 */

int            find_y_max              (const ei_linked_point_t*       first_point) {
        ei_linked_point_t *current = first_point->next;
        int y_max = first_point -> point.y;
        // we browse trough all the list to find the y max
        while (current != NULL) {
                if (current->point . y > y_max) {
                        y_max = current-> point.y;
                }
                current = current->next;
        }
        return y_max;
}





// SEGMENT FUNCTIONS
/**
 * @brief Find the Y max of a side / segment
 * @param seg
 * @return the y max of the side
 */
int find_y_max_side(struct segment* seg){
        if (seg -> p1 -> y  >= seg -> p2 -> y){
                return seg -> p1 -> y;
        }
        else{
                return seg -> p2 -> y;
        }
}
/**
 * @brief Find the Y min of a side / segment
 * @param seg
 * @return the y min of a side
 */
int find_y_min_side(struct segment* seg){
        if (seg -> p1 -> y <= seg -> p2 -> y){
                return seg -> p1 -> y;
        } else{
                return seg -> p2 -> y;
        }
}



/**
 * Compute the slope of a line
 * @param x0 : abs of p0
 * @param y0 : ord of p0
 * @param x1 : abs of p1
 * @param y1 : ord of p1
 * @return reciprocal slope of a line, floatwise
 */
float           compute_rs (ei_point_t p1 , ei_point_t p2){
        // we compute the reciprocal slope (operations in float)
        float res = (float)  ((float) (p1.x - p2.x) / (float) (p1.y - p2.y));
        return res;
}




// SIDE FUNCTIONS
/**
 * @brief Get the Length of a polygon
 * @param first_point
 * @return the length of the polygon + 1
 */
uint32_t  get_length_lp(const struct ei_linked_point_t* first_point){
        if (first_point == NULL){
                return 0;
        }
        struct ei_linked_point_t  *next = first_point -> next;

        uint32_t cpt = 1;
        // we browse till we arrive to NULL and incrementing bby 1 the counter
        while ((next != NULL) && (next != first_point)){
                cpt++;
                next = next -> next;
        }
        return cpt;
}
/**
 * @brief Check if a Side Table is empty or not
 * @param ST
 * @param len
 * @return EI_TRUE if empty, EI_FALSE otherwise
 */

ei_bool_t is_empty( struct side **ST, int len){
        for (int i = 0; i < len; i ++){
                if (ST[i] != NULL){
                        return EI_FALSE;
                }
        }
        return EI_TRUE;
}
/**
 * @brief Print a side list. Useful to debug
 * @param sl Side List
 */

void display_sl(struct side *sl){
        while (sl != NULL){
                //printf(" --> [ymax = %i, xymin = %f, rs = %f]", sl->ymax, sl -> xymin, sl -> rs);
                sl = sl -> next;
        }
        //printf(" END \n");
}

/**
 * @brief Create and Insert a new side to a list of simply linked sides
 * @param sl Side List, where to insert the side
 * @param xy Side Parameter of xymin
 * @param ym Side Parameter of ymax
 * @param reci_slope
 */

void side_insert(struct side **sl, float xy, int ym, float reci_slope){
        struct side *new_head = malloc(sizeof(struct side));
        new_head -> xymin = xy;
        new_head -> ymax = ym;
        new_head -> rs = reci_slope;
        new_head -> next = *sl;
        *sl = new_head;
}

/**
 * @brief Updating the Active Side Table by adding to xymin the reciprocal slope
 * @param ast Active Side Table
 */

void update_ast(struct side **ast){
        struct side *current = *ast;
        while (current != NULL){
                (current) -> xymin = ((current) -> xymin) + ((current) -> rs );
                (current) = (current) -> next;
        }
}

/**
 * @brief Deleting the head of a list of simply linked sides
 * @param sl Side List
 */


void del_head_sl(struct side **sl){
        struct side *tmp = (*sl) -> next;
        free(*sl);
        (*sl) = tmp;
}
/**
 * @brief free a side list entirely
 * @param sl
 */
static void free_side_list(struct side **sl){
        while(*sl != NULL){
                del_head_sl(sl);
        }
        free(*sl);
}

/**
 * @brief Move definitely a simply linked list of sides in Side Table in the Active Side Table
 * @param st Side Table
 * @param ast Active Side Table
 */

void move_st_to_ast(struct side **st, struct side **ast){
        if (*st != NULL) {
                while (*st != NULL) {
                        side_insert(ast, (*st)->xymin, (*st)->ymax, (*st)->rs);
                        del_head_sl(st);

                }
        }
}

/**
 * @brief Deleting Sides which has a certain y
 * @param ast Active Side Table, a simply linked list of sides
 * @param ym ymax, the condition of deleting
 */

void del_sides_ymax(struct side **ast, int ym){

        struct side *prev = (*ast);
        struct side *cur = (*ast) -> next;
        while( cur != NULL){
                if (cur -> ymax == ym){
                        prev -> next = cur -> next;
                        free(cur);
                        cur = prev -> next;
                }
                else {
                        prev = cur;
                        cur = cur -> next;
                }
        }


        if ((*ast) -> ymax == ym){
                del_head_sl(ast);

        }



}

/**
 * @brief Fill a line y considering the AST, in a surface with a certain color
 * @param ast       Active Side Table (Table de cotes actifs) at line y
 * @param y         Line where we want to draw
 * @param surface   Surface where we want to draw
 * @param color     color we want to draw
 */

void fill_line(struct side **ast,int y, ei_surface_t surface, ei_color_t color) {
        ei_size_t surface_size;
        uint8_t *buffer = hw_surface_get_buffer(surface);
        surface_size = hw_surface_get_size(surface);
        struct side *current = *ast;
        while (current != NULL && (current)->next != NULL) {
                // we have to compute the raw color
                uint32_t raw_color = ei_map_rgba(surface, color);
                uint8_t a = color.alpha;
                // do not use alpha if the surface doesn't support it and it isn't the root surface
                if ((hw_surface_has_alpha(surface) == EI_FALSE) && (surface != ei_app_root_surface())) {
                        // draw_point will no try to compute new pixel
                        a = 255;
                }

                // compute the lower and upper bounds
                int lower_bound;
                int upper_bound;
                lower_bound = (int) ceilf(current -> xymin);
                upper_bound = (int) floorf (current -> next -> xymin);

                // fill the line with draw_point
                for (int i = lower_bound; i < upper_bound; i++) {

                        draw_point(((uint32_t *) buffer) + (y) * surface_size.width + i, raw_color, a);
                }
                // we skip a side
                current = current->next->next;
        }
}


/**
 * @brief fill a clipped polygon on a surface with a certain color. The used algorithm is the one described
 *        in the subject.
 * @param surface   where to draw the polygon
 * @param first_point   list of simply linked points, represents the polygon to draw.
 * @param color         color to fill the polygon
 */

void fill_polygon(ei_surface_t surface,
                  const ei_linked_point_t *first_point,
                  ei_color_t color){

        // initialization of ST (Side Table / Table des côtés)
        if (get_length_lp(first_point) != 0) {
                int scanline_min = find_y_min(first_point);
                int scanline_max = find_y_max(first_point);
                int len = scanline_max - scanline_min;
                int cpt = (int) get_length_lp(first_point) - 1;
                struct side **ST = malloc(len * sizeof(struct side *));
                for (int i = 0; i < len; i++) {
                        ST[i] = NULL;
                }
                for (int i = 0; i < len; i++) {
                        struct ei_linked_point_t tmp = {first_point->point, first_point->next};
                        struct ei_linked_point_t *current = &tmp;
                        struct ei_linked_point_t *suiv = current->next;
                        for (int j = 0; j < cpt; j++) {
                                // we avoid horizontal segment
                                if (current->point.y != suiv->point.y) {
                                        struct segment *seg = malloc(sizeof(struct segment));
                                        seg->p1 = &(current->point);
                                        seg->p2 = &(suiv->point);
                                        // we add the side if and only if the ymin is equal to the line of the surface
                                        if (find_y_min_side(seg) == i + scanline_min) {
                                                int ym = find_y_max_side(seg);
                                                float xy;
                                                if (seg->p1->y == ym) {
                                                        xy = (float) seg->p2->x;
                                                } else {
                                                        xy = (float) seg->p1->x;
                                                }
                                                float reci_slope = compute_rs(*(seg->p1), *(seg->p2));
                                                // after computing the fields, we insert the side
                                                side_insert(&ST[i], xy, ym, reci_slope);

                                        }
                                        free(seg);
                                }
                                current = suiv;
                                suiv = suiv->next;
                        }
                }
                struct side *AST = NULL;
                // initialization ok - begin of while loop
                int y = 0; // scanline y, initialized to the first scanline of ST
                // begin of the algorithm
                while ((is_empty(ST, len) == EI_FALSE || AST != NULL) && y < len ) {
                        // move ST[y] to AST
                        move_st_to_ast(&ST[y], &AST);
                        // delete all sides with ymax = y + scanline_min
                        if (AST != NULL) {
                                del_sides_ymax(&AST, y + scanline_min);
                        }
                        // merge sort ast by increasing abscissas
                        MS(&AST);
                        // fill the scanline y considering AST and the filling rules
                        fill_line(&AST, y + scanline_min, surface, color);

                        // incrementing y by 1
                        y++;
                        // updating the xymin to take into account the new scanline
                        update_ast(&AST);
                }
                if (AST != NULL) {
                        free_side_list(&AST);
                }
                free(ST);
        }



        else {
                return;
        }

}
/**
 * @brief delete the head of a point list
 * @param pl
 */
static void del_head_pl(ei_linked_point_t **pl){
        ei_linked_point_t *next = (*pl)->next;
        free(*pl);
        *pl = next;
}



/**
 * @brief Free a list of points representing a polygon
 * @param polygon Simply linked point list
 */
void free_polygon(ei_linked_point_t **polygon)
{
        while (*polygon != NULL){
                del_head_pl(polygon);
        }
        free(*polygon);
}
