/*
 * Inkscape::Debug::log_display_config - log display configuration
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2007 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <iostream>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include "debug/event-tracker.h"
#include "debug/logger.h"
#include "debug/simple-event.h"
#include "debug/log-display-config.h"

namespace Inkscape {

namespace Debug {

namespace {

typedef SimpleEvent<Event::CONFIGURATION> ConfigurationEvent;

class Monitor : public ConfigurationEvent {
public:
    Monitor(GdkScreen *screen, gint monitor) : ConfigurationEvent("monitor") {
        GdkRectangle area;
        gdk_screen_get_monitor_geometry(screen, monitor, &area);
        _addProperty("x", area.x);
        _addProperty("y", area.y);
        _addProperty("width", area.width);
        _addProperty("height", area.height);
    }
};

class Screen : public ConfigurationEvent {
public:
    Screen(GdkScreen *s) : ConfigurationEvent("screen"), screen(s) {
        _addProperty("width", gdk_screen_get_width(screen));
        _addProperty("height", gdk_screen_get_height(screen));
    }
    void generateChildEvents() const {
        gint n_monitors = gdk_screen_get_n_monitors(screen);
        for ( gint i = 0 ; i < n_monitors ; i++ ) {
            Logger::write<Monitor>(screen, i);
        }
    }

private:
    GdkScreen *screen;
};

class Display : public ConfigurationEvent {
public:
    Display() : ConfigurationEvent("display") {}
    void generateChildEvents() const {
        GdkDisplay *display=gdk_display_get_default();
#if GTK_CHECK_VERSION(3,10,0)
        GdkScreen *screen = gdk_display_get_screen(display, 0);
        Logger::write<Screen>(screen);
#else
        gint n_screens = gdk_display_get_n_screens(display);
        for ( gint i = 0 ; i < n_screens ; i++ ) {
            GdkScreen *screen = gdk_display_get_screen(display, i);
            Logger::write<Screen>(screen);
        }
#endif
    }
};

}

void log_display_config() {
    Logger::write<Display>();
}

}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
