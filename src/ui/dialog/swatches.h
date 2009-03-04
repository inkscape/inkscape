/** @file
 * @brief Color swatches dialog
 */
/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_DIALOGS_SWATCHES_H
#define SEEN_DIALOGS_SWATCHES_H

#include <gtkmm/textview.h>
#include <gtkmm/tooltips.h>

#include "ui/widget/panel.h"
#include "ui/previewholder.h"
#include "widgets/eek-color-def.h"

namespace Inkscape {
namespace UI {
namespace Dialogs {


void _loadPaletteFile( gchar const *filename );

/**
 * The color swatch you see on screen as a clickable box.
 */
class ColorItem : public Inkscape::UI::Previewable
{
    friend void _loadPaletteFile( gchar const *filename );
public:
    ColorItem();
    ColorItem( unsigned int r, unsigned int g, unsigned int b,
               Glib::ustring& name );
    virtual ~ColorItem();
    ColorItem(ColorItem const &other);
    virtual ColorItem &operator=(ColorItem const &other);
    virtual Gtk::Widget* getPreview(PreviewStyle style,
                                    ViewType view,
                                    ::PreviewSize size,
                                    guint ratio);
    void buttonClicked(bool secondary = false);
    eek::ColorDef def;

private:
    static void _dropDataIn( GtkWidget *widget,
                             GdkDragContext *drag_context,
                             gint x, gint y,
                             GtkSelectionData *data,
                             guint info,
                             guint event_time,
                             gpointer user_data);

    static void _dragGetColorData( GtkWidget *widget,
                                   GdkDragContext *drag_context,
                                   GtkSelectionData *data,
                                   guint info,
                                   guint time,
                                   gpointer user_data);

    static void _wireMagicColors( void* p );
    static void _colorDefChanged(void* data);

    void _linkTint( ColorItem& other, int percent );
    void _linkTone( ColorItem& other, int percent, int grayLevel );

    Gtk::Tooltips tips;
    std::vector<Gtk::Widget*> _previews;

    bool _isLive;
    bool _linkIsTone;
    int _linkPercent;
    int _linkGray;
    ColorItem* _linkSrc;
    std::vector<ColorItem*> _listeners;
};
	
class RemoveColorItem;

/**
 * A panel that displays color swatches.
 */
class SwatchesPanel : public Inkscape::UI::Widget::Panel
{
public:
    SwatchesPanel(gchar const* prefsPath = "/dialogs/swatches");
    virtual ~SwatchesPanel();

    static SwatchesPanel& getInstance();
    virtual void setOrientation( Gtk::AnchorType how );

protected:
    virtual void _handleAction( int setId, int itemId );

private:
    SwatchesPanel(SwatchesPanel const &); // no copy
    SwatchesPanel &operator=(SwatchesPanel const &); // no assign

    static SwatchesPanel* instance;

    PreviewHolder* _holder;
    ColorItem* _remove;
};

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape



#endif // SEEN_SWATCHES_H

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
