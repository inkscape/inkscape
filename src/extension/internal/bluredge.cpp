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

#include <vector>
#include "desktop.h"
#include "document.h"
#include "selection.h"
#include "helper/action.h"
#include "helper/action-context.h"
#include "preferences.h"
#include "path-chemistry.h"
#include "sp-item.h"

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
BlurEdge::load (Inkscape::Extension::Extension */*module*/)
{
    // std::cout << "Hey, I'm Blur Edge, I'm loading!" << std::endl;
    return TRUE;
}

/**
    \brief  This actually blurs the edge.
    \param  module   The effect that was called (unused)
    \param  desktop What should be edited.
*/
void
BlurEdge::effect (Inkscape::Extension::Effect *module, Inkscape::UI::View::View *desktop, Inkscape::Extension::Implementation::ImplementationDocumentCache * /*docCache*/)
{
    Inkscape::Selection * selection     = static_cast<SPDesktop *>(desktop)->selection;

    float width = module->get_param_float("blur-width");
    int   steps = module->get_param_int("num-steps");

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    double old_offset = prefs->getDouble("/options/defaultoffsetwidth/value", 1.0, "px");

    // TODO need to properly refcount the items, at least
    std::vector<SPItem*> items(selection->itemList());
    selection->clear();

    for(std::vector<SPItem*>::iterator item = items.begin();
            item != items.end(); ++item) {
        SPItem * spitem = *item;

        std::vector<Inkscape::XML::Node *> new_items(steps);
        Inkscape::XML::Document *xml_doc = desktop->doc()->getReprDoc();
        Inkscape::XML::Node * new_group = xml_doc->createElement("svg:g");
        spitem->getRepr()->parent()->appendChild(new_group);

        double orig_opacity = sp_repr_css_double_property(sp_repr_css_attr(spitem->getRepr(), "style"), "opacity", 1.0);
        char opacity_string[64];
        g_ascii_formatd(opacity_string, sizeof(opacity_string), "%f",
                        orig_opacity / (steps));

        for (int i = 0; i < steps; i++) {
            double offset = (width / (float)(steps - 1) * (float)i) - (width / 2.0);

            new_items[i] = spitem->getRepr()->duplicate(xml_doc);

            SPCSSAttr * css = sp_repr_css_attr(new_items[i], "style");
            sp_repr_css_set_property(css, "opacity", opacity_string);
            sp_repr_css_change(new_items[i], css, "style");

            new_group->appendChild(new_items[i]);
            selection->add(new_items[i]);
            sp_selected_path_to_curves(selection, static_cast<SPDesktop *>(desktop));

            if (offset < 0.0) {
                /* Doing an inset here folks */
                offset *= -1.0;
                prefs->setDoubleUnit("/options/defaultoffsetwidth/value", offset, "px");
                sp_action_perform(Inkscape::Verb::get(SP_VERB_SELECTION_INSET)->get_action(Inkscape::ActionContext(desktop)), NULL);
            } else if (offset > 0.0) {
                prefs->setDoubleUnit("/options/defaultoffsetwidth/value", offset, "px");
                sp_action_perform(Inkscape::Verb::get(SP_VERB_SELECTION_OFFSET)->get_action(Inkscape::ActionContext(desktop)), NULL);
            }

            selection->clear();
        }

        Inkscape::GC::release(new_group);
    }

    prefs->setDoubleUnit("/options/defaultoffsetwidth/value", old_offset, "px");

    selection->clear();
    selection->add(items.begin(), items.end());

    return;
}

Gtk::Widget *
BlurEdge::prefs_effect(Inkscape::Extension::Effect * module, Inkscape::UI::View::View * /*view*/, sigc::signal<void> * changeSignal, Inkscape::Extension::Implementation::ImplementationDocumentCache * /*docCache*/)
{
    return module->autogui(NULL, NULL, changeSignal);
}

#include "clear-n_.h"

void
BlurEdge::init (void)
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Inset/Outset Halo") "</name>\n"
            "<id>org.inkscape.effect.bluredge</id>\n"
            "<param name=\"blur-width\" _gui-text=\"" N_("Width:") "\" gui-description=\"" N_("Width in px of the halo") "\" scope=\"document\" type=\"float\" min=\"1.0\" max=\"50.0\">1.0</param>\n"
            "<param name=\"num-steps\" _gui-text=\"" N_("Number of steps:") "\" gui-description=\"" N_("Number of inset/outset copies of the object to make") "\" scope=\"document\" type=\"int\" min=\"5\" max=\"100\">11</param>\n"
            "<effect>\n"
                "<object-type>all</object-type>\n"
                "<effects-menu>\n"
                    "<submenu name=\"" N_("Generate from Path") "\" />\n"
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
