/** @file
 * @brief Symbols dialog
 */
/* Authors:
 *   Tavmjong Bah, Martin Owens
 *
 * Copyright (C) 2012 Tavmjong Bah
 *               2013 Martin Owens
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_SYMBOLS_H
#define INKSCAPE_UI_DIALOG_SYMBOLS_H

#include "display/drawing.h"

#include "ui/dialog/desktop-tracker.h"

#include "ui/widget/panel.h"

#include <vector>

class SPObject;

namespace Inkscape {
namespace UI {
namespace Dialog {

class SymbolColumns; // For Gtk::ListStore

/**
 * A dialog that displays selectable symbols and allows users to drag or paste
 * those symbols from the dialog into the document.
 *
 * Symbol documents are loaded from the preferences paths and displayed in a
 * drop-down list to the user. The user then selects which of the symbols
 * documents they want to get symbols from. The first document in the list is
 * always the current document.
 *
 * This then updates an icon-view with all the symbols available. Selecting one
 * puts it onto the clipboard. Dragging it or pasting it onto the canvas copies
 * the symbol from the symbol document, into the current document and places a
 * new <use element at the correct location on the canvas.
 *
 * Selected groups on the canvas can be added to the current document's symbols
 * table, and symbols can be removed from the current document. This allows
 * new symbols documents to be constructed and if saved in the prefs folder will
 * make those symbols available for all future documents.
 */

const int SYMBOL_ICON_SIZES[] = {16, 24, 32, 48, 64};

class SymbolsDialog : public UI::Widget::Panel {

public:
    SymbolsDialog( gchar const* prefsPath = "/dialogs/symbols" );
    virtual ~SymbolsDialog();

    static SymbolsDialog& getInstance();

private:
    SymbolsDialog(SymbolsDialog const &); // no copy
    SymbolsDialog &operator=(SymbolsDialog const &); // no assign

    static SymbolColumns *getColumns();

    void packless();
    void packmore();
    void zoomin();
    void zoomout();
    void rebuild();
    void insertSymbol();
    void revertSymbol();
    void defsModified(SPObject *object, guint flags);
    void selectionChanged(Inkscape::Selection *selection);
    void documentReplaced(SPDesktop *desktop, SPDocument *document);
    SPDocument* selectedSymbols();
    Glib::ustring selectedSymbolId();
    void iconChanged();
    void iconDragDataGet(const Glib::RefPtr<Gdk::DragContext>& context, Gtk::SelectionData& selection_data, guint info, guint time);

    void get_symbols();
    void add_symbols( SPDocument* symbol_document );
    void add_symbol( SPObject* symbol_document );
    SPDocument* symbols_preview_doc();

    GSList* symbols_in_doc_recursive(SPObject *r, GSList *l);
    GSList* symbols_in_doc( SPDocument* document );
    GSList* use_in_doc_recursive(SPObject *r, GSList *l);
    GSList* use_in_doc( SPDocument* document );
    gchar const* style_from_use( gchar const* id, SPDocument* document);

    Glib::RefPtr<Gdk::Pixbuf> draw_symbol(SPObject *symbol);

    /* Keep track of all symbol template documents */
    std::map<Glib::ustring, SPDocument*> symbolSets;

    // Index into sizes which is selected
    int pack_size;

    // Scale factor
    int scale_factor;

    Glib::RefPtr<Gtk::ListStore> store;
    Gtk::ComboBoxText* symbolSet;
    Gtk::IconView* iconView;
    Gtk::Button* addSymbol;
    Gtk::Button* removeSymbol;
    Gtk::Button* zoomIn;
    Gtk::Button* zoomOut;
    Gtk::ToggleButton* fitSymbol;

    void setTargetDesktop(SPDesktop *desktop);
    SPDesktop*  currentDesktop;
    DesktopTracker deskTrack;
    SPDocument* currentDocument;
    SPDocument* previewDocument; /* Document to render single symbol */

    /* For rendering the template drawing */
    unsigned key;
    Inkscape::Drawing renderDrawing;

    std::vector<sigc::connection> instanceConns;
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape


#endif // INKSCAPE_UI_DIALOG_SYMBOLS_H

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
