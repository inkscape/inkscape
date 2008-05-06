#define __SP_BUTTON_C__

/*
 * Generic button widget
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif


#if HAVE_STRING_H
#endif




#include "shortcuts.h"
#include "interface.h"

#include "icon.h"
#include "button.h"

static void sp_button_class_init (SPButtonClass *klass);
static void sp_button_init (SPButton *button);
static void sp_button_destroy (GtkObject *object);

static void sp_button_size_request (GtkWidget *widget, GtkRequisition *requisition);
static void sp_button_clicked (GtkButton *button);
static void sp_button_perform_action (SPButton *button, gpointer data);
static gint sp_button_process_event (SPButton *button, GdkEvent *event);

static void sp_button_set_action (SPButton *button, SPAction *action);
static void sp_button_set_doubleclick_action (SPButton *button, SPAction *action);
static void sp_button_action_set_active (SPAction *action, unsigned int active, void *data);
static void sp_button_action_set_sensitive (SPAction *action, unsigned int sensitive, void *data);
static void sp_button_action_set_shortcut (SPAction *action, unsigned int shortcut, void *data);
static void sp_button_set_composed_tooltip (GtkTooltips *tooltips, GtkWidget *widget, SPAction *action);

static GtkToggleButtonClass *parent_class;
SPActionEventVector button_event_vector = {
	{NULL},
	 NULL,
	 sp_button_action_set_active,
	 sp_button_action_set_sensitive,
         sp_button_action_set_shortcut,
         NULL
};

GType sp_button_get_type(void)
{
    static GType type = 0;
    if (!type) {
        GTypeInfo info = {
            sizeof(SPButtonClass),
            0, // base_init
            0, // base_finalize
            (GClassInitFunc)sp_button_class_init,
            0, // class_finalize
            0, // class_data
            sizeof(SPButton),
            0, // n_preallocs
            (GInstanceInitFunc)sp_button_init,
            0 // value_table
        };
        type = g_type_register_static(GTK_TYPE_TOGGLE_BUTTON, "SPButton", &info, static_cast<GTypeFlags>(0));
    }
    return type;
}

static void
sp_button_class_init (SPButtonClass *klass)
{
	GtkObjectClass *object_class=(GtkObjectClass *)klass;
	GtkWidgetClass *widget_class=(GtkWidgetClass *)klass;
	GtkButtonClass *button_class=(GtkButtonClass *)klass;

	parent_class = (GtkToggleButtonClass *)g_type_class_peek_parent (klass);

	object_class->destroy = sp_button_destroy;
	widget_class->size_request = sp_button_size_request;
	button_class->clicked = sp_button_clicked;
}

static void
sp_button_init (SPButton *button)
{
	button->action = NULL;
	button->doubleclick_action = NULL;
	button->tooltips = NULL;

	gtk_container_set_border_width (GTK_CONTAINER (button), 0);

	GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (button), GTK_CAN_FOCUS);
	GTK_WIDGET_UNSET_FLAGS (GTK_WIDGET (button), GTK_CAN_DEFAULT);

	g_signal_connect_after (G_OBJECT (button), "clicked", G_CALLBACK (sp_button_perform_action), NULL);
	g_signal_connect_after (G_OBJECT (button), "event", G_CALLBACK (sp_button_process_event), NULL);
}

static void
sp_button_destroy (GtkObject *object)
{
	SPButton *button;

	button = SP_BUTTON (object);

	if (button->tooltips) {
		g_object_unref (G_OBJECT (button->tooltips));
		button->tooltips = NULL;
	}

	if (button->action) {
		sp_button_set_action (button, NULL);
	}

	if (button->doubleclick_action) {
		sp_button_set_doubleclick_action (button, NULL);
	}

	((GtkObjectClass *) (parent_class))->destroy (object);
}

static void
sp_button_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
	GtkWidget *child;

	child = gtk_bin_get_child (GTK_BIN (widget));
	if (child) {
		gtk_widget_size_request (GTK_WIDGET (child), requisition);
	} else {
		requisition->width = 0;
		requisition->height = 0;
	}

	requisition->width += 2 + 2 * MAX (2, widget->style->xthickness);
	requisition->height += 2 + 2 * MAX (2, widget->style->ythickness);
}

static void
sp_button_clicked (GtkButton *button)
{
	SPButton *sp_button=SP_BUTTON (button);

	if (sp_button->type == SP_BUTTON_TYPE_TOGGLE) {
		((GtkButtonClass *) (parent_class))->clicked (button);
	}
}

static gint 
sp_button_process_event (SPButton *button, GdkEvent *event)
{
	switch (event->type) {
	case GDK_2BUTTON_PRESS:
		if (button->doubleclick_action) {
			sp_action_perform (button->doubleclick_action, NULL);
		}
		return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

static void
sp_button_perform_action (SPButton *button, gpointer /*data*/)
{
	if (button->action) {
		sp_action_perform (button->action, NULL);
	}
}


GtkWidget *
sp_button_new( Inkscape::IconSize size, SPButtonType type, SPAction *action, SPAction *doubleclick_action, GtkTooltips *tooltips )
{
	SPButton *button;

	button = (SPButton *)g_object_new (SP_TYPE_BUTTON, NULL);

	button->type = type;
	button->lsize = CLAMP( size, Inkscape::ICON_SIZE_MENU, Inkscape::ICON_SIZE_DECORATION );
	button->tooltips = tooltips;

	if (tooltips) g_object_ref ((GObject *) tooltips);

	sp_button_set_action (button, action);
	if (doubleclick_action)
		sp_button_set_doubleclick_action (button, doubleclick_action);

	// The Inkscape style is no-relief buttons
	gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);

	return (GtkWidget *) button;
}

void
sp_button_toggle_set_down (SPButton *button, gboolean down)
{
	g_return_if_fail (button->type == SP_BUTTON_TYPE_TOGGLE);
	g_signal_handlers_block_by_func (G_OBJECT (button), (gpointer)G_CALLBACK (sp_button_perform_action), NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), (unsigned int)down);
	g_signal_handlers_unblock_by_func (G_OBJECT (button), (gpointer)G_CALLBACK (sp_button_perform_action), NULL);
}

static void
sp_button_set_doubleclick_action (SPButton *button, SPAction *action)
{
	if (button->doubleclick_action) {
		nr_object_unref ((NRObject *) button->doubleclick_action);
	}
	button->doubleclick_action = action;
	if (action) {
		button->doubleclick_action = (SPAction *) nr_object_ref ((NRObject *) action);
	}
}

static void
sp_button_set_action (SPButton *button, SPAction *action)
{
	GtkWidget *child;

	if (button->action) {
		nr_active_object_remove_listener_by_data ((NRActiveObject *) button->action, button);
		nr_object_unref ((NRObject *) button->action);
		child = gtk_bin_get_child (GTK_BIN (button));
		if (child) {
			gtk_container_remove (GTK_CONTAINER (button), child);
		}
	}
	button->action = action;
	if (action) {
		button->action = (SPAction *) nr_object_ref ((NRObject *) action);
		nr_active_object_add_listener ((NRActiveObject *) action, (NRObjectEventVector *) &button_event_vector, sizeof (SPActionEventVector), button);
		if (action->image) {
			child = sp_icon_new (button->lsize, action->image);
			gtk_widget_show (child);
			gtk_container_add (GTK_CONTAINER (button), child);
		}
	}

	if (button->tooltips) {
		sp_button_set_composed_tooltip (button->tooltips, (GtkWidget *) button, action);
	}
}

static void
sp_button_action_set_active (SPAction */*action*/, unsigned int active, void *data)
{
	SPButton *button;
	button = (SPButton *) data;
	if (button->type != SP_BUTTON_TYPE_TOGGLE) {
		return;
	}

	/* temporarily lobotomized until SPActions are per-view */
	if (0 && !active != !SP_BUTTON_IS_DOWN (button)) {
		sp_button_toggle_set_down (button, active);
	}
}

static void
sp_button_action_set_sensitive (SPAction */*action*/, unsigned int sensitive, void *data)
{
	gtk_widget_set_sensitive (GTK_WIDGET (data), sensitive);
}

static void
sp_button_action_set_shortcut (SPAction *action, unsigned int /*shortcut*/, void *data)
{
	SPButton *button=SP_BUTTON (data);
	if (button->tooltips) {
		sp_button_set_composed_tooltip (button->tooltips, GTK_WIDGET (button), action);
	}
}

static void
sp_button_set_composed_tooltip (GtkTooltips *tooltips, GtkWidget *widget, SPAction *action)
{
	if (action) {
		unsigned int shortcut = sp_shortcut_get_primary (action->verb);
		if (shortcut) {
			// there's both action and shortcut

			gchar        key[256];
			sp_ui_shortcut_string (shortcut, key);

			gchar *tip = g_strdup_printf ("%s (%s)", action->tip, key);
			gtk_tooltips_set_tip (tooltips, widget, tip, NULL);
			g_free (tip);

		} else {
			// action has no shortcut
			gtk_tooltips_set_tip (tooltips, widget, action->tip, NULL);
		}
	} else {
		// no action
		gtk_tooltips_set_tip (tooltips, widget, NULL, NULL);
	}
}

GtkWidget *
sp_button_new_from_data( Inkscape::IconSize size,
			 SPButtonType type,
			 Inkscape::UI::View::View *view,
			 const gchar *name,
			 const gchar *tip,
			 GtkTooltips *tooltips )
{
	GtkWidget *button;
	SPAction *action=sp_action_new(view, name, name, tip, name, 0);
	button = sp_button_new (size, type, action, NULL, tooltips);
	nr_object_unref ((NRObject *) action);
	return button;
}

