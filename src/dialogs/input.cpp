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
#include <list>
#include <set>

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

#define noTEST_WITH_GOOD_TABLET 1
#define noTEST_WITH_BAD_TABLET 1

#if defined(TEST_WITH_GOOD_TABLET) || defined(TEST_WITH_BAD_TABLET)
static int testDeviceCount = 0;
static GdkDevice* testDevices = 0;

// Defined at the end of the file to keep debugging out of the way.
static void initTestDevices();
#endif

static std::list<GdkDevice *> getInputDevices()
{
    std::list<GdkDevice*> devices;

#if defined(TEST_WITH_GOOD_TABLET) || defined(TEST_WITH_BAD_TABLET)
    initTestDevices();
    for (int i = 0; i < testDeviceCount; i++) {
        devices.push_back(&testDevices[i]);
    }
#else
    for (GList *ptr = gdk_devices_list(); ptr; ptr = ptr->next) {
        GdkDevice *device = static_cast<GdkDevice *>(ptr->data);
        devices.push_back(device);
    }
#endif

    return devices;
}

// wrap these GDK calls to be able to intercept for testing.

static bool setDeviceMode( GdkDevice *device, GdkInputMode mode )
{
#if defined(TEST_WITH_GOOD_TABLET) || defined(TEST_WITH_BAD_TABLET)
    (void)device;
    (void)mode;
    bool retVal = true; // Can't let the Gdk call be called with bad data
#else
    bool retVal = gdk_device_set_mode(device, mode);
#endif
    return retVal;
}

static void setDeviceAxisUse( GdkDevice *device, guint index, GdkAxisUse use )
{
#if defined(TEST_WITH_GOOD_TABLET) && !defined(TEST_WITH_BAD_TABLET)
    (void)device;
    (void)index;
    (void)use;
#else
    gdk_device_set_axis_use(device, index, use);
#endif
}

static void setDeviceKey( GdkDevice* device, guint index, guint keyval, GdkModifierType modifiers )
{
#if defined(TEST_WITH_GOOD_TABLET) && !defined(TEST_WITH_BAD_TABLET)
    (void)device;
    (void)index;
    (void)keyval;
    (void)modifiers;
#else
    gdk_device_set_key(device, index, keyval, modifiers);
#endif
}


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

static gchar const *axis_use_strings[GDK_AXIS_LAST] = {
    "ignore", "x", "y", "pressure", "xtilt", "ytilt", "wheel"
};

static const int RUNAWAY_MAX = 1000;

static Glib::ustring getBaseDeviceName(GdkInputSource source)
{
    Glib::ustring name;
    switch (source) {
        case GDK_SOURCE_MOUSE:
            name ="pointer";
            break;
        case GDK_SOURCE_PEN:
            name ="pen";
            break;
        case GDK_SOURCE_ERASER:
            name ="eraser";
            break;
        case GDK_SOURCE_CURSOR:
            name ="cursor";
            break;
        default:
            name = "tablet";
    }
    return name;
}

static Glib::ustring createSanitizedPath(GdkDevice* device, std::set<Glib::ustring> &seenPaths)
{
    // LP #334800: tablet device names on Windows sometimes contain funny junk like
    // \x03, \xf2, etc. Moreover this junk changes between runs.
    // If the tablet name contains unprintable or non-ASCII characters,
    // we use some default name.
    // This might break if someone has two tablets with broken names, but it's
    // not possible to do anything 100% correct then.
    bool broken = false;

    if (!device->name || (*(device->name) == 0)) {
        broken = true;
    } else {
        for (gchar const *s = device->name; *s; ++s) {
            if ((*s < 0x20) || (*s >= 0x7f)) {
                broken = true;
                break;
            }
        }
    }

    Glib::ustring device_path;
    if (broken) {
        Glib::ustring base = Glib::ustring("/devices/") + getBaseDeviceName(device->source);
        int num = 1;
        device_path = base;
        while ((seenPaths.find(device_path) != seenPaths.end()) && (num < RUNAWAY_MAX)) {
            device_path = Glib::ustring::compose("%1%2", base, ++num);
        }
    } else {
        device_path += Glib::ustring("/devices/") + device->name;
    }

    seenPaths.insert(device_path);

    return device_path;
}

void sp_input_load_from_preferences(void)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    std::list<GdkDevice *> devices = getInputDevices();
    std::set<Glib::ustring> seenPaths;
    for (std::list<GdkDevice *>::iterator it = devices.begin(); it != devices.end(); ++it) {
        GdkDevice *device = *it;

//         g_message("    s:%d m:%d hc:%d a:%d k:%d [%s]", device->source, device->mode, device->has_cursor, device->num_axes, device->num_keys, device->name);
//         for (int i = 0; i < device->num_axes; i++) {
//             GdkDeviceAxis &axis = device->axes[i];
//             g_message("        axis[%d] u:%d  min:%f max:%f", i, axis.use, axis.min, axis.max);
//         }

        Glib::ustring device_path = createSanitizedPath(device, seenPaths);
//         if (device_path != (Glib::ustring("/devices/") + device->name)) {
//             g_message("        re-name [%s]", device_path.c_str());
//         }

        Glib::ustring device_mode = prefs->getString(device_path + "/mode");

        GdkInputMode mode = GDK_MODE_DISABLED;
        if (device_mode == "screen") {
            mode = GDK_MODE_SCREEN;
        } else if (device_mode == "window") {
            mode = GDK_MODE_WINDOW;
        }

        if (device->mode != mode) {
            setDeviceMode(device, mode);
        }

        Glib::ustring::size_type pos0, pos1;
        GdkAxisUse axis_use;

        //temp_ptr = repr->attribute("axes");
        Glib::ustring const axes_str = prefs->getString(device_path + "/axes");
        pos0 = pos1 = 0;
        for (gint i=0; i < device->num_axes; i++) {
            pos1 = axes_str.find(';', pos0);
            if (pos1 == Glib::ustring::npos) {
                break;  // Too few axis specifications
            }

            axis_use = GDK_AXIS_IGNORE;
            for (gint j=0; j < GDK_AXIS_LAST; j++) {
                if (!strcmp(axes_str.substr(pos0, pos1-pos0).c_str(), axis_use_strings[j])) {
                    axis_use = static_cast<GdkAxisUse>(j);
                    break;
                }
            }
            setDeviceAxisUse(device, i, axis_use);
            pos0 = pos1 + 1;
        }

        guint keyval;
        GdkModifierType modifier;

        Glib::ustring const keys_str = prefs->getString(device_path + "/keys");
        pos0 = pos1 = 0;
        for (gint i=0; i < device->num_keys; i++) {
            pos1 = keys_str.find(';', pos0);
            if (pos1 == Glib::ustring::npos) {
                break;  // Too few key specifications
            }

            gtk_accelerator_parse(keys_str.substr(pos0, pos1-pos0).c_str(), &keyval, &modifier);
            setDeviceKey(device, i, keyval, modifier);
            pos0 = pos1 + 1;
        }
    }
}

void sp_input_save_to_preferences(void)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();

    std::list<GdkDevice *> devices = getInputDevices();
    std::set<Glib::ustring> seenPaths;
    for (std::list<GdkDevice *>::iterator it = devices.begin(); it != devices.end(); ++it) {
        GdkDevice *device = *it;

        Glib::ustring device_path = createSanitizedPath(device, seenPaths);

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

// /////////////////////////////////
// For debugging:
// /////////////////////////////////


#if defined(TEST_WITH_GOOD_TABLET)
static void initTestDevices()
{
    static bool init = false;
    if (!init) {
        static GdkDevice devs[5] = {};
        int i = 0; // use variable instead of constant to allow devices to be moved around, commented out, etc.

        {
            // Laptop trackpad
            devs[i].name = g_strdup("pointer");
            devs[i].source = GDK_SOURCE_MOUSE;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Tablet stylus
            devs[i].name = g_strdup("pen");
            devs[i].source = GDK_SOURCE_PEN;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0},
                                          {GDK_AXIS_PRESSURE, 0, 1},
                                          {GDK_AXIS_XTILT, -1, 1},
                                          {GDK_AXIS_YTILT, -1, 1}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Puck
            devs[i].name = g_strdup("cursor");
            devs[i].source = GDK_SOURCE_CURSOR;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0},
                                          {GDK_AXIS_PRESSURE, 0, 1},
                                          {GDK_AXIS_XTILT, -1, 1},
                                          {GDK_AXIS_YTILT, -1, 1}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Back of tablet stylus
            devs[i].name = g_strdup("eraser");
            devs[i].source = GDK_SOURCE_ERASER;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0},
                                          {GDK_AXIS_PRESSURE, 0, 1},
                                          {GDK_AXIS_XTILT, -1, 1},
                                          {GDK_AXIS_YTILT, -1, 1}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Main (composit) mouse device
            devs[i].name = g_strdup("Core Pointer");
            devs[i].source = GDK_SOURCE_MOUSE;
            devs[i].mode = GDK_MODE_SCREEN;
            devs[i].has_cursor = 1;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        testDeviceCount = i;
        testDevices = devs;
        init = true;
    }
}
#elif defined(TEST_WITH_BAD_TABLET)

/**
 * Uses the current time in seconds to change a name to be unique from one
 * run of the program to the next.
 */
void perturbName(gchar *str)
{
    if (str) {
        GTimeVal when = {0,0};
        g_get_current_time(&when);
        gchar *tmp = g_strdup_printf("%ld", when.tv_sec);

        size_t partLen = strlen(tmp);
        size_t len = strlen(str);
        if (len > (partLen + 4)) {
            size_t pos = (len - partLen) / 2;
            for (size_t i = 0; i < partLen; i++) {
                str[pos + i] = tmp[i];
            }
        }
        g_free(tmp);
    }
}

static void initTestDevices()
{
    static bool init = false;
    if (!init) {
        static GdkDevice devs[5] = {};
        int i = 0; // use variable instead of constant to allow devices to be moved around, commented out, etc.

        {
            // Main (composit) mouse device
            devs[i].name = g_strdup("Core Pointer");
            devs[i].source = GDK_SOURCE_MOUSE;
            devs[i].mode = GDK_MODE_SCREEN;
            devs[i].has_cursor = 1;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Back of tablet stylus
            devs[i].name = g_strdup("\346\205\227\347\221\254\347\201\257\345\220\240\346\211\241\346\225\254t\303\265\006 \347\211\220\347\215\245\347\225\263\346\225\262\345\214\240\347\245\264\347\225\254s\357\227\230#\354\234\274C\356\232\210\307\255\350\271\214\310\201\350\222\200\310\201\356\202\250\310\200\350\223\260\310\201\356\202\250\310\200");
            perturbName(devs[i].name);
            devs[i].source = GDK_SOURCE_ERASER;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0},
                                          {GDK_AXIS_PRESSURE, 0, 1},
                                          {GDK_AXIS_XTILT, -1, 1},
                                          {GDK_AXIS_YTILT, -1, 1}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Tablet stylus
            devs[i].name = g_strdup("\346\205\227\347\221\254\347\201\257\345\220\240\346\211\241\346\225\254t\303\265\006 \347\211\220\347\215\245\347\225\263\346\225\262\345\214\240\347\245\264\347\225\254s\357\227\230#\354\234\274C\341\221\230\307\255\343\277\214\310\202\343\230\200\310\202\331\270\310\202\343\231\260\310\202\331\270\310\202");
            perturbName(devs[i].name);
            devs[i].source = GDK_SOURCE_PEN;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0},
                                          {GDK_AXIS_PRESSURE, 0, 1},
                                          {GDK_AXIS_XTILT, -1, 1},
                                          {GDK_AXIS_YTILT, -1, 1}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        {
            // Tablet stylus
            devs[i].name = g_strdup("\346\205\227\347\221\254\347\201\257\345\220\240\346\211\241\346\225\254t\303\265\006 \347\211\220\347\215\245\347\225\263\346\225\262\345\214\240\347\245\264\347\225\254s\357\227\230#\354\234\274C\341\221\230\307\255\343\277\214\310\202\343\230\200\310\202\331\270\310\202\343\231\260\310\202\331\270\310\202");
            perturbName(devs[i].name);
            devs[i].source = GDK_SOURCE_PEN;
            devs[i].mode = GDK_MODE_DISABLED;
            devs[i].has_cursor = 0;
            static GdkDeviceAxis tmp[] = {{GDK_AXIS_X, 0, 0},
                                          {GDK_AXIS_Y, 0, 0},
                                          {GDK_AXIS_PRESSURE, 0, 1},
                                          {GDK_AXIS_XTILT, -1, 1},
                                          {GDK_AXIS_YTILT, -1, 1}};
            devs[i].num_axes = G_N_ELEMENTS(tmp);
            devs[i].axes = tmp;
            devs[i].num_keys = 0;
            devs[i].keys = 0;
            i++;
        }

        testDeviceCount = i;
        testDevices = devs;
        init = true;
    }
}
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
