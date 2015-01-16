/*
 * viewBox helper class, common code used by root, symbol, marker, pattern, image, view
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com> (code extracted from symbol.cpp)
 *   Tavmjong Bah <tavmjong@free.fr>
 *   Johan Engelen
 *
 * Copyright (C) 2013-2014 Tavmjong Bah, authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include <2geom/transforms.h>

#include "viewbox.h"
#include "attributes.h"
#include "enums.h"
#include "sp-item.h"

SPViewBox::SPViewBox()
    : viewBox_set(false)
    , viewBox()
    , aspect_set(false)
    , aspect_align(SP_ASPECT_XMID_YMID) // Default per spec
    , aspect_clip(SP_ASPECT_MEET)
    , c2p(Geom::identity())
{
}

void SPViewBox::set_viewBox(const gchar* value) {

  if (value) {
    gchar *eptr = const_cast<gchar*>(value); // const-cast necessary because of const-incorrect interface definition of g_ascii_strtod

    double x = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    double y = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    double width = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    double height = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    if ((width > 0) && (height > 0)) {
      /* Set viewbox */
      this->viewBox = Geom::Rect::from_xywh(x, y, width, height);
      this->viewBox_set = true;
    } else {
      this->viewBox_set = false;
    }
  } else {
    this->viewBox_set = false;
  }

  // The C++ way?  -- not necessarily using iostreams
  // std::string sv( value );
  // std::replace( sv.begin(), sv.end(), ',', ' ');
  // std::stringstream ss( sv );
  // double x, y, width, height;
  // ss >> x >> y >> width >> height;
}

void SPViewBox::set_preserveAspectRatio(const gchar* value) {

  /* Do setup before, so we can use break to escape */
  this->aspect_set = false;
  this->aspect_align = SP_ASPECT_XMID_YMID; // Default per spec
  this->aspect_clip = SP_ASPECT_MEET;

  if (value) {
    const gchar *p = value;

    while (*p && (*p == 32)) {
      p += 1;
    }

    if (!*p) {
      return;
    }

    const gchar *e = p;

    while (*e && (*e != 32)) {
      e += 1;
    }

    int len = e - p;

    if (len > 8) {  // Can't have buffer overflow as 8 < 256
      return;
    }

    gchar c[256];
    memcpy (c, value, len);

    c[len] = 0;

    /* Now the actual part */
    unsigned int align = SP_ASPECT_NONE;
    if (!strcmp (c, "none")) {
      align = SP_ASPECT_NONE;
    } else if (!strcmp (c, "xMinYMin")) {
      align = SP_ASPECT_XMIN_YMIN;
    } else if (!strcmp (c, "xMidYMin")) {
      align = SP_ASPECT_XMID_YMIN;
    } else if (!strcmp (c, "xMaxYMin")) {
      align = SP_ASPECT_XMAX_YMIN;
    } else if (!strcmp (c, "xMinYMid")) {
      align = SP_ASPECT_XMIN_YMID;
    } else if (!strcmp (c, "xMidYMid")) {
      align = SP_ASPECT_XMID_YMID;
    } else if (!strcmp (c, "xMaxYMid")) {
      align = SP_ASPECT_XMAX_YMID;
    } else if (!strcmp (c, "xMinYMax")) {
      align = SP_ASPECT_XMIN_YMAX;
    } else if (!strcmp (c, "xMidYMax")) {
      align = SP_ASPECT_XMID_YMAX;
    } else if (!strcmp (c, "xMaxYMax")) {
      align = SP_ASPECT_XMAX_YMAX;
    } else {
      return;
    }

    unsigned int clip = SP_ASPECT_MEET;

    while (*e && (*e == 32)) {
      e += 1;
    }

    if (*e) {
      if (!strcmp (e, "meet")) {
        clip = SP_ASPECT_MEET;
      } else if (!strcmp (e, "slice")) {
        clip = SP_ASPECT_SLICE;
      } else {
        return;
      }
    }

    this->aspect_set = true;
    this->aspect_align = align;
    this->aspect_clip = clip;
  }
}

// Apply scaling from viewbox
void SPViewBox::apply_viewbox(const Geom::Rect& in) {

    /* Determine actual viewbox in viewport coordinates */
    /* These are floats since SVGLength is a float: See bug 1374614 */
    float x = 0.0;
    float y = 0.0;
    float width = in.width();
    float height = in.height();
    // std::cout << "  width: " << width << " height: " << height << std::endl;

    if (this->aspect_align != SP_ASPECT_NONE) {
      /* Things are getting interesting */
      double scalex = in.width() / ((float) this->viewBox.width());
      double scaley = in.height() / ((float) this->viewBox.height());
      double scale = (scalex + scaley)/2.0; // default if aspect ratio is not changing
      if (!Geom::are_near(scalex / scaley, 1.0, Geom::EPSILON))
        scale = (this->aspect_clip == SP_ASPECT_MEET) ? MIN (scalex, scaley) : MAX (scalex, scaley);
      width  = this->viewBox.width()  * scale;
      height = this->viewBox.height() * scale;

      /* Now place viewbox to requested position */
      switch (this->aspect_align) {
        case SP_ASPECT_XMIN_YMIN:
          break;
        case SP_ASPECT_XMID_YMIN:
          x = 0.5 * (in.width() - width);
          break;
        case SP_ASPECT_XMAX_YMIN:
          x = 1.0 * (in.width() - width);
          break;
        case SP_ASPECT_XMIN_YMID:
          y = 0.5 * (in.height() - height);
          break;
        case SP_ASPECT_XMID_YMID:
          x = 0.5 * (in.width() - width);
          y = 0.5 * (in.height() - height);
          break;
        case SP_ASPECT_XMAX_YMID:
          x = 1.0 * (in.width() - width);
          y = 0.5 * (in.height() - height);
          break;
        case SP_ASPECT_XMIN_YMAX:
          y = 1.0 * (in.height() - height);
          break;
        case SP_ASPECT_XMID_YMAX:
          x = 0.5 * (in.width() - width);
          y = 1.0 * (in.height() - height);
          break;
        case SP_ASPECT_XMAX_YMAX:
          x = 1.0 * (in.width() - width);
          y = 1.0 * (in.height() - height);
          break;
        default:
          break;
      }
    }

    /* Viewbox transform from scale and position */
    Geom::Affine q;
    q[0] = width / ((float) this->viewBox.width());
    q[1] = 0.0;
    q[2] = 0.0;
    q[3] = height / ((float) this->viewBox.height());
    q[4] = x - q[0] * ((float) this->viewBox.left());
    q[5] = y - q[3] * ((float) this->viewBox.top());

    // std::cout << "  q\n" << q << std::endl;

    /* Append viewbox transformation */
    this->c2p = q * this->c2p;
}

SPItemCtx SPViewBox::get_rctx(const SPItemCtx* ictx) {

  /* Create copy of item context */
  SPItemCtx rctx = *ictx;

  /* Calculate child to parent transformation */
  /* Apply parent translation (set up as viewport) */
  this->c2p = Geom::Translate(rctx.viewport.min());

  if (this->viewBox_set) {
    // Adjusts c2p for viewbox
    apply_viewbox( rctx.viewport );
  }

  rctx.i2doc = this->c2p * rctx.i2doc;

  /* If viewBox is set initialize child viewport */
  /* Otherwise it is already correct */
  if (this->viewBox_set) {
    rctx.viewport = this->viewBox;
    rctx.i2vp = Geom::identity();
  }

  return rctx;
}

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
