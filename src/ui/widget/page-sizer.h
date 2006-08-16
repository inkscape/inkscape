/** \file
 * \brief Widget for specifying page size; part of Document Preferences dialog.
 *
 * Author:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_PAGE_SIZER__H
#define INKSCAPE_UI_WIDGET_PAGE_SIZER__H

#include <gtkmm.h>
#include <sigc++/sigc++.h>
#include "ui/widget/registry.h"
#include "ui/widget/registered-widget.h"
#include "helper/units.h"


namespace Inkscape {    
namespace UI {
namespace Widget {

/**
 * Data class used to store common paper dimensions.  Used to make
 * PageSizer's _paperSizeTable. 
 */ 
class PaperSize
{
public:

    /**
     * Default constructor
     */
    PaperSize()
        { init(); }

    /**
     * Main constructor.  Use this one.
     */
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

    /**
     * Copy constructor
     */
    PaperSize(const PaperSize &other)
        { assign(other); }
        
    /**
     * Assignment operator
     */	     
    PaperSize &operator=(const PaperSize &other)
        { assign(other); return *this; }

    /**
     * Destructor
     */	     
	virtual ~PaperSize()
	    {}
	    
    /**
     * Name of this paper specification
     */	     
    Glib::ustring name;
    
    /**
     * The lesser of the two dimensions
     */	     
    double smaller;
    
    /**
     * The greater of the two dimensions
     */	     
    double larger;
    
    /**
     * The units (px, pt, mm, etc) of this specification
     */	     
    SPUnitId unit;

private:

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

};





/**
 * A compound widget that allows the user to select the desired
 * page size.  This widget is used in DocumentPreferences 
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

    /**
     * Set up or reset this widget
     */	     
    void init (Registry& reg);
    
    /**
     * Set the page size to the given dimensions.  If 'changeList' is
     * true, then reset the paper size list to the closest match
     */
    void setDim (double w, double h, bool changeList=true);
 
protected:

    /**
     * Our handy table of all 'standard' paper sizes.
     */	     
    std::map<Glib::ustring, PaperSize> paperSizeTable;

    /**
     *	Find the closest standard paper size in the table, to the
     */
    int find_paper_size (double w, double h) const;
 
    void fire_fit_canvas_to_selection_or_drawing();
    
    //### Dimension spinboxes
    RegisteredUnitMenu   _dimensionUnits;
    RegisteredScalarUnit _dimensionWidth;
	RegisteredScalarUnit _dimensionHeight;
    //callback
    void on_value_changed();
    sigc::connection    _changedw_connection;
	sigc::connection    _changedh_connection;
    
    //### The Paper Size selection list
    Gtk::ComboBoxText _paperSizeList;
    //callback
    void on_paper_size_list_changed();
    sigc::connection    _paper_size_list_connection;
    
    //### Button to select 'portrait' orientation
    Gtk::RadioButton    _portraitButton;
    //callback
    void on_portrait();
    sigc::connection    _portrait_connection;

    //### Button to select 'landscape' orientation
	Gtk::RadioButton    _landscapeButton;
	//callback
    void on_landscape();
	sigc::connection    _landscape_connection;

    Registry            *_widgetRegistry;

    //### state - whether we are currently landscape or portrait
    bool                 _landscape;

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
