/**
    \file bluredge.cpp
 
    A plug-in to add an effect to blur the edges of an object. 
*/
/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "desktop.h"
#include "selection.h"
#include "helper/action.h"
#include "prefs-utils.h"
#include "path-chemistry.h"
#include "sp-item.h"

#include "util/glib-list-iterators.h"

#include "extension/effect.h"
#include "extension/system.h"


#include "bluredge.h"

namespace Inkscape {
namespace Extension {
namespace Internal {


/**
    \brief  A function to allocated anything -- just an example here
    \param  module  Unused
    \return Whether the load was sucessful
*/
bool
BlurEdge::load (Inkscape::Extension::Extension *module)
{
    // std::cout << "Hey, I'm Blur Edge, I'm loading!" << std::endl;
    return TRUE;
}

/**
    \brief  This actually blurs the edge.
    \param  module   The effect that was called (unused)
    \param  document What should be edited.
*/
void
BlurEdge::effect (Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document)
{
    Inkscape::Selection * selection     = ((SPDesktop *)document)->selection;

    float width = module->get_param_float("blur-width");
    int   steps = module->get_param_int("num-steps");

    double old_offset = prefs_get_double_attribute("options.defaultoffsetwidth", "value", 1.0);

    using Inkscape::Util::GSListConstIterator;
    // TODO need to properly refcount the items, at least
    std::list<SPItem *> items;
    items.insert<GSListConstIterator<SPItem *> >(items.end(), selection->itemList(), NULL);
    selection->clear();

    std::list<SPItem *> new_items;
    for(std::list<SPItem *>::iterator item = items.begin();
            item != items.end(); item++) {
        SPItem * spitem = *item;

        Inkscape::XML::Node * new_items[steps];
        Inkscape::XML::Node * new_group = sp_repr_new("svg:g");
        (SP_OBJECT_REPR(spitem)->parent())->appendChild(new_group);
        /** \todo  Need to figure out how to get from XML::Node to SPItem */
        /* new_items.push_back(); */

        double orig_opacity = sp_repr_css_double_property(sp_repr_css_attr(SP_OBJECT_REPR(spitem), "style"), "opacity", 1.0);
        char opacity_string[64];
        g_ascii_formatd(opacity_string, sizeof(opacity_string), "%f",
                        orig_opacity / (steps));

        for (int i = 0; i < steps; i++) {
            double offset = (width / (float)(steps - 1) * (float)i) - (width / 2.0);

            new_items[i] = (SP_OBJECT_REPR(spitem))->duplicate();

            SPCSSAttr * css = sp_repr_css_attr(new_items[i], "style");
            sp_repr_css_set_property(css, "opacity", opacity_string);
            sp_repr_css_change(new_items[i], css, "style");

            new_group->appendChild(new_items[i]);
            selection->add(new_items[i]);
            sp_selected_path_to_curves();

            if (offset < 0.0) {
                /* Doing an inset here folks */
                offset *= -1.0;
                prefs_set_double_attribute("options.defaultoffsetwidth", "value", offset);
                sp_action_perform(Inkscape::Verb::get(SP_VERB_SELECTION_INSET)->get_action(document), NULL);
            } else if (offset == 0.0) {
            } else {
                prefs_set_double_attribute("options.defaultoffsetwidth", "value", offset);
                sp_action_perform(Inkscape::Verb::get(SP_VERB_SELECTION_OFFSET)->get_action(document), NULL);
            }

            selection->clear();
        }

    }

    prefs_set_double_attribute("options.defaultoffsetwidth", "value", old_offset);

    selection->clear();
    selection->add(items.begin(), items.end());
    selection->add(new_items.begin(), new_items.end());

    return;
}

Gtk::Widget *
BlurEdge::prefs_effect(Inkscape::Extension::Effect * module, Inkscape::UI::View::View * view)
{
    return module->autogui();
}

#include "clear-n_.h"

void
BlurEdge::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension>\n"
            "<name>" N_("Blur Edge") "</name>\n"
            "<id>org.inkscape.effect.bluredge</id>\n"
            "<param name=\"blur-width\" gui-text=\"" N_("Blur Width") "\" type=\"float\" min=\"1.0\" max=\"50.0\">1.0</param>\n"
            "<param name=\"num-steps\" gui-text=\"" N_("Number of Steps") "\" type=\"int\" min=\"5\" max=\"100\">11</param>\n"
            "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                    "<submenu name=\"" N_("Enhance Path") "\" />\n"
                "</effects-menu>\n"
            "</effect>\n"
        "</inkscape-extension>\n" , new BlurEdge());
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
