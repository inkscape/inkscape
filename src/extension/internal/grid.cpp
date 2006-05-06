/**
    \file grid.cpp
 
    A plug-in to add a grid creation effect into Inkscape.
*/
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "sp-object.h"
#include "util/glib-list-iterators.h"

#include "extension/effect.h"
#include "extension/system.h"


#include "grid.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
    \brief  A function to allocated anything -- just an example here
    \param  module  Unused
    \return Whether the load was sucessful
*/
bool
Grid::load (Inkscape::Extension::Extension *module)
{
    // std::cout << "Hey, I'm Grid, I'm loading!" << std::endl;
    return TRUE;
}

/**
    \brief  This actually draws the grid.
    \param  module   The effect that was called (unused)
    \param  document What should be edited.
*/
void
Grid::effect (Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document)
{
    Inkscape::Selection * selection     = ((SPDesktop *)document)->selection;

    NR::Rect bounding_area = NR::Rect(NR::Point(0,0), NR::Point(100,100));
    if (selection->isEmpty()) {
        /* get page size */
        SPDocument * doc = document->doc();
        bounding_area = NR::Rect(NR::Point(0,0),
                                 NR::Point(sp_document_width(doc),
                                           sp_document_height(doc)));
    } else {
        bounding_area = selection->bounds();

        gdouble doc_height  =  sp_document_height(document->doc());
        NR::Rect temprec = NR::Rect(NR::Point(bounding_area.min()[NR::X], doc_height - bounding_area.min()[NR::Y]),
                                    NR::Point(bounding_area.max()[NR::X], doc_height - bounding_area.max()[NR::Y]));

        bounding_area = temprec;
    }


    float xspacing = module->get_param_float("xspacing");
    float yspacing = module->get_param_float("yspacing");
    float line_width = module->get_param_float("lineWidth");
    float xoffset = module->get_param_float("xoffset");
    float yoffset = module->get_param_float("yoffset");

    // std::cout << "Spacing: " << spacing;
    // std::cout << " Line Width: " << line_width;
    // std::cout << " Offset: " << offset << std::endl;

    Glib::ustring path_data;

    for (NR::Point start_point = bounding_area.min();
            start_point[NR::X] + xoffset <= (bounding_area.max())[NR::X];
            start_point[NR::X] += xspacing) {
        NR::Point end_point = start_point;
        end_point[NR::Y] = (bounding_area.max())[NR::Y];
        gchar floatstring[64];

        path_data += "M ";
        sprintf(floatstring, "%f", start_point[NR::X] + xoffset);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", start_point[NR::Y]);
        path_data += floatstring;
        path_data += " L ";
        sprintf(floatstring, "%f", end_point[NR::X] + xoffset);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", end_point[NR::Y]);
        path_data += floatstring;
        path_data += " ";
    }

    for (NR::Point start_point = bounding_area.min();
            start_point[NR::Y] + yoffset <= (bounding_area.max())[NR::Y];
            start_point[NR::Y] += yspacing) {
        NR::Point end_point = start_point;
        end_point[NR::X] = (bounding_area.max())[NR::X];
        gchar floatstring[64];

        path_data += "M ";
        sprintf(floatstring, "%f", start_point[NR::X]);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", start_point[NR::Y] + yoffset);
        path_data += floatstring;
        path_data += " L ";
        sprintf(floatstring, "%f", end_point[NR::X]);
        path_data += floatstring;
        path_data += " ";
        sprintf(floatstring, "%f", end_point[NR::Y] + yoffset);
        path_data += floatstring;
        path_data += " ";
    }

    // std::cout << "Path Data: " << path_data << std::endl;

    Inkscape::XML::Node * current_layer = ((SPDesktop *)document)->currentLayer()->repr;
    Inkscape::XML::Node * path = sp_repr_new("svg:path");

    path->setAttribute("d", path_data.c_str());

    Glib::ustring style("fill:none;fill-opacity:0.75000000;fill-rule:evenodd;stroke:#000000;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000");
    style += ";stroke-width:";
    gchar floatstring[64];
    sprintf(floatstring, "%f", line_width);
    style += floatstring;
    style += "pt";
    path->setAttribute("style", style.c_str());

    // Glib::ustring transform("scale(1.25 1.25)");
    // path->setAttribute("transform", transform.c_str());

    current_layer->appendChild(path);

    return;
}

/** \brief  A class to make an adjustment that uses Extension params */
class PrefAdjustment : public Gtk::Adjustment {
    /** Extension that this relates to */
    Inkscape::Extension::Extension * _ext;
    /** The string which represents the parameter */
    char * _pref;
public:
    /** \brief  Make the adjustment using an extension and the string
                describing the parameter. */
    PrefAdjustment(Inkscape::Extension::Extension * ext, char * pref) :
            Gtk::Adjustment(0.0, 0.0, 10.0, 0.1), _ext(ext), _pref(pref) {
        this->set_value(_ext->get_param_float(_pref)); 
        this->signal_value_changed().connect(sigc::mem_fun(this, &PrefAdjustment::val_changed));
        return;
    };

    void val_changed (void);
}; /* class PrefAdjustment */

/** \brief  A function to respond to the value_changed signal from the
            adjustment.

    This function just grabs the value from the adjustment and writes
    it to the parameter.  Very simple, but yet beautiful.
*/
void
PrefAdjustment::val_changed (void)
{
    // std::cout << "Value Changed to: " << this->get_value() << std::endl;
    _ext->set_param_float(_pref, this->get_value());
    return;
}

/** \brief  A function to get the prefences for the grid
    \param  moudule  Module which holds the params
    \param  view     Unused today - may get style information in the future.

    Uses AutoGUI for creating the GUI.
*/
Gtk::Widget *
Grid::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view)
{
    SPDocument * current_document = view->doc();

    using Inkscape::Util::GSListConstIterator;
    GSListConstIterator<SPItem *> selected = sp_desktop_selection((SPDesktop *)view)->itemList();
    Inkscape::XML::Node * first_select = NULL;
    if (selected != NULL) 
        first_select = SP_OBJECT_REPR(*selected);

    return module->autogui(current_document, first_select);
}

#include "clear-n_.h"

void
Grid::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
            "<name>" N_("Grid") "</name>\n"
            "<id>org.inkscape.effect.grid</id>\n"
            "<param name=\"lineWidth\" gui-text=\"" N_("Line Width") "\" type=\"float\">1.0</param>\n"
            "<param name=\"xspacing\" gui-text=\"" N_("Horizontal Spacing") "\" type=\"float\">10.0</param>\n"
            "<param name=\"yspacing\" gui-text=\"" N_("Vertical Spacing") "\" type=\"float\">10.0</param>\n"
            "<param name=\"xoffset\" gui-text=\"" N_("Horizontal Offset") "\" type=\"float\">5.0</param>\n"
            "<param name=\"yoffset\" gui-text=\"" N_("Vertical Offset") "\" type=\"float\">5.0</param>\n"
            "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                    "<submenu name=\"" N_("Render") "\" />\n"
                "</effects-menu>\n"
                "<menu-tip>" N_("Draw a path which is a grid") "</menu-tip>\n"
            "</effect>\n"
        "</inkscape-extension>\n", new Grid());
    return;
}

}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
