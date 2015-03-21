/**
 * @file
 *
 * Paper-size widget and helper functions
 */
/*
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *   Bob Jamison <ishmal@users.sf.net>
 *   Abhishek Sharma
 *
 * Copyright (C) 2000 - 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "page-sizer.h"

#include <cmath>
#include <string>
#include <string.h>
#include <sstream>
#include <vector>

#include <glibmm/i18n.h>

#include <2geom/transforms.h>


#include "document.h"
#include "desktop.h"
#include "helper/action.h"
#include "helper/action-context.h"
#include "util/units.h"
#include "inkscape.h"
#include "sp-namedview.h"
#include "sp-root.h"
#include "ui/widget/button.h"
#include "ui/widget/scalar-unit.h"
#include "verbs.h"
#include "xml/node.h"
#include "xml/repr.h"

using std::pair;
using Inkscape::Util::unit_table;

namespace Inkscape {
namespace UI {
namespace Widget {

    /** \note
     * The ISO page sizes in the table below differ from ghostscript's idea of page sizes (by
     * less than 1pt).  Being off by <1pt should be OK for most purposes, but may cause fuzziness
     * (antialiasing) problems when printing to 72dpi or 144dpi printers or bitmap files due to
     * postscript's different coordinate system (y=0 meaning bottom of page in postscript and top
     * of page in SVG).  I haven't looked into whether this does in fact cause fuzziness, I merely
     * note the possibility.  Rounding done by extension/internal/ps.cpp (e.g. floor/ceil calls)
     * will also affect whether fuzziness occurs.
     *
     * The remainder of this comment discusses the origin of the numbers used for ISO page sizes in
     * this table and in ghostscript.
     *
     * The versions here, in mm, are the official sizes according to
     * <a href="http://en.wikipedia.org/wiki/Paper_sizes">http://en.wikipedia.org/wiki/Paper_sizes</a>
     * at 2005-01-25.  (The ISO entries in the below table
     * were produced mechanically from the table on that page.)
     *
     * (The rule seems to be that A0, B0, ..., D0. sizes are rounded to the nearest number of mm
     * from the "theoretical size" (i.e. 1000 * sqrt(2) or pow(2.0, .25) or the like), whereas
     * going from e.g. A0 to A1 always take the floor of halving -- which by chance coincides
     * exactly with flooring the "theoretical size" for n != 0 instead of the rounding to nearest
     * done for n==0.)
     *
     * Ghostscript paper sizes are given in gs_statd.ps according to gs(1).  gs_statd.ps always
     * uses an integer number of pt: sometimes gs_statd.ps rounds to nearest (e.g. a1), sometimes
     * floors (e.g. a10), sometimes ceils (e.g. a8).
     *
     * I'm not sure how ghostscript's gs_statd.ps was calculated: it isn't just rounding the
     * "theoretical size" of each page to pt (see a0), nor is it rounding the a0 size times an
     * appropriate power of two (see a1).  Possibly it was prepared manually, with a human applying
     * inconsistent rounding rules when converting from mm to pt.
     */
    /** \todo
     * Should we include the JIS B series (used in Japan)
     * (JIS B0 is sometimes called JB0, and similarly for JB1 etc)?
     * Should we exclude B7--B10 and A7--10 to make the list smaller ?
     * Should we include any of the ISO C, D and E series (see below) ?
     */

struct PaperSizeRec {
    char const * const name;  //name
    double const smaller;     //lesser dimension
    double const larger;      //greater dimension
    Glib::ustring const unit;      //units
};

// list of page formats that should be in landscape automatically
static std::vector<std::string> lscape_papers;

static void
fill_landscape_papers() {
    lscape_papers.push_back("US #10 Envelope");
    lscape_papers.push_back("DL Envelope");
    lscape_papers.push_back("Banner 468x60");
    lscape_papers.push_back("Business Card (ISO 7810)");
    lscape_papers.push_back("Business Card (US)");
    lscape_papers.push_back("Business Card (Europe)");
    lscape_papers.push_back("Business Card (Aus/NZ)");
}

static PaperSizeRec const inkscape_papers[] = {
    { "A4",                210,  297, "mm" },
    { "US Letter",         8.5,   11, "in" },
    { "US Legal",          8.5,   14, "in" },
    { "US Executive",     7.25, 10.5, "in" },
    { "A0",                841, 1189, "mm" },
    { "A1",                594,  841, "mm" },
    { "A2",                420,  594, "mm" },
    { "A3",                297,  420, "mm" },
    { "A5",                148,  210, "mm" },
    { "A6",                105,  148, "mm" },
    { "A7",                 74,  105, "mm" },
    { "A8",                 52,   74, "mm" },
    { "A9",                 37,   52, "mm" },
    { "A10",                26,   37, "mm" },
    { "B0",               1000, 1414, "mm" },
    { "B1",                707, 1000, "mm" },
    { "B2",                500,  707, "mm" },
    { "B3",                353,  500, "mm" },
    { "B4",                250,  353, "mm" },
    { "B5",                176,  250, "mm" },
    { "B6",                125,  176, "mm" },
    { "B7",                 88,  125, "mm" },
    { "B8",                 62,   88, "mm" },
    { "B9",                 44,   62, "mm" },
    { "B10",                31,   44, "mm" },



//#if 0
         /*
         Whether to include or exclude these depends on how
         big we mind our page size menu
         becoming.  C series is used for envelopes;
         don't know what D and E series are used for.
         */

    { "C0",                917, 1297, "mm" },
    { "C1",                648,  917, "mm" },
    { "C2",                458,  648, "mm" },
    { "C3",                324,  458, "mm" },
    { "C4",                229,  324, "mm" },
    { "C5",                162,  229, "mm" },
    { "C6",                114,  162, "mm" },
    { "C7",                 81,  114, "mm" },
    { "C8",                 57,   81, "mm" },
    { "C9",                 40,   57, "mm" },
    { "C10",                28,   40, "mm" },
    { "D1",                545,  771, "mm" },
    { "D2",                385,  545, "mm" },
    { "D3",                272,  385, "mm" },
    { "D4",                192,  272, "mm" },
    { "D5",                136,  192, "mm" },
    { "D6",                 96,  136, "mm" },
    { "D7",                 68,   96, "mm" },
    { "E3",                400,  560, "mm" },
    { "E4",                280,  400, "mm" },
    { "E5",                200,  280, "mm" },
    { "E6",                140,  200, "mm" },
//#endif



    { "CSE",               462,  649, "pt" },
    { "US #10 Envelope", 4.125,  9.5, "in" },
    /* See http://www.hbp.com/content/PCR_envelopes.cfm for a much larger list of US envelope
       sizes. */
    { "DL Envelope",       110,  220, "mm" },
    { "Ledger/Tabloid",     11,   17, "in" },
    /* Note that `Folio' (used in QPrinter/KPrinter) is deliberately absent from this list, as it
       means different sizes to different people: different people may expect the width to be
       either 8, 8.25 or 8.5 inches, and the height to be either 13 or 13.5 inches, even
       restricting our interpretation to foolscap folio.  If you wish to introduce a folio-like
       page size to the list, then please consider using a name more specific than just `Folio' or
       `Foolscap Folio'. */
    { "Banner 468x60",      60,  468, "px" },
    { "Icon 16x16",         16,   16, "px" },
    { "Icon 32x32",         32,   32, "px" },
    { "Icon 48x48",         48,   48, "px" },
    /* business cards */
    { "Business Card (ISO 7810)", 53.98, 85.60, "mm" },
    { "Business Card (US)",             2,     3.5,  "in" },
    { "Business Card (Europe)",        55,    85,    "mm" },
    { "Business Card (Aus/NZ)",        55,    90,    "mm" },

    // Start Arch Series List


    { "Arch A",         9,    12,    "in" },  // 229 x 305 mm
    { "Arch B",        12,    18,    "in" },  // 305 x 457 mm
    { "Arch C",        18,    24,    "in" },  // 457 x 610 mm
    { "Arch D",        24,    36,    "in" },  // 610 x 914 mm
    { "Arch E",        36,    48,    "in" },  // 914 x 1219 mm
    { "Arch E1",       30,    42,    "in" },  // 762 x 1067 mm

    /*
     * The above list of Arch sizes were taken from the following site:
     * http://en.wikipedia.org/wiki/Paper_size
     * Further detail can be found at http://www.ansi.org
     * Sizes are assumed to be arbitrary rounding to MM unless shown to be otherwise
     * No conflicting information was found regarding sizes in MM
     * September 2009 - DAK
     */

    { NULL,                     0,    0, "px" },
};



//########################################################################
//# P A G E    S I Z E R
//########################################################################

/**
 * Constructor
 */
PageSizer::PageSizer(Registry & _wr)
    : Gtk::VBox(false,4),
      _dimensionUnits( _("U_nits:"), "units", _wr ),
      _dimensionWidth( _("_Width:"), _("Width of paper"), "width", _dimensionUnits, _wr ),
      _dimensionHeight( _("_Height:"), _("Height of paper"), "height", _dimensionUnits, _wr ),
      _marginTop( _("T_op margin:"), _("Top margin"), "fit-margin-top", _wr ),
      _marginLeft( _("L_eft:"), _("Left margin"), "fit-margin-left", _wr),
      _marginRight( _("Ri_ght:"), _("Right margin"), "fit-margin-right", _wr),
      _marginBottom( _("Botto_m:"), _("Bottom margin"), "fit-margin-bottom", _wr),
      _lockMarginUpdate(false),
      _scaleX(_("Scale _x:"), _("Scale X"), "scale-x", _wr),
      _scaleY(_("Scale _y:"), _("Scale Y"), "scale-y", _wr),
      _lockScaleUpdate(false),
      _viewboxX(_("X:"),      _("X"),      "viewbox-x", _wr),
      _viewboxY(_("Y:"),      _("Y"),      "viewbox-y", _wr),
      _viewboxW(_("Width:"),  _("Width"),  "viewbox-width", _wr),
      _viewboxH(_("Height:"), _("Height"), "viewbox-height", _wr),
      _lockViewboxUpdate(false),
      _widgetRegistry(&_wr)
{
    // set precision of scalar entry boxes
    _wr.setUpdating (true);
    _dimensionWidth.setDigits(5);
    _dimensionHeight.setDigits(5);
    _marginTop.setDigits(5);
    _marginLeft.setDigits(5);
    _marginRight.setDigits(5);
    _marginBottom.setDigits(5);
    _scaleX.setDigits(5);
    _scaleY.setDigits(5);
    _viewboxX.setDigits(2);
    _viewboxY.setDigits(2);
    _viewboxW.setDigits(2);
    _viewboxH.setDigits(2);

    _scaleX.setRange( 0.00001, 100000 );
    _scaleY.setRange( 0.00001, 100000 );
    _viewboxX.setRange( -100000, 100000 );
    _viewboxY.setRange( -100000, 100000 );
    _viewboxW.setRange( 0, 200000 );
    _viewboxH.setRange( 0, 200000 );

    _scaleY.set_sensitive (false); // We only want to display Y scale.

    _wr.setUpdating (false);

    //# Set up the Paper Size combo box
    _paperSizeListStore = Gtk::ListStore::create(_paperSizeListColumns);
    _paperSizeList.set_model(_paperSizeListStore);
    _paperSizeList.append_column(_("Name"),
                                 _paperSizeListColumns.nameColumn);
    _paperSizeList.append_column(_("Description"),
                                 _paperSizeListColumns.descColumn);
    _paperSizeList.set_headers_visible(false);
    _paperSizeListSelection = _paperSizeList.get_selection();
    _paper_size_list_connection =
        _paperSizeListSelection->signal_changed().connect (
            sigc::mem_fun (*this, &PageSizer::on_paper_size_list_changed));
    _paperSizeListScroller.add(_paperSizeList);
    _paperSizeListScroller.set_shadow_type(Gtk::SHADOW_IN);
    _paperSizeListScroller.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_ALWAYS);
    _paperSizeListScroller.set_size_request(-1, 90);

    fill_landscape_papers();

    for (PaperSizeRec const *p = inkscape_papers; p->name; p++)
    {
        Glib::ustring name = p->name;
        char formatBuf[80];
        snprintf(formatBuf, 79, "%0.1f x %0.1f", p->smaller, p->larger);
        Glib::ustring desc = formatBuf;
        desc.append(" " + p->unit);
        PaperSize paper(name, p->smaller, p->larger, unit_table.getUnit(p->unit));
        _paperSizeTable[name] = paper;
        Gtk::TreeModel::Row row = *(_paperSizeListStore->append());
        row[_paperSizeListColumns.nameColumn] = name;
        row[_paperSizeListColumns.descColumn] = desc;
        }
    //Gtk::TreeModel::iterator iter = _paperSizeListStore->children().begin();
    //if (iter)
    //    _paperSizeListSelection->select(iter);


    pack_start (_paperSizeListScroller, true, true, 0);

    //## Set up orientation radio buttons
    pack_start (_orientationBox, false, false, 0);
    _orientationLabel.set_label(_("Orientation:"));
    _orientationBox.pack_start(_orientationLabel, false, false, 0);
    _landscapeButton.set_use_underline();
    _landscapeButton.set_label(_("_Landscape"));
    _landscapeButton.set_active(true);
    Gtk::RadioButton::Group group = _landscapeButton.get_group();
    _orientationBox.pack_end (_landscapeButton, false, false, 5);
    _portraitButton.set_use_underline();
    _portraitButton.set_label(_("_Portrait"));
    _portraitButton.set_active(true);
    _orientationBox.pack_end (_portraitButton, false, false, 5);
    _portraitButton.set_group (group);
    _portraitButton.set_active (true);

    // Setting default custom unit to document unit
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    SPNamedView *nv = dt->getNamedView();
    _wr.setUpdating (true);
    if (nv->page_size_units) {
        _dimensionUnits.setUnit(nv->page_size_units->abbr);
    } else if (nv->display_units) {
        _dimensionUnits.setUnit(nv->display_units->abbr);
    }
    _wr.setUpdating (false);
    

    //## Set up custom size frame
    _customFrame.set_label(_("Custom size"));
    pack_start (_customFrame, false, false, 0);
    _customFrame.add(_customDimTable);

    _customDimTable.set_border_width(4);

#if WITH_GTKMM_3_0
    _customDimTable.set_row_spacing(4);
    _customDimTable.set_column_spacing(4);

    _dimensionWidth.set_hexpand();
    _dimensionWidth.set_vexpand();
    _customDimTable.attach(_dimensionWidth,        0, 0, 1, 1);

    _dimensionUnits.set_hexpand();
    _dimensionUnits.set_vexpand();
    _customDimTable.attach(_dimensionUnits,        1, 0, 1, 1);

    _dimensionHeight.set_hexpand();
    _dimensionHeight.set_vexpand();
    _customDimTable.attach(_dimensionHeight,       0, 1, 1, 1);

    _fitPageMarginExpander.set_hexpand();
    _fitPageMarginExpander.set_vexpand();
    _customDimTable.attach(_fitPageMarginExpander, 0, 2, 2, 1);
#else
    _customDimTable.resize(3, 2);
    _customDimTable.set_row_spacings(4);
    _customDimTable.set_col_spacings(4);
    _customDimTable.attach(_dimensionWidth,        0,1, 0,1);
    _customDimTable.attach(_dimensionUnits,        1,2, 0,1);
    _customDimTable.attach(_dimensionHeight,       0,1, 1,2);
    _customDimTable.attach(_fitPageMarginExpander, 0,2, 2,3);
#endif
    
    _dimTabOrderGList = NULL;
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _dimensionWidth.gobj());
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _dimensionHeight.gobj());
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _dimensionUnits.gobj());
    _dimTabOrderGList = g_list_append(_dimTabOrderGList, _fitPageMarginExpander.gobj());
    Glib::ListHandle<Widget *> dimFocusChain(_dimTabOrderGList, Glib::OWNERSHIP_NONE);
    _customDimTable.set_focus_chain(dimFocusChain);    

    //## Set up fit page expander
    _fitPageMarginExpander.set_use_underline();
    _fitPageMarginExpander.set_label(_("Resi_ze page to content..."));
    _fitPageMarginExpander.add(_marginTable);
    
    //## Set up margin settings
    _marginTable.set_border_width(4);

#if WITH_GTKMM_3_0
    _marginTable.set_row_spacing(4);
    _marginTable.set_column_spacing(4);

    _marginTopAlign.set_hexpand();
    _marginTopAlign.set_vexpand();
    _marginTable.attach(_marginTopAlign,     0, 0, 2, 1);

    _marginLeftAlign.set_hexpand();
    _marginLeftAlign.set_vexpand();
    _marginTable.attach(_marginLeftAlign,    0, 1, 1, 1);

    _marginRightAlign.set_hexpand();
    _marginRightAlign.set_vexpand();
    _marginTable.attach(_marginRightAlign,   1, 1, 1, 1);

    _marginBottomAlign.set_hexpand();
    _marginBottomAlign.set_vexpand();
    _marginTable.attach(_marginBottomAlign,  0, 2, 2, 1);

    _fitPageButtonAlign.set_hexpand();
    _fitPageButtonAlign.set_vexpand();
    _marginTable.attach(_fitPageButtonAlign, 0, 3, 2, 1);
#else
    _marginTable.set_border_width(4);
    _marginTable.set_row_spacings(4);
    _marginTable.set_col_spacings(4);
    _marginTable.attach(_marginTopAlign,     0,2, 0,1);
    _marginTable.attach(_marginLeftAlign,    0,1, 1,2);
    _marginTable.attach(_marginRightAlign,   1,2, 1,2);
    _marginTable.attach(_marginBottomAlign,  0,2, 2,3);
    _marginTable.attach(_fitPageButtonAlign, 0,2, 3,4);
#endif
    
    _marginTopAlign.set(0.5, 0.5, 0.0, 1.0);
    _marginTopAlign.add(_marginTop);
    _marginLeftAlign.set(0.0, 0.5, 0.0, 1.0);
    _marginLeftAlign.add(_marginLeft);
    _marginRightAlign.set(1.0, 0.5, 0.0, 1.0);
    _marginRightAlign.add(_marginRight);
    _marginBottomAlign.set(0.5, 0.5, 0.0, 1.0);
    _marginBottomAlign.add(_marginBottom);
    
    _fitPageButtonAlign.set(0.5, 0.5, 0.0, 1.0);
    _fitPageButtonAlign.add(_fitPageButton);
    _fitPageButton.set_use_underline();
    _fitPageButton.set_label(_("_Resize page to drawing or selection"));
    _fitPageButton.set_tooltip_text(_("Resize the page to fit the current selection, or the entire drawing if there is no selection"));

    _scaleFrame.set_label(_("Scale"));
    pack_start (_scaleFrame, false, false, 0);
    _scaleFrame.add(_scaleTable);

    _scaleTable.set_border_width(4);

#if WITH_GTKMM_3_0
    _scaleTable.set_row_spacing(4);
    _scaleTable.set_column_spacing(4);

    _scaleTable.attach(_scaleX,        0, 0, 1, 1);
    _scaleTable.attach(_scaleY,        1, 0, 1, 1);

    _scaleTable.attach(_scaleLabel,    2, 0, 1, 1);
    _scaleTable.attach(_scaleWarning,  0, 1, 2, 1);
    _viewboxExpander.set_hexpand();
    _viewboxExpander.set_vexpand();
    _scaleTable.attach(_viewboxExpander, 0, 2, 2, 1);
#else
    _scaleTable.resize(3, 2);
    _scaleTable.set_row_spacings(4);
    _scaleTable.set_col_spacings(4);
    _scaleTable.attach(_scaleX,        0,1, 0,1);
    _scaleTable.attach(_scaleY,        1,2, 0,1);
    _scaleTable.attach(_scaleLabel,    2,3, 0,1);
    _scaleTable.attach(_scaleWarning,  0,3, 1,2, Gtk::FILL);
    _scaleTable.attach(_viewboxExpander, 0,3, 2,3);
#endif
    
    _scaleWarning.set_label(_("While SVG allows non-uniform scaling it is recommended to use only uniform scaling in Inkscape. To set a non-uniform scaling, set the 'viewBox' directly."));
    _scaleWarning.set_line_wrap( true );

    _viewboxExpander.set_use_underline();
    _viewboxExpander.set_label(_("_Viewbox..."));
    _viewboxExpander.add(_viewboxTable);

#if WITH_GTKMM_3_0
    _viewboxTable.set_row_spacing(2);
    _viewboxTable.set_column_spacing(2);

    _viewboxX.set_hexpand();
    _viewboxX.set_vexpand();
    _viewboxTable.attach(_viewboxX,      0, 0, 1, 1);

    _viewboxY.set_hexpand();
    _viewboxY.set_vexpand();
    _viewboxTable.attach(_viewboxY,      1, 0, 1, 1);

    _viewboxW.set_hexpand();
    _viewboxW.set_vexpand();
    _viewboxTable.attach(_viewboxW,      0, 1, 1, 1);

    _viewboxH.set_hexpand();
    _viewboxH.set_vexpand();
    _viewboxTable.attach(_viewboxH,      1, 1, 1, 1);

#else
    _viewboxTable.set_border_width(4);
    _viewboxTable.set_row_spacings(2);
    _viewboxTable.set_col_spacings(2);
    _viewboxTable.attach(_viewboxX,     0,1, 0,1);
    _viewboxTable.attach(_viewboxY,     1,2, 0,1);
    _viewboxTable.attach(_viewboxW,     0,1, 1,2);
    _viewboxTable.attach(_viewboxH,     1,2, 1,2);
#endif

    _wr.setUpdating (true);
    updateScaleUI();
    _wr.setUpdating (false);
}


/**
 * Destructor
 */
PageSizer::~PageSizer()
{
    g_list_free(_dimTabOrderGList);
}



/**
 * Initialize or reset this widget
 */
void
PageSizer::init ()
{
    _landscape_connection = _landscapeButton.signal_toggled().connect (sigc::mem_fun (*this, &PageSizer::on_landscape));
    _portrait_connection = _portraitButton.signal_toggled().connect (sigc::mem_fun (*this, &PageSizer::on_portrait));
    _changedw_connection = _dimensionWidth.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_value_changed));
    _changedh_connection = _dimensionHeight.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_value_changed));
    _changedu_connection = _dimensionUnits.getUnitMenu()->signal_changed().connect (sigc::mem_fun (*this, &PageSizer::on_units_changed));
    _fitPageButton.signal_clicked().connect(sigc::mem_fun(*this, &PageSizer::fire_fit_canvas_to_selection_or_drawing));
    _changeds_connection  = _scaleX.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_scale_changed));
    _changedvx_connection = _viewboxX.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_viewbox_changed));
    _changedvy_connection = _viewboxY.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_viewbox_changed));
    _changedvw_connection = _viewboxW.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_viewbox_changed));
    _changedvh_connection = _viewboxH.signal_value_changed().connect (sigc::mem_fun (*this, &PageSizer::on_viewbox_changed));
    show_all_children();
}


/**
 * Set document dimensions (if not called by Doc prop's update()) and
 * set the PageSizer's widgets and text entries accordingly. If
 * 'changeList' is true, then adjust the paperSizeList to show the closest
 * standard page size.
 *
 * \param w, h
 * \param changeList whether to modify the paper size list
 */
void
PageSizer::setDim (Inkscape::Util::Quantity w, Inkscape::Util::Quantity h, bool changeList, bool changeSize)
{
    static bool _called = false;
    if (_called) {
        return;
    }

    _called = true;

    _paper_size_list_connection.block();
    _landscape_connection.block();
    _portrait_connection.block();
    _changedw_connection.block();
    _changedh_connection.block();

    _unit = w.unit->abbr;

    if (SP_ACTIVE_DESKTOP && !_widgetRegistry->isUpdating()) {
        SPDocument *doc = SP_ACTIVE_DESKTOP->getDocument();
        Inkscape::Util::Quantity const old_height = doc->getHeight();
        doc->setWidthAndHeight (w, h, changeSize);
        // The origin for the user is in the lower left corner; this point should remain stationary when
        // changing the page size. The SVG's origin however is in the upper left corner, so we must compensate for this
        if (changeSize) {
            Geom::Translate const vert_offset(Geom::Point(0, (old_height.value("px") - h.value("px"))));
            doc->getRoot()->translateChildItems(vert_offset);
        }
        DocumentUndo::done(doc, SP_VERB_NONE, _("Set page size"));
    }

    if ( w != h ) {
        _landscapeButton.set_sensitive(true);
        _portraitButton.set_sensitive (true);
        _landscape = ( w > h );
        _landscapeButton.set_active(_landscape ? true : false);
        _portraitButton.set_active (_landscape ? false : true);
    } else {
        _landscapeButton.set_sensitive(false);
        _portraitButton.set_sensitive (false);
    }

    if (changeList)
        {
        Gtk::TreeModel::Row row = (*find_paper_size(w, h));
        if (row)
            _paperSizeListSelection->select(row);
        }

    _dimensionWidth.setUnit(w.unit->abbr);
    _dimensionWidth.setValue (w.quantity);
    _dimensionHeight.setUnit(h.unit->abbr);
    _dimensionHeight.setValue (h.quantity);


    _paper_size_list_connection.unblock();
    _landscape_connection.unblock();
    _portrait_connection.unblock();
    _changedw_connection.unblock();
    _changedh_connection.unblock();

    _called = false;
}

/**
 * Updates the scalar widgets for the fit margins.  (Just changes the value
 * of the ui widgets to match the xml).
 */
void 
PageSizer::updateFitMarginsUI(Inkscape::XML::Node *nv_repr)
{
    if (!_lockMarginUpdate) {
        double value = 0.0;
        if (sp_repr_get_double(nv_repr, "fit-margin-top", &value)) {
            _marginTop.setValue(value);
        }
        if (sp_repr_get_double(nv_repr, "fit-margin-left", &value)) {
            _marginLeft.setValue(value);
        }
        if (sp_repr_get_double(nv_repr, "fit-margin-right", &value)) {
            _marginRight.setValue(value);
        }
        if (sp_repr_get_double(nv_repr, "fit-margin-bottom", &value)) {
            _marginBottom.setValue(value);
        }
    }
}


/**
 * Returns an iterator pointing to a row in paperSizeListStore which
 * contains a paper of the specified size, or
 * paperSizeListStore->children().end() if no such paper exists.
 *
 * The code is not tested for the case where w and h have different units.
 */
Gtk::ListStore::iterator
PageSizer::find_paper_size (Inkscape::Util::Quantity w, Inkscape::Util::Quantity h) const
{
    using Inkscape::Util::Quantity;
    using std::swap;

    // The code below assumes that w < h, so make sure that's the case:
    if ( h < w ) {
        swap(h,w);
    }

    g_return_val_if_fail(w <= h, _paperSizeListStore->children().end());

    std::map<Glib::ustring, PaperSize>::const_iterator iter;
    for (iter = _paperSizeTable.begin() ;
         iter != _paperSizeTable.end() ; ++iter) {
        PaperSize paper = iter->second;
        Quantity smallX (paper.smaller, paper.unit);
        Quantity largeX (paper.larger, paper.unit);

        g_return_val_if_fail(smallX.quantity < largeX.quantity + 0.001, _paperSizeListStore->children().end());

        if ( are_near(w, smallX, 0.1) && are_near(h, largeX, 0.1) ) {
            Gtk::ListStore::iterator p = _paperSizeListStore->children().begin();
            Gtk::ListStore::iterator pend = _paperSizeListStore->children().end();
            // We need to search paperSizeListStore explicitly for the
            // specified paper size because it is sorted in a different
            // way than paperSizeTable (which is sorted alphabetically)
            for ( ; p != pend; ++p) {
                if ((*p)[_paperSizeListColumns.nameColumn] == paper.name) {
                    return p;
                }
            }
        }
    }
    return _paperSizeListStore->children().end();
}



/**
 * Tell the desktop to fit the page size to the selection or drawing.
 */
void
PageSizer::fire_fit_canvas_to_selection_or_drawing()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }
    SPDocument *doc;
    SPNamedView *nv;
    Inkscape::XML::Node *nv_repr;
  
    if ((doc = SP_ACTIVE_DESKTOP->getDocument())
        && (nv = sp_document_namedview(doc, 0))
        && (nv_repr = nv->getRepr())) {
        _lockMarginUpdate = true;
        sp_repr_set_svg_double(nv_repr, "fit-margin-top", _marginTop.getValue());
        sp_repr_set_svg_double(nv_repr, "fit-margin-left", _marginLeft.getValue());
        sp_repr_set_svg_double(nv_repr, "fit-margin-right", _marginRight.getValue());
        sp_repr_set_svg_double(nv_repr, "fit-margin-bottom", _marginBottom.getValue());
        _lockMarginUpdate = false;
    }

    Verb *verb = Verb::get( SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING );
    if (verb) {
        SPAction *action = verb->get_action(Inkscape::ActionContext(dt));
        if (action) {
            sp_action_perform(action, NULL);
        }
    }
}



/**
 * Paper Size list callback for when a user changes the selection
 */
void
PageSizer::on_paper_size_list_changed()
{
    //Glib::ustring name = _paperSizeList.get_active_text();
    Gtk::TreeModel::iterator miter = _paperSizeListSelection->get_selected();
    if(!miter)
        {
        //error?
        return;
        }
    Gtk::TreeModel::Row row = *miter;
    Glib::ustring name = row[_paperSizeListColumns.nameColumn];
    std::map<Glib::ustring, PaperSize>::const_iterator piter =
                    _paperSizeTable.find(name);
    if (piter == _paperSizeTable.end()) {
        g_warning("paper size '%s' not found in table", name.c_str());
        return;
    }
    PaperSize paper = piter->second;
    Inkscape::Util::Quantity w = Inkscape::Util::Quantity(paper.smaller, paper.unit);
    Inkscape::Util::Quantity h = Inkscape::Util::Quantity(paper.larger, paper.unit);

    if (std::find(lscape_papers.begin(), lscape_papers.end(), paper.name.c_str()) != lscape_papers.end()) {
        // enforce landscape mode if this is desired for the given page format
        _landscape = true;
    } else {
        // otherwise we keep the current mode
        _landscape = _landscapeButton.get_active();
    }

    if (_landscape)
        setDim (h, w, false);
    else
        setDim (w, h, false);

}


/**
 * Portrait button callback
 */
void
PageSizer::on_portrait()
{
    if (!_portraitButton.get_active())
        return;
    Inkscape::Util::Quantity w = Inkscape::Util::Quantity(_dimensionWidth.getValue(""), _dimensionWidth.getUnit());
    Inkscape::Util::Quantity h = Inkscape::Util::Quantity(_dimensionHeight.getValue(""), _dimensionHeight.getUnit());
    if (h < w) {
        setDim (h, w);
    }
}


/**
 * Landscape button callback
 */
void
PageSizer::on_landscape()
{
    if (!_landscapeButton.get_active())
        return;
    Inkscape::Util::Quantity w = Inkscape::Util::Quantity(_dimensionWidth.getValue(""), _dimensionWidth.getUnit());
    Inkscape::Util::Quantity h = Inkscape::Util::Quantity(_dimensionHeight.getValue(""), _dimensionHeight.getUnit());
    if (w < h) {
        setDim (h, w);
    }
}


/**
 * Update scale widgets
 */
void
PageSizer::updateScaleUI()
{

    static bool _called = false;
    if (_called) {
        return;
    }

    _called = true;

    _changeds_connection.block();
    _changedvx_connection.block();
    _changedvy_connection.block();
    _changedvw_connection.block();
    _changedvh_connection.block();

    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (dt) {
        SPDocument *doc = dt->getDocument();

        // Update scale
        Geom::Scale scale = doc->getDocumentScale();
        SPNamedView *nv = dt->getNamedView();

        std::stringstream ss;
        ss << _("User units per ") << nv->display_units->abbr << "." ;
        _scaleLabel.set_text( ss.str() );

        if( !_lockScaleUpdate ) {

            double scaleX_inv =
                Inkscape::Util::Quantity::convert( scale[Geom::X], "px", nv->display_units );
            if( scaleX_inv > 0 ) {
                _scaleX.setValue(1.0/scaleX_inv);
            } else {
                // Should never happen
                std::cerr << "PageSizer::updateScaleUI(): Invalid scale value: " << scaleX_inv << std::endl;
                _scaleX.setValue(1.0);
            }
        }

        {  // Don't need to lock as scaleY widget not linked to callback.
            double scaleY_inv =
                Inkscape::Util::Quantity::convert( scale[Geom::Y], "px", nv->display_units );
            if( scaleY_inv > 0 ) {
                _scaleY.setValue(1.0/scaleY_inv);
            } else {
                // Should never happen
                std::cerr << "PageSizer::updateScaleUI(): Invalid scale value: " << scaleY_inv << std::endl;
                _scaleY.setValue(1.0);
            }
        }

        if( !_lockViewboxUpdate ) {
            Geom::Rect viewBox = doc->getViewBox();
            _viewboxX.setValue( viewBox.min()[Geom::X] );
            _viewboxY.setValue( viewBox.min()[Geom::Y] );
            _viewboxW.setValue( viewBox.width() );
            _viewboxH.setValue( viewBox.height() );
        }
        
    } else {
        // Should never happen
        std::cerr << "PageSizer::updateScaleUI(): No active desktop." << std::endl;
        _scaleLabel.set_text( "Unknown scale" );
    }

    _changeds_connection.unblock();
    _changedvx_connection.unblock();
    _changedvy_connection.unblock();
    _changedvw_connection.unblock();
    _changedvh_connection.unblock();

    _called = false;
}


/**
 * Callback for the dimension widgets
 */
void
PageSizer::on_value_changed()
{
    if (_widgetRegistry->isUpdating()) return;
    if (_unit != _dimensionUnits.getUnit()->abbr) return;
    setDim (Inkscape::Util::Quantity(_dimensionWidth.getValue(""), _dimensionUnits.getUnit()),
            Inkscape::Util::Quantity(_dimensionHeight.getValue(""), _dimensionUnits.getUnit()));
}

void
PageSizer::on_units_changed()
{
    if (_widgetRegistry->isUpdating()) return;
    _unit = _dimensionUnits.getUnit()->abbr;
    setDim (Inkscape::Util::Quantity(_dimensionWidth.getValue(""), _dimensionUnits.getUnit()),
            Inkscape::Util::Quantity(_dimensionHeight.getValue(""), _dimensionUnits.getUnit()),
            true, false);
}

/**
 * Callback for scale widgets
 */
void
PageSizer::on_scale_changed()
{
    if (_widgetRegistry->isUpdating()) return;

    double value = _scaleX.getValue();
    if( value > 0 ) {

        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        if (dt) {
            SPDocument *doc = dt->getDocument();
            SPNamedView *nv = dt->getNamedView();

            double scaleX_inv = Inkscape::Util::Quantity(1.0/value, nv->display_units ).value("px");

            _lockScaleUpdate = true;
            doc->setDocumentScale( 1.0/scaleX_inv );
            updateScaleUI();
            _lockScaleUpdate = false;
            DocumentUndo::done(doc, SP_VERB_NONE, _("Set page scale"));
        }
    }
}

/**
 * Callback for viewbox widgets
 */
void
PageSizer::on_viewbox_changed()
{
    if (_widgetRegistry->isUpdating()) return;

    double viewboxX = _viewboxX.getValue();
    double viewboxY = _viewboxY.getValue();
    double viewboxW = _viewboxW.getValue();
    double viewboxH = _viewboxH.getValue();

    if( viewboxW > 0 && viewboxH > 0) {
        SPDesktop *dt = SP_ACTIVE_DESKTOP;
        if (dt) {
            SPDocument *doc = dt->getDocument();
            _lockViewboxUpdate = true;
            doc->setViewBox( Geom::Rect::from_xywh( viewboxX, viewboxY, viewboxW, viewboxH ) );
            updateScaleUI();
            _lockViewboxUpdate = false;
            DocumentUndo::done(doc, SP_VERB_NONE, _("Set 'viewBox'"));
        }
    } else {
        std::cerr
            << "PageSizer::on_viewbox_changed(): width and height must both be greater than zero."
            << std::endl;
    }
}

} // namespace Widget
} // namespace UI
} // namespace Inkscape

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
