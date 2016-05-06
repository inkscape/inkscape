
#ifndef SEEN_PREVIEW_HOLDER_H
#define SEEN_PREVIEW_HOLDER_H
/*
 * A simple interface for previewing representations.
 * Used by Swatches
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtkmm/box.h>
#include <gtkmm/bin.h>

namespace Gtk {
#if WITH_GTKMM_3_0
class Grid;
#else
class Table;
#endif
}

#include "previewfillable.h"
#include "../widgets/eek-preview.h"
#include "enums.h"

namespace Inkscape {
namespace UI {

class PreviewHolder : public Gtk::VBox, public PreviewFillable
{
public:
    PreviewHolder();
    virtual ~PreviewHolder();

    virtual void clear();
    virtual void addPreview( Previewable* preview );
    virtual void freezeUpdates();
    virtual void thawUpdates();
    virtual void setStyle( ::PreviewSize size, ViewType view, guint ratio, ::BorderStyle border );
    virtual void setOrientation(SPAnchorType how);
    virtual int getColumnPref() const { return _prefCols; }
    virtual void setColumnPref( int cols );
    virtual ::PreviewSize getPreviewSize() const { return _baseSize; }
    virtual ViewType getPreviewType() const { return _view; }
    virtual guint getPreviewRatio() const { return _ratio; }
    virtual ::BorderStyle getPreviewBorder() const { return _border; }
    virtual void setWrap( bool b );
    virtual bool getWrap() const { return _wrap; }

protected:
    virtual void on_size_allocate( Gtk::Allocation& allocation );
    virtual bool on_scroll_event(GdkEventScroll*);
//    virtual void on_size_request( Gtk::Requisition* requisition );


private:
    void rebuildUI();
    void calcGridSize( const Gtk::Widget* thing, int itemCount, int& width, int& height );

    std::vector<Previewable*> items;
    Gtk::Bin *_scroller;

#if WITH_GTKMM_3_0
    Gtk::Grid *_insides;
#else
    Gtk::Table *_insides;
#endif

    int _prefCols;
    bool _updatesFrozen;
    SPAnchorType _anchor;
    ::PreviewSize _baseSize;
    guint _ratio;
    ViewType _view;
    bool _wrap;
    ::BorderStyle _border;
};

} //namespace UI
} //namespace Inkscape

#endif // SEEN_PREVIEW_HOLDER_H

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
