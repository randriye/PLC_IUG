//
// Created by sylvain on 10/05/2021.
//

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "clipper.h"
#include "ei_types.h"
#include <string.h>



/**
 * \brief   Tells if a point is in an area
 *
 * @param   axe             if axe is 0, we work with the x_axis , if axe is 1 we work with the y_axis
 * @param   up_ok           if up_ok is 0, the function tells if the point is above or on the left of
 *                          the axe (1).
 *                          if up_ok is 1, the function tells if the point is bellow or on the right of
 *                          the axe (1).
 * @param   value           the value of the y  (case axe = x_axis),
 *                          the value of the x  (case axe = y_axis).
 * @param   *pt             the pointer of the point we use to compare
 * @returns  1 if the point is in the area, 0  otherwise
 */

static uint8_t is_point_inside(int value, uint8_t up_ok, uint8_t axe, ei_linked_clipped_point_t *pt) {

        //the function tells if the point is bellow or on the right of the axe
        if (up_ok) {
                // y_axe
                if (axe) {
                        if (pt->point.y < value) {
                                return 0;}
                // x_axe
                } else {
                        if (pt->point.x < value) {
                                return 0;}
                }

        // the function tells if the point is above or on the left of the axe .
        } else {
                // y_axe
                if (axe) {
                        if (pt->point.y > value) {
                                return 0;}
                // x_axe
                } else {
                        if (pt->point.x > value) {
                                return 0;}
                }
        }
        // end
        return 1;
}



/**
 * \brief   Defines the new point on border after the clipping
 *
 * @param   axe             if axe is 0, we work with the x_axis , if axe is 1 we work with the y_axis
 * @param   value           the value of the y  (case axe = x_axis),
 *                          the value of the x  (case axe = y_axis).
 * @param   *p1 *p2         the pointers of the points of the segment we will clip
 *                          select the p1 if we clip with eh x_axis, p2 otherwise
 *
 * @returns  the new point on the border
 */
static ei_linked_clipped_point_t compute_intersection(int value, uint8_t axe, ei_linked_clipped_point_t *p1, ei_linked_clipped_point_t *p2) {

        ei_linked_clipped_point_t pi;

        // we work with the x_axis
        if (axe == 0) {
                float dy_dx = ((float) p2->point.y - (float) p1->point.y) / ((float) p2->point.x - (float) p1->point.x);
                pi.point.x = value;
                if ((float) value - (float) p1->point.x < 0) {
                        pi.point.y = p1->point.y + (int) roundf(((float) p1->point.x - (float) value) * dy_dx);
                }
                else {
                        pi.point.y = p1->point.y + (int) roundf(((float) value - (float) p1->point.x) * dy_dx);
                }
        }
        // we work with the y_axis
        else {

                float dx_dy = ((float) p2->point.x - (float) p1->point.x) / ((float) p2->point.y - (float) p1->point.y);
                pi.point.y = value;
                if ((float) value - (float) p1->point.y < 0) {
                        pi.point.x = p1->point.x + (int) roundf(((float) p1->point.y - (float) value) * dx_dy);
                }
                else {
                        pi.point.x = p1->point.x + (int) roundf(((float) value - (float) p1->point.y) * dx_dy);
                }
        }

        return pi;
}



/**
 * \brief   Add a point on the head of a list
 *
 * @param   ** head         head of the list
 * @param   **current       where we will put the new value
 * @param   *to_add         the new value
 */
static void add_point(ei_linked_clipped_point_t **head, ei_linked_clipped_point_t **current, ei_linked_clipped_point_t *to_add) {

        ei_linked_clipped_point_t *new_point = malloc(sizeof(ei_linked_clipped_point_t));
        memcpy(new_point, to_add, sizeof(ei_linked_clipped_point_t));

        if (*head == NULL) {
                *head = new_point;
                *current = new_point;
        }
        else {
                (*current)->next = new_point;
                *current = new_point;
        }
        new_point->next = *head;
}



/**
 * \brief   Clip the polygon with a clipper
 *
 * @param   *polygon        the polygon we need to clip
 * @param   *clipper        the clipper
 *
 * @return the new clipped polygon
 */
ei_linked_clipped_point_t *ei_polygone_clipper_SH(const ei_linked_point_t *polygone, const ei_rect_t *clipper)
{
        // if there is no polygon
        if (polygone == NULL) {
                return NULL;}

        // head of the new list
        ei_linked_clipped_point_t *head = malloc(sizeof(ei_linked_clipped_point_t));
        head->parent_point = polygone->point;
        head->point = polygone->point;
        ei_linked_point_t *current_src = polygone->next;
        ei_linked_clipped_point_t *current_dst = head;

        // create clipped_points cells
        while ((current_src != NULL) && (current_src != polygone)) {
                ei_linked_clipped_point_t *pt = malloc(sizeof(ei_linked_clipped_point_t));
                pt->parent_point = current_src->point;
                pt->point = current_src->point;
                current_dst->next = pt;
                current_dst = pt;
                current_src = current_src->next;
        }
        current_dst->next = head;

        // determination of our case of clipping
        int value = 0;
        uint8_t up_ok = 0;
        for (int i = 0; i < 4; i++) {
                switch (i) {
                        case 0:
                                value = clipper->top_left.x + clipper->size.width;
                                up_ok = 0;
                                break;
                        case 1:
                                value = clipper->top_left.y + clipper->size.height;
                                up_ok = 0;
                                break;
                        case 2:
                                value = clipper->top_left.x;
                                up_ok = 1;
                                break;
                        case 3:
                                value = clipper->top_left.y;
                                up_ok = 1;
                                break;
                        default:
                                break;
                }

                // insert a the new head point
                if ((head != NULL) && (head->next != NULL)) {
                        ei_linked_clipped_point_t *new_head = NULL;
                        ei_linked_clipped_point_t *previous = head;
                        ei_linked_clipped_point_t *current;
                        ei_linked_clipped_point_t *new_current;
                        do {
                                current = previous->next;
                                ei_linked_clipped_point_t to_add;
                                if (is_point_inside(value, up_ok, i%2, previous)) {
                                        if (is_point_inside(value, up_ok, i%2, current)){
                                                add_point(&new_head, &new_current, current);
                                        }
                                        else {
                                                to_add = compute_intersection(value, i%2, current, previous);
                                                add_point(&new_head, &new_current, &to_add);
                                        }
                                }
                                else {
                                        if (is_point_inside(value, up_ok, i%2, current)) {
                                                to_add = compute_intersection(value, i%2, current, previous);
                                                add_point(&new_head, &new_current, &to_add);
                                                if ((to_add.point.x != current->point.x) || (to_add.point.y != current->point.y)) {
                                                        add_point(&new_head, &new_current, current);
                                                }
                                        }
                                }


                                previous = current;
                        } while ((previous != head));

                        // free
                        ei_linked_clipped_point_t *current_cell = head;
                        if (head != NULL) {
                                do {
                                        ei_linked_clipped_point_t *tmp = current_cell->next;
                                        free(current_cell);
                                        current_cell = tmp;
                                } while (current_cell != head);
                        }
                        head = new_head;
                }
        }

        // post process
        ei_linked_clipped_point_t *cur = head;
        while ((head != NULL) && (cur->next != head)) {
                if ((cur->point.x == cur->next->point.x) && (cur->point.y == cur->next->point.y)) {
                        ei_linked_clipped_point_t *tmp = cur->next;
                        cur->next = cur->next->next;
                        free(tmp);
                }
                else {
                        cur = cur->next;
                }
        }

        // the end of the list
        ei_linked_clipped_point_t *yevann_point = malloc(sizeof(ei_linked_point_t));
        yevann_point->point.x = 0;
        yevann_point->point.y = 0;
        if (head != NULL) {
                yevann_point->point = head->point;
                yevann_point->next = head->next;
                head->next = NULL;
        }
        else {
                yevann_point->next = NULL;
        }

        return yevann_point;
}




/**
 * \brief   Clip a segment with a clipper
 *
 * @param   *p1 *p2         two points which determine the segment
 * @param   *clipper        the clipper
 *
 * @return  0 if the 2 points is out of the clipper,
 *          1  if we need to use the 2 points
 */
uint8_t ei_line_clipper(ei_clipped_point_t *p1, ei_clipped_point_t *p2, const ei_rect_t *clipper)
{
        p1->point = p1->parent_point;
        p2->point = p2->parent_point;

        uint8_t clipping_code_1 =  (p1->point.x < clipper->top_left.x) |
                                  ((p1->point.x > (clipper->top_left.x + clipper->size.width)) << 1) |
                                  ((p1->point.y < clipper->top_left.y) << 2) |
                                  ((p1->point.y > (clipper->top_left.y + clipper->size.height)) << 3);

        uint8_t clipping_code_2 =  (p2->point.x < clipper->top_left.x) |
                                  ((p2->point.x > (clipper->top_left.x + clipper->size.width)) << 1) |
                                  ((p2->point.y < clipper->top_left.y) << 2) |
                                  ((p2->point.y > (clipper->top_left.y + clipper->size.height)) << 3);

        // easy case
        if (!(clipping_code_1 | clipping_code_2)) {
                return 1;
        }

        // easy case
        else if (clipping_code_1 & clipping_code_2) {
                return 0;
        }

        // less easy case
        // we need to clip
        else {
                uint8_t i = 0;
                float dy_dx = ((float) p2->point.y - (float) p1->point.y) / ((float) p2->point.x - (float) p1->point.x);
                float dx_dy = ((float) p2->point.x - (float) p1->point.x) / ((float) p2->point.y - (float) p1->point.y);

                while (clipping_code_1) {
                        if (clipping_code_1 & 0x01) {
                               switch (i) {
                                       case 0:
                                               p1->point.x = clipper->top_left.x;
                                               p1->point.y = p1->point.y + (int) roundf(((float) clipper->top_left.x - (float) p1->parent_point.x)*dy_dx);
                                               break;
                                       case 1:
                                               p1->point.x = clipper->top_left.x + clipper->size.width;
                                               p1->point.y = p1->point.y - (int) roundf(((float) p1->parent_point.x - (float) clipper->top_left.x + (float) clipper->size.width)*dy_dx);
                                               break;
                                       case 2:
                                               p1->point.x = p1->point.x + (int) roundf(((float) clipper->top_left.y - (float) p1->parent_point.y)*dx_dy);
                                               p1->point.y = clipper->top_left.y;
                                               break;
                                       case 3:
                                               p1->point.x = p1->point.y - (int) roundf(((float) p1->parent_point.y - (float) clipper->top_left.y + (float) clipper->size.height)*dy_dx);
                                               p1->point.y = clipper->top_left.y + clipper->size.height;
                                               break;
                                       default:
                                               break;
                               }
                               clipping_code_1 =  (p1->point.x < clipper->top_left.x) |
                                                   ((p1->point.x > (clipper->top_left.x + clipper->size.width)) << 1) |
                                                   ((p1->point.y < clipper->top_left.y) << 2) |
                                                   ((p1->point.y > (clipper->top_left.y + clipper->size.height)) << 3);
                               i = 0;
                        }
                        else {
                                clipping_code_1 >>= 1;
                                i++;
                        }
                }
                i = 0;
                while (clipping_code_2) {
                        if (clipping_code_2 & 0x01) {
                                switch (i) {
                                        case 0:
                                                p2->point.x = clipper->top_left.x;
                                                p2->point.y = p2->point.y + (int) roundf(((float) clipper->top_left.x - (float) p2->parent_point.x)*dy_dx);
                                                break;
                                        case 1:
                                                p2->point.x = clipper->top_left.x + clipper->size.width;
                                                p2->point.y = p2->point.y - (int) roundf(((float) p2->parent_point.x - (float) clipper->top_left.x - (float) clipper->size.width)*dy_dx);
                                                break;
                                        case 2:
                                                p2->point.y = clipper->top_left.y;
                                                p2->point.x = p2->point.x + (int) roundf(((float) clipper->top_left.y - (float) p2->parent_point.y)*dx_dy);
                                                break;
                                        case 3:
                                                p2->point.y = clipper->top_left.y + clipper->size.height;
                                                p2->point.x = p2->point.x - (int) roundf(((float) p2->parent_point.y - (float) clipper->top_left.y - (float) clipper->size.height)*dx_dy);
                                                break;
                                        default:
                                                break;
                                }
                                clipping_code_2 =  (p2->point.x < clipper->top_left.x) |
                                                   ((p2->point.x > (clipper->top_left.x + clipper->size.width)) << 1) |
                                                   ((p2->point.y < clipper->top_left.y) << 2) |
                                                   ((p2->point.y > (clipper->top_left.y + clipper->size.height)) << 3);
                                i = 0;
                        }
                        else {
                                clipping_code_2 >>= 1;
                                i++;
                        }
                }
                return 1;
        }
}
