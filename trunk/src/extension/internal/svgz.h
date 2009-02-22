/*
 * Code to handle compressed SVG loading and saving. Almost identical to svg
 * routines, but separated for simpler extension maintenance.
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_SVGZ_H
#define SEEN_SVGZ_H

#include "svg.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class Svgz : public Svg {
public:
    static void init( void );
};

} } }  // namespace Inkscape, Extension, Implementation
#endif // SEEN_SVGZ_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
