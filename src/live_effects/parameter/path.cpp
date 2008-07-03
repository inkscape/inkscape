#define INKSCAPE_LIVEPATHEFFECT_PARAMETER_PATH_CPP

/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "live_effects/parameter/path.h"
#include "live_effects/effect.h"
#include "libnr/n-art-bpath-2geom.h"
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
// clipboard support
#include "ui/clipboard.h"
// required for linking to other paths
#include "uri.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "display/curve.h"


namespace Inkscape {

namespace LivePathEffect {

PathParam::PathParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, const gchar * default_value)
    : Parameter(label, tip, key, wr, effect),
      _pathvector(),
      _pwd2(),
      must_recalculate_pwd2(false),
      href(NULL),
      ref( (SPObject*)effect->getLPEObj() )
{
    defvalue = g_strdup(default_value);
    param_readSVGValue(defvalue);
    oncanvas_editable = true;

    ref_changed_connection = ref.changedSignal().connect(sigc::mem_fun(*this, &PathParam::ref_changed));
}

PathParam::~PathParam()
{
    remove_link();

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
        remove_link();
        must_recalculate_pwd2 = true;

        if (strvalue[0] == '#') {
            if (href)
                g_free(href);
            href = g_strdup(strvalue);

            // Now do the attaching, which emits the changed signal.
            try {
                ref.attach(Inkscape::URI(href));
            } catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                ref.detach();
                _pathvector = sp_svg_read_pathv(defvalue);
            }
        } else {
            _pathvector = sp_svg_read_pathv(strvalue);
        }

        signal_path_changed.emit();
        return true;
    }

    return false;
}

gchar *
PathParam::param_getSVGValue() const
{
    if (href) {
        return href;
    } else {
        gchar * svgd = sp_svg_write_path( _pathvector );
        return svgd;
    }
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

    pIcon = Gtk::manage( sp_icon_get_icon( "edit_clone", Inkscape::ICON_SIZE_BUTTON) );
    pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_link_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    tooltips->set_tip(*pButton, _("Link to path"));

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
        shape_editor->set_item(ref.getObject());
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
    if (!href) {
        // TODO: recode this to apply transform to _pathvector instead?

        // only apply transform when not referring to other path
        ensure_pwd2();
        param_set_and_write_new_value( _pwd2 * postmul );
    }
}

void
PathParam::param_set_and_write_new_value (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & newpath)
{
    remove_link();
    _pathvector = Geom::path_from_piecewise(newpath, LPE_CONVERSION_TOLERANCE);
    gchar * svgd = sp_svg_write_path( _pathvector );
    param_write_to_repr(svgd);
    g_free(svgd);
    // force value upon pwd2 and don't recalculate.
    _pwd2 = newpath;
    must_recalculate_pwd2 = false;
}

void
PathParam::param_set_and_write_new_value (std::vector<Geom::Path> const & newpath)
{
    remove_link();
    _pathvector = newpath;
    must_recalculate_pwd2 = true;

    gchar * svgd = sp_svg_write_path( _pathvector );
    param_write_to_repr(svgd);
    g_free(svgd);
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
PathParam::start_listening(SPObject * to)
{
    if ( to == NULL ) {
        return;
    }
    linked_delete_connection = to->connectDelete(sigc::mem_fun(*this, &PathParam::linked_delete));
    linked_modified_connection = to->connectModified(sigc::mem_fun(*this, &PathParam::linked_modified));
    linked_modified(to, SP_OBJECT_MODIFIED_FLAG); // simulate linked_modified signal, so that path data is updated
}

void
PathParam::quit_listening(void)
{
    linked_modified_connection.disconnect();
    linked_delete_connection.disconnect();
}

void
PathParam::ref_changed(SPObject */*old_ref*/, SPObject *new_ref)
{
    quit_listening();
    if ( new_ref ) {
        start_listening(new_ref);
    }
}

void
PathParam::remove_link()
{
    if (href) {
        ref.detach();
        g_free(href);
        href = NULL;
    }
}

void
PathParam::linked_delete(SPObject */*deleted*/)
{
    quit_listening();
    remove_link();
    param_set_and_write_new_value (_pathvector);
}

void
PathParam::linked_modified(SPObject *linked_obj, guint /*flags*/)
{
    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(linked_obj)) {
        curve = sp_shape_get_curve(SP_SHAPE(linked_obj));
    }
    if (SP_IS_TEXT(linked_obj)) {
        curve = SP_TEXT(linked_obj)->getNormalizedBpath();
    }

    if (curve == NULL) {
        // curve invalid, set default value
        _pathvector = sp_svg_read_pathv(defvalue);
    } else {
        _pathvector = curve->get_pathvector();
        curve->unref();
    }

    must_recalculate_pwd2 = true;
    signal_path_changed.emit();
    SP_OBJECT(param_effect->getLPEObj())->requestModified(SP_OBJECT_MODIFIED_FLAG);
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
PathParam::paste_param_path(const char *svgd)
{
    if (svgd == "")
        return;

    // remove possible link to path
    remove_link();

    param_write_to_repr(svgd);
    signal_path_pasted.emit();
    sp_document_done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, 
                     _("Paste path parameter"));
}

void
PathParam::on_paste_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    Glib::ustring svgd = cm->getPathParameter();
    paste_param_path(svgd.data());
}

void
PathParam::on_copy_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    cm->copyPathParameter(this);
}

void
PathParam::on_link_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    Glib::ustring pathid = cm->getShapeOrTextObjectId();

    if (pathid == "") {
        return;
    }

    // add '#' at start to make it an uri.
    pathid.insert(pathid.begin(), '#');
    if ( href && strcmp(pathid.c_str(), href) == 0 ) {
        // no change, do nothing
        return;
    } else {
        // TODO:
        // check if id really exists in document, or only in clipboard document: if only in clipboard then invalid
        // check if linking to object to which LPE is applied (maybe delegated to PathReference

        param_write_to_repr(pathid.c_str());
        sp_document_done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT, 
                         _("Link path parameter to path"));
    }
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
