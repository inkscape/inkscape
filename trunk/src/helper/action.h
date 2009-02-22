#ifndef __SP_ACTION_H__
#define __SP_ACTION_H__

/** \file
 * Inkscape UI action implementation
 */

/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Lauris Kaplinski
 *
 * This code is in public domain
 */

/** A macro to get the GType for actions */
#define SP_TYPE_ACTION (sp_action_get_type())
/** A macro to cast and check the cast of changing an object to an action */
#define SP_ACTION(o) (NR_CHECK_INSTANCE_CAST((o), SP_TYPE_ACTION, SPAction))
/** A macro to check whether or not something is an action */
#define SP_IS_ACTION(o) (NR_CHECK_INSTANCE_TYPE((o), SP_TYPE_ACTION))

#include "helper/helper-forward.h"
#include "libnr/nr-object.h"
#include "forward.h"

#include <glibmm/ustring.h>
//class Inkscape::UI::View::View; 

namespace Inkscape {
class Verb;
}


/** This is a structure that is used to hold all the possible
    actions that can be taken with an action.  These are the
    function pointers available. */
struct SPActionEventVector {
    NRObjectEventVector object_vector;                                        /**< Parent class */
    void (* perform)(SPAction *action, void *ldata, void *pdata);             /**< Actually do the action of the event.  Called by sp_perform_action */
    void (* set_active)(SPAction *action, unsigned active, void *data);       /**< Callback for activation change */
    void (* set_sensitive)(SPAction *action, unsigned sensitive, void *data); /**< Callback for a change in sensitivity */
    void (* set_shortcut)(SPAction *action, unsigned shortcut, void *data);   /**< Callback for setting the shortcut for this function */
    void (* set_name)(SPAction *action, Glib::ustring, void *data);           /**< Callback for setting the name for this function */
};

/** All the data that is required to be an action.  This
    structure identifies the action and has the data to
	create menus and toolbars for the action */
struct SPAction : public NRActiveObject {
    unsigned sensitive : 1;  /**< Value to track whether the action is sensitive */
    unsigned active : 1;     /**< Value to track whether the action is active */
    Inkscape::UI::View::View *view;            /**< The View to which this action is attached */
    gchar *id;               /**< The identifier for the action */
    gchar *name;             /**< Full text name of the action */
    gchar *tip;              /**< A tooltip to describe the action */
    gchar *image;            /**< An image to visually identify the action */
    Inkscape::Verb *verb;    /**< The verb that produced this action */
};

/** The action class is the same as its parent. */
struct SPActionClass {
    NRActiveObjectClass parent_class; /**< Parent Class */
};

NRType sp_action_get_type();

SPAction *sp_action_new(Inkscape::UI::View::View *view,
			gchar const *id,
			gchar const *name,
			gchar const *tip,
			gchar const *image,
			Inkscape::Verb *verb);

void sp_action_perform(SPAction *action, void *data);
void sp_action_set_active(SPAction *action, unsigned active);
void sp_action_set_sensitive(SPAction *action, unsigned sensitive);
void sp_action_set_name (SPAction *action, Glib::ustring);
Inkscape::UI::View::View *sp_action_get_view(SPAction *action);

#endif


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
