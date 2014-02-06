#ifndef __SP_VIEWBOX_H__
#define __SP_VIEWBOX_H__

/*
 * viewBox helper class, common code used by root, symbol, marker, pattern, image, view
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com> (code extracted from sp-symbol.h)
 *   Tavmjong Bah
 *
 * Copyright (C) 2013 Tavmjong Bah, authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include <2geom/rect.h>
#include <glib.h>

namespace Geom {
class Rect;
}
class SPItemCtx;

class SPViewBox {

 public:

  SPViewBox();
  ~SPViewBox();

  /* viewBox; */
  unsigned int viewBox_set : 1;
  Geom::Rect viewBox;  // Could use optrect

  /* preserveAspectRatio */
  unsigned int aspect_set : 1;
  unsigned int aspect_align : 4;
  unsigned int aspect_clip : 1;

  /* Child to parent additional transform */
  Geom::Affine c2p;

  void set_viewBox(const gchar* value);
  void set_preserveAspectRatio(const gchar* value);

  /* Adjusts c2p for viewbox */
  void apply_viewbox(const Geom::Rect& in);

  SPItemCtx get_rctx( const SPItemCtx* ictx);

};

#endif

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-basic-offset:2
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2:fileencoding=utf-8:textwidth=99 :
