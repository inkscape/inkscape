
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
#include "../widgets/eek-preview.h"
#include "enums.h"

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
    virtual void setStyle( ::PreviewSize size, ViewType type, guint ratio, ::BorderStyle border ) = 0;
    virtual void setOrientation(SPAnchorType how) = 0;
    virtual ::PreviewSize getPreviewSize() const = 0;
    virtual ViewType getPreviewType() const = 0;
    virtual guint getPreviewRatio() const = 0;
    virtual ::BorderStyle getPreviewBorder() const = 0;
    virtual void setWrap( bool b ) = 0;
    virtual bool getWrap() const = 0;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
