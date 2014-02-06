/*
 * viewBox helper class, common code used by root, symbol, marker, pattern, image, view
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com> (code extracted from symbol.cpp)
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 2013 Tavmjong Bah, authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

#include <2geom/transforms.h>

#include "viewbox.h"
#include "attributes.h"
#include "enums.h"
#include "sp-item.h"

SPViewBox::SPViewBox() {

  this->viewBox_set  = FALSE;

  this->aspect_set   = FALSE;
  this->aspect_align = SP_ASPECT_XMID_YMID; // Default per spec;
  this->aspect_clip  = SP_ASPECT_MEET;

  this->c2p = Geom::identity();
}

SPViewBox::~SPViewBox() {
}

void SPViewBox::set_viewBox(const gchar* value) {

  if (value) {
    double x, y, width, height;
    char *eptr;

    eptr = (gchar *) value;
    x = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    y = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    width = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    height = g_ascii_strtod (eptr, &eptr);

    while (*eptr && ((*eptr == ',') || (*eptr == ' '))) {
      eptr++;
    }

    if ((width > 0) && (height > 0)) {
      /* Set viewbox */
      this->viewBox = Geom::Rect::from_xywh(x, y, width, height);
      this->viewBox_set = TRUE;
    } else {
      this->viewBox_set = FALSE;
    }
  } else {
    this->viewBox_set = FALSE;
  }

  // The C++ way?
  // std::string sv( value );
  // std::replace( sv.begin(), sv.end(), ',', ' ');
  // std::stringstream ss( sv );
  // double x, y, width, height;
  // ss >> x >> y >> width >> height;
}

void SPViewBox::set_preserveAspectRatio(const gchar* value) {

  /* Do setup before, so we can use break to escape */
  this->aspect_set = FALSE;
  this->aspect_align = SP_ASPECT_XMID_YMID; // Default per spec
  this->aspect_clip = SP_ASPECT_MEET;

  if (value) {
    int len;
    gchar c[256];
    const gchar *p, *e;
    unsigned int align, clip;
    p = value;

    while (*p && *p == 32) {
      p += 1;
    }

    if (!*p) {
      return;
    }

    e = p;

    while (*e && *e != 32) {
      e += 1;
    }

    len = e - p;

    if (len > 8) {
      return;
    }

    memcpy (c, value, len);

    c[len] = 0;

    /* Now the actual part */
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

    clip = SP_ASPECT_MEET;

    while (*e && *e == 32) {
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

    this->aspect_set = TRUE;
    this->aspect_align = align;
    this->aspect_clip = clip;
  }
}

// Apply scaling from viewbox
void SPViewBox::apply_viewbox(const Geom::Rect& in) {

    /* Determine actual viewbox in viewport coordinates */
    double x = 0.0;
    double y = 0.0;
    double width = in.width();
    double height = in.height();
    // std::cout << "  width: " << width << " height: " << height << std::endl;

    if (this->aspect_align != SP_ASPECT_NONE) {
      double scalex, scaley, scale;
      /* Things are getting interesting */
      scalex = in.width() / this->viewBox.width();
      scaley = in.height() / this->viewBox.height();
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
    q[0] = width / this->viewBox.width();
    q[1] = 0.0;
    q[2] = 0.0;
    q[3] = height / this->viewBox.height();
    q[4] = -this->viewBox.left() * q[0] + x;
    q[5] = -this->viewBox.top()  * q[3] + y;

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
