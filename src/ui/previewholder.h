
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

namespace Inkscape {
namespace UI {

class PreviewHolder : public Gtk::VBox, public PreviewFillable
{
public:
    PreviewHolder();
    virtual ~PreviewHolder();

    virtual void clear();
    virtual void addPreview( Previewable* preview );
    virtual void setStyle(Gtk::BuiltinIconSize size, ViewType view);
    virtual Gtk::BuiltinIconSize getPreviewSize() const { return _baseSize; }
    virtual ViewType getPreviewType() const { return _view; }

private:
    void rebuildUI();

    std::vector<Previewable*> items;
    Gtk::Bin *_scroller;
    Gtk::Table *_insides;
    Gtk::BuiltinIconSize _baseSize;
    ViewType _view;
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
