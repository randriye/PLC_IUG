//
// Created by sylvain on 10/05/2021.
//

#ifndef PROJETC_IG_CLIPPER_H
#define PROJETC_IG_CLIPPER_H

#include <stdint.h>
#include "ei_types.h"


/**
 * A clipped_point is a point with the parent point (the previous point before clipping)
 */
typedef struct {
        ei_point_t point;
        ei_point_t parent_point;
} ei_clipped_point_t;




/**
 * A linked_clipped_point is a liked_point with the parent point (the previous point before clipping)
 */
typedef struct ei_linked_clipped_point_t {
        ei_point_t point;
        struct ei_linked_clipped_point_t *next;
        ei_point_t parent_point;
} ei_linked_clipped_point_t;




/**
 * \brief   Clip the polygon with a clipper
 *
 * @param   *polygon        the polygon we need to clip
 * @param   *clipper        the clipper
 *
 * @return the new clipped polygon
 */
ei_linked_clipped_point_t *ei_polygone_clipper_SH(const ei_linked_point_t *polygone, const ei_rect_t *clipper);




/**
 * \brief   Clip a segment with a clipper
 *
 * @param   *p1 *p2         two points which determine the segment
 * @param   *clipper        the clipper
 *
 * @return  0 if the 2 points is out of the clipper,
 *          1  if we need to use the 2 points
 */
uint8_t         ei_line_clipper         (ei_clipped_point_t *p1,
                                         ei_clipped_point_t *p2,
                                         const ei_rect_t *clipper);


#endif //PROJETC_IG_CLIPPER_H
