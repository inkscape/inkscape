#define __SP_ACTION_C__

/** \file
 * SPAction implementation
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
static void sp_action_finalize (NRObject *object);

static NRActiveObjectClass *parent_class;

/**
 * Register SPAction class and return its type.
 */
NRType
sp_action_get_type (void)
{
	static unsigned int type = 0;
	if (!type) {
		type = nr_object_register_type (NR_TYPE_ACTIVE_OBJECT,
						"SPAction",
						sizeof (SPActionClass),
						sizeof (SPAction),
						(void (*) (NRObjectClass *)) sp_action_class_init,
						(void (*) (NRObject *)) sp_action_init);
	}
	return type;
}

/**
 * SPAction vtable initialization.
 */
static void
sp_action_class_init (SPActionClass *klass)
{
	NRObjectClass * object_class;

	object_class = (NRObjectClass *) klass;

	parent_class = (NRActiveObjectClass *) (((NRObjectClass *) klass)->parent);

	object_class->finalize = sp_action_finalize;
	object_class->cpp_ctor = NRObject::invoke_ctor<SPAction>;
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
}

/**
 * Called before SPAction object destruction.
 */
static void
sp_action_finalize (NRObject *object)
{
	SPAction *action;

	action = (SPAction *) object;

	if (action->image) free (action->image);
	if (action->tip) free (action->tip);
	if (action->name) free (action->name);
	if (action->id) free (action->id);

	((NRObjectClass *) (parent_class))->finalize (object);
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
	SPAction *action = (SPAction *)nr_object_new(SP_TYPE_ACTION);

	action->view = view;
	action->sensitive = TRUE;
	if (id) action->id = strdup (id);
	if (name) action->name = strdup (name);
	if (tip) action->tip = strdup (tip);
	if (image) action->image = strdup (image);
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
	\return   None
	\brief    Executes an action
	\param    action   The action to be executed
	\param    data     Data that is passed into the action.  This depends
	                   on the situation that the action is used in.

	This function implements the 'action' in SPActions.  It first validates
	its parameters, making sure it got an action passed in.  Then it
	turns that action into its parent class of NRActiveObject.  The
	NRActiveObject allows for listeners to be attached to it.  This
	function goes through those listeners and calls them with the
	vector that was attached to the listener.
*/
void
sp_action_perform (SPAction *action, void * data)
{
	NRActiveObject *aobject;

	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

        Inkscape::Debug::EventTracker<ActionEvent> tracker(action);

	aobject = NR_ACTIVE_OBJECT(action);
	if (aobject->callbacks) {
		unsigned int i;
		for (i = 0; i < aobject->callbacks->length; i++) {
			NRObjectListener *listener;
			SPActionEventVector *avector;

			listener = &aobject->callbacks->listeners[i];
			avector = (SPActionEventVector *) listener->vector;

			if ((listener->size >= sizeof (SPActionEventVector)) && avector != NULL && avector->perform != NULL) {
				avector->perform (action, listener->data, data);
			}
		}
	}
}

/**
 * Change activation in all actions that can be taken with the action.
 */
void
sp_action_set_active (SPAction *action, unsigned int active)
{
	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

	if (active != action->active) {
		NRActiveObject *aobject;
		action->active = active;
		aobject = (NRActiveObject *) action;
		if (aobject->callbacks) {
			unsigned int i;
			for (i = 0; i < aobject->callbacks->length; i++) {
				NRObjectListener *listener;
				SPActionEventVector *avector;
				listener = aobject->callbacks->listeners + i;
				avector = (SPActionEventVector *) listener->vector;
				if ((listener->size >= sizeof (SPActionEventVector)) && avector->set_active) {
					avector->set_active (action, active, listener->data);
				}
			}
		}
	}
}

/**
 * Change sensitivity in all actions that can be taken with the action.
 */
void
sp_action_set_sensitive (SPAction *action, unsigned int sensitive)
{
	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

	if (sensitive != action->sensitive) {
		NRActiveObject *aobject;
		action->sensitive = sensitive;
		aobject = (NRActiveObject *) action;
		if (aobject->callbacks) {
			unsigned int i;
			for (i = 0; i < aobject->callbacks->length; i++) {
				NRObjectListener *listener;
				SPActionEventVector *avector;
				listener = aobject->callbacks->listeners + i;
				avector = (SPActionEventVector *) listener->vector;
				if ((listener->size >= sizeof (SPActionEventVector)) && avector->set_sensitive) {
					avector->set_sensitive (action, sensitive, listener->data);
				}
			}
		}
	}
}


/**
 * Change name for all actions that can be taken with the action.
 */
void
sp_action_set_name (SPAction *action, Glib::ustring name)
{
	nr_return_if_fail (action != NULL);
	nr_return_if_fail (SP_IS_ACTION (action));

        NRActiveObject *aobject;
        g_free(action->name);
        action->name = g_strdup(name.c_str());
        aobject = (NRActiveObject *) action;
        if (aobject->callbacks) {
            unsigned int i;
            for (i = 0; i < aobject->callbacks->length; i++) {
                NRObjectListener *listener;
                SPActionEventVector *avector;
                listener = aobject->callbacks->listeners + i;
                avector = (SPActionEventVector *) listener->vector;
                if ((listener->size >= sizeof (SPActionEventVector)) && avector->set_name) {
                    avector->set_name (action, name, listener->data);
                }
            }
        }
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
