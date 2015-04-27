/*
 * Copyright (C) Theodore Janeczko 2012 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "live_effects/parameter/originalpatharray.h"

#include <gtkmm/widget.h>
#include <gtkmm/icontheme.h>
#include <gtkmm/imagemenuitem.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/scrolledwindow.h>

#include <glibmm/i18n.h>

#include "inkscape.h"
#include "icon-size.h"
#include "widgets/icon.h"
#include "ui/clipboard.h"
#include "svg/svg.h"
#include "svg/stringstream.h"
#include "originalpath.h"
#include "uri.h"
#include "display/curve.h"

#include <2geom/coord.h>
#include <2geom/point.h>
#include "sp-shape.h"
#include "sp-text.h"
#include "live_effects/effect.h"

#include "verbs.h"
#include "document-undo.h"
#include "document.h"

namespace Inkscape {

namespace LivePathEffect {

class OriginalPathArrayParam::ModelColumns : public Gtk::TreeModel::ColumnRecord
{
public:

    ModelColumns()
    {
        add(_colObject);
        add(_colLabel);
        add(_colReverse);
    }
    virtual ~ModelColumns() {}

    Gtk::TreeModelColumn<PathAndDirection*> _colObject;
    Gtk::TreeModelColumn<Glib::ustring> _colLabel;
    Gtk::TreeModelColumn<bool> _colReverse;
};

OriginalPathArrayParam::OriginalPathArrayParam( const Glib::ustring& label,
        const Glib::ustring& tip,
        const Glib::ustring& key,
        Inkscape::UI::Widget::Registry* wr,
        Effect* effect )
: Parameter(label, tip, key, wr, effect), 
        _vector(),
        _tree(),
        _text_renderer(),
        _toggle_renderer(),
        _scroller()
{    
    _model = new ModelColumns();
    _store = Gtk::TreeStore::create(*_model);
    _tree.set_model(_store);

    _tree.set_reorderable(true);
    _tree.enable_model_drag_dest (Gdk::ACTION_MOVE);
    
    _text_renderer = manage(new Gtk::CellRendererText());
    int nameColNum = _tree.append_column(_("Name"), *_text_renderer) - 1;
    _name_column = _tree.get_column(nameColNum);
    _name_column->add_attribute(_text_renderer->property_text(), _model->_colLabel);

    _tree.set_expander_column( *_tree.get_column(nameColNum) );
    _tree.set_search_column(_model->_colLabel);
    
    Gtk::CellRendererToggle * _toggle_renderer = manage(new Gtk::CellRendererToggle());
    int toggleColNum = _tree.append_column(_("Reverse"), *_toggle_renderer) - 1;
    Gtk::TreeViewColumn* col = _tree.get_column(toggleColNum);
    _toggle_renderer->set_activatable(true);
    _toggle_renderer->signal_toggled().connect(sigc::mem_fun(*this, &OriginalPathArrayParam::on_reverse_toggled));
    col->add_attribute(_toggle_renderer->property_active(), _model->_colReverse);
    
    //quick little hack -- newer versions of gtk gave the item zero space allotment
    _scroller.set_size_request(-1, 120);

    _scroller.add(_tree);
    _scroller.set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
    //_scroller.set_shadow_type(Gtk::SHADOW_IN);
    
    oncanvas_editable = true;
    
}

OriginalPathArrayParam::~OriginalPathArrayParam()
{
    while (!_vector.empty()) {
        PathAndDirection *w = _vector.back();
        _vector.pop_back();
        unlink(w);
        delete w;
    }
    delete _model;
}

void OriginalPathArrayParam::on_reverse_toggled(const Glib::ustring& path)
{
    Gtk::TreeModel::iterator iter = _store->get_iter(path);
    Gtk::TreeModel::Row row = *iter;
    PathAndDirection *w = row[_model->_colObject];
    row[_model->_colReverse] = !row[_model->_colReverse];
    w->reversed = row[_model->_colReverse];
    
    gchar * full = param_getSVGValue();
    param_write_to_repr(full);
    g_free(full);
    DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                       _("Link path parameter to path"));
}

void OriginalPathArrayParam::param_set_default()
{
    
}

Gtk::Widget* OriginalPathArrayParam::param_newWidget()
{
    Gtk::VBox* vbox = Gtk::manage(new Gtk::VBox());
    Gtk::HBox* hbox = Gtk::manage(new Gtk::HBox());

    vbox->pack_start(_scroller, Gtk::PACK_EXPAND_WIDGET);
    
    
    { // Paste path to link button
        Gtk::Widget *pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_PASTE, Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalPathArrayParam::on_link_button_click));
        hbox->pack_start(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Link to path"));
    }
    
    { // Remove linked path
        Gtk::Widget *pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_REMOVE, Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalPathArrayParam::on_remove_button_click));
        hbox->pack_start(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Remove Path"));
    }
    
    { // Move Down
        Gtk::Widget *pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_GO_DOWN, Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalPathArrayParam::on_down_button_click));
        hbox->pack_end(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Move Down"));
    }
    
    { // Move Down
        Gtk::Widget *pIcon = Gtk::manage( sp_icon_get_icon( GTK_STOCK_GO_UP, Inkscape::ICON_SIZE_BUTTON) );
        Gtk::Button *pButton = Gtk::manage(new Gtk::Button());
        pButton->set_relief(Gtk::RELIEF_NONE);
        pIcon->show();
        pButton->add(*pIcon);
        pButton->show();
        pButton->signal_clicked().connect(sigc::mem_fun(*this, &OriginalPathArrayParam::on_up_button_click));
        hbox->pack_end(*pButton, Gtk::PACK_SHRINK);
        pButton->set_tooltip_text(_("Move Up"));
    }
    
    vbox->pack_end(*hbox, Gtk::PACK_SHRINK);
    
    vbox->show_all_children(true);
    
    return vbox;
}

bool OriginalPathArrayParam::_selectIndex(const Gtk::TreeIter& iter, int* i)
{
    if ((*i)-- <= 0) {
        _tree.get_selection()->select(iter);
        return true;
    }
    return false;
}

void OriginalPathArrayParam::on_up_button_click()
{
    Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        
        int i = -1;
        std::vector<PathAndDirection*>::iterator piter = _vector.begin();
        for (std::vector<PathAndDirection*>::iterator iter = _vector.begin(); iter != _vector.end(); piter = iter, i++, iter++) {
            if (*iter == row[_model->_colObject]) {
                _vector.erase(iter);
                _vector.insert(piter, row[_model->_colObject]);
                break;
            }
        }
        
        gchar * full = param_getSVGValue();
        param_write_to_repr(full);
        g_free(full);
        
        DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                       _("Move path up"));
        
        _store->foreach_iter(sigc::bind<int*>(sigc::mem_fun(*this, &OriginalPathArrayParam::_selectIndex), &i));
    }
}

void OriginalPathArrayParam::on_down_button_click()
{
    Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;

        int i = 0;
        for (std::vector<PathAndDirection*>::iterator iter = _vector.begin(); iter != _vector.end(); i++, iter++) {
            if (*iter == row[_model->_colObject]) {
                std::vector<PathAndDirection*>::iterator niter = _vector.erase(iter);
                if (niter != _vector.end()) {
                    niter++;
                    i++;
                }
                _vector.insert(niter, row[_model->_colObject]);
                break;
            }
        }
        
        gchar * full = param_getSVGValue();
        param_write_to_repr(full);
        g_free(full);
        
        DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                       _("Move path down"));
        
        _store->foreach_iter(sigc::bind<int*>(sigc::mem_fun(*this, &OriginalPathArrayParam::_selectIndex), &i));
    }
}

void OriginalPathArrayParam::on_remove_button_click()
{
    Gtk::TreeModel::iterator iter = _tree.get_selection()->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        remove_link(row[_model->_colObject]);
        
        gchar * full = param_getSVGValue();
        param_write_to_repr(full);
        g_free(full);
        
        DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                       _("Remove path"));
    }
    
}

void
OriginalPathArrayParam::on_link_button_click()
{
    Inkscape::UI::ClipboardManager *cm = Inkscape::UI::ClipboardManager::get();
    Glib::ustring pathid = cm->getShapeOrTextObjectId(SP_ACTIVE_DESKTOP);

    if (pathid == "") {
        return;
    }
    // add '#' at start to make it an uri.
    pathid.insert(pathid.begin(), '#');
    
    Inkscape::SVGOStringStream os;
    bool foundOne = false;
    for (std::vector<PathAndDirection*>::const_iterator iter = _vector.begin(); iter != _vector.end(); iter++) {
        if (foundOne) {
            os << "|";
        } else {
            foundOne = true;
        }
        os << (*iter)->href << "," << ((*iter)->reversed ? "1" : "0");
    }
    
    if (foundOne) {
        os << "|";
    }
    
    os << pathid.c_str() << ",0";
    
    param_write_to_repr(os.str().c_str());
    DocumentUndo::done(param_effect->getSPDoc(), SP_VERB_DIALOG_LIVE_PATH_EFFECT,
                       _("Link path parameter to path"));
}

void OriginalPathArrayParam::unlink(PathAndDirection* to)
{
    to->linked_modified_connection.disconnect();
    to->linked_delete_connection.disconnect();
    to->ref.detach();
    to->_pathvector = Geom::PathVector();
    if (to->href) {
        g_free(to->href);
        to->href = NULL;
    }    
}

void OriginalPathArrayParam::remove_link(PathAndDirection* to)
{
    unlink(to);
    for (std::vector<PathAndDirection*>::iterator iter = _vector.begin(); iter != _vector.end(); iter++) {
        if (*iter == to) {
            PathAndDirection *w = *iter;
            _vector.erase(iter);
            delete w;
            return;
        }
    }
}

void OriginalPathArrayParam::linked_delete(SPObject */*deleted*/, PathAndDirection* /*to*/)
{
    //remove_link(to);
    
    gchar * full = param_getSVGValue();
    param_write_to_repr(full);
    g_free(full);
}

bool OriginalPathArrayParam::_updateLink(const Gtk::TreeIter& iter, PathAndDirection* pd)
{
    Gtk::TreeModel::Row row = *iter;
    if (row[_model->_colObject] == pd) {
        SPObject *obj = pd->ref.getObject();
        row[_model->_colLabel] = obj && obj->getId() ? ( obj->label() ? obj->label() : obj->getId() ) : pd->href;
        return true;
    }
    return false;
}

void OriginalPathArrayParam::linked_changed(SPObject */*old_obj*/, SPObject *new_obj, PathAndDirection* to)
{
    to->linked_delete_connection.disconnect();
    to->linked_modified_connection.disconnect();
    to->linked_transformed_connection.disconnect();
    
    if (new_obj && SP_IS_ITEM(new_obj)) {
        to->linked_delete_connection = new_obj->connectDelete(sigc::bind<PathAndDirection*>(sigc::mem_fun(*this, &OriginalPathArrayParam::linked_delete), to));
        to->linked_modified_connection = new_obj->connectModified(sigc::bind<PathAndDirection*>(sigc::mem_fun(*this, &OriginalPathArrayParam::linked_modified), to));
        to->linked_transformed_connection = SP_ITEM(new_obj)->connectTransformed(sigc::bind<PathAndDirection*>(sigc::mem_fun(*this, &OriginalPathArrayParam::linked_transformed), to));

        linked_modified(new_obj, SP_OBJECT_MODIFIED_FLAG, to);
    } else {
        to->_pathvector = Geom::PathVector();
        SP_OBJECT(param_effect->getLPEObj())->requestModified(SP_OBJECT_MODIFIED_FLAG);
        _store->foreach_iter(sigc::bind<PathAndDirection*>(sigc::mem_fun(*this, &OriginalPathArrayParam::_updateLink), to));
    }
}

void OriginalPathArrayParam::setPathVector(SPObject *linked_obj, guint /*flags*/, PathAndDirection* to)
{
    if (!to) {
        return;
    }
    SPCurve *curve = NULL;
    if (SP_IS_SHAPE(linked_obj)) {
        curve = SP_SHAPE(linked_obj)->getCurveBeforeLPE();
    }
    if (SP_IS_TEXT(linked_obj)) {
        curve = SP_TEXT(linked_obj)->getNormalizedBpath();
    }

    if (curve == NULL) {
        // curve invalid, set empty pathvector
        to->_pathvector = Geom::PathVector();
    } else {
        to->_pathvector = curve->get_pathvector();
        curve->unref();
    }
}

void OriginalPathArrayParam::linked_modified(SPObject *linked_obj, guint flags, PathAndDirection* to)
{
    if (!to) {
        return;
    }
    setPathVector(linked_obj, flags, to);
    SP_OBJECT(param_effect->getLPEObj())->requestModified(SP_OBJECT_MODIFIED_FLAG);
    _store->foreach_iter(sigc::bind<PathAndDirection*>(sigc::mem_fun(*this, &OriginalPathArrayParam::_updateLink), to));
}

bool OriginalPathArrayParam::param_readSVGValue(const gchar* strvalue)
{
    if (strvalue) {
        while (!_vector.empty()) {
            PathAndDirection *w = _vector.back();
            unlink(w);
            _vector.pop_back();
            delete w;
        }
        _store->clear();

        gchar ** strarray = g_strsplit(strvalue, "|", 0);
        for (gchar ** iter = strarray; *iter != NULL; iter++) {
            if ((*iter)[0] == '#') {
                gchar ** substrarray = g_strsplit(*iter, ",", 0);
                PathAndDirection* w = new PathAndDirection((SPObject *)param_effect->getLPEObj());
                w->href = g_strdup(*substrarray);
                w->reversed = *(substrarray+1) != NULL && (*(substrarray+1))[0] == '1';

                w->linked_changed_connection = w->ref.changedSignal().connect(sigc::bind<PathAndDirection *>(sigc::mem_fun(*this, &OriginalPathArrayParam::linked_changed), w));
                w->ref.attach(URI(w->href));

                _vector.push_back(w);

                Gtk::TreeModel::iterator iter = _store->append();
                Gtk::TreeModel::Row row = *iter;
                SPObject *obj = w->ref.getObject();

                row[_model->_colObject] = w;
                row[_model->_colLabel] = obj ? ( obj->label() ? obj->label() : obj->getId() ) : w->href;
                row[_model->_colReverse] = w->reversed;
                g_strfreev (substrarray);
            }
        }
        g_strfreev (strarray);
        return true;
    }
    return false;
}

gchar * OriginalPathArrayParam::param_getSVGValue() const
{
    Inkscape::SVGOStringStream os;
    bool foundOne = false;
    for (std::vector<PathAndDirection*>::const_iterator iter = _vector.begin(); iter != _vector.end(); iter++) {
        if (foundOne) {
            os << "|";
        } else {
            foundOne = true;
        }
        os << (*iter)->href << "," << ((*iter)->reversed ? "1" : "0");
    }
    gchar * str = g_strdup(os.str().c_str());
    return str;
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
