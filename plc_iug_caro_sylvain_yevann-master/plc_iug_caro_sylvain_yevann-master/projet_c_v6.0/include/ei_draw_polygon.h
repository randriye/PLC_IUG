//
// Created by randriamora on 18/05/2021.
//

#ifndef PROJETC_IG_EI_DRAW_POLYGON_H
#define PROJETC_IG_EI_DRAW_POLYGON_H

struct side;

struct segment;

// MERGE SORTING FUNCTIONS
/**
 * @brief Recursive Function. Merge Side List
 * @param ls1 First Side List
 * @param ls2 Second Side List
 * @return
 */
struct side * MSL(struct side* ls1, struct side* ls2);


/**
 * @brief Split in Two Halfs a Side of List
 * @param ls Side List
 * @param front The Front list
 * @param back The Back List
 */
void split_ls (struct side * ls, struct side ** front, struct side ** back);


/**
 * @brief Merge Sort a Side List
 * @param ls The Side List to sort
 */
void MS(struct side **ls);

// LINKED POINTS FUNCTIONS
/**
 * @brief Find the y_min of a polygon
 * @param first_point : linked points
 * @return the y_min of a polygon
 */
int           find_y_min             (const ei_linked_point_t*    first_point);


/**
 * @brief Find the y_max of a polygon
 * @param first_point : linked points
 * @return the y_max of a polygon
 */
int            find_y_max              (const ei_linked_point_t*       first_point);




// SEGMENT FUNCTIONS
/**
 * @brief Find the Y max of a side / segment
 * @param seg
 * @return
 */
int find_y_max_side(struct segment* seg);


/**
 * @brief Find the Y min of a side / segment
 * @param seg
 * @return
 */
int find_y_min_side(struct segment* seg);


/**
 * Compute the slope of a line
 * @param x0 : abs of p0
 * @param y0 : ord of p0
 * @param x1 : abs of p1
 * @param y1 : ord of p1
 * @return
 */
float           compute_rs (ei_point_t p1 , ei_point_t p2);


// SIDE FUNCTIONS
/**
 * @brief Get the Length of a polygon
 * @param first_point
 * @return
 */
uint32_t  get_length_lp(const struct ei_linked_point_t* first_point);

/**
 * @brief Check if a Side Table is empty or not
 * @param ST
 * @param len
 * @return
 */
ei_bool_t is_empty( struct side *ST[], int len);


/**
 * @brief Print a side list. Useful to debug
 * @param sl Side List
 */

void display_sl(struct side *sl);


/**
 * @brief Create and Insert a new side to a list of simply linked sides
 * @param sl Side List, where to insert the side
 * @param xy Side Parameter of xymin
 * @param ym Side Parameter of ymax
 * @param reci_slope
 */
void side_insert(struct side **sl, float xy, int ym, float reci_slope);


/**
 * @brief Updating the Active Side Table by adding to xymin the reciprocal slope
 * @param ast Active Side Table
 */
void update_ast(struct side **ast);


/**
 * @brief Deleting the head of a list of simply linked sides
 * @param sl Side List
 */


void del_head_sl(struct side **sl);
/**
 * @brief free a side list entirely
 * @param sl
 */
static void free_side_list(struct side **sl);


/**
 * @brief Move definitely a simply linked list of sides in Side Table in the Active Side Table
 * @param st Side Table
 * @param ast Active Side Table
 */
void move_st_to_ast(struct side **st, struct side **ast);

//int get_length_ls(struct side *sl);

/**
 * @brief Deleting Sides which has a certain y
 * @param ast Active Side Table, a simply linked list of sides
 * @param ym ymax, the condition of deleting
 */
void del_sides_ymax(struct side **ast, int ym);

/**
 * @brief Fill a line y considering the AST, in a surface with a certain color
 * @param ast       Active Side Table (Table de cotes actifs) at line y
 * @param y         Line where we want to draw
 * @param surface   Surface where we want to draw
 * @param color     color we want to draw
 */
void fill_line(struct side **ast,int y, ei_surface_t surface, ei_color_t color);


/**
 * @brief fill a clipped polygon on a surface with a certain color. The used algorithm is the one described
 *        in the subject.
 * @param surface   where to draw the polygon
 * @param first_point   list of simply linked points, represents the polygon to draw.
 * @param color         color to fill the polygon
 */
void fill_polygon(ei_surface_t surface,
                  const ei_linked_point_t *first_point,
                  ei_color_t color);

/**
 * @brief delete the head of a point list
 * @param pl
 */
static void del_head_pl(ei_linked_point_t **pl);
/**
 * @brief Free a list of points representing a polygon
 * @param polygon Simply linked point list
 */
void free_polygon(ei_linked_point_t **polygon);

#endif //PROJETC_IG_EI_DRAW_POLYGON_H
