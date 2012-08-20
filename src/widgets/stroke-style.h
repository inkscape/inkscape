/**
 * @file
 * Widgets used in the stroke style dialog.
 */
/* Author:
 *   Lauris Kaplinski <lauris@ximian.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_STROKE_STYLE_H
#define SEEN_DIALOGS_STROKE_STYLE_H

#include "widgets/dash-selector.h"
#include <gtkmm/radiobutton.h>
#include <gtkmm/table.h>
#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "desktop-style.h"
#include "dialogs/dialog-events.h"
#include "display/canvas-bpath.h" // for SP_STROKE_LINEJOIN_*
#include "display/drawing.h"
#include "document-private.h"
#include "document-undo.h"
#include "gradient-chemistry.h"
#include "helper/stock-items.h"
#include "helper/unit-menu.h"
#include "helper/units.h"
#include "inkscape.h"
#include "io/sys.h"
#include "marker.h"
#include "preferences.h"
#include "path-prefix.h"
#include "selection.h"
#include "sp-linear-gradient.h"
#include "sp-namedview.h"
#include "sp-pattern.h"
#include "sp-radial-gradient.h"
#include "sp-rect.h"
#include "sp-text.h"
#include "style.h"
#include "svg/css-ostringstream.h"
#include "ui/cache/svg_preview_cache.h"
#include "ui/icon-names.h"
#include "widgets/icon.h"
#include "widgets/paint-selector.h"
#include "widgets/sp-widget.h"
#include "widgets/spw-utilities.h"
#include "ui/widget/spinbutton.h"
#include "xml/repr.h"

#include "stroke-style.h"
#include "stroke-marker-selector.h"
#include "fill-style.h" // to get sp_fill_style_widget_set_desktop
#include "fill-n-stroke-factory.h"

#include "verbs.h"

namespace Gtk {
class Widget;
class Container;
}

/**
 * Creates an instance of a paint style widget.
 */
Gtk::Widget *sp_stroke_style_paint_widget_new(void);

/**
 * Creates an instance of a line style widget.
 */
Gtk::Widget *sp_stroke_style_line_widget_new(void);

/**
 * Switches a line or paint style widget to track the given desktop.
 */
void sp_stroke_style_widget_set_desktop(Gtk::Widget *widget, SPDesktop *desktop);

SPObject *getMarkerObj(gchar const *n, SPDocument *doc);

namespace Inkscape {

class StrokeStyle : public Gtk::VBox
{
public:
    StrokeStyle();
    ~StrokeStyle();
    void setDesktop(SPDesktop *desktop);

private:


    void updateLine();
    void updateAllMarkers(GSList const *objects);
    void updateMarkerHist(SPMarkerLoc const which);
    void setDashSelectorFromStyle(SPDashSelector *dsel, SPStyle *style);
    void setJoinType (unsigned const jointype);
    void setCapType (unsigned const captype);
    void setJoinButtons(Gtk::ToggleButton *active);
    void setCapButtons(Gtk::ToggleButton *active);
    void scaleLine();
    void setScaledDash(SPCSSAttr *css, int ndash, double *dash, double offset, double scale);
    void setMarkerColor(SPItem *item, SPObject *marker,  MarkerComboBox *marker_combo);
    SPObject *forkMarker(SPItem *item, SPObject *marker, MarkerComboBox *marker_combo);
    const char *getItemColorForMarker(SPItem *item, Inkscape::PaintTarget fill_or_stroke, MarkerComboBox *marker_combo);

    Gtk::RadioButton * makeRadioButton(Gtk::RadioButton *tb, char const *icon,
                           Gtk::HBox *hb, gchar const *key, gchar const *data);
    static gboolean setStrokeWidthUnit(SPUnitSelector *,
                                          SPUnit const *old,
                                          SPUnit const *new_units,
                                          StrokeStyle *spw);

    // Callback functions
    void selectionModifiedCB(guint flags);
    void selectionChangedCB();
    void widthChangedCB();
    void miterLimitChangedCB();
    void lineDashChangedCB();
    static void markerSelectCB(MarkerComboBox *marker_combo, StrokeStyle *spw, SPMarkerLoc const which);
    static void buttonToggledCB(Gtk::ToggleButton *tb, StrokeStyle *spw);


    MarkerComboBox *startMarkerCombo;
    MarkerComboBox *midMarkerCombo;
    MarkerComboBox *endMarkerCombo;
    Gtk::Table *table;
#if WITH_GTKMM_3_0
    Glib::RefPtr<Gtk::Adjustment> *widthAdj;
    Glib::RefPtr<Gtk::Adjustment> *miterLimitAdj;
#else
    Gtk::Adjustment *widthAdj;
    Gtk::Adjustment *miterLimitAdj;
#endif
    Inkscape::UI::Widget::SpinButton *miterLimitSpin;
    Inkscape::UI::Widget::SpinButton *widthSpin;
    GtkWidget *unitSelector;
    Gtk::RadioButton *joinMiter;
    Gtk::RadioButton *joinRound;
    Gtk::RadioButton *joinBevel;
    Gtk::RadioButton *capButt;
    Gtk::RadioButton *capRound;
    Gtk::RadioButton *capSquare;
    SPDashSelector *dashSelector;

    gboolean update;
    SPDesktop *desktop;
    sigc::connection selectChangedConn;
    sigc::connection selectModifiedConn;
    sigc::connection startMarkerConn;
    sigc::connection midMarkerConn;
    sigc::connection endMarkerConn;
};

} // namespace Inkscape

#endif // SEEN_DIALOGS_STROKE_STYLE_H

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
