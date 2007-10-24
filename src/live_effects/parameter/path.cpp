#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_PATH_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/path.h"
#include "live_effects/effect.h"
#include "live_effects/n-art-bpath-2geom.h"
#include "svg/svg.h"
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/d2.h>

#include "ui/widget/point.h"
#include "widgets/icon.h"
#include <gtk/gtkstock.h>
#include "selection-chemistry.h"

#include "desktop.h"
#include "inkscape.h"
#include "message-stack.h"
#include "verbs.h"
#include "document.h"

// needed for on-canvas editting:
#include "tools-switch.h"
#include "shape-editor.h"
#include "node-context.h"
#include "desktop-handles.h"
#include "selection.h"

namespace Inkscape {

namespace LivePathEffect {

PathParam::PathParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, const gchar * default_value)
    : Parameter(label, tip, key, wr, effect)
{
    _widget = NULL;
    _tooltips = NULL;
    edit_button = NULL;
    defvalue = g_strdup(default_value);
    param_readSVGValue(defvalue);
}

PathParam::~PathParam()
{
    if (_tooltips)
        delete _tooltips;
    // _widget is managed by GTK so do not delete!

    g_free(defvalue);
}

void
PathParam::param_set_default()
{
    param_readSVGValue(defvalue);
}

bool
PathParam::param_readSVGValue(const gchar * strvalue)
{
    if (strvalue) {
        Geom::Piecewise<Geom::D2<Geom::SBasis> > newpath;
        std::vector<Geom::Path> temppath = SVGD_to_2GeomPath(strvalue);
        for (unsigned int i=0; i < temppath.size(); i++) {
            newpath.concat( temppath[i].toPwSb() );
        }
        *( dynamic_cast<Geom::Piecewise<Geom::D2<Geom::SBasis> > *> (this) ) = newpath;
        signal_path_changed.emit();
        return true;
    }

    return false;
}

gchar *
PathParam::param_writeSVGValue() const
{
    const std::vector<Geom::Path> temppath =
        Geom::path_from_piecewise(* dynamic_cast<const Geom::Piecewise<Geom::D2<Geom::SBasis> > *> (this), LPE_CONVERSION_TOLERANCE);
    gchar * svgd = SVGD_from_2GeomPath( temppath );
    return svgd;
}

Gtk::Widget *
PathParam::param_getWidget()
{
    if (!_widget) {
        _widget = Gtk::manage(new Gtk::HBox());
        _tooltips = new Gtk::Tooltips();

        Gtk::Label* pLabel = Gtk::manage(new Gtk::Label(param_label));
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pLabel, true, true);
        _tooltips->set_tip(*pLabel, param_tooltip);

        Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( "draw_node", Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_edit_button_click));
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
        _tooltips->set_tip(*pButton, _("Edit on-canvas"));
        edit_button = pButton;

        pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_PASTE, Inkscape::ICON_SIZE_BUTTON) );
        pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_paste_button_click));
        static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
        _tooltips->set_tip(*pButton, _("Paste path"));

        static_cast<Gtk::HBox*>(_widget)->show_all_children();

    }
    return dynamic_cast<Gtk::Widget *> (_widget);
}

void
PathParam::on_edit_button_click()
{
    // Switch to node edit tool:
    tools_switch_current(TOOLS_NODES);

    // set this parameter to edit:
    ShapeEditor * shape_editor = SP_NODE_CONTEXT( SP_ACTIVE_DESKTOP->event_context )->shape_editor;
    SPItem * item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    shape_editor->set_item_livepatheffect_parameter(item, SP_OBJECT(param_effect->getLPEObj()), param_key.c_str());
}

void
PathParam::on_paste_button_click()
{
    // check if something is in the clipboard
    GSList * clipboard = sp_selection_get_clipboard();
    if (clipboard == NULL || clipboard->data == NULL) {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Nothing on the clipboard."));
        return;
    }

    Inkscape::XML::Node *repr = (Inkscape::XML::Node *) clipboard->data;
    if (!strcmp (repr->name(), "svg:path")) {
        const char * svgd = repr->attribute("d");
        if (svgd) {
            if (strchr(svgd,'A')) { // FIXME: temporary hack until 2Geom supports arcs in SVGD
                SP_ACTIVE_DESKTOP->messageStack()->flash( Inkscape::WARNING_MESSAGE,
                            _("This effect does not support arcs yet, try to convert to path.") );
                return;
            } else {
                param_write_to_repr(svgd);
                signal_path_pasted.emit();
                sp_document_done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, 
                                 _("Paste path parameter"));
            }
        }
    } else {
        SP_ACTIVE_DESKTOP->messageStack()->flash(Inkscape::WARNING_MESSAGE, _("Clipboard does not contain a path."));
        return;
    }
}

void
PathParam::param_write_to_repr(const char * svgd)
{
    param_effect->getRepr()->setAttribute(param_key.c_str(), svgd);
}


} /* namespace LivePathEffect */

} /* namespace Inkscape */

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
