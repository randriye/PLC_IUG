//
// Created by caroline on 23/05/2021.
//

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "ei_types.h"
#include "relief.h"

#define RELIEF_COLOR_OFFSET 50



/**
 * \brief   Takes a color and return another colors : a lighter one or a darker one for the relief
 * @param   color                     the color taken
 * @param   color change              tells us if the wi return a lighter or a darker color
 */

ei_color_t get_relief_color(ei_color_t color, ei_relief_color_change_t color_change)
{
        ei_color_t new_color;

        // the lighter color
        if (color_change == ei_relief_light) {

        // The green color
                // if we exceed 255, we set the value at 255
                if ((uint16_t) color.green + RELIEF_COLOR_OFFSET > 255) {
                        new_color.green = 255;
                } else {
                        new_color.green = color.green + RELIEF_COLOR_OFFSET;}

        // The red color
                // if we exceed 255, we set the value at 255
                if ((uint16_t) color.red + RELIEF_COLOR_OFFSET > 255) {
                        new_color.red = 255;
                } else {
                        new_color.red = color.red + RELIEF_COLOR_OFFSET;}

        // The blue color
                // if we exceed 255, we set the value at 255
                if ((uint16_t) color.blue + RELIEF_COLOR_OFFSET > 255) {
                        new_color.blue = 255;
                } else {
                        new_color.blue = color.blue + RELIEF_COLOR_OFFSET;}
        }

        // the darker color
        else {

        // The green color
                // if we exceed 255, we set the value at 255
                if ((uint16_t) color.green - RELIEF_COLOR_OFFSET < 0) {
                        new_color.green = 0;
                } else {
                        new_color.green = color.green - RELIEF_COLOR_OFFSET; }

        // The red color
                // if we exceed 255, we set the value at 255
                if ((uint16_t) color.red - RELIEF_COLOR_OFFSET < 0) {
                        new_color.red = 0;
                } else {
                        new_color.red = color.red - RELIEF_COLOR_OFFSET;}

        // The blue color
                // if we exceed 255, we set the value at 255
                if ((uint16_t) color.blue - RELIEF_COLOR_OFFSET < 0) {
                        new_color.blue = 0;
                } else {
                        new_color.blue = color.blue - RELIEF_COLOR_OFFSET;}
        }

        // we keep the same alpha
        new_color.alpha = color.alpha;

        return new_color;
}



/**
 * \brief   Takes a ei_rect_t and return the list of points of  the relief behind (border).
 *
 * @param   widget_button_size        the size of the button (rectangle)
 * @param   border_width              the width of the border
 * @param   part                      do we draw the upper part or the lower part?
 */
ei_linked_point_t *rect_frame(ei_rect_t widget_size_without_relief, int border_width, ei_relief_part_t part)
{
        // initialization of the list of point of our relief
        ei_linked_point_t *relief_frame;

        // first point
        ei_linked_point_t *switching_point = malloc(sizeof(ei_linked_point_t));

        relief_frame = switching_point;

// the corner point
        // the upper part of the relief
        if (part == ei_relief_up) {
                switching_point->point.x = widget_size_without_relief.top_left.x - border_width;
                switching_point->point.y = widget_size_without_relief.top_left.y - border_width;
        }
        // the bottom part of the relief
        else {
                switching_point->point.x = widget_size_without_relief.top_left.x + widget_size_without_relief.size.width + border_width;
                switching_point->point.y = widget_size_without_relief.top_left.y + widget_size_without_relief.size.height + border_width;
        }

        // creation of our point of the relief polygon
        ei_linked_point_t *diag_inf = malloc(sizeof(ei_linked_point_t));
        ei_linked_point_t *middle_west = malloc(sizeof(ei_linked_point_t));
        ei_linked_point_t *middle_east = malloc(sizeof(ei_linked_point_t));
        ei_linked_point_t *diag_sup = malloc(sizeof(ei_linked_point_t));

        // the lower part of the diagonal
        diag_inf->next = middle_west;
        diag_inf->point.x = widget_size_without_relief.top_left.x - border_width;
        diag_inf->point.y = widget_size_without_relief.top_left.y + widget_size_without_relief.size.height + border_width;

        // the west point of the middle
        middle_west->next = middle_east;
        middle_west->point.x = widget_size_without_relief.top_left.x + border_width;
        middle_west->point.y = widget_size_without_relief.top_left.y + widget_size_without_relief.size.height - border_width;

        // the est point of the middle segment
        middle_east->next = diag_sup;
        middle_east->point.x = widget_size_without_relief.top_left.x + widget_size_without_relief.size.width - border_width;
        middle_east->point.y = widget_size_without_relief.top_left.y + border_width;

        // the lower part of the diagonal segment
        diag_sup->point.x = widget_size_without_relief.top_left.x + widget_size_without_relief.size.width + border_width;
        diag_sup->point.y = widget_size_without_relief.top_left.y - border_width;

        switching_point->next = diag_inf;

        // ending with the yevann_point
        ei_linked_point_t *yevann_point = malloc(sizeof(ei_linked_point_t));
        yevann_point->point = switching_point->point;
        yevann_point->next = NULL;
        diag_sup->next = yevann_point;

        return relief_frame;
}



/**
 * \brief   Application of Bresenham for an arc
 * @param   center          the center of the arc
 * @param   point           the browsing point
 * @param   *arc            the list of the previous points
 * @param   increment_x     the direction of the x browsing
 * @param   incremnet_y     the direction of the y browsing
 * @param   xy_inverted     indicate if we browse ont the x or y axis
 */

static inline void			bresenham_arc                           (ei_point_t center,
                                                                                ei_point_t* point,
                                                                                ei_linked_point_t*arc,
                                                                                uint32_t * count,
                                                                                float increment_x,
                                                                                float increment_y,
                                                                                int* m,
                                                                                int xy_inverted){

        // sign_xy is the sign of (increment_x * increment_y)
        int sign_xy = increment_x <= 0 ? -1 : 1;
        sign_xy = increment_y <= 0 ? -sign_xy : sign_xy;

        // application of Bresenham

        // browse on the y-axis
        if (xy_inverted) {
                if (*m * sign_xy < 0) {
                        *m += (int) floorf( (16 * increment_x * ((float) point->x) + 8)) ;
                        (point->x) += (int) floorf((2 * increment_x));
                }
                *m += (int) floorf((8 * increment_y * ((float) point->y) + 13));
                (point->y) += (int) floorf(increment_y);

        // browse on the x-axis
        } else {
                if (*m * sign_xy > 0) {
                        *m += (int) floorf((16 * increment_y * ((float) point->y) + 8));
                        (point->y) += (int) floorf(2 * increment_y);
                }
                *m += (int) floorf(8 * increment_x * ((float) point->x) + 13);
                (point->x) += (int) floorf(increment_x);
        }

        // update the list with the new point
        arc[*count].point.x = (point->x) + center.x; arc[*count].point.y = (point->y) + center.y;
        arc[*count - 1].next = &(arc[*count]);
        *count+=1;


}

/**
 * \brief   find in which dial a point is and how we should browse the x and y axis for the
 *          Bresenham algorithm for an arc
 *          There are 8 dials, the first one is between O and pi/4
 *          Warning : it is the trigonometric direction for the screen, so it is clockwise
 *          for the user.
 *
 * @param   point               the point we need to find dial for
 * @param   *dial               the argument we will change and return
 * @param   *increment_x        how we should browse the x axis
 * @param   *increment_y        how we should browse the y axis
 */

static void                             find_dial (ei_point_t point,
                                                   uint8_t* dial,
                                                   float* increment_x,
                                                   float* increment_y){
        if (point.x <= 0) {
                *increment_y = (float) -0.5; *increment_x = point.y >= 0 ? (float) -0.5 : (float) 0.5;
                if (*increment_x == -0.5){
                        *dial = point.y > -point.x ? 3 : 4;
                }else{
                        *dial = point.y > point.x ? 5 : 6;
                }
        } else {
                *increment_y = (float) 0.5; *increment_x = point.y >= 0 ? (float) -0.5 : (float) 0.5;
                if (*increment_x == 0.5){
                        *dial = point.x > - point.y ? 8 : 7;
                }else{
                        *dial = point.x >  point.y ? 1 : 2;
                }
        }
}



/**
 * \brief   Draw an arc with a start and an end.
 *          The arc is returned with the smallest angle at the beginning and trigonometric
 *          direction.
 *          Warning : it is the trigonometric direction for the screen, so it is clockwise
 *          for the user.
 *
 * @param   center        the center point of the arc
 * @param   radius        the radius of the arc
 * @param   start_angle   the start angle of the arc
 * @param   end_angle     the end angle of the arc
 */
ei_linked_point_t*               draw_arc                       (ei_point_t center,
                                                                 uint32_t radius,
                                                                 float start_angle,
                                                                 float end_angle,
                                                                 ei_linked_point_t** last) {
// if radius = 0
        if (radius == 0){
                ei_linked_point_t * point = malloc (2*sizeof (ei_linked_point_t));
                point[0].point.x = center.x ; point[0].point.y = center.y;
                point[0].next = NULL;
                *last = &(point[0]);
                return point;
        }

        ei_point_t point, end_point;

// if we want a circle
        if (fabs((double) end_angle-start_angle)> 2*M_PI ||  end_angle >=  2*M_PI )
                end_angle = (float) 6.283;


// set the start and the end point with the angle
        if (start_angle > end_angle) {
                point.x = (int) floorf(((float) radius) * cosf(end_angle)); point.y = (int) floorf(((float)radius) * sinf(end_angle));
                end_point.x = (int) floorf(((float)radius) * cosf(start_angle)); end_point.y = (int) floorf(((float)radius) * sinf(start_angle));
        } else {
                point.x = (int) floorf(((float)radius) * cosf(start_angle)); point.y = (int) floorf(((float)radius) * sinf(start_angle));
                end_point.x = (int) floorf(((float)radius) * cosf(end_angle)); end_point.y = (int) floorf(((float)radius) * sinf(end_angle));}

// create our list
        ei_linked_point_t* arc = malloc(sizeof(ei_linked_point_t)*7*radius);
        arc[0].point.x = point.x + center.x;  arc[0].point.y = point.y + center.y;
        uint32_t count = 1;

// find in which dial point and end_point are and define the increment
        uint8_t dial, end_dial;
        float increment_x, increment_y, end_x, end_y;
        find_dial (point, &dial, &increment_x, &increment_y);
        find_dial (end_point, &end_dial, &end_x, &end_y);

// Bresenham
        int xy_inverted, m ;

        // 8 dial
        for (int i = 0; i < 8; i++) {

                // initialization of m
                m = (int) floorf((float) (4*((float) pow(point.x, 2) + (float)pow(point.y, 2) + 2 * increment_x * ((float)point.x) + 2 * increment_y * ((float)point.y) - pow(radius, 2)) + 5));

                // change the value of increments, coordinates and dial
                find_dial (point, &dial, &increment_x, &increment_y);
                if(abs(point.x) > abs(point.y)) {
                        increment_y *= 2;
                        xy_inverted = 1;
                        point.y += (int) floorf(increment_y);
                }else{
                        increment_x *= 2;
                        xy_inverted = 0;}


                // if point and end_point in the same dial
                if (end_dial == dial){
                        while ( abs(point.x - end_point.x) > 2   ||  abs(point.x != end_point.x) > 2 ){
                                bresenham_arc( center, &point, arc, &count, increment_x, increment_y, &m, xy_inverted );}
                        break;


                // the other case
                }else{
                        if(xy_inverted) {
                                while (point.y != 0 && abs(point.x) > abs(point.y)){
                                        bresenham_arc (center, &point, arc, &count, increment_x, increment_y, &m, xy_inverted);}
                        } else {
                                while (point.x != 0 && abs(point.y) > abs(point.x)) {
                                        bresenham_arc(center, &point, arc, &count, increment_x, increment_y, &m,
                                                      xy_inverted);}
                        }

                        // end of the browsing
                        if ((point.x == 0 || abs(point.x) == abs(point.y)) && fabs((double)increment_x) > 0.5){
                                point.x += (int) (increment_x);
                                arc[count].point.x = point.x + center.x; arc[count].point.y = point.y + center.y;
                                arc[count - 1].next = &(arc[count]);
                                count += 1;
                        }

                        if ((point.y == 0 || abs(point.x) == abs(point.y)) && fabs((double)increment_y) > 0.5){
                                point.y += (int)(increment_y);
                                arc[count].point.x = point.x + center.x; arc[count].point.y = point.y + center.y;
                                arc[count - 1].next = &(arc[count]);
                                count += 1;
                        }
                }
        }

        // the end point
        if (end_angle >= 7){
                arc[count].point.x = arc[0].point.x ;
                arc[count].point.y = arc[0].point.y ;
        }else{
                arc[count].point.x = end_point.x + center.x;
                arc[count].point.y = end_point.y + center.y;
        }
        arc[count].next = NULL;

        // ending
        arc[count-1].next = &(arc[count]);
        *last = &(arc[count]);
        
        return arc;
}



/**
 * \brief   Takes a ei_rect_t and return the list of points of the relief behind (border) with
 *          rounded frame
 *
 * @param   widget_button_size        the size of the button (rectangle)
 * @param   border_width              the width of the border
 * @param   corner_radius             the radius or the arc on the border
 * @param   part                      do we draw the upper part or the lower part?
 */

ei_rounded_frame_free*      rounded_frame               (ei_rect_t widget_button_size,
                                                        int border_width,
                                                        int corner_radius,
                                                        ei_relief_part_t part) {
// structure to free
        ei_rounded_frame_free* frame = malloc(sizeof(ei_linked_point_t**) + sizeof(ei_linked_point_t*));
        ei_linked_point_t** free = malloc (7*sizeof(ei_linked_point_t*));

// create our list
        // we need 3 arcs
        ei_linked_point_t *arc_1;
        ei_linked_point_t *arc_2;
        ei_linked_point_t *arc_3;

        // 3 segments
        ei_linked_point_t *middle_segment = malloc(2 * sizeof(ei_linked_point_t));
        ei_linked_point_t *first_segment = malloc(2 * sizeof(ei_linked_point_t));
        ei_linked_point_t *second_segment = malloc(2 * sizeof(ei_linked_point_t));

        ei_point_t center;

        // create our ei_rect_t of our border
        ei_rect_t border;
        border.top_left.x = widget_button_size.top_left.x - border_width ;
        border.top_left.y = widget_button_size.top_left.y - border_width;
        border.size.height = widget_button_size.size.height + 2 * border_width ;
        border.size.width = widget_button_size.size.width + 2 * border_width;

        // calculate our 2 middle point
        ei_point_t middle_1 = {border.top_left.x + border.size.height / 2, border.top_left.y + border.size.height / 2};
        ei_point_t middle_2 = {border.top_left.x + border.size.width - border.size.height / 2,border.top_left.y + border.size.height / 2};

        // need 3 ends of arc
        ei_linked_point_t *end_arc_1;
        ei_linked_point_t *end_arc_2;
        ei_linked_point_t *end_arc_3;

        // the end_point
        ei_linked_point_t *yevann_point = malloc(sizeof(ei_linked_point_t));

// beginning of the building
        // if we need to built the up side
        if (part == ei_relief_up) {

                // the segment in the middle
                middle_segment[0].point.x = middle_2.x;
                middle_segment[0].point.y = middle_2.y;
                middle_segment[1].point.x = middle_1.x;
                middle_segment[1].point.y = middle_1.y;
                middle_segment[0].next = &(middle_segment[1]);

                // the bottom left arc
                center.x = border.top_left.x + corner_radius;
                center.y = border.top_left.y + border.size.height - corner_radius;
                arc_1 = draw_arc(center, corner_radius, 3 * (float) M_PI / 4, (float) M_PI, &end_arc_1);
                middle_segment[1].next = arc_1;

                // the left segment
                first_segment[0].point.x = border.top_left.x;
                first_segment[0].point.y = border.top_left.y + border.size.height - corner_radius;
                first_segment[1].point.x = border.top_left.x;
                first_segment[1].point.y = border.top_left.y + corner_radius;
                first_segment[0].next = &(first_segment[1]);
                end_arc_1->next = first_segment;

                // the top left arc
                center.x = border.top_left.x + corner_radius;
                center.y = border.top_left.y + corner_radius;
                arc_2 = draw_arc(center, corner_radius, (float) M_PI, 3 * (float) M_PI / 2, &end_arc_2);
                first_segment[1].next = arc_2;

                // the top segment
                second_segment[0].point.x = border.top_left.x + corner_radius;
                second_segment[0].point.y = border.top_left.y;
                second_segment[1].point.x =  border.top_left.x + border.size.width - corner_radius;
                second_segment[1].point.y = border.top_left.y;
                second_segment[0].next = &(second_segment[1]);
                end_arc_2->next = second_segment;

                // the top right arc
                center.x = border.top_left.x + border.size.width - corner_radius;
                center.y = border.top_left.y + corner_radius;
                arc_3 = draw_arc(center, corner_radius, (float)-M_PI / 4, (float)-M_PI / 2, &end_arc_3);
                second_segment[1].next = arc_3;

                //end of the polygon
                (yevann_point -> point).x = middle_2.x;
                (yevann_point -> point).y = middle_2.y;
                yevann_point -> next = NULL;
                end_arc_3 -> next = yevann_point;

                // if we need to built the bottom side
        } else {
                // the segment in the middle
                middle_segment[0].point.x = middle_1.x;
                middle_segment[0].point.y = middle_1.y;
                middle_segment[1].point.x = middle_2.x;
                middle_segment[1].point.y = middle_2.y;
                middle_segment[0].next = &(middle_segment[1]);

                // the top right arc
                center.x = border.top_left.x + border.size.width - corner_radius;
                center.y = border.top_left.y + corner_radius;
                arc_1 = draw_arc(center, corner_radius, 7 * (float) M_PI / 4, (float) 7, &end_arc_1);

                middle_segment[1].next = arc_1;

                // the right segment
                first_segment[0].point.x = center.x + corner_radius;
                first_segment[0].point.y = center.y;
                first_segment[1].point.x = center.x + corner_radius;
                first_segment[1].point.y = border.top_left.y + border.size.height - corner_radius;
                first_segment[0].next = &(first_segment[1]);
                end_arc_1->next = first_segment;

                // the bottom right arc
                center.x = border.top_left.x + border.size.width - corner_radius;
                center.y = border.top_left.y + border.size.height - corner_radius;
                arc_2 = draw_arc(center, corner_radius, (float) 0, (float) M_PI/2, &end_arc_2);
                end_arc_2->next = second_segment;

                first_segment[1].next = arc_2;

                // the bottom segment
                second_segment[0].point.x = center.x;
                second_segment[0].point.y = center.y + corner_radius;
                second_segment[1].point.x = border.top_left.x + border.size.width - corner_radius;
                second_segment[1].point.y = center.y + corner_radius;
                second_segment[0].next = &(second_segment[1]);

                // the bottom left arc
                center.x = border.top_left.x + corner_radius;
                center.y = border.top_left.y + border.size.height - corner_radius;
                arc_3 = draw_arc(center, corner_radius, (float) M_PI / 2, 3*(float) M_PI/4, &end_arc_3);

                second_segment[1].next = arc_3;

                //end of the polygon
                (yevann_point -> point).x = middle_1.x;
                (yevann_point -> point).y = middle_1.y;
                yevann_point -> next = NULL;
                end_arc_3 -> next = yevann_point;
        }

        // our rounded_frame_free
        frame -> rounded_frame = middle_segment;
        free[0] = middle_segment;
        free[1] = arc_1;
        free[2] = first_segment;
        free[3] = arc_2;
        free[4] = second_segment;
        free[5] = arc_3;
        free[6] = yevann_point;
        frame -> free_pointer = free;

        return frame;
}



/**
 * \brief   Takes a ei_rect_t and returns the rectangle with the corners rounded
 * @param   button_size_without_relief        the rectangle
 * @param   corner_radius             the radius or the arc on the border
 */

ei_rounded_frame_free*      rounded_rect                      (ei_rect_t rectangle,
                                                           int corner_radius) {

        // structure to free
        ei_rounded_frame_free* rect = malloc(sizeof(ei_linked_point_t**) + sizeof(ei_linked_point_t*));
        ei_linked_point_t** free = malloc (8*sizeof(ei_linked_point_t*));

        // create our list
        // we need 4 arcs
        ei_linked_point_t *arc_bottom_right;
        ei_linked_point_t *arc_bottom_left;
        ei_linked_point_t *arc_up_left;
        ei_linked_point_t *arc_up_right;

        // 4 segments
        ei_linked_point_t *segment_bottom = malloc(2 * sizeof(ei_linked_point_t));
        ei_linked_point_t *segment_left = malloc(2 * sizeof(ei_linked_point_t));
        ei_linked_point_t *segment_up = malloc(2 * sizeof(ei_linked_point_t));
        ei_linked_point_t *segment_right = malloc(2 * sizeof(ei_linked_point_t));

        ei_point_t center;


        // need 4 ends of arc
        ei_linked_point_t *end_arc_1;
        ei_linked_point_t *end_arc_2;
        ei_linked_point_t *end_arc_3;
        ei_linked_point_t *end_arc_4;


        // the bottom right arc
        center.x = rectangle.top_left.x + rectangle.size.width - corner_radius;
        center.y = rectangle.top_left.y + rectangle.size.height - corner_radius;
        arc_bottom_right = draw_arc(center, corner_radius, (float) 0, (float) M_PI / 2, &end_arc_1);

        // the bottom segment
        segment_bottom[0].point.x = center.x;
        segment_bottom[0].point.y = center.y + corner_radius;
        segment_bottom[1].point.x = rectangle.top_left.x + rectangle.size.width - corner_radius;
        segment_bottom[1].point.y = center.y + corner_radius;
        segment_bottom[0].next = &(segment_bottom[1]);
        end_arc_1->next = segment_bottom;

        // the bottom left arc
        center.x = rectangle.top_left.x + corner_radius;
        center.y = rectangle.top_left.y + rectangle.size.height - corner_radius;
        arc_bottom_left = draw_arc(center, corner_radius, (float) M_PI / 2, (float) M_PI, &end_arc_2);
        segment_bottom[1].next = arc_bottom_left;

        // the left segment
        segment_left[0].point.x = rectangle.top_left.x;
        segment_left[0].point.y = rectangle.top_left.y + rectangle.size.height - corner_radius;
        segment_left[1].point.x = rectangle.top_left.x;
        segment_left[1].point.y = rectangle.top_left.y + corner_radius;
        segment_left[0].next = &(segment_left[1]);
        end_arc_2->next = segment_left;

        // the top left arc
        center.x = rectangle.top_left.x + corner_radius;
        center.y = rectangle.top_left.y + corner_radius;
        arc_up_left = draw_arc(center, corner_radius, (float) M_PI, 3 * (float) M_PI / 2, &end_arc_3);
        segment_left[1].next = arc_up_left;

        // the top segment
        segment_up[0].point.x = rectangle.top_left.x + corner_radius;
        segment_up[0].point.y = rectangle.top_left.y;
        segment_up[1].point.x = rectangle.top_left.x + rectangle.size.width - corner_radius;
        segment_up[1].point.y = rectangle.top_left.y;
        segment_up[0].next = &(segment_up[1]);
        end_arc_3->next = segment_up;

        // the top left arc
        center.x = rectangle.top_left.x + rectangle.size.width - corner_radius;
        center.y = rectangle.top_left.y + corner_radius;
        arc_up_right = draw_arc(center, corner_radius, 3*(float)M_PI/2, (float)7, &end_arc_4);
        segment_up[1].next = arc_up_right;

        // the left segment
        segment_right[0].point.x = center.x + corner_radius;
        segment_right[0].point.y = center.y;
        segment_right[1].point.x = center.x + corner_radius;
        segment_right[1].point.y = rectangle.top_left.y + rectangle.size.height - corner_radius;
        segment_right[0].next = &(segment_right[1]);
        end_arc_4->next = segment_right;

        //the end
        segment_right[1].next = NULL;

        // our rounded_frame_free
        rect -> rounded_frame = arc_bottom_right;
        free[0] = arc_bottom_right;
        free[1] = arc_bottom_left;
        free[2] = arc_up_left;
        free[3] = arc_up_right;
        free[4] = segment_left;
        free[5] = segment_up;
        free[6] = segment_right;
        free[7] = segment_bottom;
        rect -> free_pointer = free;

        return rect;
}




