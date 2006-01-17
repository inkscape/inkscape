#define __SP_INPUT_C__

/*
 * Extended input devices dialog
 *
 * Authors:
 *   Nicklas Lindgren <nili@lysator.liu.se>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <gtk/gtksignal.h>
#include <gtk/gtkinputdialog.h>
#include <glibmm/ustring.h>

#include "../inkscape.h"
#include "../macros.h"
#include "../verbs.h"
#include "../interface.h"
#include "../xml/repr.h"

#include "dialog-events.h"
#include "../prefs-utils.h"


static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static gchar *prefs_path = "dialogs.input";

static void
sp_input_dialog_destroy (GtkObject *object, gpointer data)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_input_dialog_delete (GtkObject *object, GdkEvent *event, gpointer data)
{
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    prefs_set_int_attribute (prefs_path, "x", x);
    prefs_set_int_attribute (prefs_path, "y", y);
    prefs_set_int_attribute (prefs_path, "w", w);
    prefs_set_int_attribute (prefs_path, "h", h);

    return FALSE; // which means, go ahead and destroy it

}

static gchar *axis_use_strings[GDK_AXIS_LAST] = {
    "ignore", "x", "y", "pressure", "xtilt", "ytilt", "wheel"
};

void
sp_input_load_from_preferences (void)
{
    Inkscape::XML::Node *devices = inkscape_get_repr(INKSCAPE, "devices");
    if (devices == NULL)
        return;

    Inkscape::XML::Node *repr;
    GList *list_ptr;

    for (list_ptr = gdk_devices_list(); list_ptr != NULL; list_ptr = list_ptr->next) {
        GdkDevice *device = static_cast<GdkDevice *>(list_ptr->data);
        repr = sp_repr_lookup_child(devices, "id", device->name);
        if (repr != NULL) {
            GdkInputMode mode;
            const gchar *attribute = repr->attribute("mode");

            if (attribute == NULL)
                mode = GDK_MODE_DISABLED;
            else if (!strcmp(attribute, "screen"))
                mode = GDK_MODE_SCREEN;
            else if (!strcmp(attribute, "window"))
                mode = GDK_MODE_WINDOW;
            else
                mode = GDK_MODE_DISABLED;

            if (device->mode != mode) {
                gdk_device_set_mode(device, mode);
            }

            const gchar *temp_ptr;
            Glib::ustring::size_type pos0;
            Glib::ustring::size_type pos1;
            gint i;
            gint j;

            GdkAxisUse axis_use;

            temp_ptr = repr->attribute("axes");
            if (temp_ptr != NULL) {
                const Glib::ustring temp_str = temp_ptr;
                pos0 = pos1 = 0;
                for (i=0; i < device->num_axes; i++) {
                    pos1 = temp_str.find(";", pos0);
                    if (pos1 == Glib::ustring::npos)
                        break;  // Too few axis specifications

                    axis_use = GDK_AXIS_IGNORE;
                    for (j=0; j < GDK_AXIS_LAST; j++)
                        if (!strcmp(temp_str.substr(pos0, pos1-pos0).c_str(), axis_use_strings[j])) {
                            axis_use = static_cast<GdkAxisUse>(j);
                            break;
                        }
                    gdk_device_set_axis_use(device, i, axis_use);
                    pos0 = pos1 + 1;
                }
            }

            guint keyval;
            GdkModifierType modifier;

            temp_ptr = repr->attribute("keys");
            if (temp_ptr != NULL) {
                const Glib::ustring temp_str = temp_ptr;
                pos0 = pos1 = 0;
                for (i=0; i < device->num_keys; i++) {
                    pos1 = temp_str.find(";", pos0);
                    if (pos1 == Glib::ustring::npos)
                        break;  // Too few key specifications

                    gtk_accelerator_parse(temp_str.substr(pos0, pos1-pos0).c_str(), &keyval, &modifier);
                    gdk_device_set_key(device, i, keyval, modifier);
                    pos0 = pos1 + 1;
                }
            }
        }
    }
}

void
sp_input_save_to_preferences (void)
{
    Inkscape::XML::Node *devices = inkscape_get_repr(INKSCAPE, "devices");
    if (devices == NULL)
        // TODO: find a clean way to add a node to the preferences root, or
        // give an error message
        return;

    Inkscape::XML::Node *repr;
    GList *list_ptr;

    for (list_ptr = gdk_devices_list(); list_ptr != NULL; list_ptr = list_ptr->next) {
        gint i;
        Glib::ustring temp_attribute;
        GdkDevice *device = static_cast<GdkDevice *>(list_ptr->data);

        repr = sp_repr_lookup_child(devices, "id", device->name);
        if (repr == NULL) {
            repr = sp_repr_new("group");
            repr->setAttribute("id", device->name);
            devices->appendChild(repr);
            Inkscape::GC::release(repr);
        }
        switch (device->mode) {
            default:
            case GDK_MODE_DISABLED: {
                repr->setAttribute("mode", "disabled");
                break;
            }
            case GDK_MODE_SCREEN: {
                repr->setAttribute("mode", "screen");
                break;
            }
            case GDK_MODE_WINDOW: {
                repr->setAttribute("mode", "window");
                break;
            }
        }

        temp_attribute = "";
        for (i=0; i < device->num_axes; i++) {
            temp_attribute += axis_use_strings[device->axes[i].use];
            temp_attribute += ";";
        }
        repr->setAttribute("axes", temp_attribute.c_str());

        temp_attribute = "";
        for (i=0; i < device->num_keys; i++) {
            temp_attribute += gtk_accelerator_name(device->keys[i].keyval, device->keys[i].modifiers);
            temp_attribute += ";";
        }
        repr->setAttribute("keys", temp_attribute.c_str());
    }
}

static void
sp_input_save_button (GtkObject *object, gpointer data)
{
    sp_input_save_to_preferences();
}

/**
 * \brief  Dialog
 *
 */
void
sp_input_dialog (void)
{
    if (dlg == NULL) {

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_INPUT), title);

        dlg = gtk_input_dialog_new();

        if (x == -1000 || y == -1000) {
            x = prefs_get_int_attribute (prefs_path, "x", 0);
            y = prefs_get_int_attribute (prefs_path, "y", 0);
        }

        if (w ==0 || h == 0) {
            w = prefs_get_int_attribute (prefs_path, "w", 0);
            h = prefs_get_int_attribute (prefs_path, "h", 0);
        }

        if (x != 0 || y != 0) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
        }

        if (w && h) {
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        }

        sp_transientize (dlg);
        wd.win = dlg;
        wd.stop = 0;

        g_signal_connect   ( G_OBJECT (INKSCAPE), "activate_desktop", G_CALLBACK (sp_transientize_callback), &wd);
        gtk_signal_connect ( GTK_OBJECT (dlg), "event", GTK_SIGNAL_FUNC (sp_dialog_event_handler), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "destroy", G_CALLBACK (sp_input_dialog_destroy), dlg);
        gtk_signal_connect ( GTK_OBJECT (dlg), "delete_event", G_CALLBACK (sp_input_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "shut_down", G_CALLBACK (sp_input_dialog_delete), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_hide", G_CALLBACK (sp_dialog_hide), dlg);
        g_signal_connect   ( G_OBJECT (INKSCAPE), "dialogs_unhide", G_CALLBACK (sp_dialog_unhide), dlg);

        // Dialog-specific stuff
        gtk_signal_connect_object (GTK_OBJECT(GTK_INPUT_DIALOG(dlg)->close_button),
                                   "clicked",
                                   (GtkSignalFunc)gtk_widget_destroy,
                                   GTK_OBJECT(dlg));
        gtk_signal_connect (GTK_OBJECT(GTK_INPUT_DIALOG(dlg)->save_button),
                            "clicked",
                            (GtkSignalFunc)sp_input_save_button, NULL);
    }

    gtk_window_present ((GtkWindow *) dlg);
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
