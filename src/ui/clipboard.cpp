/**
 * @file
 * System-wide clipboard management - implementation.
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk@o2.pl>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Incorporates some code from selection-chemistry.cpp, see that file for more credits.
 *   Abhishek Sharma
 *   Tavmjong Bah
 *
 * Copyright (C) 2008 authors
 * Copyright (C) 2010 Jon A. Cruz
 * Copyright (C) 2012 Tavmjong Bah (Symbol additions)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#include <gtkmm/clipboard.h>
#include "ui/clipboard.h"

// TODO: reduce header bloat if possible

#include "file.h" // for file_import, used in _pasteImage
#include <list>
#include <algorithm>
#include <glibmm/ustring.h>
#include <glibmm/i18n.h>
#include <glib/gstdio.h> // for g_file_set_contents etc., used in _onGet and paste
#include "inkgc/gc-core.h"
#include "xml/repr.h"
#include "inkscape.h"
#include "io/stringstream.h"
#include "desktop.h"

#include "desktop-style.h" // for sp_desktop_set_style, used in _pasteStyle
#include "document.h"
#include "document-private.h"
#include "selection.h"
#include "message-stack.h"
#include "context-fns.h"
#include "ui/tools/dropper-tool.h" // used in copy()
#include "style.h"
#include "extension/db.h" // extension database
#include "extension/input.h"
#include "extension/output.h"
#include "selection-chemistry.h"
#include <2geom/rect.h>
#include <2geom/transforms.h>
#include "box3d.h"
#include "gradient-drag.h"
#include "sp-marker.h"
#include "sp-item.h"
#include "sp-item-transform.h" // for sp_item_scale_rel, used in _pasteSize
#include "sp-path.h"
#include "sp-pattern.h"
#include "sp-shape.h"
#include "sp-gradient.h"
#include "sp-gradient-reference.h"
#include "sp-linear-gradient.h"
#include "sp-radial-gradient.h"
#include "sp-clippath.h"
#include "sp-mask.h"
#include "sp-textpath.h"
#include "sp-rect.h"
#include "sp-use.h"
#include "sp-symbol.h"
#include "live_effects/lpeobject.h"
#include "live_effects/lpeobject-reference.h"
#include "live_effects/parameter/path.h"
#include "svg/svg.h" // for sp_svg_transform_write, used in _copySelection
#include "svg/css-ostringstream.h" // used in copy
#include "ui/tools/text-tool.h"
#include "text-editing.h"
#include "ui/tools-switch.h"
#include "path-chemistry.h"
#include "util/units.h"
#include "helper/png-write.h"
#include "svg/svg-color.h"
#include "sp-namedview.h"
#include "snap.h"
#include "persp3d.h"
#include "preferences.h"

/// Made up mimetype to represent Gdk::Pixbuf clipboard contents.
#define CLIPBOARD_GDK_PIXBUF_TARGET "image/x-gdk-pixbuf"

#define CLIPBOARD_TEXT_TARGET "text/plain"

#ifdef WIN32
#include <windows.h>
#endif

namespace Inkscape {
namespace UI {


/**
 * Default implementation of the clipboard manager.
 */
class ClipboardManagerImpl : public ClipboardManager {
public:
    virtual void copy(SPDesktop *desktop);
    virtual void copyPathParameter(Inkscape::LivePathEffect::PathParam *);
    virtual void copySymbol(Inkscape::XML::Node* symbol, gchar const* style, bool user_symbol);
    virtual bool paste(SPDesktop *desktop, bool in_place);
    virtual bool pasteStyle(SPDesktop *desktop);
    virtual bool pasteSize(SPDesktop *desktop, bool separately, bool apply_x, bool apply_y);
    virtual bool pastePathEffect(SPDesktop *desktop);
    virtual Glib::ustring getPathParameter(SPDesktop* desktop);
    virtual Glib::ustring getShapeOrTextObjectId(SPDesktop *desktop);
    virtual const gchar *getFirstObjectID();

    ClipboardManagerImpl();
    ~ClipboardManagerImpl();

private:
    void _copySelection(Inkscape::Selection *);
    void _copyUsedDefs(SPItem *);
    void _copyGradient(SPGradient *);
    void _copyPattern(SPPattern *);
    void _copyTextPath(SPTextPath *);
    Inkscape::XML::Node *_copyNode(Inkscape::XML::Node *, Inkscape::XML::Document *, Inkscape::XML::Node *);

    bool _pasteImage(SPDocument *doc);
    bool _pasteText(SPDesktop *desktop);
    void _applyPathEffect(SPItem *, gchar const *);
    SPDocument *_retrieveClipboard(Glib::ustring = "");

    // clipboard callbacks
    void _onGet(Gtk::SelectionData &, guint);
    void _onClear();

    // various helpers
    void _createInternalClipboard();
    void _discardInternalClipboard();
    Inkscape::XML::Node *_createClipNode();
    Geom::Scale _getScale(SPDesktop *desktop, Geom::Point const &min, Geom::Point const &max, Geom::Rect const &obj_rect, bool apply_x, bool apply_y);
    Glib::ustring _getBestTarget();
    void _setClipboardTargets();
    void _setClipboardColor(guint32);
    void _userWarn(SPDesktop *, char const *);

    void _inkscape_wait_for_targets(std::list<Glib::ustring> &);

    // private properites
    SPDocument *_clipboardSPDoc; ///< Document that stores the clipboard until someone requests it
    Inkscape::XML::Node *_defs; ///< Reference to the clipboard document's defs node
    Inkscape::XML::Node *_root; ///< Reference to the clipboard's root node
    Inkscape::XML::Node *_clipnode; ///< The node that holds extra information
    Inkscape::XML::Document *_doc; ///< Reference to the clipboard's Inkscape::XML::Document

    // we need a way to copy plain text AND remember its style;
    // the standard _clipnode is only available in an SVG tree, hence this special storage
    SPCSSAttr *_text_style; ///< Style copied along with plain text fragment

    Glib::RefPtr<Gtk::Clipboard> _clipboard; ///< Handle to the system wide clipboard - for convenience
    std::list<Glib::ustring> _preferred_targets; ///< List of supported clipboard targets
};


ClipboardManagerImpl::ClipboardManagerImpl()
    : _clipboardSPDoc(NULL),
      _defs(NULL),
      _root(NULL),
      _clipnode(NULL),
      _doc(NULL),
      _text_style(NULL),
      _clipboard( Gtk::Clipboard::get() )
{
    // Clipboard Formats: http://msdn.microsoft.com/en-us/library/ms649013(VS.85).aspx
    // On Windows, most graphical applications can handle CF_DIB/CF_BITMAP and/or CF_ENHMETAFILE
    // GTK automatically presents an "image/bmp" target as CF_DIB/CF_BITMAP
    // Presenting "image/x-emf" as CF_ENHMETAFILE must be done by Inkscape ?

    // push supported clipboard targets, in order of preference
    _preferred_targets.push_back("image/x-inkscape-svg");
    _preferred_targets.push_back("image/svg+xml");
    _preferred_targets.push_back("image/svg+xml-compressed");
    _preferred_targets.push_back("image/x-emf");
    _preferred_targets.push_back("CF_ENHMETAFILE");
    _preferred_targets.push_back("WCF_ENHMETAFILE"); // seen on Wine
    _preferred_targets.push_back("application/pdf");
    _preferred_targets.push_back("image/x-adobe-illustrator");
}


ClipboardManagerImpl::~ClipboardManagerImpl() {}


/**
 * Copy selection contents to the clipboard.
 */
void ClipboardManagerImpl::copy(SPDesktop *desktop)
{
    if ( desktop == NULL ) {
        return;
    }
    Inkscape::Selection *selection = desktop->getSelection();

    // Special case for when the gradient dragger is active - copies gradient color
    if (desktop->event_context->get_drag()) {
        GrDrag *drag = desktop->event_context->get_drag();
        if (drag->hasSelection()) {
            guint32 col = drag->getColor();

            // set the color as clipboard content (text in RRGGBBAA format)
            _setClipboardColor(col);

            // create a style with this color on fill and opacity in master opacity, so it can be
            // pasted on other stops or objects
            if (_text_style) {
                sp_repr_css_attr_unref(_text_style);
                _text_style = NULL;
            }
            _text_style = sp_repr_css_attr_new();
            // print and set properties
            gchar color_str[16];
            g_snprintf(color_str, 16, "#%06x", col >> 8);
            sp_repr_css_set_property(_text_style, "fill", color_str);
            float opacity = SP_RGBA32_A_F(col);
            if (opacity > 1.0) {
                opacity = 1.0; // safeguard
            }
            Inkscape::CSSOStringStream opcss;
            opcss << opacity;
            sp_repr_css_set_property(_text_style, "opacity", opcss.str().data());

            _discardInternalClipboard();
            return;
        }
    }

    // Special case for when the color picker ("dropper") is active - copies color under cursor
    if (tools_isactive(desktop, TOOLS_DROPPER)) {
        //_setClipboardColor(sp_dropper_context_get_color(desktop->event_context));
    	_setClipboardColor(SP_DROPPER_CONTEXT(desktop->event_context)->get_color());
        _discardInternalClipboard();
        return;
    }

    // Special case for when the text tool is active - if some text is selected, copy plain text,
    // not the object that holds it; also copy the style at cursor into
    if (tools_isactive(desktop, TOOLS_TEXT)) {
        _discardInternalClipboard();
        Glib::ustring selected_text = Inkscape::UI::Tools::sp_text_get_selected_text(desktop->event_context);
        _clipboard->set_text(selected_text);
        if (_text_style) {
            sp_repr_css_attr_unref(_text_style);
            _text_style = NULL;
        }
        _text_style = Inkscape::UI::Tools::sp_text_get_style_at_cursor(desktop->event_context);
        return;
    }

    if (selection->isEmpty()) {  // check whether something is selected
        _userWarn(desktop, _("Nothing was copied."));
        return;
    }
    _discardInternalClipboard();

    _createInternalClipboard();   // construct a new clipboard document
    _copySelection(selection);   // copy all items in the selection to the internal clipboard
    fit_canvas_to_drawing(_clipboardSPDoc);

    _setClipboardTargets();
}


/**
 * Copy a Live Path Effect path parameter to the clipboard.
 * @param pp The path parameter to store in the clipboard.
 */
void ClipboardManagerImpl::copyPathParameter(Inkscape::LivePathEffect::PathParam *pp)
{
    if ( pp == NULL ) {
        return;
    }
    gchar *svgd = sp_svg_write_path( pp->get_pathvector() );
    if ( svgd == NULL || *svgd == '\0' ) {
        return;
    }

    _discardInternalClipboard();
    _createInternalClipboard();

    Inkscape::XML::Node *pathnode = _doc->createElement("svg:path");
    pathnode->setAttribute("d", svgd);
    g_free(svgd);
    _root->appendChild(pathnode);
    Inkscape::GC::release(pathnode);

    fit_canvas_to_drawing(_clipboardSPDoc);
    _setClipboardTargets();
}

/**
 * Copy a symbol from the symbol dialog.
 * @param symbol The Inkscape::XML::Node for the symbol.
 */
void ClipboardManagerImpl::copySymbol(Inkscape::XML::Node* symbol, gchar const* style, bool user_symbol)
{
    //std::cout << "ClipboardManagerImpl::copySymbol" << std::endl;
    if ( symbol == NULL ) {
        return;
    }

    _discardInternalClipboard();
    _createInternalClipboard();

    // We add "_duplicate" to have a well defined symbol name that
    // bypasses the "prevent_id_classes" routine. We'll get rid of it
    // when we paste.
    Inkscape::XML::Node *repr = symbol->duplicate(_doc);
    Glib::ustring symbol_name = repr->attribute("id");

    symbol_name += "_inkscape_duplicate";
    repr->setAttribute("id",    symbol_name.c_str());
    _defs->appendChild(repr);

    Glib::ustring id("#");
    id += symbol->attribute("id");

    gdouble scale_units = 1; // scale from "px" to "document-units"
    Inkscape::XML::Node *nv_repr = SP_ACTIVE_DESKTOP->getNamedView()->getRepr();
    if (nv_repr->attribute("inkscape:document-units"))
        scale_units = Inkscape::Util::Quantity::convert(1, "px", nv_repr->attribute("inkscape:document-units"));
    SPObject *cmobj = _clipboardSPDoc->getObjectByRepr(repr);
    if (cmobj && !user_symbol) { // convert only stock symbols
        if (!Geom::are_near(scale_units, 1.0, Geom::EPSILON)) {
            dynamic_cast<SPGroup *>(cmobj)->scaleChildItemsRec(Geom::Scale(scale_units),
                                            Geom::Point(0, SP_ACTIVE_DESKTOP->getDocument()->getHeight().value("px")), 
                                            false);
        }
    }

    Inkscape::XML::Node *use = _doc->createElement("svg:use");
    use->setAttribute("xlink:href", id.c_str() );
    // Set a default style in <use> rather than <symbol> so it can be changed.
    use->setAttribute("style", style );
    if (!Geom::are_near(scale_units, 1.0, Geom::EPSILON)) {
        gchar *transform_str = sp_svg_transform_write(Geom::Scale(1.0/scale_units));
        use->setAttribute("transform", transform_str);
        g_free(transform_str);
    }
    _root->appendChild(use);

    // This min and max sets offsets, we don't have any so set to zero.
    sp_repr_set_point(_clipnode, "min", Geom::Point(0,0));
    sp_repr_set_point(_clipnode, "max", Geom::Point(0,0));

    fit_canvas_to_drawing(_clipboardSPDoc);
    _setClipboardTargets();
}

/**
 * Paste from the system clipboard into the active desktop.
 * @param in_place Whether to put the contents where they were when copied.
 */
bool ClipboardManagerImpl::paste(SPDesktop *desktop, bool in_place)
{
    // do any checking whether we really are able to paste before requesting the contents
    if ( desktop == NULL ) {
        return false;
    }
    if ( Inkscape::have_viable_layer(desktop, desktop->messageStack()) == false ) {
        return false;
    }

    Glib::ustring target = _getBestTarget();

    // Special cases of clipboard content handling go here
    // Note that target priority is determined in _getBestTarget.
    // TODO: Handle x-special/gnome-copied-files and text/uri-list to support pasting files

    // if there is an image on the clipboard, paste it
    if ( target == CLIPBOARD_GDK_PIXBUF_TARGET ) {
        return _pasteImage(desktop->doc());
    }
    // if there's only text, paste it into a selected text object or create a new one
    if ( target == CLIPBOARD_TEXT_TARGET ) {
        return _pasteText(desktop);
    }

    // otherwise, use the import extensions
    SPDocument *tempdoc = _retrieveClipboard(target);
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("Nothing on the clipboard."));
        return false;
    }

    sp_import_document(desktop, tempdoc, in_place);
    tempdoc->doUnref();

    return true;
}

/**
 * Returns the id of the first visible copied object.
 */
const gchar *ClipboardManagerImpl::getFirstObjectID()
{
    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc == NULL ) {
        return NULL;
    }

    Inkscape::XML::Node *root = tempdoc->getReprRoot();

    if (!root) {
        return NULL;
    }

    Inkscape::XML::Node *ch = root->firstChild();
    while (ch != NULL &&
           strcmp(ch->name(), "svg:g") &&
           strcmp(ch->name(), "svg:path") &&
           strcmp(ch->name(), "svg:use") &&
           strcmp(ch->name(), "svg:text") &&
           strcmp(ch->name(), "svg:image") &&
           strcmp(ch->name(), "svg:rect")
        ) {
        ch = ch->next();
    }

    if (ch) {
        return ch->attribute("id");
    }

    return NULL;
}


/**
 * Implements the Paste Style action.
 */
bool ClipboardManagerImpl::pasteStyle(SPDesktop *desktop)
{
    if (desktop == NULL) {
        return false;
    }

    // check whether something is selected
    Inkscape::Selection *selection = desktop->getSelection();
    if (selection->isEmpty()) {
        _userWarn(desktop, _("Select <b>object(s)</b> to paste style to."));
        return false;
    }

    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc == NULL ) {
        // no document, but we can try _text_style
        if (_text_style) {
            sp_desktop_set_style(desktop, _text_style);
            return true;
        } else {
            _userWarn(desktop, _("No style on the clipboard."));
            return false;
        }
    }

    Inkscape::XML::Node *root = tempdoc->getReprRoot();
    Inkscape::XML::Node *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);

    bool pasted = false;

    if (clipnode) {
        desktop->doc()->importDefs(tempdoc);
        SPCSSAttr *style = sp_repr_css_attr(clipnode, "style");
        sp_desktop_set_style(desktop, style);
        pasted = true;
    }
    else {
        _userWarn(desktop, _("No style on the clipboard."));
    }

    tempdoc->doUnref();
    return pasted;
}


/**
 * Resize the selection or each object in the selection to match the clipboard's size.
 * @param separately Whether to scale each object in the selection separately
 * @param apply_x Whether to scale the width of objects / selection
 * @param apply_y Whether to scale the height of objects / selection
 */
bool ClipboardManagerImpl::pasteSize(SPDesktop *desktop, bool separately, bool apply_x, bool apply_y)
{
    if (!apply_x && !apply_y) {
        return false; // pointless parameters
    }

    if ( desktop == NULL ) {
        return false;
    }
    Inkscape::Selection *selection = desktop->getSelection();
    if (selection->isEmpty()) {
        _userWarn(desktop, _("Select <b>object(s)</b> to paste size to."));
        return false;
    }

    // FIXME: actually, this should accept arbitrary documents
    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("No size on the clipboard."));
        return false;
    }

    // retrieve size ifomration from the clipboard
    Inkscape::XML::Node *root = tempdoc->getReprRoot();
    Inkscape::XML::Node *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);
    bool pasted = false;
    if (clipnode) {
        Geom::Point min, max;
        sp_repr_get_point(clipnode, "min", &min);
        sp_repr_get_point(clipnode, "max", &max);

        // resize each object in the selection
        if (separately) {
        	std::vector<SPItem*> itemlist=selection->itemList();
            for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();i++){
                SPItem *item = *i;
                if (item) {
                    Geom::OptRect obj_size = item->desktopVisualBounds();
                    if ( obj_size ) {
                        sp_item_scale_rel(item, _getScale(desktop, min, max, *obj_size, apply_x, apply_y));
                    }
                } else {
                    g_assert_not_reached();
                }
            }
        }
        // resize the selection as a whole
        else {
            Geom::OptRect sel_size = selection->visualBounds();
            if ( sel_size ) {
                sp_selection_scale_relative(selection, sel_size->midpoint(),
                                            _getScale(desktop, min, max, *sel_size, apply_x, apply_y));
            }
        }
        pasted = true;
    }
    tempdoc->doUnref();
    return pasted;
}


/**
 * Applies a path effect from the clipboard to the selected path.
 */
bool ClipboardManagerImpl::pastePathEffect(SPDesktop *desktop)
{
    /** @todo FIXME: pastePathEffect crashes when moving the path with the applied effect,
        segfaulting in fork_private_if_necessary(). */

    if ( desktop == NULL ) {
        return false;
    }

    Inkscape::Selection *selection = desktop->getSelection();
    if (selection && selection->isEmpty()) {
        _userWarn(desktop, _("Select <b>object(s)</b> to paste live path effect to."));
        return false;
    }

    SPDocument *tempdoc = _retrieveClipboard("image/x-inkscape-svg");
    if ( tempdoc ) {
        Inkscape::XML::Node *root = tempdoc->getReprRoot();
        Inkscape::XML::Node *clipnode = sp_repr_lookup_name(root, "inkscape:clipboard", 1);
        if ( clipnode ) {
            gchar const *effectstack = clipnode->attribute("inkscape:path-effect");
            if ( effectstack ) {
                desktop->doc()->importDefs(tempdoc);
                // make sure all selected items are converted to paths first (i.e. rectangles)
                sp_selected_to_lpeitems(desktop);
                std::vector<SPItem*> itemlist=selection->itemList();
                for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();i++){
                    SPItem *item = *i;
                    _applyPathEffect(item, effectstack);
                }

                return true;
            }
        }
    }

    // no_effect:
    _userWarn(desktop, _("No effect on the clipboard."));
    return false;
}


/**
 * Get LPE path data from the clipboard.
 * @return The retrieved path data (contents of the d attribute), or "" if no path was found
 */
Glib::ustring ClipboardManagerImpl::getPathParameter(SPDesktop* desktop)
{
    SPDocument *tempdoc = _retrieveClipboard(); // any target will do here
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("Nothing on the clipboard."));
        return "";
    }
    Inkscape::XML::Node *root = tempdoc->getReprRoot();
    Inkscape::XML::Node *path = sp_repr_lookup_name(root, "svg:path", -1); // unlimited search depth
    if ( path == NULL ) {
        _userWarn(desktop, _("Clipboard does not contain a path."));
        tempdoc->doUnref();
        return "";
    }
    gchar const *svgd = path->attribute("d");
    return svgd;
}


/**
 * Get object id of a shape or text item from the clipboard.
 * @return The retrieved id string (contents of the id attribute), or "" if no shape or text item was found.
 */
Glib::ustring ClipboardManagerImpl::getShapeOrTextObjectId(SPDesktop *desktop)
{
    // https://bugs.launchpad.net/inkscape/+bug/1293979
    // basically, when we do a depth-first search, we're stopping
    // at the first object to be <svg:path> or <svg:text>.
    // but that could then return the id of the object's 
    // clip path or mask, not the original path!
	
    SPDocument *tempdoc = _retrieveClipboard(); // any target will do here
    if ( tempdoc == NULL ) {
        _userWarn(desktop, _("Nothing on the clipboard."));
        return "";
    }
    Inkscape::XML::Node *root = tempdoc->getReprRoot();

    // 1293979: strip out the defs of the document
    root->removeChild(tempdoc->getDefs()->getRepr());

    Inkscape::XML::Node *repr = sp_repr_lookup_name(root, "svg:path", -1); // unlimited search depth
    if ( repr == NULL ) {
        repr = sp_repr_lookup_name(root, "svg:text", -1);
    }

    if ( repr == NULL ) {
        _userWarn(desktop, _("Clipboard does not contain a path."));
        tempdoc->doUnref();
        return "";
    }
    gchar const *svgd = repr->attribute("id");
    return svgd;
}


/**
 * Iterate over a list of items and copy them to the clipboard.
 */
void ClipboardManagerImpl::_copySelection(Inkscape::Selection *selection)
{
    // copy the defs used by all items
	std::vector<SPItem*> itemlist=selection->itemList();
    for(std::vector<SPItem*>::const_iterator i=itemlist.begin();i!=itemlist.end();i++){
        SPItem *item = *i;
        if (item) {
            _copyUsedDefs(item);
        } else {
            g_assert_not_reached();
        }
    }

    // copy the representation of the items
    std::vector<SPItem*> sorted_items(itemlist);
    sort(sorted_items.begin(),sorted_items.end(),sp_object_compare_position);

    for(std::vector<SPItem*>::const_iterator i=sorted_items.begin();i!=sorted_items.end();i++){
        SPItem *item = *i;
        if (item) {
            Inkscape::XML::Node *obj = item->getRepr();
            Inkscape::XML::Node *obj_copy = _copyNode(obj, _doc, _root);

            // copy complete inherited style
            SPCSSAttr *css = sp_repr_css_attr_inherited(obj, "style");
            sp_repr_css_set(obj_copy, css, "style");
            sp_repr_css_attr_unref(css);

            Geom::Affine transform=item->i2doc_affine();

            // write the complete accumulated transform passed to us
            // (we're dealing with unattached representations, so we write to their attributes
            // instead of using sp_item_set_transform)
            SPUse *use=dynamic_cast<SPUse *>(item);
            if( use && selection->includes(use->get_original()) ){//we are copying something whose parent is also copied (!)
                transform = ((SPItem*)(use->get_original()->parent))->i2doc_affine().inverse() * transform;
            }
            gchar *transform_str = sp_svg_transform_write(transform );


            obj_copy->setAttribute("transform", transform_str);
            g_free(transform_str);
        }
    }

    // copy style for Paste Style action
    if (!sorted_items.empty()) {
        SPObject *object = sorted_items[0];
        SPItem *item = dynamic_cast<SPItem *>(object);
        if (item) {
            SPCSSAttr *style = take_style_from_item(item);
            sp_repr_css_set(_clipnode, style, "style");
            sp_repr_css_attr_unref(style);
        }

        // copy path effect from the first path
        if (object) {
            gchar const *effect =object->getRepr()->attribute("inkscape:path-effect");
            if (effect) {
                _clipnode->setAttribute("inkscape:path-effect", effect);
            }
        }
    }

    Geom::OptRect size = selection->visualBounds();
    if (size) {
        sp_repr_set_point(_clipnode, "min", size->min());
        sp_repr_set_point(_clipnode, "max", size->max());
    }

}


/**
 * Recursively copy all the definitions used by a given item to the clipboard defs.
 */
void ClipboardManagerImpl::_copyUsedDefs(SPItem *item)
{
    // copy fill and stroke styles (patterns and gradients)
    SPStyle *style = item->style;

    if (style && (style->fill.isPaintserver())) {
        SPPaintServer *server = item->style->getFillPaintServer();
        if ( dynamic_cast<SPLinearGradient *>(server) || dynamic_cast<SPRadialGradient *>(server) ) {
            _copyGradient(dynamic_cast<SPGradient *>(server));
        }
        SPPattern *pattern = dynamic_cast<SPPattern *>(server);
        if ( pattern ) {
            _copyPattern(pattern);
        }
    }
    if (style && (style->stroke.isPaintserver())) {
        SPPaintServer *server = item->style->getStrokePaintServer();
        if ( dynamic_cast<SPLinearGradient *>(server) || dynamic_cast<SPRadialGradient *>(server) ) {
            _copyGradient(dynamic_cast<SPGradient *>(server));
        }
        SPPattern *pattern = dynamic_cast<SPPattern *>(server);
        if ( pattern ) {
            _copyPattern(pattern);
        }
    }

    // For shapes, copy all of the shape's markers
    SPShape *shape = dynamic_cast<SPShape *>(item);
    if (shape) {
        for (int i = 0 ; i < SP_MARKER_LOC_QTY ; i++) {
            if (shape->_marker[i]) {
                _copyNode(shape->_marker[i]->getRepr(), _doc, _defs);
            }
        }
    }

    // For lpe items, copy lpe stack if applicable
    SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
    if (lpeitem) {
        if (lpeitem->hasPathEffect()) {
            for (PathEffectList::iterator it = lpeitem->path_effect_list->begin(); it != lpeitem->path_effect_list->end(); ++it)
            {
                LivePathEffectObject *lpeobj = (*it)->lpeobject;
                if (lpeobj) {
                    _copyNode(lpeobj->getRepr(), _doc, _defs);
                }
            }
        }
    }

    // For 3D boxes, copy perspectives
    {
        SPBox3D *box = dynamic_cast<SPBox3D *>(item);
        if (box) {
            _copyNode(box3d_get_perspective(box)->getRepr(), _doc, _defs);
        }
    }

    // Copy text paths
    {
        SPText *text = dynamic_cast<SPText *>(item);
        SPTextPath *textpath = (text) ? dynamic_cast<SPTextPath *>(text->firstChild()) : NULL;
        if (textpath) {
            _copyTextPath(textpath);
        }
    }

    // Copy clipping objects
    if (item->clip_ref){
        if (item->clip_ref->getObject()) {
            _copyNode(item->clip_ref->getObject()->getRepr(), _doc, _defs);
        }
    }
    // Copy mask objects
    if (item->mask_ref){
        if (item->mask_ref->getObject()) {
            SPObject *mask = item->mask_ref->getObject();
            _copyNode(mask->getRepr(), _doc, _defs);
            // recurse into the mask for its gradients etc.
            for (SPObject *o = mask->children ; o != NULL ; o = o->next) {
                SPItem *childItem = dynamic_cast<SPItem *>(o);
                if (childItem) {
                    _copyUsedDefs(childItem);
                }
            }
        }
    }
    
    // Copy filters
    if (style->getFilter()) {
        SPObject *filter = style->getFilter();
        if (dynamic_cast<SPFilter *>(filter)) {
            _copyNode(filter->getRepr(), _doc, _defs);
        }
    }

    // recurse
    for (SPObject *o = item->children ; o != NULL ; o = o->next) {
        SPItem *childItem = dynamic_cast<SPItem *>(o);
        if (childItem) {
            _copyUsedDefs(childItem);
        }
    }
}


/**
 * Copy a single gradient to the clipboard's defs element.
 */
void ClipboardManagerImpl::_copyGradient(SPGradient *gradient)
{
    while (gradient) {
        // climb up the refs, copying each one in the chain
        _copyNode(gradient->getRepr(), _doc, _defs);
        if (gradient->ref){
            gradient = gradient->ref->getObject();
        }
        else {
            gradient = NULL;
        }
    }
}


/**
 * Copy a single pattern to the clipboard document's defs element.
 */
void ClipboardManagerImpl::_copyPattern(SPPattern *pattern)
{
    // climb up the references, copying each one in the chain
    while (pattern) {
        _copyNode(pattern->getRepr(), _doc, _defs);

        // items in the pattern may also use gradients and other patterns, so recurse
        for ( SPObject *child = pattern->firstChild() ; child ; child = child->getNext() ) {
            SPItem *childItem = dynamic_cast<SPItem *>(child);
            if (childItem) {
                _copyUsedDefs(childItem);
            }
        }
        if (pattern->ref){
            pattern = pattern->ref->getObject();
        }
        else{
            pattern = NULL;
        }
    }
}


/**
 * Copy a text path to the clipboard's defs element.
 */
void ClipboardManagerImpl::_copyTextPath(SPTextPath *tp)
{
    SPItem *path = sp_textpath_get_path_item(tp);
    if (!path) {
        return;
    }
    Inkscape::XML::Node *path_node = path->getRepr();

    // Do not copy the text path to defs if it's already copied
    if (sp_repr_lookup_child(_root, "id", path_node->attribute("id"))) {
        return;
    }
    _copyNode(path_node, _doc, _defs);
}


/**
 * Copy a single XML node from one document to another.
 * @param node The node to be copied
 * @param target_doc The document to which the node is to be copied
 * @param parent The node in the target document which will become the parent of the copied node
 * @return Pointer to the copied node
 */
Inkscape::XML::Node *ClipboardManagerImpl::_copyNode(Inkscape::XML::Node *node, Inkscape::XML::Document *target_doc, Inkscape::XML::Node *parent)
{
    Inkscape::XML::Node *dup = node->duplicate(target_doc);
    parent->appendChild(dup);
    Inkscape::GC::release(dup);
    return dup;
}


/**
 * Retrieve a bitmap image from the clipboard and paste it into the active document.
 */
bool ClipboardManagerImpl::_pasteImage(SPDocument *doc)
{
    if ( doc == NULL ) {
        return false;
    }

    // retrieve image data
    Glib::RefPtr<Gdk::Pixbuf> img = _clipboard->wait_for_image();
    if (!img) {
        return false;
    }

    // TODO unify with interface.cpp's sp_ui_drag_data_received()
    // AARGH stupid
    Inkscape::Extension::DB::InputList o;
    Inkscape::Extension::db.get_input_list(o);
    Inkscape::Extension::DB::InputList::const_iterator i = o.begin();
    while (i != o.end() && strcmp( (*i)->get_mimetype(), "image/png" ) != 0) {
        ++i;
    }
    Inkscape::Extension::Extension *png = *i;
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    Glib::ustring attr_saved = prefs->getString("/dialogs/import/link");
    bool ask_saved = prefs->getBool("/dialogs/import/ask");
    prefs->setString("/dialogs/import/link", "embed");
    prefs->setBool("/dialogs/import/ask", false);
    png->set_gui(false);

    gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-import", NULL );
    img->save(filename, "png");
    file_import(doc, filename, png);
    g_free(filename);
    prefs->setString("/dialogs/import/link", attr_saved);
    prefs->setBool("/dialogs/import/ask", ask_saved);
    png->set_gui(true);

    return true;
}

/**
 * Paste text into the selected text object or create a new one to hold it.
 */
bool ClipboardManagerImpl::_pasteText(SPDesktop *desktop)
{
    if ( desktop == NULL ) {
        return false;
    }

    // if the text editing tool is active, paste the text into the active text object
    if (tools_isactive(desktop, TOOLS_TEXT)) {
        return Inkscape::UI::Tools::sp_text_paste_inline(desktop->event_context);
    }

    // try to parse the text as a color and, if successful, apply it as the current style
    SPCSSAttr *css = sp_repr_css_attr_parse_color_to_fill(_clipboard->wait_for_text());
    if (css) {
        sp_desktop_set_style(desktop, css);
        return true;
    }

    return false;
}


/**
 * Applies a pasted path effect to a given item.
 */
void ClipboardManagerImpl::_applyPathEffect(SPItem *item, gchar const *effectstack)
{
    if ( item == NULL ) {
        return;
    }
    if ( dynamic_cast<SPRect *>(item) ) {
        return;
    }

    SPLPEItem *lpeitem = dynamic_cast<SPLPEItem *>(item);
    if (lpeitem)
    {
        // for each effect in the stack, check if we need to fork it before adding it to the item
        lpeitem->forkPathEffectsIfNecessary(1);

        std::istringstream iss(effectstack);
        std::string href;
        while (std::getline(iss, href, ';'))
        {
            SPObject *obj = sp_uri_reference_resolve(_clipboardSPDoc, href.c_str());
            if (!obj) {
                return;
            }
            LivePathEffectObject *lpeobj = LIVEPATHEFFECT(obj);
            lpeitem->addPathEffect(lpeobj);
        }
    }
}


/**
 * Retrieve the clipboard contents as a document.
 * @return Clipboard contents converted to SPDocument, or NULL if no suitable content was present
 */
SPDocument *ClipboardManagerImpl::_retrieveClipboard(Glib::ustring required_target)
{
    Glib::ustring best_target;
    if ( required_target == "" ) {
        best_target = _getBestTarget();
    } else {
        best_target = required_target;
    }

    if ( best_target == "" ) {
        return NULL;
    }

    // FIXME: Temporary hack until we add memory input.
    // Save the clipboard contents to some file, then read it
    gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-import", NULL );

    bool file_saved = false;
    Glib::ustring target = best_target;

#ifdef WIN32
    if (best_target == "CF_ENHMETAFILE" || best_target == "WCF_ENHMETAFILE")
    {   // Try to save clipboard data as en emf file (using win32 api)
        if (OpenClipboard(NULL)) {
            HGLOBAL hglb = GetClipboardData(CF_ENHMETAFILE);
            if (hglb) {
                HENHMETAFILE hemf = CopyEnhMetaFile((HENHMETAFILE) hglb, filename);
                if (hemf) {
                    file_saved = true;
                    target = "image/x-emf";
                    DeleteEnhMetaFile(hemf);
                }
            }
            CloseClipboard();
        }
    }
#endif

    if (!file_saved) {
        if ( !_clipboard->wait_is_target_available(best_target) ) {
            return NULL;
        }

        // doing this synchronously makes better sense
        // TODO: use another method because this one is badly broken imo.
        // from documentation: "Returns: A SelectionData object, which will be invalid if retrieving the given target failed."
        // I don't know how to check whether an object is 'valid' or not, unusable if that's not possible...
        Gtk::SelectionData sel = _clipboard->wait_for_contents(best_target);
        target = sel.get_target();  // this can crash if the result was invalid of last function. No way to check for this :(

        // FIXME: Temporary hack until we add memory input.
        // Save the clipboard contents to some file, then read it
        g_file_set_contents(filename, (const gchar *) sel.get_data(), sel.get_length(), NULL);
    }

    // there is no specific plain SVG input extension, so if we can paste the Inkscape SVG format,
    // we use the image/svg+xml mimetype to look up the input extension
    if (target == "image/x-inkscape-svg") {
        target = "image/svg+xml";
    }
    // Use the EMF extension to import metafiles
    if (target == "CF_ENHMETAFILE" || target == "WCF_ENHMETAFILE") {
        target = "image/x-emf";
    }

    Inkscape::Extension::DB::InputList inlist;
    Inkscape::Extension::db.get_input_list(inlist);
    Inkscape::Extension::DB::InputList::const_iterator in = inlist.begin();
    for (; in != inlist.end() && target != (*in)->get_mimetype() ; ++in) {
    };
    if ( in == inlist.end() ) {
        return NULL; // this shouldn't happen unless _getBestTarget returns something bogus
    }

    SPDocument *tempdoc = NULL;
    try {
        tempdoc = (*in)->open(filename);
    } catch (...) {
    }
    g_unlink(filename);
    g_free(filename);

    return tempdoc;
}


/**
 * Callback called when some other application requests data from Inkscape.
 *
 * Finds a suitable output extension to save the internal clipboard document,
 * then saves it to memory and sets the clipboard contents.
 */
void ClipboardManagerImpl::_onGet(Gtk::SelectionData &sel, guint /*info*/)
{
    g_assert( _clipboardSPDoc != NULL );

    Glib::ustring target = sel.get_target();
    if (target == "") {
        return; // this shouldn't happen
    }

    if (target == CLIPBOARD_TEXT_TARGET) {
        target = "image/x-inkscape-svg";
    }

    Inkscape::Extension::DB::OutputList outlist;
    Inkscape::Extension::db.get_output_list(outlist);
    Inkscape::Extension::DB::OutputList::const_iterator out = outlist.begin();
    for ( ; out != outlist.end() && target != (*out)->get_mimetype() ; ++out) {
    };
    if ( out == outlist.end() && target != "image/png") {
        return; // this also shouldn't happen
    }

    // FIXME: Temporary hack until we add support for memory output.
    // Save to a temporary file, read it back and then set the clipboard contents
    gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-export", NULL );
    gsize len; gchar *data;

    try {
        if (out == outlist.end() && target == "image/png")
        {
            gdouble dpi = Inkscape::Util::Quantity::convert(1, "in", "px");
            guint32 bgcolor = 0x00000000;

            Geom::Point origin (_clipboardSPDoc->getRoot()->x.computed, _clipboardSPDoc->getRoot()->y.computed);
            Geom::Rect area = Geom::Rect(origin, origin + _clipboardSPDoc->getDimensions());

            unsigned long int width = (unsigned long int) (Inkscape::Util::Quantity::convert(area.width(), "px", "in") * dpi + 0.5);
            unsigned long int height = (unsigned long int) (Inkscape::Util::Quantity::convert(area.height(), "in", "px") * dpi + 0.5);

            // read from namedview
            Inkscape::XML::Node *nv = sp_repr_lookup_name (_clipboardSPDoc->rroot, "sodipodi:namedview");
            if (nv && nv->attribute("pagecolor")) {
                bgcolor = sp_svg_read_color(nv->attribute("pagecolor"), 0xffffff00);
            }
            if (nv && nv->attribute("inkscape:pageopacity")) {
                double opacity = 1.0;
                sp_repr_get_double(nv, "inkscape:pageopacity", &opacity);
                bgcolor |= SP_COLOR_F_TO_U(opacity);
            }
            std::vector<SPItem*> x;
            sp_export_png_file(_clipboardSPDoc, filename, area, width, height, dpi, dpi, bgcolor, NULL, NULL, true, x);
        }
        else
        {
            if (!(*out)->loaded()) {
                // Need to load the extension.
                (*out)->set_state(Inkscape::Extension::Extension::STATE_LOADED);
            }
            (*out)->save(_clipboardSPDoc, filename);
        }
        g_file_get_contents(filename, &data, &len, NULL);

        sel.set(8, (guint8 const *) data, len);
    } catch (...) {
    }

    g_unlink(filename); // delete the temporary file
    g_free(filename);
}


/**
 * Callback when someone else takes the clipboard.
 *
 * When the clipboard owner changes, this callback clears the internal clipboard document
 * to reduce memory usage.
 */
void ClipboardManagerImpl::_onClear()
{
    // why is this called before _onGet???
    //_discardInternalClipboard();
}


/**
 * Creates an internal clipboard document from scratch.
 */
void ClipboardManagerImpl::_createInternalClipboard()
{
    if ( _clipboardSPDoc == NULL ) {
        _clipboardSPDoc = SPDocument::createNewDoc(NULL, false, true);
        //g_assert( _clipboardSPDoc != NULL );
        _defs = _clipboardSPDoc->getDefs()->getRepr();
        _doc = _clipboardSPDoc->getReprDoc();
        _root = _clipboardSPDoc->getReprRoot();

        _clipnode = _doc->createElement("inkscape:clipboard");
        _root->appendChild(_clipnode);
        Inkscape::GC::release(_clipnode);

        // once we create a SVG document, style will be stored in it, so flush _text_style
        if (_text_style) {
            sp_repr_css_attr_unref(_text_style);
            _text_style = NULL;
        }
    }
}


/**
 * Deletes the internal clipboard document.
 */
void ClipboardManagerImpl::_discardInternalClipboard()
{
    if ( _clipboardSPDoc != NULL ) {
        _clipboardSPDoc->doUnref();
        _clipboardSPDoc = NULL;
        _defs = NULL;
        _doc = NULL;
        _root = NULL;
        _clipnode = NULL;
    }
}


/**
 * Get the scale to resize an item, based on the command and desktop state.
 */
Geom::Scale ClipboardManagerImpl::_getScale(SPDesktop *desktop, Geom::Point const &min, Geom::Point const &max, Geom::Rect const &obj_rect, bool apply_x, bool apply_y)
{
    double scale_x = 1.0;
    double scale_y = 1.0;

    if (apply_x) {
        scale_x = (max[Geom::X] - min[Geom::X]) / obj_rect[Geom::X].extent();
    }
    if (apply_y) {
        scale_y = (max[Geom::Y] - min[Geom::Y]) / obj_rect[Geom::Y].extent();
    }
    // If the "lock aspect ratio" button is pressed and we paste only a single coordinate,
    // resize the second one by the same ratio too
    if (desktop->isToolboxButtonActive("lock")) {
        if (apply_x && !apply_y) {
            scale_y = scale_x;
        }
        if (apply_y && !apply_x) {
            scale_x = scale_y;
        }
    }

    return Geom::Scale(scale_x, scale_y);
}


/**
 * Find the most suitable clipboard target.
 */
Glib::ustring ClipboardManagerImpl::_getBestTarget()
{
    // GTKmm's wait_for_targets() is broken, see the comment in _inkscape_wait_for_targets()
    std::list<Glib::ustring> targets; // = _clipboard->wait_for_targets();
    _inkscape_wait_for_targets(targets);

    // clipboard target debugging snippet
    /*
    g_message("Begin clipboard targets");
    for ( std::list<Glib::ustring>::iterator x = targets.begin() ; x != targets.end(); ++x )
        g_message("Clipboard target: %s", (*x).data());
    g_message("End clipboard targets\n");
    //*/

    for (std::list<Glib::ustring>::iterator i = _preferred_targets.begin() ;
        i != _preferred_targets.end() ; ++i)
    {
        if ( std::find(targets.begin(), targets.end(), *i) != targets.end() ) {
            return *i;
        }
    }
#ifdef WIN32
    if (OpenClipboard(NULL))
    {   // If both bitmap and metafile are present, pick the one that was exported first.
        UINT format = EnumClipboardFormats(0);
        while (format) {
            if (format == CF_ENHMETAFILE || format == CF_DIB || format == CF_BITMAP) {
                break;
            }
            format = EnumClipboardFormats(format);
        }
        CloseClipboard();

        if (format == CF_ENHMETAFILE) {
            return "CF_ENHMETAFILE";
        }
        if (format == CF_DIB || format == CF_BITMAP) {
            return CLIPBOARD_GDK_PIXBUF_TARGET;
        }
    }

    if (IsClipboardFormatAvailable(CF_ENHMETAFILE)) {
        return "CF_ENHMETAFILE";
    }
#endif
    if (_clipboard->wait_is_image_available()) {
        return CLIPBOARD_GDK_PIXBUF_TARGET;
    }
    if (_clipboard->wait_is_text_available()) {
        return CLIPBOARD_TEXT_TARGET;
    }

    return "";
}


/**
 * Set the clipboard targets to reflect the mimetypes Inkscape can output.
 */
void ClipboardManagerImpl::_setClipboardTargets()
{
    Inkscape::Extension::DB::OutputList outlist;
    Inkscape::Extension::db.get_output_list(outlist);

#if WITH_GTKMM_3_0
    std::vector<Gtk::TargetEntry> target_list;
#else
    std::list<Gtk::TargetEntry> target_list;
#endif

    bool plaintextSet = false;
    for (Inkscape::Extension::DB::OutputList::const_iterator out = outlist.begin() ; out != outlist.end() ; ++out) {
        if ( !(*out)->deactivated() ) {
            Glib::ustring mime = (*out)->get_mimetype();
            if (mime != CLIPBOARD_TEXT_TARGET) {
                if ( !plaintextSet && (mime.find("svg") == Glib::ustring::npos) ) {
                    target_list.push_back(Gtk::TargetEntry(CLIPBOARD_TEXT_TARGET));
                    plaintextSet = true;
                }
                target_list.push_back(Gtk::TargetEntry(mime));
            }
        }
    }

    // Add PNG export explicitly since there is no extension for this...
    // On Windows, GTK will also present this as a CF_DIB/CF_BITMAP
    target_list.push_back(Gtk::TargetEntry( "image/png" ));

    _clipboard->set(target_list,
        sigc::mem_fun(*this, &ClipboardManagerImpl::_onGet),
        sigc::mem_fun(*this, &ClipboardManagerImpl::_onClear));

#ifdef WIN32
    // If the "image/x-emf" target handled by the emf extension would be
    // presented as a CF_ENHMETAFILE automatically (just like an "image/bmp"
    // is presented as a CF_BITMAP) this code would not be needed.. ???
    // Or maybe there is some other way to achieve the same?

    // Note: Metafile is the only format that is rendered and stored in clipboard
    // on Copy, all other formats are rendered only when needed by a Paste command.

    // FIXME: This should at least be rewritten to use "delayed rendering".
    //        If possible make it delayed rendering by using GTK API only.

    if (OpenClipboard(NULL)) {
        if ( _clipboardSPDoc != NULL ) {
            const Glib::ustring target = "image/x-emf";

            Inkscape::Extension::DB::OutputList outlist;
            Inkscape::Extension::db.get_output_list(outlist);
            Inkscape::Extension::DB::OutputList::const_iterator out = outlist.begin();
            for ( ; out != outlist.end() && target != (*out)->get_mimetype() ; ++out) {
            }
            if ( out != outlist.end() ) {
                // FIXME: Temporary hack until we add support for memory output.
                // Save to a temporary file, read it back and then set the clipboard contents
                gchar *filename = g_build_filename( g_get_tmp_dir(), "inkscape-clipboard-export.emf", NULL );

                try {
                    (*out)->save(_clipboardSPDoc, filename);
                    HENHMETAFILE hemf = GetEnhMetaFileA(filename);
                    if (hemf) {
                        SetClipboardData(CF_ENHMETAFILE, hemf);
                        DeleteEnhMetaFile(hemf);
                    }
                } catch (...) {
                }
                g_unlink(filename); // delete the temporary file
                g_free(filename);
            }
        }
        CloseClipboard();
    }
#endif
}


/**
 * Set the string representation of a 32-bit RGBA color as the clipboard contents.
 */
void ClipboardManagerImpl::_setClipboardColor(guint32 color)
{
    gchar colorstr[16];
    g_snprintf(colorstr, 16, "%08x", color);
    _clipboard->set_text(colorstr);
}


/**
 * Put a notification on the mesage stack.
 */
void ClipboardManagerImpl::_userWarn(SPDesktop *desktop, char const *msg)
{
    desktop->messageStack()->flash(Inkscape::WARNING_MESSAGE, msg);
}


// GTKMM's clipboard::wait_for_targets is buggy and might return bogus, see
//
// https://bugs.launchpad.net/inkscape/+bug/296778
// http://mail.gnome.org/archives/gtk-devel-list/2009-June/msg00062.html
//
// for details. Until this has been fixed upstream we will use our own implementation
// of this method, as copied from /gtkmm-2.16.0/gtk/gtkmm/clipboard.cc.
void ClipboardManagerImpl::_inkscape_wait_for_targets(std::list<Glib::ustring> &listTargets)
{
    //Get a newly-allocated array of atoms:
    GdkAtom* targets = NULL;
    gint n_targets = 0;
    gboolean test = gtk_clipboard_wait_for_targets( gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), &targets, &n_targets );
    if (!test || (targets == NULL)) {
        return;
    }

    //Add the targets to the C++ container:
    for (int i = 0; i < n_targets; i++)
    {
        //Convert the atom to a string:
        gchar* const atom_name = gdk_atom_name(targets[i]);

        Glib::ustring target;
        if (atom_name) {
            target = Glib::ScopedPtr<char>(atom_name).get(); //This frees the gchar*.
        }

        listTargets.push_back(target);
    }
}

/* #######################################
          ClipboardManager class
   ####################################### */

ClipboardManager *ClipboardManager::_instance = NULL;

ClipboardManager::ClipboardManager() {}
ClipboardManager::~ClipboardManager() {}
ClipboardManager *ClipboardManager::get()
{
    if ( _instance == NULL ) {
        _instance = new ClipboardManagerImpl;
    }

    return _instance;
}

} // namespace Inkscape
} // namespace IO

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
