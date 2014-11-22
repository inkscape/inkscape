/*
 * Copyright (C) Johan Engelen 2007 <j.b.c.engelen@utwente.nl>
 *   Abhishek Sharma
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "ui/widget/point.h"
#include <glibmm/i18n.h>

#include "live_effects/parameter/path.h"
#include "live_effects/effect.h"
#include "svg/svg.h"
#include <2geom/svg-path-parser.h>
#include <2geom/sbasis-to-bezier.h>
#include <2geom/pathvector.h>
#include <2geom/d2.h>

#include "widgets/icon.h"
#include <gtk/gtk.h>
#include "selection-chemistry.h"
#include "xml/repr.h"
#include "desktop.h"
#include "inkscape.h"
#include "message-stack.h"
#include "verbs.h"
#include "document.h"
#include "document-undo.h"

// needed for on-canvas editting:
#include "ui/tools-switch.h"
#include "ui/shape-editor.h"
#include "desktop-handles.h"
#include "selection.h"
// clipboard support
#include "ui/clipboard.h"
// required for linking to other paths
#include "uri.h"
#include "sp-shape.h"
#include "sp-text.h"
#include "display/curve.h"

#include "ui/tools/node-tool.h"
#include "ui/tool/multi-path-manipulator.h"
#include "ui/tool/shape-record.h"

#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include "ui/icon-names.h"

namespace Inkscape {

namespace LivePathEffect {

PathParam::PathParam( const Glib::ustring& label, const Glib::ustring& tip,
                      const Glib::ustring& key, Inkscape::UI::Widget::Registry* wr,
                      Effect* effect, const gchar * default_value)
    : Parameter(label, tip, key, wr, effect),
      changed(true),
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
PathParam::get_pathvector() const
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
                //lp:1299948
                SPItem* i = ref.getObject();
                if (i) {
                    linked_modified_callback(i, SP_OBJECT_MODIFIED_FLAG);
                } // else: document still processing new events. Repr of the linked object not created yet.
            } catch (Inkscape::BadURIException &e) {
                g_warning("%s", e.what());
                ref.detach();
                _pathvector = sp_svg_read_pathv(defvalue);
            }
        } else {
            _pathvector = sp_svg_read_pathv(strvalue);
        }

        emit_changed();
        return true;
    }

    return false;
}

gchar *
PathParam::param_getSVGValue() const
{
    if (href) {
        return g_strdup(href);
    } else {
        gchar * svgd = sp_svg_write_path( _pathvector );
        return svgd;
    }
}

Gtk::Widget *
PathParam::param_newWidget()
{
    Gtk::HBox * _widget = Gtk::manage(new Gtk::HBox());

    Gtk::Label* pLabel = Gtk::manage(new Gtk::Label(param_label));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pLabel, true, true);
    pLabel->set_tooltip_text(param_tooltip);

    Gtk::Widget*  pIcon = Gtk::manage( sp_icon_get_icon( INKSCAPE_ICON("tool-node-editor"), Inkscape::ICON_SIZE_BUTTON) );
    Gtk::Button * pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_edit_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    pButton->set_tooltip_text(_("Edit on-canvas"));

    pIcon = Gtk::manage( sp_icon_get_icon( INKSCAPE_ICON("edit-copy"), Inkscape::ICON_SIZE_BUTTON) );
    pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_copy_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    pButton->set_tooltip_text(_("Copy path"));

    pIcon = Gtk::manage( sp_icon_get_icon( INKSCAPE_ICON("edit-paste"), Inkscape::ICON_SIZE_BUTTON) );
    pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_paste_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    pButton->set_tooltip_text(_("Paste path"));

    pIcon = Gtk::manage( sp_icon_get_icon( INKSCAPE_ICON("edit-clone"), Inkscape::ICON_SIZE_BUTTON) );
    pButton = Gtk::manage(new Gtk::Button());
    pButton->set_relief(Gtk::RELIEF_NONE);
    pIcon->show();
    pButton->add(*pIcon);
    pButton->show();
    pButton->signal_clicked().connect(sigc::mem_fun(*this, &PathParam::on_link_button_click));
    static_cast<Gtk::HBox*>(_widget)->pack_start(*pButton, true, true);
    pButton->set_tooltip_text(_("Link to path on clipboard"));

    static_cast<Gtk::HBox*>(_widget)->show_all_children();

    return dynamic_cast<Gtk::Widget *> (_widget);
}

void
PathParam::param_editOncanvas(SPItem *item, SPDesktop * dt)
{
    using namespace Inkscape::UI;

    // TODO remove the tools_switch atrocity.
    if (!tools_isactive(dt, TOOLS_NODES)) {
        tools_switch(dt, TOOLS_NODES);
    }

    Inkscape::UI::Tools::NodeTool *nt = static_cast<Inkscape::UI::Tools::NodeTool*>(dt->event_context);
    std::set<ShapeRecord> shapes;
    ShapeRecord r;

    r.role = SHAPE_ROLE_LPE_PARAM;
    r.edit_transform = item->i2dt_affine(); // TODO is it right?
    if (!href) {
        r.item = reinterpret_cast<SPItem*>(param_effect->getLPEObj());
        r.lpe_key = param_key;
    } else {
        r.item = ref.getObject();
    }
    shapes.insert(r);
    nt->_multipath->setItems(shapes);
}

void
PathParam::param_setup_nodepath(Inkscape::NodePath::Path *)
{
    // TODO this method should not exist at all!
}

void
PathParam::addCanvasIndicators(SPLPEItem const*/*lpeitem*/, std::vector<Geom::PathVector> &hp_vec)
{
    hp_vec.push_back(_pathvector);
}

/*
 * Only applies transform when not referring to other path!
 */
void
PathParam::param_transform_multiply(Geom::Affine const& postmul, bool /*set*/)
{
    // only apply transform when not referring to other path
    if (!href) {
        set_new_value( _pathvector * postmul, true );
    }
}

/*
 * See comments for set_new_value(std::vector<Geom::Path>).
 */
void
PathParam::set_new_value (Geom::Piecewise<Geom::D2<Geom::SBasis> > const & newpath, bool write_to_svg)
{
    remove_link();
    _pathvector = Geom::path_from_piecewise(newpath, LPE_CONVERSION_TOLERANCE);

    if (write_to_svg) {
        gchar * svgd = sp_svg_write_path( _pathvector );
        param_write_to_repr(svgd);
        g_free(svgd);

        // After the whole "writing to svg avalanche of function calling": force value upon pwd2 and don't recalculate.
        _pwd2 = newpath;
        must_recalculate_pwd2 = false;
    } else {
        _pwd2 = newpath;
        must_recalculate_pwd2 = false;
        emit_changed();
    }
}

/*
 * This method sets new path data.
 * If this PathParam refers to another path, this link is removed (and replaced with explicit path data).
 *
 * If write_to_svg = true :
 *          The new path data is written to SVG. In this case the signal_path_changed signal
 *          is not directly emited in this method, because writing to SVG
 *          triggers the LPEObject to which this belongs to call Effect::setParameter which calls
 *          PathParam::readSVGValue, which finally emits the signal_path_changed signal.
 * If write_to_svg = false :
 *          The new path data is not written to SVG. This method will emit the signal_path_changed signal.
 */
void
PathParam::set_new_value (std::vector<Geom::Path> const &newpath, bool write_to_svg)
{
    remove_link();
    _pathvector = newpath;
    must_recalculate_pwd2 = true;

    if (write_to_svg) {
        gchar * svgd = sp_svg_write_path( _pathvector );
        param_write_to_repr(svgd);
        g_free(svgd);
    } else {
        emit_changed();
    }
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
PathParam::emit_changed()
{
    changed = true;
    signal_path_changed.emit();
}

void
PathParam::start_listening(SPObject * to)
{
    if ( to == NULL ) {
        return;
    }
    linked_delete_connection = to->connectDelete(sigc::mem_fun(*this, &PathParam::linked_delete));
    linked_modified_connection = to->connectModified(sigc::mem_fun(*this, &PathParam::linked_modified));
    if (SP_IS_ITEM(to)) {
        linked_transformed_connection = SP_ITEM(to)->connectTransformed(sigc::mem_fun(*this, &PathParam::linked_transformed));
    }
    linked_modified(to, SP_OBJECT_MODIFIED_FLAG); // simulate linked_modified signal, so that path data is updated
}

void
PathParam::quit_listening(void)
{
    linked_modified_connection.disconnect();
    linked_delete_connection.disconnect();
    linked_transformed_connection.disconnect();
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
    set_new_value (_pathvector, true);
}

void PathParam::linked_modified(SPObject *linked_obj, guint flags)
{
    linked_modified_callback(linked_obj, flags);
}

void PathParam::linked_transformed(Geom::Affine const *rel_transf, SPItem *moved_item)
{
    linked_transformed_callback(rel_transf, moved_item);
}

void
PathParam::linked_modified_callback(SPObject *linked_obj, guint /*flags*/)
{
    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(linked_obj)) {
        curve = SP_SHAPE(linked_obj)->getCurve();
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
    emit_changed();
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
    // only recognize a non-null, non-empty string
    if (svgd && *svgd) {
        // remove possible link to path
        remove_link();

        param_write_to_repr(svgd);
        signal_path_pasted.emit();
    }
}

void
PathParam::on_paste_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    Glib::ustring svgd = cm->getPathParameter(SP_ACTIVE_DESKTOP);
    paste_param_path(svgd.data());
    DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                       _("Paste path parameter"));
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
    Glib::ustring pathid = cm->getShapeOrTextObjectId(SP_ACTIVE_DESKTOP);

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
        DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
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
