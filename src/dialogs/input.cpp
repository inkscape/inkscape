/** @file
 * @brief Extended input devices dialog
 */
/* Authors:
 *   Nicklas Lindgren <nili@lysator.liu.se>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <gtk/gtksignal.h>
#include <gtk/gtkinputdialog.h>
#include <glibmm/ustring.h>

#include "macros.h"
#include "verbs.h"
#include "inkscape.h"
#include "interface.h"
#include "xml/repr.h"

#include "dialogs/dialog-events.h"
#include "preferences.h"

#define MIN_ONSCREEN_DISTANCE 50

static GtkWidget *dlg = NULL;
static win_data wd;

// impossible original values to make sure they are read from prefs
static gint x = -1000, y = -1000, w = 0, h = 0;
static Glib::ustring const prefs_path = "/dialogs/input/";

static void
sp_input_dialog_destroy (GtkObject */*object*/, gpointer /*data*/)
{
    sp_signal_disconnect_by_data (INKSCAPE, dlg);
    wd.win = dlg = NULL;
    wd.stop = 0;
}

static gboolean
sp_input_dialog_delete (GtkObject */*object*/, GdkEvent */*event*/, gpointer /*data*/)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    gtk_window_get_position ((GtkWindow *) dlg, &x, &y);
    gtk_window_get_size ((GtkWindow *) dlg, &w, &h);

    if (x<0) x=0;
    if (y<0) y=0;

    prefs->setInt(prefs_path + "x", x);
    prefs->setInt(prefs_path + "y", y);
    prefs->setInt(prefs_path + "w", w);
    prefs->setInt(prefs_path + "h", h);

    return FALSE; // which means, go ahead and destroy it

}

static const gchar *axis_use_strings[GDK_AXIS_LAST] = {
    "ignore", "x", "y", "pressure", "xtilt", "ytilt", "wheel"
};

void
sp_input_load_from_preferences (void)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    for (GList *list_ptr = gdk_devices_list(); list_ptr != NULL; list_ptr = list_ptr->next) {
        GdkDevice *device = static_cast<GdkDevice *>(list_ptr->data);
        //repr = sp_repr_lookup_child(devices, "id", device->name);
        Glib::ustring device_path = Glib::ustring("/devices/") + device->name;
        if (/*repr != NULL*/ 1) {
            GdkInputMode mode;
            Glib::ustring device_mode = prefs->getString(device_path + "/mode");

            if (device_mode.empty())
                mode = GDK_MODE_DISABLED;
            else if (device_mode == "screen")
                mode = GDK_MODE_SCREEN;
            else if (device_mode == "window")
                mode = GDK_MODE_WINDOW;
            else
                mode = GDK_MODE_DISABLED;

            if (device->mode != mode) {
                gdk_device_set_mode(device, mode);
            }

            Glib::ustring::size_type pos0, pos1;
            GdkAxisUse axis_use;

            //temp_ptr = repr->attribute("axes");
            Glib::ustring const axes_str = prefs->getString(device_path + "/axes");
            pos0 = pos1 = 0;
            for (gint i=0; i < device->num_axes; i++) {
                pos1 = axes_str.find(';', pos0);
                if (pos1 == Glib::ustring::npos)
                    break;  // Too few axis specifications

                axis_use = GDK_AXIS_IGNORE;
                for (gint j=0; j < GDK_AXIS_LAST; j++)
                    if (!strcmp(axes_str.substr(pos0, pos1-pos0).c_str(), axis_use_strings[j])) {
                        axis_use = static_cast<GdkAxisUse>(j);
                        break;
                    }
                gdk_device_set_axis_use(device, i, axis_use);
                pos0 = pos1 + 1;
            }

            guint keyval;
            GdkModifierType modifier;

            Glib::ustring const keys_str = prefs->getString(device_path + "/keys");
            pos0 = pos1 = 0;
            for (gint i=0; i < device->num_keys; i++) {
                pos1 = keys_str.find(';', pos0);
                if (pos1 == Glib::ustring::npos)
                    break;  // Too few key specifications

                gtk_accelerator_parse(keys_str.substr(pos0, pos1-pos0).c_str(), &keyval, &modifier);
                gdk_device_set_key(device, i, keyval, modifier);
                pos0 = pos1 + 1;
            }
        }
    }
}

void
sp_input_save_to_preferences (void)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    for (GList *list_ptr = gdk_devices_list(); list_ptr != NULL; list_ptr = list_ptr->next) {
        GdkDevice *device = static_cast<GdkDevice *>(list_ptr->data);

        //repr = sp_repr_lookup_child(devices, "id", device->name);
        Glib::ustring device_path = Glib::ustring("/devices/") + device->name;

        switch (device->mode) {
            default:
            case GDK_MODE_DISABLED: {
                prefs->setString(device_path + "/mode", "disabled");
                break;
            }
            case GDK_MODE_SCREEN: {
                prefs->setString(device_path + "/mode", "screen");
                break;
            }
            case GDK_MODE_WINDOW: {
                prefs->setString(device_path + "/mode", "window");
                break;
            }
        }

        Glib::ustring temp_attribute = "";
        for (gint i=0; i < device->num_axes; i++) {
            temp_attribute += axis_use_strings[device->axes[i].use];
            temp_attribute += ";";
        }
        prefs->setString(device_path + "/axes", temp_attribute);

        temp_attribute = "";
        for (gint i=0; i < device->num_keys; i++) {
            temp_attribute += gtk_accelerator_name(device->keys[i].keyval, device->keys[i].modifiers);
            temp_attribute += ";";
        }
        prefs->setString(device_path + "/keys", temp_attribute);
    }
}

static void
sp_input_save_button (GtkObject */*object*/, gpointer /*data*/)
{
    sp_input_save_to_preferences();
}

void
sp_input_dialog (void)
{
    if (dlg == NULL) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();

        gchar title[500];
        sp_ui_dialog_title_string (Inkscape::Verb::get(SP_VERB_DIALOG_INPUT), title);

        dlg = gtk_input_dialog_new();

        if (x == -1000 || y == -1000) {
            x = prefs->getInt(prefs_path + "x", -1000);
            y = prefs->getInt(prefs_path + "y", -1000);
        }

        if (w ==0 || h == 0) {
            w = prefs->getInt(prefs_path + "w", 0);
            h = prefs->getInt(prefs_path + "h", 0);
        }

//        if (x<0) x=0;
//        if (y<0) y=0;

        if (w && h) {
            gtk_window_resize ((GtkWindow *) dlg, w, h);
        }
        if (x >= 0 && y >= 0 && (x < (gdk_screen_width()-MIN_ONSCREEN_DISTANCE)) && (y < (gdk_screen_height()-MIN_ONSCREEN_DISTANCE))) {
            gtk_window_move ((GtkWindow *) dlg, x, y);
        } else {
            gtk_window_set_position(GTK_WINDOW(dlg), GTK_WIN_POS_CENTER);
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
