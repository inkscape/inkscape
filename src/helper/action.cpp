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
#include "desktop.h"
#include "document.h"
#include "helper/action.h"

static void sp_action_finalize (GObject *object);

G_DEFINE_TYPE(SPAction, sp_action, G_TYPE_OBJECT);

/**
 * SPAction vtable initialization.
 */
static void
sp_action_class_init (SPActionClass *klass)
{
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
    action->context = Inkscape::ActionContext();
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

    G_OBJECT_CLASS(sp_action_parent_class)->finalize (object);
}

/**
 * Create new SPAction object and set its properties.
 */
SPAction *
sp_action_new(Inkscape::ActionContext const &context,
              const gchar *id,
              const gchar *name,
              const gchar *tip,
              const gchar *image,
              Inkscape::Verb * verb)
{
    SPAction *action = (SPAction *)g_object_new(SP_TYPE_ACTION, NULL);

    action->context = context;
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
        SPDocument *document = action->context.getDocument();
        if (document) {
            _addProperty(share_static_string("document"), document->serial());
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
    g_return_if_fail (action != NULL);
    g_return_if_fail (SP_IS_ACTION (action));

    g_free(action->name);
    action->name = g_strdup(name.data());
    action->signal_set_name.emit(name);
}

/**
 * Return Document associated with the action.
 */
SPDocument *
sp_action_get_document (SPAction *action)
{
    g_return_val_if_fail (SP_IS_ACTION (action), NULL);
    return action->context.getDocument();
}

/**
 * Return Selection associated with the action
 */
Inkscape::Selection *
sp_action_get_selection (SPAction *action)
{
    g_return_val_if_fail (SP_IS_ACTION (action), NULL);
    return action->context.getSelection();
}

/**
 * Return View associated with the action, if any.
 */
Inkscape::UI::View::View *
sp_action_get_view (SPAction *action)
{
    g_return_val_if_fail (SP_IS_ACTION (action), NULL);
    return action->context.getView();
}

/**
 * Return Desktop associated with the action, if any.
 */
SPDesktop *
sp_action_get_desktop (SPAction *action)
{
    // TODO: this slightly horrible storage of a UI::View::View*, and 
    // casting to an SPDesktop*, is only done because that's what was
    // already the norm in the Inkscape codebase. This seems wrong. Surely
    // we should store an SPDesktop* in the first place? Is there a case
    // of actions being carried out on a View that is not an SPDesktop?
      return static_cast<SPDesktop *>(sp_action_get_view(action));
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
