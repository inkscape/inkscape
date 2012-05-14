/**
 * @file
 * Measure aux toolbar
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   bulia byak <buliabyak@users.sf.net>
 *   Frank Felfe <innerspace@iname.com>
 *   John Cliff <simarilius@yahoo.com>
 *   David Turner <novalis@gnu.org>
 *   Josh Andler <scislac@scislac.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Maximilian Albert <maximilian.albert@gmail.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Abhishek Sharma
 *   Kris De Gussem <Kris.DeGussem@gmail.com>
 *
 * Copyright (C) 2004 David Turner
 * Copyright (C) 2003 MenTaLguY
 * Copyright (C) 1999-2011 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "toolbox.h"
#include "measure-toolbar.h"

#include "../desktop.h"
#include "../desktop-handles.h"
#include "document-undo.h"
#include "../verbs.h"
#include "../inkscape.h"
#include "../selection-chemistry.h"
#include "../selection.h"
#include "../ege-adjustment-action.h"
#include "../ege-output-action.h"
#include "../ege-select-one-action.h"
#include "../ink-action.h"
#include "../ink-comboboxentry-action.h"
#include "../widgets/button.h"
#include "../widgets/spinbutton-events.h"
#include "ui/widget/spinbutton.h"
#include "../widgets/spw-utilities.h"
#include "../widgets/widget-sizes.h"
#include "../xml/repr.h"
#include "ui/uxmanager.h"
#include "../ui/icon-names.h"
#include "../helper/unit-menu.h"
#include "../helper/units.h"
#include "../helper/unit-tracker.h"
#include "../pen-context.h"
#include "../sp-namedview.h"

using Inkscape::UnitTracker;
using Inkscape::UI::UXManager;
using Inkscape::DocumentUndo;
using Inkscape::UI::ToolboxFactory;
using Inkscape::UI::PrefPusher;

//########################
//##  Measure Toolbox   ##
//########################

static void
sp_measure_fontsize_value_changed(GtkAdjustment *adj, GObject *tbl)
{
    SPDesktop *desktop = (SPDesktop *) g_object_get_data( tbl, "desktop" );

    if (DocumentUndo::getUndoSensitive(sp_desktop_document(desktop))) {
        Inkscape::Preferences *prefs = Inkscape::Preferences::get();
        prefs->setInt(Glib::ustring("/tools/measure/fontsize"),
            gtk_adjustment_get_value(adj));
    }
}

static void measure_unit_changed(GtkAction* /*act*/, GObject* tbl)
{
    UnitTracker* tracker = reinterpret_cast<UnitTracker*>(g_object_get_data(tbl, "tracker"));
    SPUnit const *unit = tracker->getActiveUnit();
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    prefs->setInt("/tools/measure/unitid", unit->unit_id);
}

void sp_measure_toolbox_prep(SPDesktop * desktop, GtkActionGroup* mainActions, GObject* holder)
{
    UnitTracker* tracker = new UnitTracker( SP_UNIT_ABSOLUTE | SP_UNIT_DEVICE );
    tracker->setActiveUnit( sp_desktop_namedview(desktop)->doc_units );
    g_object_set_data( holder, "tracker", tracker );

    EgeAdjustmentAction *eact = 0;

    /* Font Size */
    {
        eact = create_adjustment_action( "MeasureFontSizeAction",
                                         _("Font Size"), _("Font Size:"),
                                         _("The font size to be used in the measurement labels"),
                                         "/tools/measure/fontsize", 0.0,
                                         GTK_WIDGET(desktop->canvas), NULL, holder, FALSE, NULL,
                                         10, 36, 1.0, 4.0,
                                         0, 0, 0,
                                         sp_measure_fontsize_value_changed);
        gtk_action_group_add_action( mainActions, GTK_ACTION(eact) );
    }


    // units label
    {
        EgeOutputAction* act = ege_output_action_new( "measure_units_label", _("Units:"), _("The units to be used for the measurements"), 0 );
        ege_output_action_set_use_markup( act, TRUE );
        g_object_set( act, "visible-overflown", FALSE, NULL );
        gtk_action_group_add_action( mainActions, GTK_ACTION( act ) );
    }

    // units menu
    {
        GtkAction* act = tracker->createAction( "MeasureUnitsAction", _("Units:"), _("The units to be used for the measurements") );
        g_signal_connect_after( G_OBJECT(act), "changed", G_CALLBACK(measure_unit_changed), (GObject*)holder );
        gtk_action_group_add_action( mainActions, act );
    }
} // end of sp_measure_toolbox_prep()


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
