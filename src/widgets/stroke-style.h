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

// WHOA! talk about header bloat!

#ifndef SEEN_DIALOGS_STROKE_STYLE_H
#define SEEN_DIALOGS_STROKE_STYLE_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "widgets/dash-selector.h"
#include <gtkmm/radiobutton.h>

#if WITH_GTKMM_3_0
#include <gtkmm/grid.h>
#else
#include <gtkmm/table.h>
#endif

#include <glibmm/i18n.h>

#include "desktop.h"
#include "desktop-style.h"
#include "ui/dialog-events.h"
#include "display/canvas-bpath.h" // for SP_STROKE_LINEJOIN_*
#include "display/drawing.h"
#include "document-private.h"
#include "document-undo.h"
#include "gradient-chemistry.h"
#include "helper/stock-items.h"
#include "inkscape.h"
#include "io/sys.h"
#include "sp-marker.h"
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

namespace Inkscape {
    namespace Util {
        class Unit;
    }
    namespace UI {
        namespace Widget {
            class UnitMenu;
        }
    }
}

struct { gchar const *key; gint value; } const SPMarkerNames[] = {
    {"marker-all", SP_MARKER_LOC},
    {"marker-start", SP_MARKER_LOC_START},
    {"marker-mid", SP_MARKER_LOC_MID},
    {"marker-end", SP_MARKER_LOC_END},
    {"", SP_MARKER_LOC_QTY},
    {NULL, -1}
};

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
class StrokeStyleButton;

class StrokeStyle : public Gtk::VBox
{
public:
    StrokeStyle();
    ~StrokeStyle();
    void setDesktop(SPDesktop *desktop);

private:
    /** List of valid types for the stroke-style radio-button widget */
    enum StrokeStyleButtonType {
        STROKE_STYLE_BUTTON_JOIN, ///< A button to set the line-join style
        STROKE_STYLE_BUTTON_CAP,  ///< A button to set the line-cap style
        STROKE_STYLE_BUTTON_ORDER ///< A button to set the paint-order style
    };
    
    /**
     * A custom radio-button for setting the stroke style.  It can be configured
     * to set either the join or cap style by setting the button_type field.
     */
    class StrokeStyleButton : public Gtk::RadioButton {
        public:
            StrokeStyleButton(Gtk::RadioButtonGroup &grp,
                              char const            *icon,
                              StrokeStyleButtonType  button_type,
                              gchar const           *stroke_style);

            /** Get the type (line/cap) of the stroke-style button */
            inline StrokeStyleButtonType get_button_type() {return button_type;}

            /** Get the stroke style attribute associated with the button */
            inline gchar const * get_stroke_style() {return stroke_style;}

        private:
            StrokeStyleButtonType button_type; ///< The type (line/cap) of the button
            gchar const *stroke_style;         ///< The stroke style associated with the button
    };

    void updateLine();
    void updateAllMarkers(std::vector<SPItem*> const &objects);
    void updateMarkerHist(SPMarkerLoc const which);
    void setDashSelectorFromStyle(SPDashSelector *dsel, SPStyle *style);
    void setJoinType (unsigned const jointype);
    void setCapType (unsigned const captype);
    void setPaintOrder (gchar const *paint_order);
    void setJoinButtons(Gtk::ToggleButton *active);
    void setCapButtons(Gtk::ToggleButton *active);
    void setPaintOrderButtons(Gtk::ToggleButton *active);
    void scaleLine();
    void setScaledDash(SPCSSAttr *css, int ndash, double *dash, double offset, double scale);
    void setMarkerColor(SPObject *marker, int loc, SPItem *item);
    SPObject *forkMarker(SPObject *marker, int loc, SPItem *item);
    const char *getItemColorForMarker(SPItem *item, Inkscape::PaintTarget fill_or_stroke, int loc);

    StrokeStyleButton * makeRadioButton(Gtk::RadioButtonGroup &grp,
                                        char const            *icon,
                                        Gtk::HBox             *hb,
                                        StrokeStyleButtonType  button_type,
                                        gchar const           *stroke_style);

    // Callback functions
    void selectionModifiedCB(guint flags);
    void selectionChangedCB();
    void widthChangedCB();
    void miterLimitChangedCB();
    void lineDashChangedCB();
    void unitChangedCB();
    static void markerSelectCB(MarkerComboBox *marker_combo, StrokeStyle *spw, SPMarkerLoc const which);
    static void buttonToggledCB(StrokeStyleButton *tb, StrokeStyle *spw);


    MarkerComboBox *startMarkerCombo;
    MarkerComboBox *midMarkerCombo;
    MarkerComboBox *endMarkerCombo;
#if WITH_GTKMM_3_0
    Gtk::Grid *table;
    Glib::RefPtr<Gtk::Adjustment> *widthAdj;
    Glib::RefPtr<Gtk::Adjustment> *miterLimitAdj;
#else
    Gtk::Table *table;
    Gtk::Adjustment *widthAdj;
    Gtk::Adjustment *miterLimitAdj;
#endif
    Inkscape::UI::Widget::SpinButton *miterLimitSpin;
    Inkscape::UI::Widget::SpinButton *widthSpin;
    Inkscape::UI::Widget::UnitMenu *unitSelector;
    StrokeStyleButton *joinMiter;
    StrokeStyleButton *joinRound;
    StrokeStyleButton *joinBevel;
    StrokeStyleButton *capButt;
    StrokeStyleButton *capRound;
    StrokeStyleButton *capSquare;
    StrokeStyleButton *paintOrderFSM;
    StrokeStyleButton *paintOrderSFM;
    StrokeStyleButton *paintOrderFMS;
    StrokeStyleButton *paintOrderMFS;
    StrokeStyleButton *paintOrderSMF;
    StrokeStyleButton *paintOrderMSF;
    SPDashSelector *dashSelector;

    gboolean update;
    SPDesktop *desktop;
    sigc::connection selectChangedConn;
    sigc::connection selectModifiedConn;
    sigc::connection startMarkerConn;
    sigc::connection midMarkerConn;
    sigc::connection endMarkerConn;
    sigc::connection unitChangedConn;
    
    Inkscape::Util::Unit const *_old_unit;
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
