/** \file
 * \brief Widget for specifying page size; part of Document Preferences dialog.
 *
 * Author:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_PAGE_SIZER__H
#define INKSCAPE_UI_WIDGET_PAGE_SIZER__H

#include <sigc++/sigc++.h>
#include <gtkmm.h>
#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"
#include "helper/units.h"


namespace Inkscape {    
namespace UI {
namespace Widget {

/**
 * Class used to store common paper dimensions
 */ 
class PaperSize
{
public:
    PaperSize()
        { init(); }
    PaperSize(const Glib::ustring &nameArg,
	          double smallerArg,
	          double largerArg,
			  SPUnitId unitArg)
	    {
	    name    = nameArg;
	    smaller = smallerArg;
	    larger  = largerArg;
	    unit    = unitArg;
	    }

    PaperSize(const PaperSize &other)
        { assign(other); }
        
    PaperSize &operator=(const PaperSize &other)
        { assign(other); return *this; }

	virtual ~PaperSize()
	    {}
	    
	void init()
	    {
	    name    = "";
	    smaller = 0.0;
	    larger  = 0.0;
	    unit    = SP_UNIT_PX;
	    }

	void assign(const PaperSize &other)
	    {
	    name    = other.name;
	    smaller = other.smaller;
	    larger  = other.larger;
	    unit    = other.unit;
        }

    Glib::ustring name;
    double smaller;
    double larger;
    SPUnitId unit;
};





/**
 * Widget containing all widgets for specifying page size.
 */ 
class PageSizer : public Gtk::VBox
{
public:

    /**
     * Constructor
     */
    PageSizer();

    /**
     * Destructor
     */
    virtual ~PageSizer();

    void init (Registry& reg);
    void setDim (double w, double h, bool changeList=true);
    bool                 _landscape;

protected:

    int find_paper_size (double w, double h) const;
    void fire_fit_canvas_to_selection_or_drawing();
    void on_portrait();
    void on_landscape();
    void on_value_changed();
    void on_paper_size_list_changed();
    
    RegisteredUnitMenu   _rum;
    RegisteredScalarUnit _rusw, _rush;
    
    //# Various things for a ComboBox
    Gtk::ComboBoxText _paperSizeList;
    std::map<Glib::ustring, PaperSize> paperSizeTable;
    
    Gtk::RadioButton    _portraitButton;
	Gtk::RadioButton    _landscapeButton;
    sigc::connection    _paper_size_list_connection;
    sigc::connection    _portrait_connection;
	sigc::connection    _landscape_connection;
    sigc::connection    _changedw_connection;
	sigc::connection    _changedh_connection;
    Registry            *_wr;
};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif /* INKSCAPE_UI_WIDGET_PAGE_SIZER__H */

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
