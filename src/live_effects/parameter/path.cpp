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
#include "xml/repr.h"
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
#include "nodepath.h"

namespace Inkscape {

namespace LivePathEffect {

PathParam::PathParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, const gchar * default_value)
    : Parameter(label, tip, key, wr, effect),
      _pathvector(),
      _pwd2(),
      must_recalculate_pwd2(false),
      href(NULL)
{
    defvalue = g_strdup(default_value);
    param_readSVGValue(defvalue);
    oncanvas_editable = true;
}

PathParam::~PathParam()
{
    g_free(defvalue);
}

std::vector<Geom::Path> const &
PathParam::get_pathvector()
{
    return _pathvector;
}

Geom::Piecewise<Geom::D2<Geom::SBasis> > const &
PathParam::get_pwd2()
{
    ensure_pwd2();
    return _pwd2;
}

void
PathParam::param_set_default()
{
    param_readSVGValue(defvalue);
}

void
PathParam::param_set_and_write_default()
{
    param_write_to_repr(defvalue);
}

bool
PathParam::param_readSVGValue(const gchar * strvalue)
{
    if (strvalue) {
        _pathvector.clear();
        if (href) {
            g_free(href);
            href = NULL;
        }
        must_recalculate_pwd2 = true;

        if (false /*if strvalue is xlink*/) {
            href = g_strdup(strvalue);
            update_from_referred();
            // TODO: add listener, because we must update when referred updates. we must always be up-to-date with referred path data
        } else {
            _pathvector = SVGD_to_2GeomPath(strvalue);
        }

        signal_path_changed.emit();
        return true;
    }

    return false;
}

gchar *
PathParam::param_writeSVGValue() const
{
    gchar * svgd = SVGD_from_2GeomPath( _pathvector );
    return svgd;
}

Gtk::Widget *
PathParam::param_newWidget(Gtk::Tooltips * tooltips)
{
    Gtk::HBox * _widget = Gtk::manage(new Gtk::HBox());

    Gtk::Label* pLabel = Gtk::manage(new Gtk::Label(param_label));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pLabel, true, true);
    tooltips->set_tip(*pLabel, param_tooltip);

    Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( "draw_node", Inkscape::ICON_SIZE_BUTTON) );
    Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_edit_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    tooltips->set_tip(*pButton, _("Edit on-canvas"));

    pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_COPY, Inkscape::ICON_SIZE_BUTTON) );
    pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_copy_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    tooltips->set_tip(*pButton, _("Copy path"));

    pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_PASTE, Inkscape::ICON_SIZE_BUTTON) );
    pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_paste_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    tooltips->set_tip(*pButton, _("Paste path"));

    static_cast<Gtk::HBox*>(_widget)->show_all_children();

    return dynamic_cast<Gtk::Widget *> (_widget);
}

void
PathParam::param_editOncanvas(SPItem * item, SPDesktop * dt)
{
    // If not already in nodecontext, goto it!
    if (!tools_isactive(dt, TOOLS_NODES)) {
        tools_switch_current(TOOLS_NODES);
    }

    ShapeEditor * shape_editor = SP_NODE_CONTEXT( dt->event_context )->shape_editor;
    if (!href) {
        shape_editor->set_item_lpe_path_parameter(item, SP_OBJECT(param_effect->getLPEObj()), param_key.c_str());
    } else {
        // set referred item for editing
    }
}

void
PathParam::param_setup_nodepath(Inkscape::NodePath::Path *np)
{
    np->show_helperpath = true;
    np->helperpath_rgba = 0x009000ff;
    np->helperpath_width = 1.0;
}

void
PathParam::param_transform_multiply(Geom::Matrix const& postmul, bool /*set*/)
{
    // TODO: recode this to apply transform to _pathvector instead?
    if (!href) {
        // only apply transform when not referring to other path
        ensure_pwd2();
        param_set_and_write_new_value( _pwd2 * postmul );
    }
}

void
PathParam::param_set_and_write_new_value (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & newpath)
{
    _pathvector = Geom::path_from_piecewise(newpath, LPE_CONVERSION_TOLERANCE);
    gchar * svgd = SVGD_from_2GeomPath( _pathvector );
    param_write_to_repr(svgd);
    g_free(svgd);
    // force value upon pwd2 and don't recalculate.
    _pwd2 = newpath;
    must_recalculate_pwd2 = false;
}

void
PathParam::ensure_pwd2()
{
    if (must_recalculate_pwd2) {
        _pwd2.clear();
        for (unsigned int i=0; i < _pathvector.size(); i++) {
            _pwd2.concat( _pathvector[i].toPwSb() );
        }

        must_recalculate_pwd2 = false;
    }
}

void
PathParam::update_from_referred()
{
    if (!href) {
        g_warning("PathParam::update_from_referred - logical error, this should not possible");
        return;
    }

    // TODO: implement!
    
    // optimize, only update from referred when referred changed.
}

/* CALLBACK FUNCTIONS FOR THE BUTTONS */
void
PathParam::on_edit_button_click()
{
    SPItem * item = sp_desktop_selection(SP_ACTIVE_DESKTOP)->singleItem();
    if (item != NULL) {
        param_editOncanvas(item, SP_ACTIVE_DESKTOP);
    }
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
PathParam::on_copy_button_click()
{
    sp_selection_copy_lpe_pathparam(this);
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
