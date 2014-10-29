/*
 * Inkscape::Debug::GdkEventLatencyTracker - tracks backlog of GDK events
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2008 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gdk/gdk.h>

#include "debug/gdk-event-latency-tracker.h"
#include "preferences.h"

namespace Inkscape {
namespace Debug {

GdkEventLatencyTracker::GdkEventLatencyTracker()
    : start_seconds(0.0), max_latency(0.0), skew(1.0), last_elapsed(0.0), last_seconds(0.0)
{
    elapsed.stop();
    elapsed.reset();
}

boost::optional<double> GdkEventLatencyTracker::process(GdkEvent const *event) {
    guint32 const timestamp=gdk_event_get_time(const_cast<GdkEvent *>(event));
    if (timestamp == GDK_CURRENT_TIME) {
        return boost::optional<double>();
    }
    double const timestamp_seconds = timestamp / 1000.0;

    if (start_seconds == 0.0) {
        elapsed.start();
        start_seconds = timestamp_seconds;
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        skew = prefs->getDoubleLimited("/debug/latency/skew", 1.0, 0.5, 2.0);
        return boost::optional<double>(0.0);
    } else {
        last_elapsed = elapsed.elapsed();
        last_seconds = timestamp_seconds;
        double const current_seconds = (last_elapsed * skew) + start_seconds;
        double delta = current_seconds - timestamp_seconds;
        if (delta < 0.0) {
            start_seconds += -delta;
            delta = 0.0;
        } else if (delta > max_latency) {
            max_latency = delta;
        }
        return boost::optional<double>(delta);
    }
}

double GdkEventLatencyTracker::getSkew() {
    double val = 0.0;
    if ((last_elapsed > 0.0) && (last_seconds > 0.0)) {
        val = (last_seconds - start_seconds) / last_elapsed;
    }
    return val;
}

GdkEventLatencyTracker &GdkEventLatencyTracker::default_tracker() {
    static GdkEventLatencyTracker tracker;
    return tracker;
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
