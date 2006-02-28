
#ifndef SEEN_PREVIEWFILLABLE_H
#define SEEN_PREVIEWFILLABLE_H
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


#include "previewable.h"


namespace Inkscape {
namespace UI {

class PreviewFillable
{
public:
    virtual ~PreviewFillable() {}
    virtual void clear() = 0;
    virtual void addPreview( Previewable* preview ) = 0;
    virtual void freezeUpdates() = 0;
    virtual void thawUpdates() = 0;
    virtual void setStyle(Gtk::BuiltinIconSize size, ViewType type) = 0;
    virtual void setOrientation( Gtk::AnchorType how ) = 0;
    virtual Gtk::BuiltinIconSize getPreviewSize() const = 0;
    virtual ViewType getPreviewType() const = 0;
};


} //namespace UI
} //namespace Inkscape


#endif // SEEN_PREVIEWFILLABLE_H

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
