/**
    \file grid.cpp

    A plug-in to add a grid creation effect into Inkscape.
*/
/*
 * Copyright (C) 2004-2005  Ted Gould <ted@gould.cx>
 * Copyright (C) 2007  MenTaLguY <mental@rydia.net>
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

#include "svg/path-string.h"

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
Grid::load (Inkscape::Extension::Extension */*module*/)
{
    // std::cout << "Hey, I'm Grid, I'm loading!" << std::endl;
    return TRUE;
}

namespace {

void build_lines(int axis, Geom::Rect bounding_area, float offset,
                 float spacing, SVG::PathString &path_data)
{
    Geom::Point point_offset(0.0, 0.0);
    point_offset[axis] = offset;

    for (Geom::Point start_point = bounding_area.min();
            start_point[axis] + offset <= (bounding_area.max())[axis];
            start_point[axis] += spacing) {
        Geom::Point end_point = start_point;
        end_point[1-axis] = (bounding_area.max())[1-axis];

        path_data.moveTo(start_point + point_offset)
                 .lineTo(end_point + point_offset);
    }
}

}

/**
    \brief  This actually draws the grid.
    \param  module   The effect that was called (unused)
    \param  document What should be edited.
*/
void
Grid::effect (Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document, Inkscape::Extension::Implementation::ImplementationDocumentCache * /*docCache*/)
{
    Inkscape::Selection * selection     = ((SPDesktop *)document)->selection;

    Geom::Rect bounding_area = Geom::Rect(Geom::Point(0,0), Geom::Point(100,100));
    if (selection->isEmpty()) {
        /* get page size */
        SPDocument * doc = document->doc();
        bounding_area = Geom::Rect(  Geom::Point(0,0),
                                     Geom::Point(sp_document_width(doc), sp_document_height(doc))  );
    } else {
        Geom::OptRect bounds = selection->bounds();
        if (bounds) {
            bounding_area = *bounds;
        }
        Geom::Rect temprec = Geom::Rect(Geom::Point(bounding_area.min()[Geom::X], bounding_area.min()[Geom::Y]),
                                    Geom::Point(bounding_area.max()[Geom::X], bounding_area.max()[Geom::Y]));

        bounding_area = temprec;
    }

    float spacings[2] = { module->get_param_float("xspacing"),
                          module->get_param_float("yspacing") };
    float line_width = module->get_param_float("lineWidth");
    float offsets[2] = { module->get_param_float("xoffset"),
                         module->get_param_float("yoffset") };

    SVG::PathString path_data;
    for ( int axis = 0 ; axis < 2 ; ++axis ) {
        build_lines(axis, bounding_area, offsets[axis], spacings[axis], path_data);
    }

    Inkscape::XML::Document * xml_doc = sp_document_repr_doc(document->doc());
    Inkscape::XML::Node * current_layer = static_cast<SPDesktop *>(document)->currentLayer()->repr;
    Inkscape::XML::Node * path = xml_doc->createElement("svg:path");

    path->setAttribute("d", path_data.c_str());

    Glib::ustring style("fill:none;fill-opacity:0.75000000;fill-rule:evenodd;stroke:#000000;stroke-linecap:butt;stroke-linejoin:miter;stroke-opacity:1.0000000");
    style += ";stroke-width:";
    gchar floatstring[64];
    sprintf(floatstring, "%f", line_width);
    style += floatstring;
    style += "pt";
    path->setAttribute("style", style.c_str());

    current_layer->appendChild(path);
		Inkscape::GC::release(path);

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
Grid::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal, Inkscape::Extension::Implementation::ImplementationDocumentCache * /*docCache*/)
{
    SPDocument * current_document = view->doc();

    using Inkscape::Util::GSListConstIterator;
    GSListConstIterator<SPItem *> selected = sp_desktop_selection((SPDesktop *)view)->itemList();
    Inkscape::XML::Node * first_select = NULL;
    if (selected != NULL)
        first_select = SP_OBJECT_REPR(*selected);

    return module->autogui(current_document, first_select, changeSignal);
}

#include "clear-n_.h"

void
Grid::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Grid") "</name>\n"
            "<id>org.inkscape.effect.grid</id>\n"
            "<param name=\"lineWidth\" gui-text=\"" N_("Line Width") "\" type=\"float\">1.0</param>\n"
            "<param name=\"xspacing\" gui-text=\"" N_("Horizontal Spacing") "\" type=\"float\" min=\"0.1\" max=\"1000\">10.0</param>\n"
            "<param name=\"yspacing\" gui-text=\"" N_("Vertical Spacing") "\" type=\"float\" min=\"0.1\" max=\"1000\">10.0</param>\n"
            "<param name=\"xoffset\" gui-text=\"" N_("Horizontal Offset") "\" type=\"float\" min=\"0.0\" max=\"1000\">0.0</param>\n"
            "<param name=\"yoffset\" gui-text=\"" N_("Vertical Offset") "\" type=\"float\" min=\"0.0\" max=\"1000\">0.0</param>\n"
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
