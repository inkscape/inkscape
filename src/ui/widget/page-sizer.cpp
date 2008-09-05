/** \file
 *
 * Paper-size widget and helper functions
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *   Bob Jamison <ishmal@users.sf.net>
 *
 * Copyright (C) 2000 - 2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <string.h>

#include <cmath>
#include <gtkmm.h>
#include "ui/widget/button.h"

#include "ui/widget/scalar-unit.h"

#include "helper/units.h"
#include "inkscape.h"
#include "verbs.h"
#include "desktop-handles.h"
#include "document.h"
#include "desktop.h"
#include "page-sizer.h"
#include "helper/action.h"

using std::pair;

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
    SPUnitId const unit;      //units
};

static PaperSizeRec const inkscape_papers[] = {
    { "A4",                210,  297, SP_UNIT_MM },
    { "US Letter",         8.5,   11, SP_UNIT_IN },
    { "US Legal",          8.5,   14, SP_UNIT_IN },
    { "US Executive",     7.25, 10.5, SP_UNIT_IN },
    { "A0",                841, 1189, SP_UNIT_MM },
    { "A1",                594,  841, SP_UNIT_MM },
    { "A2",                420,  594, SP_UNIT_MM },
    { "A3",                297,  420, SP_UNIT_MM },
    { "A5",                148,  210, SP_UNIT_MM },
    { "A6",                105,  148, SP_UNIT_MM },
    { "A7",                 74,  105, SP_UNIT_MM },
    { "A8",                 52,   74, SP_UNIT_MM },
    { "A9",                 37,   52, SP_UNIT_MM },
    { "A10",                26,   37, SP_UNIT_MM },
    { "B0",               1000, 1414, SP_UNIT_MM },
    { "B1",                707, 1000, SP_UNIT_MM },
    { "B2",                500,  707, SP_UNIT_MM },
    { "B3",                353,  500, SP_UNIT_MM },
    { "B4",                250,  353, SP_UNIT_MM },
    { "B5",                176,  250, SP_UNIT_MM },
    { "B6",                125,  176, SP_UNIT_MM },
    { "B7",                 88,  125, SP_UNIT_MM },
    { "B8",                 62,   88, SP_UNIT_MM },
    { "B9",                 44,   62, SP_UNIT_MM },
    { "B10",                31,   44, SP_UNIT_MM },



//#if 0
         /*
         Whether to include or exclude these depends on how
         big we mind our page size menu
         becoming.  C series is used for envelopes;
         don't know what D and E series are used for.
         */

    { "C0",                917, 1297, SP_UNIT_MM },
    { "C1",                648,  917, SP_UNIT_MM },
    { "C2",                458,  648, SP_UNIT_MM },
    { "C3",                324,  458, SP_UNIT_MM },
    { "C4",                229,  324, SP_UNIT_MM },
    { "C5",                162,  229, SP_UNIT_MM },
    { "C6",                114,  162, SP_UNIT_MM },
    { "C7",                 81,  114, SP_UNIT_MM },
    { "C8",                 57,   81, SP_UNIT_MM },
    { "C9",                 40,   57, SP_UNIT_MM },
    { "C10",                28,   40, SP_UNIT_MM },
    { "D1",                545,  771, SP_UNIT_MM },
    { "D2",                385,  545, SP_UNIT_MM },
    { "D3",                272,  385, SP_UNIT_MM },
    { "D4",                192,  272, SP_UNIT_MM },
    { "D5",                136,  192, SP_UNIT_MM },
    { "D6",                 96,  136, SP_UNIT_MM },
    { "D7",                 68,   96, SP_UNIT_MM },
    { "E3",                400,  560, SP_UNIT_MM },
    { "E4",                280,  400, SP_UNIT_MM },
    { "E5",                200,  280, SP_UNIT_MM },
    { "E6",                140,  200, SP_UNIT_MM },
//#endif



    { "CSE",               462,  649, SP_UNIT_PT },
    { "US #10 Envelope", 4.125,  9.5, SP_UNIT_IN },
    // TODO: Select landscape by default.
    /* See http://www.hbp.com/content/PCR_envelopes.cfm for a much larger list of US envelope
       sizes. */
    { "DL Envelope",       110,  220, SP_UNIT_MM },
    // TODO: Select landscape by default.
    { "Ledger/Tabloid",     11,   17, SP_UNIT_IN },
    /* Note that `Folio' (used in QPrinter/KPrinter) is deliberately absent from this list, as it
       means different sizes to different people: different people may expect the width to be
       either 8, 8.25 or 8.5 inches, and the height to be either 13 or 13.5 inches, even
       restricting our interpretation to foolscap folio.  If you wish to introduce a folio-like
       page size to the list, then please consider using a name more specific than just `Folio' or
       `Foolscap Folio'. */
    { "Banner 468x60",      60,  468, SP_UNIT_PX },
    // TODO: Select landscape by default.
    { "Icon 16x16",         16,   16, SP_UNIT_PX },
    { "Icon 32x32",         32,   32, SP_UNIT_PX },
    { "Icon 48x48",         48,   48, SP_UNIT_PX },
    { NULL,                  0,    0, SP_UNIT_PX },
};



//########################################################################
//# P A G E    S I Z E R
//########################################################################

//The default unit for this widget and its calculations
static const SPUnit _px_unit = sp_unit_get_by_id (SP_UNIT_PX);


/**
 * Constructor
 */
PageSizer::PageSizer(Registry & _wr)
    : Gtk::VBox(false,4),
      _dimensionUnits( _("U_nits:"), "units", _wr ),
      _dimensionWidth( _("_Width:"), _("Width of paper"), "width", _dimensionUnits, _wr ),
      _dimensionHeight( _("_Height:"), _("Height of paper"), "height", _dimensionUnits, _wr ),
      _widgetRegistry(&_wr)
{
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

    for (PaperSizeRec const *p = inkscape_papers; p->name; p++)
    {
        Glib::ustring name = p->name;
        char formatBuf[80];
        snprintf(formatBuf, 79, "%0.1f x %0.1f", p->smaller, p->larger);
        Glib::ustring desc = formatBuf;
        if (p->unit == SP_UNIT_IN)
            desc.append(" in");
        else if (p->unit == SP_UNIT_MM)
             desc.append(" mm");
        else if (p->unit == SP_UNIT_PX)
            desc.append(" px");
        PaperSize paper(name, p->smaller, p->larger, p->unit);
        _paperSizeTable[name] = paper;
        Gtk::TreeModel::Row row = *(_paperSizeListStore->append());
        row[_paperSizeListColumns.nameColumn] = name;
        row[_paperSizeListColumns.descColumn] = desc;
        }
    //Gtk::TreeModel::iterator iter = _paperSizeListStore->children().begin();
    //if (iter)
    //    _paperSizeListSelection->select(iter);


    pack_start (_paperSizeListBox, true, true, 0);
    _paperSizeListLabel.set_label(_("P_age size:"));
    _paperSizeListLabel.set_use_underline();
    _paperSizeListBox.pack_start (_paperSizeListLabel, false, false, 0);
    _paperSizeListLabel.set_mnemonic_widget (_paperSizeList);
    _paperSizeListBox.pack_start (_paperSizeListScroller, true, true, 0);

    //## Set up orientation radio buttons
    pack_start (_orientationBox, false, false, 0);
    _orientationLabel.set_label(_("Page orientation:"));
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

    //## Set up custom size frame
    _customFrame.set_label(_("Custom size"));
    pack_start (_customFrame, false, false, 0);
    _customTable.resize(2, 2);
    _customTable.set_border_width (4);
    _customTable.set_row_spacings (4);
    _customTable.set_col_spacings (4);
    _customTable.attach(_dimensionWidth, 0,1,0,1);
    _customTable.attach(_dimensionUnits, 1,2,0,1);
    _customTable.attach(_dimensionHeight, 0,1,1,2);
    _customTable.attach(_fitPageButton,              1,2,1,2);
    _customFrame.add(_customTable);

    _fitPageButton.set_use_underline();
    _fitPageButton.set_label(_("_Fit page to selection"));
    _tips.set_tip(_fitPageButton, _("Resize the page to fit the current selection, or the entire drawing if there is no selection"));
}


/**
 * Destructor
 */
PageSizer::~PageSizer()
{
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
    _fitPageButton.signal_clicked().connect(sigc::mem_fun(*this, &PageSizer::fire_fit_canvas_to_selection_or_drawing));

    show_all_children();
}


/**
 * Set document dimensions (if not called by Doc prop's update()) and
 * set the PageSizer's widgets and text entries accordingly. If
 * 'chageList' is true, then adjust the paperSizeList to show the closest
 * standard page size.
 *
 * \param w, h given in px
 * \param changeList whether to modify the paper size list
 */
void
PageSizer::setDim (double w, double h, bool changeList)
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

    if (SP_ACTIVE_DESKTOP && !_widgetRegistry->isUpdating()) {
        SPDocument *doc = sp_desktop_document(SP_ACTIVE_DESKTOP);
        sp_document_set_width (doc, w, &_px_unit);
        sp_document_set_height (doc, h, &_px_unit);
        sp_document_done (doc, SP_VERB_NONE, _("Set page size"));
    }

    _landscape = ( w > h );
    _landscapeButton.set_active(_landscape ? true : false);
    _portraitButton.set_active (_landscape ? false : true);

    if (changeList)
        {
        Gtk::TreeModel::Row row = (*find_paper_size(w, h));
        if (row)
            _paperSizeListSelection->select(row);
        }

    Unit const& unit = _dimensionUnits.getUnit();
    _dimensionWidth.setValue (w / unit.factor);
    _dimensionHeight.setValue (h / unit.factor);

    _paper_size_list_connection.unblock();
    _landscape_connection.unblock();
    _portrait_connection.unblock();
    _changedw_connection.unblock();
    _changedh_connection.unblock();

    _called = false;
}


/**
 * Returns an iterator pointing to a row in paperSizeListStore which
 * contains a paper of the specified size (specified in px), or
 * paperSizeListStore->children().end() if no such paper exists.
 */
Gtk::ListStore::iterator
PageSizer::find_paper_size (double w, double h) const
{
    double smaller = w;
    double larger  = h;
    if ( h < w ) {
        smaller = h; larger = w;
    }

    g_return_val_if_fail(smaller <= larger, _paperSizeListStore->children().end());

    std::map<Glib::ustring, PaperSize>::const_iterator iter;
    for (iter = _paperSizeTable.begin() ;
         iter != _paperSizeTable.end() ; iter++) {
        PaperSize paper = iter->second;
        SPUnit const &i_unit = sp_unit_get_by_id(paper.unit);
        double smallX = sp_units_get_pixels(paper.smaller, i_unit);
        double largeX = sp_units_get_pixels(paper.larger,  i_unit);

        g_return_val_if_fail(smallX <= largeX, _paperSizeListStore->children().end());

        if ((std::abs(smaller - smallX) <= 0.1) &&
            (std::abs(larger  - largeX) <= 0.1)   ) {
            Gtk::ListStore::iterator p;
            // We need to search paperSizeListStore explicitly for the
            // specified paper size because it is sorted in a different
            // way than paperSizeTable (which is sorted alphabetically)
            for (p = _paperSizeListStore->children().begin(); p != _paperSizeListStore->children().end(); p++) {
                if ((*p)[_paperSizeListColumns.nameColumn] == paper.name) {
                    return p;
                }
            }
        }
    }
    return _paperSizeListStore->children().end();
}



/**
 * Tell the desktop to change the page size
 */
void
PageSizer::fire_fit_canvas_to_selection_or_drawing()
{
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) {
        return;
    }
    Verb *verb = Verb::get( SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING );
    if (verb) {
        SPAction *action = verb->get_action(dt);
        if (action)
            sp_action_perform(action, NULL);
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
    double w = paper.smaller;
    double h = paper.larger;
    SPUnit const &src_unit = sp_unit_get_by_id (paper.unit);
    sp_convert_distance (&w, &src_unit, &_px_unit);
    sp_convert_distance (&h, &src_unit, &_px_unit);

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
    double w = _dimensionWidth.getValue ("px");
    double h = _dimensionHeight.getValue ("px");
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
    double w = _dimensionWidth.getValue ("px");
    double h = _dimensionHeight.getValue ("px");
    if (w < h) {
        setDim (h, w);
    }
}

/**
 * Callback for the dimension widgets
 */
void
PageSizer::on_value_changed()
{
    if (_widgetRegistry->isUpdating()) return;

    setDim (_dimensionWidth.getValue("px"),
            _dimensionHeight.getValue("px"));
}


} // namespace Widget
} // namespace UI
} // namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
