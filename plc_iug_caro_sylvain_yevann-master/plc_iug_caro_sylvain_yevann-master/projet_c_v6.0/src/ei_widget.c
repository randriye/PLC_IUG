//
// Created by sylvain on 18/05/2021.
//

#include <string.h>
#include <stdlib.h>
#include "ei_widget_frame.h"
#include "ei_widget_toplevel.h"
#include "ei_widget_button.h"
#include "ei_draw.h"
#include "ei_widgetclass.h"
#include "ei_placer.h"
#include "ei_widget.h"
#include "ei_types.h"
#include "picker.h"
#include "assert.h"
#include "string.h"

/**
 * \brief	Update the geometry of a widget using the "placer" geometry manager.
 *
 * @param	widget		The widget to place.
 */
static void		ei_place_update	(struct ei_widget_t*	widget)
{

        if (widget ->placer_params == NULL) {
                return;
        }

        // w and rel_width
        if ((widget->placer_params->w == NULL) || (widget->placer_params->w_data == 0)) {
                // requested size - we have to consider the rel_width
                if ((widget->placer_params->rw == NULL) || (widget->placer_params->rw_data == 0)) {
                        widget->placer_params->w = &(widget->placer_params->w_data);
                        widget->placer_params->w_data = widget->requested_size.width;
                        widget->placer_params->rw = NULL;
                }
        }

        // h and rel_height
        if ((widget->placer_params->h == NULL) || (widget->placer_params->h_data == 0)) {
                // requested size - we have to consider the rel_height
                if ((widget->placer_params->rh == NULL) || (widget->placer_params->rh_data == 0)) {
                        widget -> placer_params -> h_data = widget -> requested_size.height;
                        widget -> placer_params -> h = &(widget -> placer_params -> h_data);
                        widget -> placer_params -> rh = NULL;
                }
        }
}

/**
 * @brief	Creates a new instance of a widget of some particular class, as a descendant of
 *		an existing widget.
 *
 *		The widget is not displayed on screen until it is managed by a geometry manager.
 *		When no more needed, the widget must be released by calling \ref ei_widget_destroy.
 *
 * @param	class_name	The name of the class of the widget that is to be created.
 * @param	parent 		A pointer to the parent widget. Can not be NULL.
 * @param	user_data	A pointer provided by the programmer for private use. May be NULL.
 * @param	destructor	A pointer to a function to call before destroying a widget structure. May be NULL.
 *
 * @return			The newly created widget, or NULL if there was an error.
 */
ei_widget_t*		ei_widget_create		(ei_widgetclass_name_t	class_name,
                                             ei_widget_t*		parent,
                                             void*			user_data,
                                             ei_widget_destructor_t destructor)
{
        // get allocate function for this class
        ei_widgetclass_t *used_class = ei_widgetclass_from_name(class_name);

        // allocate widget
        ei_widget_t *widget = used_class->allocfunc();

        // fill some widget data
        widget->parent = parent;

        if (user_data != NULL) {
                widget->user_data = user_data;
        }

        if (destructor != NULL) {
                widget->destructor = destructor;
        }

        widget->content_rect = &widget->screen_location;

        widget->next_sibling = NULL;

        set_widget_picking(widget);

        // set children from parent's point of view
        ei_widget_t *wparent = widget->parent;
        widget->children_head = NULL;
        widget->children_tail = NULL;
        widget->next_sibling = wparent->children_head;
        wparent->children_head = widget;
        if (wparent->children_tail == NULL) {
                wparent->children_tail = widget;
        }
//        ei_widget_t *wparent = widget->parent;
//        widget->children_head = NULL;
//        widget->children_tail = NULL;
//        widget->next_sibling = NULL;
//        if (wparent->children_tail != NULL) {
//                wparent->children_tail->next_sibling = widget;
//        }
//        wparent->children_tail = widget;
//        if (wparent->children_head == NULL) {
//                wparent->children_head = widget;
//        }

        return widget;
}

/**
 * @brief	Destroys a widget. Calls its destructor if it was provided.
 * 		Removes the widget from the screen if it is currently managed by the placer.
 * 		Destroys all its descendants.
 *
 * @param	widget		The widget that is to be destroyed.
 */
void			ei_widget_destroy		(ei_widget_t*		widget)
{
        // delete in the widget picking hash table
        delete_widget_picking(widget);

        // call destructor
        if (widget->destructor != NULL) {
                widget->destructor(widget);
        }
        // remove from screen
        ei_placer_forget(widget);

        // remove widget from parent's list of widget
        if (widget->parent != NULL) {
                if (widget->parent->children_head == widget) {
                        widget->parent->children_head = widget->next_sibling;
                }
                else {
                        ei_widget_t *current = widget->parent->children_head;

                        while ((current != NULL) && (current->next_sibling != widget)) {
                                current = current->next_sibling;
                        }

                        current->next_sibling = widget->next_sibling;
                }
                if (widget->parent->children_tail == widget) {
                        widget->parent->children_tail = widget->next_sibling;
                }
        }

        // destroy
        widget->wclass->releasefunc(widget);
}


/**
 * @brief	Returns the widget that is at a given location on screen.
 *
 * @param	where		The location on screen, expressed in the root window coordinates.
 *
 * @return			The top-most widget at this location, or NULL if there is no widget
 *				at this location (except for the root widget).
 */
ei_widget_t*		ei_widget_pick			(ei_point_t*		where)
{
        uint32_t picking_id = get_picking_id(*where);

        return get_widget_from_picking_id(picking_id);
}

/**
 * @brief	Configures the attributes of widgets of the class "frame".
 *
 *		Parameters obey the "default" protocol: if a parameter is "NULL" and it has never
 *		been defined before, then a default value should be used (default values are
 *		specified for each parameter). If the parameter is "NULL" but was defined on a
 *		previous call, then its value must not be changed.
 *
 * @param	widget		The widget to configure.
 * @param	requested_size	The size requested for this widget, including the widget's borders.
 *				The geometry manager may override this size due to other constraints.
 *				Defaults to the "natural size" of the widget, ie. big enough to
 *				display the border and the text or the image. This may be (0, 0)
 *				if the widget has border_width=0, and no text and no image.
 * @param	color		The color of the background of the widget. Defaults to
 *				\ref ei_default_background_color.
 * @param	border_width	The width in pixel of the border decoration of the widget. The final
 *				appearance depends on the "relief" parameter. Defaults to 0.
 * @param	relief		Appearance of the border of the widget. Defaults to
 *				\ref ei_relief_none.
 * @param	text		The text to display in the widget, or NULL. Only one of the
 *				parameter "text" and "img" should be used (i.e. non-NULL). Defaults
 *				to NULL.
 * @param	text_font	The font used to display the text. Defaults to \ref ei_default_font.
 * @param	text_color	The color used to display the text. Defaults to
 *				\ref ei_font_default_color.
 * @param	text_anchor	The anchor of the text, i.e. where it is placed within the widget.
 *				Defines both the anchoring point on the parent and on the widget.
 *				Defaults to \ref ei_anc_center.
 * @param	img		The image to display in the widget, or NULL. Any surface can be
 *				used, but usually a surface returned by \ref hw_image_load. Only one
 *				of the parameter "text" and "img" should be used (i.e. non-NULL).
 				Defaults to NULL.
 * @param	img_rect	If not NULL, this rectangle defines a subpart of "img" to use as the
 *				image displayed in the widget. Defaults to NULL.
 * @param	img_anchor	The anchor of the image, i.e. where it is placed within the widget
 *				when the size of the widget is bigger than the size of the image.
 *				Defaults to \ref ei_anc_center.
 */
void			ei_frame_configure		(ei_widget_t*		widget,
                                           ei_size_t*		requested_size,
                                           const ei_color_t*	color,
                                           int*			border_width,
                                           ei_relief_t*		relief,
                                           char**			text,
                                           ei_font_t*		text_font,
                                           ei_color_t*		text_color,
                                           ei_anchor_t*		text_anchor,
                                           ei_surface_t*		img,
                                           ei_rect_t**		img_rect,
                                           ei_anchor_t*		img_anchor)
{
//        ei_frame_t *new_widget = (ei_frame_t *) widget->wclass->allocfunc();
//
//        // copy common widget data
//        memcpy(new_widget, widget, sizeof(ei_widget_t));

        ei_frame_t *new_widget = (ei_frame_t *) widget;

        new_widget->widget.wclass = ei_widgetclass_from_name("frame");

        // add default frame specific attributes if not already done
        if (new_widget->already_configured == EI_FALSE)
        {
                new_widget->widget.wclass->setdefaultsfunc(widget);
                new_widget->already_configured = EI_TRUE;
        }

        if (color != NULL) {
                new_widget->color = *color;
        }

        if (border_width != NULL) {
                new_widget->border_with = *border_width;
        }

        if (relief != NULL) {
                new_widget->relief = *relief;
        }

        if ((text != NULL) && (*text != NULL)) {
                int text_len = (int) strlen(*text);
                new_widget->text = malloc((text_len+1)* sizeof(char));
                strcpy(new_widget->text, *text);
//                new_widget->text = *text;
        }

        if (text_font != NULL) {
                new_widget->text_font = *text_font;
        }

        if (text_color != NULL) {
                new_widget->text_color = *text_color;
        }

        if (text_anchor != NULL) {
                new_widget->text_anchor = *text_anchor;
        }

        if (img != NULL) {
                ei_size_t surface_size = hw_surface_get_size(*img);
                ei_surface_t new_img = hw_surface_create(*img, surface_size, EI_FALSE);
                ei_copy_surface(new_img, NULL, *img, NULL, hw_surface_has_alpha(*img));
                new_widget->img = new_img;
        }

        if (img_rect != NULL) {

                new_widget->img_rect = malloc(sizeof(ei_rect_t));
                new_widget->img_rect->size = (*img_rect)->size;
                new_widget->img_rect->top_left = (*img_rect)->top_left;
        }

        if (img_anchor != NULL) {
                new_widget->img_anchor = *img_anchor;
        }

        // add frame specific data
        if (requested_size != NULL) {
                new_widget->widget.requested_size = *requested_size;
        }
        else {
                ei_size_t text_size = {0, 0};
                ei_size_t img_size = {0, 0};
                if (text != NULL) {
                        hw_text_compute_size(*text, new_widget->text_font, &text_size.width, &text_size.height);
                }
                if (img != NULL) {
                        img_size.width = (*img_rect)->size.width;
                        img_size.height = (*img_rect)->size.height;
                }

                if (text_size.width > img_size.width) {
                        new_widget->widget.requested_size.width = text_size.width;
                }
                else {
                        new_widget->widget.requested_size.height = img_size.height;
                }

                if (text_size.height > img_size.height) {
                        new_widget->widget.requested_size.height = text_size.height;
                }
                else {
                        new_widget->widget.requested_size.height = img_size.height;
                }
        }

        ei_place_update(widget);
}


/**
 * @brief	Configures the attributes of widgets of the class "button".
 *
 * @param	widget, requested_size, color, border_width, relief,
 *		text, text_font, text_color, text_anchor,
 *		img, img_rect, img_anchor
 *				See the parameter definition of \ref ei_frame_configure. The only
 *				difference is that relief defaults to \ref ei_relief_raised
 *				and border_width defaults to \ref k_default_button_border_width.
 * @param	corner_radius	The radius (in pixels) of the rounded corners of the button.
 *				0 means straight corners. Defaults to \ref k_default_button_corner_radius.
 * @param	callback	The callback function to call when the user clicks on the button.
 *				Defaults to NULL (no callback).
 * @param	user_param	A programmer supplied parameter that will be passed to the callback
 *				when called. Defaults to NULL.
 */
void			ei_button_configure		(ei_widget_t*		widget,
                                            ei_size_t*		requested_size,
                                            const ei_color_t*	color,
                                            int*			border_width,
                                            int*			corner_radius,
                                            ei_relief_t*		relief,
                                            char**			text,
                                            ei_font_t*		text_font,
                                            ei_color_t*		text_color,
                                            ei_anchor_t*		text_anchor,
                                            ei_surface_t*		img,
                                            ei_rect_t**		img_rect,
                                            ei_anchor_t*		img_anchor,
                                            ei_callback_t*		callback,
                                            void**			user_param)
{
        ei_button_t *new_widget = (ei_button_t *) widget;

        new_widget->widget.wclass = ei_widgetclass_from_name("button");

        // add default button specific attributes
        new_widget->widget.wclass->setdefaultsfunc(widget);
        if (new_widget->already_configured == EI_FALSE)
        {
                new_widget->widget.wclass->setdefaultsfunc(widget);
                new_widget->already_configured = EI_TRUE;
        }

        // add frame specific data

        if (color != NULL) {
                new_widget->color = *color;
        }

        if (border_width != NULL) {
                new_widget->border_with = *border_width;
        }

        if (corner_radius != NULL) {
                new_widget->corner_radius = *corner_radius;
        }

        if (relief != NULL) {
                new_widget->relief = *relief;
        }

        if ((text != NULL) && (*text != NULL)) {
                int text_len = (int) strlen(*text);
                new_widget->text = malloc((text_len+1)* sizeof(char));
                strcpy(new_widget->text, *text);
//                new_widget->text = *text;
        }

        if (text_font != NULL) {
                new_widget->text_font = *text_font;
        }

        if (text_color != NULL) {
                new_widget->text_color = *text_color;
        }

        if (text_anchor != NULL) {
                new_widget->text_anchor = *text_anchor;
        }

        if (img != NULL) {
                ei_size_t surface_size = hw_surface_get_size(*img);
                ei_surface_t new_img = hw_surface_create(*img, surface_size, EI_FALSE);
                ei_copy_surface(new_img, NULL, *img, NULL, hw_surface_has_alpha(*img));
                new_widget->img = new_img;
        }

        if (img_rect != NULL) {

                new_widget->img_rect = malloc(sizeof(ei_rect_t));
                new_widget->img_rect->size = (*img_rect)->size;
                new_widget->img_rect->top_left = (*img_rect)->top_left;
        }

        if (img_anchor != NULL) {
                new_widget->img_anchor = *img_anchor;
        }

        if (callback != NULL) {
                new_widget->callback = *callback;
        }

        if (user_param != NULL) {
                new_widget->user_param = *user_param;
        }

        if (requested_size != NULL) {
                new_widget->widget.requested_size = *requested_size;
        }
        else {
                if (text != NULL) {
                        int width;
                        int height;
                        hw_text_compute_size(new_widget->text, new_widget->text_font, &width, &height);
                        ei_size_t widget_size = {width+50, height};
                        new_widget->widget.requested_size = widget_size;
                }
                else {
                        new_widget->widget.requested_size.width = 50;
                        new_widget->widget.requested_size.height = 50;
                }
        }

        ei_place_update(widget);
}

/**
 * @brief	Configures the attributes of widgets of the class "toplevel".
 *
 * @param	widget		The widget to configure.
 * @param	requested_size	The content size requested for this widget, this does not include
 *				the decorations	(border, title bar). The geometry manager may
 *				override this size due to other constraints.
 *				Defaults to (320x240).
 * @param	color		The color of the background of the content of the widget. Defaults
 *				to \ref ei_default_background_color.
 * @param	border_width	The width in pixel of the border of the widget. Defaults to 4.
 * @param	title		The string title displayed in the title bar. Defaults to "Toplevel".
 *				Uses the font \ref ei_default_font.
 * @param	closable	If true, the toplevel is closable by the user, the toplevel must
 *				show a close button in its title bar. Defaults to \ref EI_TRUE.
 * @param	resizable	Defines if the widget can be resized horizontally and/or vertically
 *				by the user. Defaults to \ref ei_axis_both.
 * @param	min_size	For resizable widgets, defines the minimum size of its content.
 *				The default minimum size of a toplevel is (160, 120).
 *				If *min_size is NULL, this requires the toplevel to be configured to
 *				its default size.
 */
void			ei_toplevel_configure		(ei_widget_t*		widget,
                                              ei_size_t*		requested_size,
                                              ei_color_t*		color,
                                              int*			border_width,
                                              char**			title,
                                              ei_bool_t*		closable,
                                              ei_axis_set_t*		resizable,
                                              ei_size_t**		min_size)
{
        ei_toplevel_t *new_widget = (ei_toplevel_t *) widget;

        new_widget->widget.wclass = ei_widgetclass_from_name("toplevel");

        // add default top level specific attributes
        if (new_widget->already_configured == EI_FALSE)
        {
                new_widget->widget.wclass->setdefaultsfunc(widget);
                new_widget->already_configured = EI_TRUE;
        }

        if (requested_size != NULL) {
                new_widget->widget.requested_size = *requested_size;
        }

        if (color != NULL) {
                new_widget->color = *color;
        }

        if (border_width != NULL) {
                new_widget->border_width = *border_width;
        }

        if ((title != NULL) && (*title != NULL)) {
                int text_len = (int) strlen(*title);
                new_widget->title = malloc((text_len+1)* sizeof(char));
                strcpy(new_widget->title, *title);
//                new_widget->title = *title;
        }

        if (closable != NULL) {
                new_widget->closable = *closable;
        }

        if (resizable != NULL) {
                new_widget->resizable = *resizable;
        }

        if (min_size != NULL) {
                new_widget->min_size = *min_size;
        }

        ei_place_update(widget);
}

static ei_widgetclass_t **get_widgetclass_pointer() {
        static ei_widgetclass_t *ptr = NULL;
        return &ptr;
}

/**
 * @brief	Registers a class to the program so that widgets of this class can be created.
 *		This must be done only once per widged class in the application.
 *
 * @param	widgetclass	The structure describing the class.
 */
void			ei_widgetclass_register		(ei_widgetclass_t* widgetclass)
{
        ei_widgetclass_t **head = get_widgetclass_pointer();

        widgetclass->next = *head;
        *head = widgetclass;
}

/**
 * @brief	Returns the structure describing a class, from its name.
 *
 * @param	name		The name of the class of widget.
 *
 * @return			The structure describing the class.
 */
ei_widgetclass_t*	ei_widgetclass_from_name	(ei_widgetclass_name_t name)
{
        ei_widgetclass_t *head = *get_widgetclass_pointer();

        ei_widgetclass_t *current = head;

        while ((current != NULL) && (strcmp((char *) current->name, (char *) name) != 0)) {
                current = current->next;
        }

        assert(current != NULL);

        return current;
}
