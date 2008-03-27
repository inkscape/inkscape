#ifndef SEEN_INKSCAPE_CLIPBOARD_H
#define SEEN_INKSCAPE_CLIPBOARD_H

/** @file
 * @brief System-wide clipboard management - class declaration
 */
/* Authors:
 *   Krzysztof Kosiński <tweenk@o2.pl>
 *
 * Copyright (C) 2008 authors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See the file COPYING for details.
 */

#include <glibmm/ustring.h>

// forward declarations
class SPDesktop;
namespace Inkscape {
class Selection;
namespace LivePathEffect { class PathParam; }

namespace UI {

/**
 * @brief System-wide clipboard manager
 *
 * ClipboardManager takes care of manipulating the system clipboard in response
 * to user actions. It holds a complete SPDocument as the contents. This document
 * is exported using output extensions when other applications request data.
 * Copying to another instance of Inkscape is special-cased, because of the extra
 * data required (i.e. style, size, Live Path Effects parameters, etc.)
 */

class ClipboardManager {
public:
    virtual void copy() = 0;
    virtual void copyPathParameter(Inkscape::LivePathEffect::PathParam *) = 0;
    virtual bool paste(bool in_place = false) = 0;
    virtual bool pasteStyle() = 0;
    virtual bool pasteSize(bool separately, bool apply_x, bool apply_y) = 0;
    virtual bool pastePathEffect() = 0;
    virtual Glib::ustring getPathParameter() = 0;
    virtual Glib::ustring getPathObjectId() = 0;
    
    static ClipboardManager *get();
protected:
    ClipboardManager(); // singleton
    virtual ~ClipboardManager();
private:
    ClipboardManager(const ClipboardManager &); ///< no copy
    ClipboardManager &operator=(const ClipboardManager &); ///< no assign
    
    static ClipboardManager *_instance;
};

} // namespace IO
} // namespace Inkscape

#endif
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
