/** \file
 * \brief  Static class where all preferences handling happens.
 *
 * Authors:
 *   Ralf Stephan <ralf@ark.in-berlin.de>
 *
 * Copyright (C) 2005 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifndef INKSCAPE_PREFERENCES_H
#define INKSCAPE_PREFERENCES_H

namespace Inkscape {
    namespace XML {
        class Document;
    }

class Preferences
{
public:
    static XML::Document *get();
    static void load();
    static void save();
    static void loadSkeleton();
   
private:
};

} // namespace Inkscape

#endif // INKSCAPE_PREFERENCES_H

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
