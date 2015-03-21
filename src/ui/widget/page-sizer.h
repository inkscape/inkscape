/*
 * Author:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_UI_WIDGET_PAGE_SIZER_H
#define INKSCAPE_UI_WIDGET_PAGE_SIZER_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <stddef.h>
#include "ui/widget/registered-widget.h"
#include <sigc++/sigc++.h>

#include "util/units.h"

#include <gtkmm/alignment.h>
#include <gtkmm/expander.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>

#if WITH_GTKMM_3_0
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <gtkmm/radiobutton.h>

namespace Inkscape {    
namespace XML {
class Node;
}

namespace UI {
namespace Widget {

class Registry;

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
			  Inkscape::Util::Unit const *unitArg)
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
    Inkscape::Util::Unit const *unit; /// pointer to object in UnitTable, do not delete

private:

	void init()
	    {
	    name    = "";
	    smaller = 0.0;
	    larger  = 0.0;
	    unit    = unit_table.getUnit("px");
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
    PageSizer(Registry & _wr);

    /**
     * Destructor
     */
    virtual ~PageSizer();

    /**
     * Set up or reset this widget
     */	     
    void init ();
    
    /**
     * Set the page size to the given dimensions.  If 'changeList' is
     * true, then reset the paper size list to the closest match
     */
    void setDim (Inkscape::Util::Quantity w, Inkscape::Util::Quantity h, bool changeList=true, bool changeSize=true);
    
    /**
     * Updates the scalar widgets for the fit margins.  (Just changes the value
     * of the ui widgets to match the xml).
     */
    void updateFitMarginsUI(Inkscape::XML::Node *nv_repr);
 
    /**
     * Updates the scale widgets. (Just changes the values of the ui widgets.)
     */
    void updateScaleUI();

protected:

    /**
     * Our handy table of all 'standard' paper sizes.
     */	     
    std::map<Glib::ustring, PaperSize> _paperSizeTable;

    /**
     *	Find the closest standard paper size in the table, to the
     */
    Gtk::ListStore::iterator find_paper_size (Inkscape::Util::Quantity w, Inkscape::Util::Quantity h) const;
 
    void fire_fit_canvas_to_selection_or_drawing();
    
    //### The Paper Size selection list
    Gtk::HBox _paperSizeListBox;
    Gtk::Label _paperSizeListLabel;
    class PaperSizeColumns : public Gtk::TreeModel::ColumnRecord
        {
        public:
            PaperSizeColumns()
               { add(nameColumn); add(descColumn);  }
            Gtk::TreeModelColumn<Glib::ustring> nameColumn;
            Gtk::TreeModelColumn<Glib::ustring> descColumn;
        };

    PaperSizeColumns _paperSizeListColumns;
    Glib::RefPtr<Gtk::ListStore> _paperSizeListStore;
    Gtk::TreeView _paperSizeList;
    Glib::RefPtr<Gtk::TreeSelection> _paperSizeListSelection;
    Gtk::ScrolledWindow  _paperSizeListScroller;
    //callback
    void on_paper_size_list_changed();
    sigc::connection    _paper_size_list_connection;
    
    //### Portrait or landscape orientation
    Gtk::HBox           _orientationBox;
    Gtk::Label          _orientationLabel;
    Gtk::RadioButton    _portraitButton;
    Gtk::RadioButton    _landscapeButton;
    //callbacks
    void on_portrait();
    void on_landscape();
    sigc::connection    _portrait_connection;
    sigc::connection    _landscape_connection;

    //### Custom size frame
    Gtk::Frame           _customFrame;

#if WITH_GTKMM_3_0
    Gtk::Grid            _customDimTable;
#else
    Gtk::Table           _customDimTable;
#endif

    RegisteredUnitMenu   _dimensionUnits;
    RegisteredScalarUnit _dimensionWidth;
    RegisteredScalarUnit _dimensionHeight;
    GList *              _dimTabOrderGList;

    //### Fit Page options
    Gtk::Expander        _fitPageMarginExpander;

#if WITH_GTKMM_3_0
    Gtk::Grid            _marginTable;
#else
    Gtk::Table           _marginTable;
#endif

    Gtk::Alignment       _marginTopAlign;
    Gtk::Alignment       _marginLeftAlign;
    Gtk::Alignment       _marginRightAlign;
    Gtk::Alignment       _marginBottomAlign;
    RegisteredScalar     _marginTop;
    RegisteredScalar     _marginLeft;
    RegisteredScalar     _marginRight;
    RegisteredScalar     _marginBottom;
    Gtk::Alignment       _fitPageButtonAlign;
    Gtk::Button          _fitPageButton;
    bool                 _lockMarginUpdate;

    // Document scale
    Gtk::Frame           _scaleFrame;
#if WITH_GTKMM_3_0
    Gtk::Grid            _scaleTable;
#else
    Gtk::Table           _scaleTable;
#endif

    Gtk::Label           _scaleLabel;
    Gtk::Label           _scaleWarning;
    RegisteredScalar     _scaleX;
    RegisteredScalar     _scaleY;
    bool                 _lockScaleUpdate;

    // Viewbox
    Gtk::Expander        _viewboxExpander;
#if WITH_GTKMM_3_0
    Gtk::Grid            _viewboxTable;
#else
    Gtk::Table           _viewboxTable;
#endif

    RegisteredScalar     _viewboxX;
    RegisteredScalar     _viewboxY;
    RegisteredScalar     _viewboxW;
    RegisteredScalar     _viewboxH;
    bool                 _lockViewboxUpdate;

    //callback
    void on_value_changed();
    void on_units_changed();
    void on_scale_changed();
    void on_viewbox_changed();
    sigc::connection    _changedw_connection;
    sigc::connection    _changedh_connection;
    sigc::connection    _changedu_connection;
    sigc::connection    _changeds_connection;
    sigc::connection    _changedvx_connection;
    sigc::connection    _changedvy_connection;
    sigc::connection    _changedvw_connection;
    sigc::connection    _changedvh_connection;

    Registry            *_widgetRegistry;

    //### state - whether we are currently landscape or portrait
    bool                 _landscape;
    
    Glib::ustring       _unit;

};

} // namespace Widget
} // namespace UI
} // namespace Inkscape


#endif // INKSCAPE_UI_WIDGET_PAGE_SIZER_H

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
