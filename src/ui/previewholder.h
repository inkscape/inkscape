
#ifndef SEEN_PREVIEW_HOLDER_H
#define SEEN_PREVIEW_HOLDER_H
/*
 * A simple interface for previewing representations.
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include <gtkmm/box.h>
#include <gtkmm/bin.h>
#include <gtkmm/table.h>
#include "previewfillable.h"
#include "../icon-size.h"

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
    virtual void setStyle( Inkscape::IconSize size, ViewType view );
    virtual void setOrientation( Gtk::AnchorType how );
    virtual int getColumnPref() const { return _prefCols; }
    virtual void setColumnPref( int cols );
    virtual Inkscape::IconSize getPreviewSize() const { return _baseSize; }
    virtual ViewType getPreviewType() const { return _view; }
    virtual void setWrap( bool b );
    virtual bool getWrap() const { return _wrap; }

protected:
    virtual void on_size_allocate( Gtk::Allocation& allocation );
    virtual void on_size_request( Gtk::Requisition* requisition );


private:
    void rebuildUI();
    void calcGridSize( const Gtk::Widget* thing, int itemCount, int& width, int& height );

    std::vector<Previewable*> items;
    Gtk::Bin *_scroller;
    Gtk::Table *_insides;
    int _prefCols;
    bool _updatesFrozen;
    Gtk::AnchorType _anchor;
    Inkscape::IconSize _baseSize;
    ViewType _view;
    bool _wrap;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
