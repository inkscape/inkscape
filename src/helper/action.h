/** \file
 * Inkscape UI action implementation
 *//*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Lauris Kaplinski
 *
 * This code is in public domain
 */

#ifndef SEEN_INKSCAPE_SP_ACTION_H
#define SEEN_INKSCAPE_SP_ACTION_H

#include "helper/action-context.h"
#include <sigc++/signal.h>
#include <glibmm/ustring.h>

#define SP_TYPE_ACTION (sp_action_get_type())
#define SP_ACTION(o) (G_TYPE_CHECK_INSTANCE_CAST((o), SP_TYPE_ACTION, SPAction))
#define SP_ACTION_CLASS(o) (G_TYPE_CHECK_CLASS_CAST((o), SP_TYPE_ACTION, SPActionClass))
#define SP_IS_ACTION(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), SP_TYPE_ACTION))

class SPDesktop;
class SPDocument;
namespace Inkscape {

class Selection;
class Verb;

namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI
}

/** All the data that is required to be an action.  This
    structure identifies the action and has the data to
	create menus and toolbars for the action */
struct SPAction : public GObject {
    unsigned sensitive : 1;  /**< Value to track whether the action is sensitive */
    unsigned active : 1;     /**< Value to track whether the action is active */
    Inkscape::ActionContext context;  /**< The context (doc/view) to which this action is attached */
    gchar *id;               /**< The identifier for the action */
    gchar *name;             /**< Full text name of the action */
    gchar *tip;              /**< A tooltip to describe the action */
    gchar *image;            /**< An image to visually identify the action */
    Inkscape::Verb *verb;    /**< The verb that produced this action */
    
    sigc::signal<void> signal_perform;
    sigc::signal<void, bool> signal_set_sensitive;
    sigc::signal<void, bool> signal_set_active;
    sigc::signal<void, Glib::ustring const &> signal_set_name;
};

/** The action class is the same as its parent. */
struct SPActionClass {
    GObjectClass parent_class; /**< Parent Class */
};

GType sp_action_get_type();

SPAction *sp_action_new(Inkscape::ActionContext const &context,
			gchar const *id,
			gchar const *name,
			gchar const *tip,
			gchar const *image,
			Inkscape::Verb *verb);

void sp_action_perform(SPAction *action, void *data);
void sp_action_set_active(SPAction *action, unsigned active);
void sp_action_set_sensitive(SPAction *action, unsigned sensitive);
void sp_action_set_name(SPAction *action, Glib::ustring const &name);
SPDocument *sp_action_get_document(SPAction *action);
Inkscape::Selection *sp_action_get_selection(SPAction *action);
Inkscape::UI::View::View *sp_action_get_view(SPAction *action);
SPDesktop *sp_action_get_desktop(SPAction *action);

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
