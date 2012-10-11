/** @file
 * @brief Symbols dialog
 */
/* Authors:
 *   Tavmjong Bah
 *
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_UI_DIALOG_SYMBOLS_H
#define INKSCAPE_UI_DIALOG_SYMBOLS_H

#include "ui/widget/panel.h"
#include "ui/widget/button.h"

#include "ui/dialog/desktop-tracker.h"

#include "display/drawing.h"

#include <glib.h>
#include <gtkmm/treemodel.h>

#include <vector>

class SPObject;

namespace Inkscape {
namespace UI {
namespace Dialog {

class SymbolColumns; // For Gtk::ListStore

/**
 * A dialog that displays selectable symbols.
 */
class SymbolsDialog : public UI::Widget::Panel {

public:
    SymbolsDialog( gchar const* prefsPath = "/dialogs/symbols" );
    virtual ~SymbolsDialog();

    static SymbolsDialog& getInstance();

protected:


private:
    SymbolsDialog(SymbolsDialog const &); // no copy
    SymbolsDialog &operator=(SymbolsDialog const &); // no assign

    static SymbolColumns *getColumns();

    void rebuild();
    void iconChanged();

    void get_symbols();
    void draw_symbols( SPDocument* symbol_document );
    SPDocument* symbols_preview_doc();

    GSList* symbols_in_doc_recursive(SPObject *r, GSList *l);
    GSList* symbols_in_doc( SPDocument* document );
    GSList* use_in_doc_recursive(SPObject *r, GSList *l);
    GSList* use_in_doc( SPDocument* document );
    gchar const* style_from_use( gchar const* id, SPDocument* document);

    Glib::RefPtr<Gdk::Pixbuf>
    create_symbol_image(gchar const *symbol_name,
                        SPDocument *source,  Inkscape::Drawing* drawing,
                        unsigned /*visionkey*/);

    /* Keep track of all symbol template documents */
    std::map<Glib::ustring, SPDocument*> symbolSets;


    Glib::RefPtr<Gtk::ListStore> store;
    Gtk::ComboBoxText* symbolSet;
    Gtk::IconView* iconView;
    Gtk::ComboBoxText* previewScale;
    Gtk::ComboBoxText* previewSize;

    void setTargetDesktop(SPDesktop *desktop);
    SPDesktop*  currentDesktop;
    DesktopTracker deskTrack;
    SPDocument* currentDocument;
    SPDocument* previewDocument; /* Document to render single symbol */

    /* For rendering the template drawing */
    unsigned key;
    Inkscape::Drawing renderDrawing;

    std::vector<sigc::connection> instanceConns;
    sigc::connection desktopChangeConn;

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
