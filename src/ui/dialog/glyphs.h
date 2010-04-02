/**
 * Glyph selector dialog.
 */

/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2010 Jon A. Cruz
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_DIALOGS_GLYPHS_H
#define SEEN_DIALOGS_GLYPHS_H

#include <gtkmm/treemodel.h>
#include "ui/widget/panel.h"


namespace Gtk {
class Entry;
class IconView;
class Label;
class ListStore;
}

class SPFontSelector;
class font_instance;


namespace Inkscape {
namespace UI {

class PreviewHolder;

namespace Dialogs {

class GlyphColumns;

/**
 * A panel that displays character glyphs.
 */

class GlyphsPanel : public Inkscape::UI::Widget::Panel
{
public:
    GlyphsPanel(gchar const *prefsPath = "/dialogs/glyphs");
    virtual ~GlyphsPanel();

    static GlyphsPanel& getInstance();

protected:

private:
    GlyphsPanel(GlyphsPanel const &); // no copy
    GlyphsPanel &operator=(GlyphsPanel const &); // no assign

    static GlyphColumns *getColumns();

    static void fontChangeCB(SPFontSelector *fontsel, font_instance *font, GlyphsPanel *self);

    void handleFontChange(SPFontSelector *fontsel, font_instance *font);
    void glyphActivated(Gtk::TreeModel::Path const & path);
    void glyphSelectionChanged();


    Glib::RefPtr<Gtk::ListStore> store;
    Gtk::IconView *iconView;
    Gtk::Entry *entry;
    Gtk::Label *label;
};


} // namespace Dialogs
} // namespace UI
} // namespace Inkscape

#endif // SEEN_DIALOGS_GLYPHS_H
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
