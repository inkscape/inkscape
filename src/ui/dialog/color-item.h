/** @file
 * @brief Inkscape color swatch UI item.
 */
/* Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2010 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_DIALOGS_COLOR_ITEM_H
#define SEEN_DIALOGS_COLOR_ITEM_H

#include <boost/ptr_container/ptr_vector.hpp>

#include "widgets/ege-paint-def.h"
#include "ui/previewable.h"

class SPGradient;

namespace Inkscape {
namespace UI {
namespace Dialogs {

class ColorItem;

class SwatchPage
{
public:
    SwatchPage();
    ~SwatchPage();

    Glib::ustring _name;
    int _prefWidth;
    boost::ptr_vector<ColorItem> _colors;
};


/**
 * The color swatch you see on screen as a clickable box.
 */
class ColorItem : public Inkscape::UI::Previewable
{
    friend void _loadPaletteFile( gchar const *filename );
public:
    ColorItem( ege::PaintDef::ColorType type );
    ColorItem( unsigned int r, unsigned int g, unsigned int b,
               Glib::ustring& name );
    virtual ~ColorItem();
    ColorItem(ColorItem const &other);
    virtual ColorItem &operator=(ColorItem const &other);
    virtual Gtk::Widget* getPreview(PreviewStyle style,
                                    ViewType view,
                                    ::PreviewSize size,
                                    guint ratio,
                                    guint border);
    void buttonClicked(bool secondary = false);

    void setGradient(SPGradient *grad);
    SPGradient * getGradient() const { return _grad; }
    void setPattern(cairo_pattern_t *pattern);
    void setName(const Glib::ustring name);

    void setState( bool fill, bool stroke );
    bool isFill() { return _isFill; }
    bool isStroke() { return _isStroke; }

    ege::PaintDef def;

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

    static void _wireMagicColors( SwatchPage *colorSet );
    static void _colorDefChanged(void* data);

    void _updatePreviews();
    void _regenPreview(EekPreview * preview);

    void _linkTint( ColorItem& other, int percent );
    void _linkTone( ColorItem& other, int percent, int grayLevel );

    std::vector<Gtk::Widget*> _previews;

    bool _isFill;
    bool _isStroke;
    bool _isLive;
    bool _linkIsTone;
    int _linkPercent;
    int _linkGray;
    ColorItem* _linkSrc;
    SPGradient* _grad;
    cairo_pattern_t *_pattern;
    std::vector<ColorItem*> _listeners;
};

} // namespace Dialogs
} // namespace UI
} // namespace Inkscape

#endif // SEEN_DIALOGS_COLOR_ITEM_H

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
