//
// Created by sylvain on 18/05/2021.
//

#include "ei_types.h"
#include "ei_widget.h"
#include "ei_application.h"
#include "hw_interface.h"
#include "ei_widgetclass.h"
#include "stdlib.h"
#include "stdio.h"
#include "ei_event.h"
#include "ei_utils.h"
#include "ei_placer.h"
#include "picker.h"

#include "ei_widget_frame.h"
#include "ei_widget_button.h"
#include "ei_widget_toplevel.h"

static ei_widget_t *root_widget = NULL;
static ei_surface_t *root_surface = NULL;
static ei_surface_t *window_surface = NULL;
static ei_linked_rect_t *rect_list = NULL; // list of linked rectangles
static ei_bool_t is_free = EI_FALSE;

struct ei_linked_widgetclass {
        ei_widgetclass_t *widget_class;
        struct ei_linked_widgetclass *next;
};

static struct ei_linked_widgetclass *widgetclass_list = NULL;

/**
 * @brief Insert in head by allocating a linked widget class.
 * @param list the list of linked widget class
 * @param to_add the widget to add
 */
static void insert_widgetclass(struct ei_linked_widgetclass **list, ei_widgetclass_t *to_add){
        struct ei_linked_widgetclass *new_head = malloc(sizeof(struct ei_linked_widgetclass));
        new_head->widget_class = to_add;
        new_head->next = *list;
        (*list) = new_head;
}
/**
 * @brief delete the head of the widget class list
 * @param list list with the head to del
 */
static void del_head_widgetclass(struct ei_linked_widgetclass **list){
        struct ei_linked_widgetclass *next = (*list)->next;
        //free((*list)->widget_class);
        free((*list)->widget_class);
        free(*list);
        *list = next;
}
/**
 * @brief Free the list of widgetclass. called by free_all_widgets
 * @param list
 */
static void free_widgetclass_list(struct ei_linked_widgetclass **list){
        while( *list != NULL){
                del_head_widgetclass(list);
        }
        free(*list);
}
/**
 * @brief Free the list of widget class
 */
static void free_all_widgets(void){
        free_widgetclass_list(&widgetclass_list);
}
/**
 * @brief Insert in head a rectangle in a list of rect
 * @param list the list of linked rectangles
 * @param to_add the rect to add
 */
static void insert_rectangle( ei_linked_rect_t **list, ei_rect_t to_add){
        ei_linked_rect_t *new_head = malloc(sizeof(ei_linked_rect_t));
        new_head ->rect = to_add;
        new_head -> next = *list;
        *list = new_head;
}
/**
 * @brief Delete the head of the linked rect
 * @param list the list with the head to del
 */
static void del_head_rect(ei_linked_rect_t **list){
        ei_linked_rect_t *next = (*list)->next;
        free(*list);
        (*list) = next;
}
/**
 * @brief Free the entire list of linked rect
 */
static void free_rect_list(void){
        while (rect_list != NULL){
                del_head_rect(&rect_list);
        }
        free(rect_list);
}



static void register_all_widgets()
{
        // frame
        ei_widgetclass_t *frame_widgetclass = malloc(sizeof(ei_widgetclass_t));
        frame_widgetclass->allocfunc = &ei_frame_allocfunc;
        frame_widgetclass->releasefunc = &ei_frame_releasefunc;
        frame_widgetclass->drawfunc = &ei_frame_drawfunc;
        frame_widgetclass->setdefaultsfunc = &ei_frame_setdefaultsfunc;
        frame_widgetclass->geomnotifyfunc = &ei_frame_geomnotifyfunc;
        frame_widgetclass->handlefunc = &ei_frame_handlefunc;
        frame_widgetclass->name[0] = 'f';
        frame_widgetclass->name[1] = 'r';
        frame_widgetclass->name[2] = 'a';
        frame_widgetclass->name[3] = 'm';
        frame_widgetclass->name[4] = 'e';
        frame_widgetclass->name[5] = '\0';
        ei_widgetclass_register(frame_widgetclass);
        insert_widgetclass(&widgetclass_list, frame_widgetclass);

        // button
        ei_widgetclass_t *button_widgetclass = malloc(sizeof(ei_widgetclass_t));
        button_widgetclass->allocfunc = &ei_button_allocfunc;
        button_widgetclass->releasefunc = &ei_button_releasefunc;
        button_widgetclass->drawfunc = &ei_button_drawfunc;
        button_widgetclass->setdefaultsfunc = &ei_button_setdefaultsfunc;
        button_widgetclass->geomnotifyfunc = &ei_button_geomnotifyfunc;
        button_widgetclass->handlefunc = &ei_button_handlefunc;
        button_widgetclass->name[0] = 'b';
        button_widgetclass->name[1] = 'u';
        button_widgetclass->name[2] = 't';
        button_widgetclass->name[3] = 't';
        button_widgetclass->name[4] = 'o';
        button_widgetclass->name[5] = 'n';
        button_widgetclass->name[6] = '\0';
        ei_widgetclass_register(button_widgetclass);
        insert_widgetclass(&widgetclass_list, button_widgetclass);

        // toplevel
        ei_widgetclass_t *toplevel_widgetclass = malloc(sizeof(ei_widgetclass_t));
        toplevel_widgetclass->allocfunc = &ei_toplevel_allocfunc;
        toplevel_widgetclass->releasefunc = &ei_toplevel_releasefunc;
        toplevel_widgetclass->drawfunc = &ei_toplevel_drawfunc;
        toplevel_widgetclass->setdefaultsfunc = &ei_toplevel_setdefaultsfunc;
        toplevel_widgetclass->geomnotifyfunc = &ei_toplevel_geomnotifyfunc;
        toplevel_widgetclass->handlefunc = &ei_toplevel_handlefunc;
        toplevel_widgetclass->name[0] = 't';
        toplevel_widgetclass->name[1] = 'o';
        toplevel_widgetclass->name[2] = 'p';
        toplevel_widgetclass->name[3] = 'l';
        toplevel_widgetclass->name[4] = 'e';
        toplevel_widgetclass->name[5] = 'v';
        toplevel_widgetclass->name[6] = 'e';
        toplevel_widgetclass->name[7] = 'l';
        toplevel_widgetclass->name[8] = '\0';
        ei_widgetclass_register(toplevel_widgetclass);
        insert_widgetclass(&widgetclass_list, toplevel_widgetclass);
}


/**
 * \brief	Creates an application.
 *		<ul>
 *			<li> initializes the hardware (calls \ref hw_init), </li>
 *			<li> registers all classes of widget, </li>
 *			<li> creates the root window (either in a system window, or the entire screen), </li>
 *			<li> creates the root widget to access the root window. </li>
 *		</ul>
 *
 * @param	main_window_size	If "fullscreen is false, the size of the root window of the
 *					application.
 *					If "fullscreen" is true, the current monitor resolution is
 *					used as the size of the root window. \ref hw_surface_get_size
 *					can be used with \ref ei_app_root_surface to get the size.
 * @param	fullScreen		If true, the root window is the entire screen. Otherwise, it
 *					is a system window.
 */
void ei_app_create(ei_size_t main_window_size, ei_bool_t fullscreen)
{
        // initializes the hardware
        hw_init();

        // registers all classes of widget
        register_all_widgets();

        // creates the root window (either in a system window, or the entire screen)
        root_surface = hw_create_window(main_window_size, fullscreen);

        // create picking offscreen
        init_picking();

        // creates the root widget to access the root window
        ei_app_root_widget();
}

/**
 * \brief	Releases all the resources of the application, and releases the hardware
 *		(ie. calls \ref hw_quit).
 */
void ei_app_free(void)
{

        // destroy all the widget by destroying the root
        ei_widget_destroy(root_widget);

        // destroy all the rectangles
        free_rect_list();

        // free all the widget class
        free_all_widgets();

        // free the picking surface & the hash table
        deinit_picking();

        // free the main surface
        hw_surface_free(root_surface);

        // quit the screen
        hw_quit();
}

static void event_manager(ei_event_t *event) {
        if ((event->type == ei_ev_mouse_buttondown) ||
            (event->type == ei_ev_mouse_buttonup) ||
            (event->type == ei_ev_mouse_move)) {
                ei_widget_t *picked_widget;
                if (ei_event_get_active_widget() == NULL) {
                        uint32_t picking_id = get_picking_id(event->param.mouse.where);
                        picked_widget = get_widget_from_picking_id(picking_id);
                } else {
                        picked_widget = ei_event_get_active_widget();
                }
                picked_widget->wclass->handlefunc(picked_widget, event);
        } else {
//                ei_default_handle_func_t func = ei_event_get_default_handle_func();
//                func(event);
        }
        ei_default_handle_func_t func = ei_event_get_default_handle_func();
        if (func != NULL){
                func(event);
        }

}
//        if ((event->type == ei_ev_mouse_buttondown) ||
//            (event->type == ei_ev_mouse_buttonup)) {
//                ei_widget_t *picked_widget;
//                if (ei_event_get_active_widget() == NULL) {
//                        uint32_t picking_id = get_picking_id(event->param.mouse.where);
//                        picked_widget = get_widget_from_picking_id(picking_id);
//                }
//                else {
//                        picked_widget = ei_event_get_active_widget();
//                }
//                picked_widget->wclass->handlefunc(picked_widget, event);
//                ei_placer_run(picked_widget);
//        }


/**
 * \brief	Runs the application: enters the main event loop. Exits when
 *		\ref ei_app_quit_request is called.
 */
void ei_app_run(void)
{
        ei_widget_t *head_widget = ei_app_root_widget();
        ei_surface_t *head_surface = ei_app_root_surface();

        ei_placer_run(head_widget);

        ei_widget_t *current = head_widget;
        // Lock the surface for drawing, fill in white, unlock, update screen.
        ei_rect_t clipper = hw_surface_get_rect(head_surface);
        hw_surface_lock(head_surface);
        ei_surface_t picking_surface = get_picking_surface();

        head_widget->wclass->drawfunc(head_widget, head_surface, picking_surface, &clipper);

//        while (current != NULL) {
//                current->wclass->drawfunc(current, head_surface, picking_surface, &clipper);
//                clipper = *(current->content_rect);
//                current = current->children_head;
//        }
        hw_surface_unlock(head_surface);

        // temp

//        ei_copy_surface(head_surface, NULL, picking_surface, NULL, EI_FALSE);

        hw_surface_update_rects(head_surface, NULL);


        // Wait for a key press.
        ei_event_t			event;
        event.type = ei_ev_none;
        event.param.key.key_code = 0;
        while (event.param.key.key_code != SDLK_ESCAPE)
        {
                hw_event_wait_next(&event);

                hw_surface_lock(head_surface);

                event_manager(&event);

                if (is_free == EI_FALSE) {
                        current = head_widget;
                        clipper = hw_surface_get_rect(head_surface);

                        ei_linked_rect_t *current_rect = rect_list;
                        ei_widget_t *current_widget = head_widget;

                        if (current_rect == NULL) {
                                ei_rect_t head_rect = hw_surface_get_rect(head_surface);
                                ei_app_invalidate_rect(&head_rect);
                                current_rect = rect_list;
                        }

                        while (current_rect != NULL) {
                                while ((current_widget->children_head != NULL) &&
                                       (current_widget->screen_location.top_left.x < current_rect->rect.top_left.x) &&
                                       (current_widget->screen_location.top_left.y < current_rect->rect.top_left.y) &&
                                       (current_widget->screen_location.top_left.x + current_widget->screen_location.size.width > current_rect->rect.top_left.x + current_rect->rect.size.width) &&
                                       (current_widget->screen_location.top_left.y + current_widget->screen_location.size.height > current_rect->rect.top_left.y + current_rect->rect.size.height)) {

                                        current_widget = current_widget->children_head;
                                }

                                clipper = hw_surface_get_rect(head_surface);

                                if (current_widget->parent != NULL) {
                                        clipper = *(current_widget->parent->content_rect);
                                        current_widget = current_widget->parent;
                                }

                                ei_placer_run(head_widget);

                                clipper = hw_surface_get_rect(head_surface);

                                current_widget->wclass->drawfunc(head_widget, head_surface, picking_surface, &clipper);

                                current_rect = current_rect->next;
                                current_widget = head_widget;
                        }

//                      head_widget->wclass->drawfunc(head_widget, head_surface, picking_surface, &clipper);
//                      while (current != NULL) {
//                        current->wclass->drawfunc(current, head_surface, picking_surface, &clipper);
//                        clipper = *(current->content_rect);
//                        current = current->children_head;
//                }
//                        ei_copy_surface(head_surface, NULL, picking_surface, NULL, EI_FALSE);

                        hw_surface_unlock(head_surface);
                        hw_surface_update_rects(head_surface, rect_list);

                        // free rect list
                        free_rect_list();
                        rect_list = NULL;
                }

        }
}

/**
 * \brief	Adds a rectangle to the list of rectangles that must be updated on screen. The real
 *		update on the screen will be done at the right moment in the main loop.
 *
 * @param	rect		The rectangle to add, expressed in the root window coordinates.
 *				A copy is made, so it is safe to release the rectangle on return.
 */
void ei_app_invalidate_rect(ei_rect_t* rect)
{
        insert_rectangle(&rect_list, *rect);
}

/**
 * \brief	Tells the application to quite. Is usually called by an event handler (for example
 *		when pressing the "Escape" key).
 */
void ei_app_quit_request(void)
{
        // update status
        is_free = EI_TRUE;

        //ei_app_free();
}

/**
 * \brief	Returns the "root widget" of the application: a "frame" widget that span the entire
 *		root window.
 *
 * @return 			The root widget.
 */
ei_widget_t* ei_app_root_widget(void)
{
        if (root_widget == NULL) {
                ei_widgetclass_t *widgetclass_ptr = ei_widgetclass_from_name("frame");
                root_widget = widgetclass_ptr->allocfunc();
                root_widget->content_rect = &root_widget->screen_location;
                root_widget->screen_location.top_left.x = 0;
                root_widget->screen_location.top_left.y = 0;
                root_widget->screen_location.size = hw_surface_get_size(ei_app_root_surface());
                set_widget_picking(root_widget);
        }

        return root_widget;
}

/**
 * \brief	Returns the surface of the root window. Can be used to create surfaces with similar
 * 		r, g, b channels.
 *
 * @return 			The surface of the root window.
 */
ei_surface_t ei_app_root_surface(void)
{
        return root_surface;
}
