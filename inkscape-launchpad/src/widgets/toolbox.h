#ifndef SEEN_TOOLBOX_H
#define SEEN_TOOLBOX_H

/*
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Frank Felfe <innerspace@iname.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 1999-2002 Authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm/ustring.h>

#include "icon-size.h"
#include "preferences.h"

#define TOOLBAR_SLIDER_HINT "full"

typedef struct _EgeAdjustmentAction      EgeAdjustmentAction;

class SPDesktop;

namespace Inkscape {
namespace UI {
namespace Tools {

class ToolBase;

}
}
}

namespace Inkscape {
namespace UI {

namespace Widget {
    class UnitTracker;
}

/**
 * Main toolbox source.
 */
class ToolboxFactory
{
public:
    static void setToolboxDesktop(GtkWidget *toolbox, SPDesktop *desktop);
    static void setOrientation(GtkWidget* toolbox, GtkOrientation orientation);
    static void showAuxToolbox(GtkWidget* toolbox);

    static GtkWidget *createToolToolbox();
    static GtkWidget *createAuxToolbox();
    static GtkWidget *createCommandsToolbox();
    static GtkWidget *createSnapToolbox();


    static Glib::ustring getToolboxName(GtkWidget* toolbox);

    static void updateSnapToolbox(SPDesktop *desktop, Inkscape::UI::Tools::ToolBase *eventcontext, GtkWidget *toolbox);

    static Inkscape::IconSize prefToSize(Glib::ustring const &path, int base = 0 );

private:
    ToolboxFactory();
};

/**
 * A simple mediator class that keeps UI controls matched to the preference values they set.
 */
class PrefPusher : public Inkscape::Preferences::Observer
{
public:
    /**
     * Constructor for a boolean value that syncs to the supplied path.
     * Initializes the widget to the current preference stored state and registers callbacks
     * for widget changes and preference changes.
     *
     * @param act the widget to synchronize preference with.
     * @param path the path to the preference the widget is synchronized with.
     * @param callback function to invoke when changes are pushed.
     * @param cbData data to be passed on to the callback function.
     */
    PrefPusher( GtkToggleAction *act, Glib::ustring const &path, void (*callback)(GObject*) = 0, GObject *cbData = 0 );

    /**
     * Destructor that unregisters the preference callback.
     */
    virtual ~PrefPusher();

    /**
     * Callback method invoked when the preference setting changes.
     */
    virtual void notify(Inkscape::Preferences::Entry const &new_val);


private:
    /**
     * Callback hook invoked when the widget changes.
     *
     * @param act the toggle action widget that was changed.
     * @param self the PrefPusher instance the callback was registered to.
     */
   static void toggleCB( GtkToggleAction *act, PrefPusher *self );

    /**
     * Method to handle the widget change.
     */
    void handleToggled();

    GtkToggleAction *act;
    void (*callback)(GObject*);
    GObject *cbData;
    bool freeze;
};


} // namespace UI
} // namespace Inkscape


// utility

void delete_prefspusher(GObject * /*obj*/, Inkscape::UI::PrefPusher *watcher );
void purge_repr_listener( GObject* /*obj*/, GObject* tbl );
void delete_connection(GObject * /*obj*/, sigc::connection *connection);

 EgeAdjustmentAction * create_adjustment_action( gchar const *name,
                                                       gchar const *label, gchar const *shortLabel, gchar const *tooltip,
                                                       Glib::ustring const &path, gdouble def,
                                                       GtkWidget *focusTarget,
                                                       GObject *dataKludge,
                                                       gboolean altx, gchar const *altx_mark,
                                                       gdouble lower, gdouble upper, gdouble step, gdouble page,
                                                       gchar const** descrLabels, gdouble const* descrValues, guint descrCount,
                                                       void (*callback)(GtkAdjustment *, GObject *),
                                                       Inkscape::UI::Widget::UnitTracker *unit_tracker = NULL,
                                                       gdouble climb = 0.1, guint digits = 3, double factor = 1.0 );

#endif /* !SEEN_TOOLBOX_H */

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
