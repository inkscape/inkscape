/*
 * SPAction implementation.
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2003 Lauris Kaplinski
 *
 * This code is in public domain
 */

#include <string.h>

#include "debug/logger.h"
#include "debug/timestamp.h"
#include "debug/simple-event.h"
#include "debug/event-tracker.h"
#include "ui/view/view.h"
#include "document.h"
#include "helper/action.h"

static void sp_action_class_init (SPActionClass *klass);
static void sp_action_init (SPAction *action);
static void sp_action_finalize (GObject *object);

static GObjectClass *parent_class;

/**
 * Register SPAction class and return its type.
 */
GType
sp_action_get_type (void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPActionClass),
            NULL, NULL,
            (GClassInitFunc) sp_action_class_init,
            NULL, NULL,
            sizeof(SPAction),
            0,
            (GInstanceInitFunc) sp_action_init,
            NULL
        };
        type = g_type_register_static(G_TYPE_OBJECT, "SPAction", &info, (GTypeFlags)0);
    }
    return type;
}

/**
 * SPAction vtable initialization.
 */
static void
sp_action_class_init (SPActionClass *klass)
{
    parent_class = (GObjectClass*) g_type_class_ref(G_TYPE_OBJECT);

    GObjectClass *object_class = (GObjectClass *) klass;
    object_class->finalize = sp_action_finalize;
}

/**
 * Callback for SPAction object initialization.
 */
static void
sp_action_init (SPAction *action)
{
	action->sensitive = 0;
	action->active = 0;
	action->view = NULL;
	action->id = action->name = action->tip = NULL;
	action->image = NULL;
	
	new (&action->signal_perform) sigc::signal<void>();
	new (&action->signal_set_sensitive) sigc::signal<void, bool>();
	new (&action->signal_set_active) sigc::signal<void, bool>();
	new (&action->signal_set_name) sigc::signal<void, Glib::ustring const &>();
}

/**
 * Called before SPAction object destruction.
 */
static void
sp_action_finalize (GObject *object)
{
	SPAction *action = SP_ACTION(object);

	g_free (action->image);
	g_free (action->tip);
	g_free (action->name);
	g_free (action->id);

    action->signal_perform.~signal();
    action->signal_set_sensitive.~signal();
    action->signal_set_active.~signal();
    action->signal_set_name.~signal();

	parent_class->finalize (object);
}

/**
 * Create new SPAction object and set its properties.
 */
SPAction *
sp_action_new(Inkscape::UI::View::View *view,
              const gchar *id,
              const gchar *name,
              const gchar *tip,
              const gchar *image,
              Inkscape::Verb * verb)
{
	SPAction *action = (SPAction *)g_object_new(SP_TYPE_ACTION, NULL);

	action->view = view;
	action->sensitive = TRUE;
	action->id = g_strdup (id);
	action->name = g_strdup (name);
	action->tip = g_strdup (tip);
	action->image = g_strdup (image);
	action->verb = verb;

	return action;
}

namespace {

using Inkscape::Debug::SimpleEvent;
using Inkscape::Debug::Event;
using Inkscape::Util::share_static_string;
using Inkscape::Debug::timestamp;

typedef SimpleEvent<Event::INTERACTION> ActionEventBase;

class ActionEvent : public ActionEventBase {
public:
    ActionEvent(SPAction const *action)
    : ActionEventBase(share_static_string("action"))
    {
        _addProperty(share_static_string("timestamp"), timestamp());
        if (action->view) {
            SPDocument *document = action->view->doc();
            if (document) {
                _addProperty(share_static_string("document"), document->serial());
            }
        }
        _addProperty(share_static_string("verb"), action->id);
    }
};

}

/**
 * Executes an action.
 * @param action   The action to be executed.
 * @param data     ignored.
 */
void sp_action_perform(SPAction *action, void * /*data*/)
{
	g_return_if_fail (action != NULL);
	g_return_if_fail (SP_IS_ACTION (action));

        Inkscape::Debug::EventTracker<ActionEvent> tracker(action);
        action->signal_perform.emit();
}

/**
 * Change activation in all actions that can be taken with the action.
 */
void
sp_action_set_active (SPAction *action, unsigned int active)
{
	g_return_if_fail (action != NULL);
	g_return_if_fail (SP_IS_ACTION (action));

        action->signal_set_active.emit(active);
}

/**
 * Change sensitivity in all actions that can be taken with the action.
 */
void
sp_action_set_sensitive (SPAction *action, unsigned int sensitive)
{
	g_return_if_fail (action != NULL);
	g_return_if_fail (SP_IS_ACTION (action));

	action->signal_set_sensitive.emit(sensitive);
}

void
sp_action_set_name (SPAction *action, Glib::ustring const &name)
{
    g_free(action->name);
    action->name = g_strdup(name.data());
    action->signal_set_name.emit(name);
}

/**
 * Return View associated with the action.
 */
Inkscape::UI::View::View *
sp_action_get_view (SPAction *action)
{
	g_return_val_if_fail (SP_IS_ACTION (action), NULL);
	return action->view;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
