/**
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

#include <glibmm.h>
#include "icon.h"

#include "button.h"
#include "helper/action-context.h"
#include "ui/interface.h"
#include "shortcuts.h"
#include "helper/action.h"

#include <gdk/gdkkeysyms.h>

static void sp_button_dispose(GObject *object);

#if GTK_CHECK_VERSION(3, 0, 0)
static void sp_button_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width);

static void sp_button_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height);
#else
static void sp_button_size_request(GtkWidget *widget, GtkRequisition *requisition);
#endif

static void sp_button_clicked(GtkButton *button);
static void sp_button_perform_action(SPButton *button, gpointer data);
static gint sp_button_process_event(SPButton *button, GdkEvent *event);

static void sp_button_set_action(SPButton *button, SPAction *action);
static void sp_button_set_doubleclick_action(SPButton *button, SPAction *action);
static void sp_button_action_set_active(SPButton *button, bool active);
static void sp_button_set_composed_tooltip(GtkWidget *widget, SPAction *action);

G_DEFINE_TYPE(SPButton, sp_button, GTK_TYPE_TOGGLE_BUTTON);

static void sp_button_class_init(SPButtonClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
    GtkButtonClass *button_class = GTK_BUTTON_CLASS(klass);

    object_class->dispose = sp_button_dispose;
#if GTK_CHECK_VERSION(3, 0, 0)
    widget_class->get_preferred_width = sp_button_get_preferred_width;
    widget_class->get_preferred_height = sp_button_get_preferred_height;
#else
    widget_class->size_request = sp_button_size_request;
#endif
    button_class->clicked = sp_button_clicked;
}

static void sp_button_init(SPButton *button)
{
    button->action = NULL;
    button->doubleclick_action = NULL;
    new (&button->c_set_active) sigc::connection();
    new (&button->c_set_sensitive) sigc::connection();

    gtk_container_set_border_width(GTK_CONTAINER(button), 0);

    gtk_widget_set_can_focus(GTK_WIDGET(button), FALSE);
    gtk_widget_set_can_default(GTK_WIDGET(button), FALSE);

    g_signal_connect_after(G_OBJECT(button), "clicked", G_CALLBACK(sp_button_perform_action), NULL);
    g_signal_connect_after(G_OBJECT(button), "event", G_CALLBACK(sp_button_process_event), NULL);
}

static void sp_button_dispose(GObject *object)
{
    SPButton *button = SP_BUTTON(object);

    if (button->action) {
        sp_button_set_action(button, NULL);
    }
    if (button->doubleclick_action) {
        sp_button_set_doubleclick_action(button, NULL);
    }

    button->c_set_active.~connection();
    button->c_set_sensitive.~connection();

    (G_OBJECT_CLASS(sp_button_parent_class))->dispose(object);
}

#if GTK_CHECK_VERSION(3, 0, 0)
static void sp_button_get_preferred_width(GtkWidget *widget, gint *minimal_width, gint *natural_width)
{
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

    if (child) {
        gtk_widget_get_preferred_width(GTK_WIDGET(child), minimal_width, natural_width);
    } else {
        *minimal_width = 0;
        *natural_width = 0;
    }

    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkBorder padding;
    GtkBorder border;

    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);
    gtk_style_context_get_border( context, GTK_STATE_FLAG_NORMAL, &border );

    *minimal_width += MAX(2, padding.left + padding.right + border.left + border.right);
    *natural_width += MAX(2, padding.left + padding.right + border.left + border.right);
}

static void sp_button_get_preferred_height(GtkWidget *widget, gint *minimal_height, gint *natural_height)
{
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));

    if (child) {
        gtk_widget_get_preferred_height(GTK_WIDGET(child), minimal_height, natural_height);
    } else {
        *minimal_height = 0;
        *natural_height = 0;
    }

    GtkStyleContext *context = gtk_widget_get_style_context (widget);
    GtkBorder padding;
    GtkBorder border;

    gtk_style_context_get_padding(context, GTK_STATE_FLAG_NORMAL, &padding);
    gtk_style_context_get_border( context, GTK_STATE_FLAG_NORMAL, &border );

    *minimal_height += MAX(2, padding.top + padding.bottom + border.top + border.bottom);
    *natural_height += MAX(2, padding.top + padding.bottom + border.top + border.bottom);
}
#else
static void sp_button_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    GtkWidget *child = gtk_bin_get_child(GTK_BIN(widget));
    GtkStyle *style = gtk_widget_get_style(widget);

    if (child) {
        gtk_widget_size_request(GTK_WIDGET(child), requisition);
    } else {
        requisition->width = 0;
        requisition->height = 0;
    }

    requisition->width += 2 + 2 * MAX(2, style->xthickness);
    requisition->height += 2 + 2 * MAX(2, style->ythickness);
}
#endif

static void sp_button_clicked(GtkButton *button)
{
    SPButton *sp_button = SP_BUTTON(button);

    if (sp_button->type == SP_BUTTON_TYPE_TOGGLE) {
        (GTK_BUTTON_CLASS(sp_button_parent_class))->clicked(button);
    }
}

static gint sp_button_process_event(SPButton *button, GdkEvent *event)
{
    switch (event->type) {
    case GDK_2BUTTON_PRESS:
        if (button->doubleclick_action) {
            sp_action_perform(button->doubleclick_action, NULL);
        }
        return TRUE;
        break;
    default:
        break;
    }

    return FALSE;
}

static void sp_button_perform_action(SPButton *button, gpointer /*data*/)
{
    if (button->action) {
        sp_action_perform(button->action, NULL);
    }
}

GtkWidget *sp_button_new(Inkscape::IconSize size, SPButtonType type, SPAction *action, SPAction *doubleclick_action)
{
    SPButton *button = SP_BUTTON(g_object_new(SP_TYPE_BUTTON, NULL));

    button->type = type;
    button->lsize = CLAMP(size, Inkscape::ICON_SIZE_MENU, Inkscape::ICON_SIZE_DECORATION);

    sp_button_set_action(button, action);
    if (doubleclick_action)
        sp_button_set_doubleclick_action(button, doubleclick_action);

    // The Inkscape style is no-relief buttons
    gtk_button_set_relief(GTK_BUTTON(button), GTK_RELIEF_NONE);

    return GTK_WIDGET(button);
}

void sp_button_toggle_set_down(SPButton *button, gboolean down)
{
    g_return_if_fail(button->type == SP_BUTTON_TYPE_TOGGLE);
    g_signal_handlers_block_by_func(G_OBJECT(button), (gpointer)G_CALLBACK(sp_button_perform_action), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), (unsigned int)down);
    g_signal_handlers_unblock_by_func(G_OBJECT(button), (gpointer)G_CALLBACK(sp_button_perform_action), NULL);
}

static void sp_button_set_doubleclick_action(SPButton *button, SPAction *action)
{
    if (button->doubleclick_action) {
        g_object_unref(button->doubleclick_action);
    }
    button->doubleclick_action = action;
    if (action) {
        g_object_ref(action);
    }
}

static void sp_button_set_action(SPButton *button, SPAction *action)
{
    GtkWidget *child;

    if (button->action) {
        button->c_set_active.disconnect();
        button->c_set_sensitive.disconnect();
        child = gtk_bin_get_child(GTK_BIN(button));
        if (child) {
            gtk_container_remove(GTK_CONTAINER(button), child);
        }
        g_object_unref(button->action);
    }
    button->action = action;
    if (action) {
        g_object_ref(action);
        button->c_set_active = action->signal_set_active.connect(
            sigc::bind<0>(sigc::ptr_fun(&sp_button_action_set_active), SP_BUTTON(button)));
        button->c_set_sensitive = action->signal_set_sensitive.connect(
            sigc::bind<0>(sigc::ptr_fun(&gtk_widget_set_sensitive), GTK_WIDGET(button)));
        if (action->image) {
            child = sp_icon_new(button->lsize, action->image);
            gtk_widget_show(child);
            gtk_container_add(GTK_CONTAINER(button), child);
        }
    }

    sp_button_set_composed_tooltip(GTK_WIDGET(button), action);
}

static void sp_button_action_set_active(SPButton *button, bool active)
{
    if (button->type != SP_BUTTON_TYPE_TOGGLE) {
        return;
    }

    /* temporarily lobotomized until SPActions are per-view */
    if (0 && !active != !SP_BUTTON_IS_DOWN(button)) {
        sp_button_toggle_set_down(button, active);
    }
}

static void sp_button_set_composed_tooltip(GtkWidget *widget, SPAction *action)
{
    if (action) {
        unsigned int shortcut = sp_shortcut_get_primary(action->verb);
        if (shortcut != GDK_KEY_VoidSymbol) {
            // there's both action and shortcut

            gchar *key = sp_shortcut_get_label(shortcut);

            gchar *tip = g_strdup_printf("%s (%s)", action->tip, key);
            gtk_widget_set_tooltip_text(widget, tip);
            g_free(tip);
            g_free(key);
        } else {
            // action has no shortcut
            gtk_widget_set_tooltip_text(widget, action->tip);
        }
    } else {
        // no action
        gtk_widget_set_tooltip_text(widget, NULL);
    }
}

GtkWidget *sp_button_new_from_data(Inkscape::IconSize size, SPButtonType type, Inkscape::UI::View::View *view,
                                   const gchar *name, const gchar *tip)
{
    GtkWidget *button;
    SPAction *action = sp_action_new(Inkscape::ActionContext(view), name, name, tip, name, 0);
    button = sp_button_new(size, type, action, NULL);
    g_object_unref(action);
    return button;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8 :
