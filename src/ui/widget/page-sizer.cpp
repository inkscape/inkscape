/** \file
 *
 * Paper-size widget and helper functions
 *
 * Authors:
 *   bulia byak <buliabyak@users.sf.net>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon Phillips <jon@rejon.org>
 *   Ralf Stephan <ralf@ark.in-berlin.de> (Gtkmm)
 *
 * Copyright (C) 2000 - 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <cmath>
#include <gtkmm.h>
//#include <gtkmm/optionmenu.h>
//#include <gtkmm/frame.h>
//#include <gtkmm/table.h>
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
    char const * const name;
    double const smaller;
    double const larger;
    SPUnitId const unit;
};

static PaperSizeRec const inkscape_papers[] = {
    { "A4", 210, 297, SP_UNIT_MM },
    { "US Letter", 8.5, 11, SP_UNIT_IN },
    { "US Legal", 8.5, 14, SP_UNIT_IN },
    { "US Executive", 7.25, 10.5, SP_UNIT_IN },
    { "A0", 841, 1189, SP_UNIT_MM },
    { "A1", 594, 841, SP_UNIT_MM },
    { "A2", 420, 594, SP_UNIT_MM },
    { "A3", 297, 420, SP_UNIT_MM },
    { "A5", 148, 210, SP_UNIT_MM },
    { "A6", 105, 148, SP_UNIT_MM },
    { "A7", 74, 105, SP_UNIT_MM },
    { "A8", 52, 74, SP_UNIT_MM },
    { "A9", 37, 52, SP_UNIT_MM },
    { "A10", 26, 37, SP_UNIT_MM },
    { "B0", 1000, 1414, SP_UNIT_MM },
    { "B1", 707, 1000, SP_UNIT_MM },
    { "B2", 500, 707, SP_UNIT_MM },
    { "B3", 353, 500, SP_UNIT_MM },
    { "B4", 250, 353, SP_UNIT_MM },
    { "B5", 176, 250, SP_UNIT_MM },
    { "B6", 125, 176, SP_UNIT_MM },
    { "B7", 88, 125, SP_UNIT_MM },
    { "B8", 62, 88, SP_UNIT_MM },
    { "B9", 44, 62, SP_UNIT_MM },
    { "B10", 31, 44, SP_UNIT_MM },

#if 0 /* Whether to include or exclude these depends on how big we mind our page size menu
         becoming.  C series is used for envelopes; don't know what D and E series are used for. */
    { "C0", 917, 1297, SP_UNIT_MM },
    { "C1", 648, 917, SP_UNIT_MM },
    { "C2", 458, 648, SP_UNIT_MM },
    { "C3", 324, 458, SP_UNIT_MM },
    { "C4", 229, 324, SP_UNIT_MM },
    { "C5", 162, 229, SP_UNIT_MM },
    { "C6", 114, 162, SP_UNIT_MM },
    { "C7", 81, 114, SP_UNIT_MM },
    { "C8", 57, 81, SP_UNIT_MM },
    { "C9", 40, 57, SP_UNIT_MM },
    { "C10", 28, 40, SP_UNIT_MM },
    { "D1", 545, 771, SP_UNIT_MM },
    { "D2", 385, 545, SP_UNIT_MM },
    { "D3", 272, 385, SP_UNIT_MM },
    { "D4", 192, 272, SP_UNIT_MM },
    { "D5", 136, 192, SP_UNIT_MM },
    { "D6", 96, 136, SP_UNIT_MM },
    { "D7", 68, 96, SP_UNIT_MM },
    { "E3", 400, 560, SP_UNIT_MM },
    { "E4", 280, 400, SP_UNIT_MM },
    { "E5", 200, 280, SP_UNIT_MM },
    { "E6", 140, 200, SP_UNIT_MM },
#endif

    { "CSE", 462, 649, SP_UNIT_PT },
    { "US #10 Envelope", 4.125, 9.5, SP_UNIT_IN }, // TODO: Select landscape by default.
    /* See http://www.hbp.com/content/PCR_envelopes.cfm for a much larger list of US envelope
       sizes. */
    { "DL Envelope", 110, 220, SP_UNIT_MM }, // TODO: Select landscape by default.
    { "Ledger/Tabloid", 11, 17, SP_UNIT_IN },
    /* Note that `Folio' (used in QPrinter/KPrinter) is deliberately absent from this list, as it
       means different sizes to different people: different people may expect the width to be
       either 8, 8.25 or 8.5 inches, and the height to be either 13 or 13.5 inches, even
       restricting our interpretation to foolscap folio.  If you wish to introduce a folio-like
       page size to the list, then please consider using a name more specific than just `Folio' or
       `Foolscap Folio'. */
    { "Banner 468x60", 60, 468, SP_UNIT_PX },  // TODO: Select landscape by default.
    { "Icon 16x16", 16, 16, SP_UNIT_PX },
    { "Icon 32x32", 32, 32, SP_UNIT_PX },
    { NULL, 0, 0, SP_UNIT_PX },
};

//===================================================


static const SPUnit _px_unit = sp_unit_get_by_id (SP_UNIT_PX);


/*
class SizeMenuItem : public Gtk::MenuItem {
public:
    SizeMenuItem (PaperSizeRec const * paper, PageSizer * widget)
                : Gtk::MenuItem (paper ? paper->name : _("Custom")), 
                  _paper(paper),
	              _parent(widget)
	  {}
protected:
    PaperSizeRec const * _paper;
    PageSizer       *_parent;
    void            on_activate();
};

void
SizeMenuItem::on_activate()
{
    if (_parent == 0) // handle Custom entry
        return;
        
    double w = _paper->smaller, h = _paper->larger;
    SPUnit const &src_unit = sp_unit_get_by_id (_paper->unit);
    sp_convert_distance (&w, &src_unit, &_px_unit);
    sp_convert_distance (&h, &src_unit, &_px_unit);
    if (_parent->_landscape)
        _parent->setDim (h, w);
    else
        _parent->setDim (w, h);
}
*/

//---------------------------------------------------





PageSizer::PageSizer()
: Gtk::VBox(false,4)
{
    Gtk::HBox *hbox_size = manage (new Gtk::HBox (false, 4));
    pack_start (*hbox_size, false, false, 0);
    Gtk::Label *label_size = manage (new Gtk::Label (_("P_age size:"), 1.0, 0.5)); 
    label_size->set_use_underline();
    hbox_size->pack_start (*label_size, false, false, 0);
    label_size->set_mnemonic_widget (_paperSizeList);
    hbox_size->pack_start (_paperSizeList, true, true, 0);

    //# Set up the Paper Size combo box
    
    for (PaperSizeRec const *p = inkscape_papers; p->name; p++)
	    {
        Glib::ustring name = p->name;
        PaperSize paper(name, p->smaller, p->larger, p->unit);
        paperSizeTable[name] = paper;
        _paperSizeList.append_text(name);
        }

}

PageSizer::~PageSizer()
{
    _paper_size_list_connection.disconnect();
    _portrait_connection.disconnect();
    _landscape_connection.disconnect();
    _changedw_connection.disconnect();
    _changedh_connection.disconnect();
}




void
PageSizer::init (Registry& reg)
{
    Gtk::HBox *hbox_ori = manage (new Gtk::HBox);
    pack_start (*hbox_ori, false, false, 0);
    Gtk::Label *label_ori = manage (new Gtk::Label (_("Page orientation:"), 0.0, 0.5)); 
    hbox_ori->pack_start (*label_ori, false, false, 0);
    
    _landscapeButton.set_label(_("_Landscape"));
	_landscapeButton.set_active(true);
    Gtk::RadioButton::Group group = _landscapeButton.get_group();
    hbox_ori->pack_end (_landscapeButton, false, false, 5);
    _portraitButton.set_label(_("_Portrait"));
	_portraitButton.set_active(true);
    hbox_ori->pack_end (_portraitButton, false, false, 5);
    _portraitButton.set_group (group);
    _portraitButton.set_active (true);
    
    /* Custom paper frame */
    Gtk::Frame *frame = manage (new Gtk::Frame(_("Custom size")));
    pack_start (*frame, false, false, 0);
    Gtk::Table *table = manage (new Gtk::Table (5, 2, false));
    table->set_border_width (4);
    table->set_row_spacings (4);
    table->set_col_spacings (4);
    
    Inkscape::UI::Widget::Button* fit_canv =
	     manage(new Inkscape::UI::Widget::Button(_("_Fit page to selection"),
    _("Resize the page to fit the current selection, or the entire drawing if there is no selection")));

    // prevent fit_canv from expanding
    Gtk::Alignment *fit_canv_cont = manage(new Gtk::Alignment(1.0,0.5,0.0,0.0));
    fit_canv_cont->add(*fit_canv);

    frame->add (*table);
    
    _wr = &reg;

    _rum.init (_("U_nits:"), "units", *_wr);
    _rusw.init (_("_Width:"), _("Width of paper"), "width", _rum, *_wr);
    _rush.init (_("_Height:"), _("Height of paper"), "height", _rum, *_wr);

    table->attach (*_rum._label, 0,1,0,1, Gtk::FILL|Gtk::EXPAND,
	            (Gtk::AttachOptions)0,0,0);
    table->attach (*_rum._sel, 1,2,0,1, Gtk::FILL|Gtk::EXPAND,
	            (Gtk::AttachOptions)0,0,0);
    table->attach (*_rusw.getSU(), 0,2,1,2, Gtk::FILL|Gtk::EXPAND,
	            (Gtk::AttachOptions)0,0,0);
    table->attach (*_rush.getSU(), 0,2,2,3, Gtk::FILL|Gtk::EXPAND,
	           (Gtk::AttachOptions)0,0,0);
    table->attach (*fit_canv_cont, 0,2,3,4, Gtk::FILL|Gtk::EXPAND,
	           (Gtk::AttachOptions)0,0,0);

    _paper_size_list_connection = _paperSizeList.signal_changed().connect (
	        sigc::mem_fun (*this, &PageSizer::on_paper_size_list_changed));
	        
    _landscape_connection = _landscapeButton.signal_toggled().connect (
	        sigc::mem_fun (*this, &PageSizer::on_landscape));
    _portrait_connection = _portraitButton.signal_toggled().connect (
	        sigc::mem_fun (*this, &PageSizer::on_portrait));
    _changedw_connection = _rusw.getSU()->signal_value_changed().connect (
	        sigc::mem_fun (*this, &PageSizer::on_value_changed));
    _changedh_connection = _rush.getSU()->signal_value_changed().connect (
	        sigc::mem_fun (*this, &PageSizer::on_value_changed));
    fit_canv->signal_clicked().connect(
	     sigc::mem_fun(*this, &PageSizer::fire_fit_canvas_to_selection_or_drawing));
    
    show_all_children();
}


/**
 * Set document dimensions (if not called by Doc prop's update()) and
 * set the PageSizer's widgets and text entries accordingly. This is
 * somewhat slow, is there something done too often invisibly?
 *
 * \param w, h given in px
 */
void
PageSizer::setDim (double w, double h, bool changeList)
{
    static bool _called = false;
    if (_called)
	    return;

    _called = true;
    
    _paper_size_list_connection.block();
	_landscape_connection.block();
    _portrait_connection.block(); 
    _changedw_connection.block();
    _changedh_connection.block();

    if (SP_ACTIVE_DESKTOP && !_wr->isUpdating()) {
        SPDocument *doc = sp_desktop_document(SP_ACTIVE_DESKTOP);
        sp_document_set_width (doc, w, &_px_unit);
        sp_document_set_height (doc, h, &_px_unit);
        sp_document_done (doc, SP_VERB_NONE, 
                          /* TODO: annotate */ "page-sizer.cpp:301");
    } 
    
    _landscape = w>h;
    _landscapeButton.set_active(_landscape ? true : false);
    _portraitButton.set_active (_landscape ? false : true);
    
    if (changeList)
	    _paperSizeList.set_active (find_paper_size (w, h));
    
    Unit const& unit = _rum._sel->getUnit();
    _rusw.setValue (w / unit.factor);
    _rush.setValue (h / unit.factor);

    _paper_size_list_connection.unblock();
	_landscape_connection.unblock();
    _portrait_connection.unblock();
    _changedw_connection.unblock();
    _changedh_connection.unblock();
    
    _called = false;
}

/** 
 * Returns an index into inkscape_papers of a paper of the specified 
 * size (specified in px), or -1 if there's no such paper.
 */
int
PageSizer::find_paper_size (double w, double h) const
{
    double smaller = w;
    double larger  = h;
    if ( h < w ) {
        smaller = h; larger = w;
    }

    g_return_val_if_fail(smaller <= larger, -1);
    
    int index = 0;
    std::map<Glib::ustring, PaperSize>::const_iterator iter;
    for (iter = paperSizeTable.begin() ; iter != paperSizeTable.end() ; iter++) {
        PaperSize paper = iter->second;
        SPUnit const &i_unit = sp_unit_get_by_id(paper.unit);
        double smallX = sp_units_get_pixels(paper.smaller, i_unit);
        double largeX = sp_units_get_pixels(paper.larger,  i_unit);
        
        g_return_val_if_fail(smallX <= largeX, -1);
        
        if ((std::abs(smaller - smallX) <= 0.1) &&
            (std::abs(larger  - largeX) <= 0.1)   )
            return index;
            
        index++;
    }
    return -1;
}

void
PageSizer::fire_fit_canvas_to_selection_or_drawing() {
    SPDesktop *dt = SP_ACTIVE_DESKTOP;
    if (!dt) return;
    Verb *verb = Verb::get( SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING );
    if (verb) {
        SPAction *action = verb->get_action(dt);
        if (action) {
            sp_action_perform(action, NULL);        
        }
    }
}

void
PageSizer::on_paper_size_list_changed()
{
    Glib::ustring name = _paperSizeList.get_active_text();
    std::map<Glib::ustring, PaperSize>::const_iterator iter =
        paperSizeTable.find(name);
    if (iter == paperSizeTable.end()) {
        g_warning("paper size '%s' not found in table", name.c_str());
        return;
    }
    PaperSize paper = iter->second;
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

void
PageSizer::on_portrait()
{
    if (!_portraitButton.get_active())
        return;
    double w = _rusw.getSU()->getValue ("px");
    double h = _rush.getSU()->getValue ("px");
    if (h<w)
	    setDim (h, w);
}

void
PageSizer::on_landscape()
{
    if (!_landscapeButton.get_active())
        return;
    double w = _rusw.getSU()->getValue ("px");
    double h = _rush.getSU()->getValue ("px");
    if (w<h)
	    setDim (h, w);
}

void
PageSizer::on_value_changed()
{
    if (_wr->isUpdating()) return;

    setDim (_rusw.getSU()->getValue("px"),
	        _rush.getSU()->getValue("px"));
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
