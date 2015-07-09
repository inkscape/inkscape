/*
 * Helper functions to use cairo with inkscape
 *
 * Copyright (C) 2007 bulia byak
 * Copyright (C) 2008 Johan Engelen
 *
 * Released under GNU GPL
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "display/cairo-utils.h"

#include <stdexcept>
#include <glib/gstdio.h>
#include <glibmm/fileutils.h>
#include <2geom/pathvector.h>
#include <2geom/curves.h>
#include <2geom/affine.h>
#include <2geom/point.h>
#include <2geom/path.h>
#include <2geom/transforms.h>
#include <2geom/sbasis-to-bezier.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "color.h"
#include "style.h"
#include "helper/geom-curves.h"
#include "display/cairo-templates.h"

/**
 * Key for cairo_surface_t to keep track of current color interpolation value
 * Only the address of the structure is used, it is never initialized. See:
 * http://www.cairographics.org/manual/cairo-Types.html#cairo-user-data-key-t
 */
cairo_user_data_key_t ink_color_interpolation_key;
cairo_user_data_key_t ink_pixbuf_key;

namespace Inkscape {

CairoGroup::CairoGroup(cairo_t *_ct) : ct(_ct), pushed(false) {}
CairoGroup::~CairoGroup() {
    if (pushed) {
        cairo_pattern_t *p = cairo_pop_group(ct);
        cairo_pattern_destroy(p);
    }
}
void CairoGroup::push() {
    cairo_push_group(ct);
    pushed = true;
}
void CairoGroup::push_with_content(cairo_content_t content) {
    cairo_push_group_with_content(ct, content);
    pushed = true;
}
cairo_pattern_t *CairoGroup::pop() {
    if (pushed) {
        cairo_pattern_t *ret = cairo_pop_group(ct);
        pushed = false;
        return ret;
    } else {
        throw std::logic_error("Cairo group popped without pushing it first");
    }
}
Cairo::RefPtr<Cairo::Pattern> CairoGroup::popmm() {
    if (pushed) {
        cairo_pattern_t *ret = cairo_pop_group(ct);
        Cairo::RefPtr<Cairo::Pattern> retmm(new Cairo::Pattern(ret, true));
        pushed = false;
        return retmm;
    } else {
        throw std::logic_error("Cairo group popped without pushing it first");
    }
}
void CairoGroup::pop_to_source() {
    if (pushed) {
        cairo_pop_group_to_source(ct);
        pushed = false;
    }
}

CairoContext::CairoContext(cairo_t *obj, bool ref)
    : Cairo::Context(obj, ref)
{}

void CairoContext::transform(Geom::Affine const &m)
{
    cairo_matrix_t cm;
    cm.xx = m[0];
    cm.xy = m[2];
    cm.x0 = m[4];
    cm.yx = m[1];
    cm.yy = m[3];
    cm.y0 = m[5];
    cairo_transform(cobj(), &cm);
}

void CairoContext::set_source_rgba32(guint32 color)
{
    double red = SP_RGBA32_R_F(color);
    double gre = SP_RGBA32_G_F(color);
    double blu = SP_RGBA32_B_F(color);
    double alp = SP_RGBA32_A_F(color);
    cairo_set_source_rgba(cobj(), red, gre, blu, alp);
}

void CairoContext::append_path(Geom::PathVector const &pv)
{
    feed_pathvector_to_cairo(cobj(), pv);
}

Cairo::RefPtr<CairoContext> CairoContext::create(Cairo::RefPtr<Cairo::Surface> const &target)
{
    cairo_t *ct = cairo_create(target->cobj());
    Cairo::RefPtr<CairoContext> ret(new CairoContext(ct, true));
    return ret;
}


/* The class below implement the following hack:
 * 
 * The pixels formats of Cairo and GdkPixbuf are different.
 * GdkPixbuf accesses pixels as bytes, alpha is not premultiplied,
 * and successive bytes of a single pixel contain R, G, B and A components.
 * Cairo accesses pixels as 32-bit ints, alpha is premultiplied,
 * and each int contains as 0xAARRGGBB, accessed with bitwise operations.
 *
 * In other words, on a little endian system, a GdkPixbuf will contain:
 *   char *data = "rgbargbargba...."
 *   int *data = { 0xAABBGGRR, 0xAABBGGRR, 0xAABBGGRR, ... }
 * while a Cairo image surface will contain:
 *   char *data = "bgrabgrabgra...."
 *   int *data = { 0xAARRGGBB, 0xAARRGGBB, 0xAARRGGBB, ... }
 *
 * It is possible to convert between these two formats (almost) losslessly.
 * Some color information from partially transparent regions of the image
 * is lost, but the result when displaying this image will remain the same.
 *
 * The class allows interoperation between GdkPixbuf
 * and Cairo surfaces without creating a copy of the image.
 * This is implemented by creating a GdkPixbuf and a Cairo image surface
 * which share their data. Depending on what is needed at a given time,
 * the pixels are converted in place to the Cairo or the GdkPixbuf format.
 */

/** Create a pixbuf from a Cairo surface.
 * The constructor takes ownership of the passed surface,
 * so it should not be destroyed. */
Pixbuf::Pixbuf(cairo_surface_t *s)
    : _pixbuf(gdk_pixbuf_new_from_data(
        cairo_image_surface_get_data(s), GDK_COLORSPACE_RGB, TRUE, 8,
        cairo_image_surface_get_width(s), cairo_image_surface_get_height(s),
        cairo_image_surface_get_stride(s), NULL, NULL))
    , _surface(s)
    , _mod_time(0)
    , _pixel_format(PF_CAIRO)
    , _cairo_store(true)
{}

/** Create a pixbuf from a GdkPixbuf.
 * The constructor takes ownership of the passed GdkPixbuf reference,
 * so it should not be unrefed. */
Pixbuf::Pixbuf(GdkPixbuf *pb)
    : _pixbuf(pb)
    , _surface(0)
    , _mod_time(0)
    , _pixel_format(PF_GDK)
    , _cairo_store(false)
{
    _forceAlpha();
    _surface = cairo_image_surface_create_for_data(
        gdk_pixbuf_get_pixels(_pixbuf), CAIRO_FORMAT_ARGB32,
        gdk_pixbuf_get_width(_pixbuf), gdk_pixbuf_get_height(_pixbuf), gdk_pixbuf_get_rowstride(_pixbuf));
}

Pixbuf::Pixbuf(Inkscape::Pixbuf const &other)
    : _pixbuf(gdk_pixbuf_copy(other._pixbuf))
    , _surface(cairo_image_surface_create_for_data(
        gdk_pixbuf_get_pixels(_pixbuf), CAIRO_FORMAT_ARGB32,
        gdk_pixbuf_get_width(_pixbuf), gdk_pixbuf_get_height(_pixbuf), gdk_pixbuf_get_rowstride(_pixbuf)))
    , _mod_time(other._mod_time)
    , _path(other._path)
    , _pixel_format(other._pixel_format)
    , _cairo_store(false)
{}

Pixbuf::~Pixbuf()
{
    if (_cairo_store) {
        g_object_unref(_pixbuf);
        cairo_surface_destroy(_surface);
    } else {
        cairo_surface_destroy(_surface);
        g_object_unref(_pixbuf);
    }
}

Pixbuf *Pixbuf::create_from_data_uri(gchar const *uri_data)
{
    Pixbuf *pixbuf = NULL;

    bool data_is_image = false;
    bool data_is_base64 = false;

    gchar const *data = uri_data;

    while (*data) {
        if (strncmp(data,"base64",6) == 0) {
            /* base64-encoding */
            data_is_base64 = true;
            data_is_image = true; // Illustrator produces embedded images without MIME type, so we assume it's image no matter what
            data += 6;
        }
        else if (strncmp(data,"image/png",9) == 0) {
            /* PNG image */
            data_is_image = true;
            data += 9;
        }
        else if (strncmp(data,"image/jpg",9) == 0) {
            /* JPEG image */
            data_is_image = true;
            data += 9;
        }
        else if (strncmp(data,"image/jpeg",10) == 0) {
            /* JPEG image */
            data_is_image = true;
            data += 10;
        }
        else if (strncmp(data,"image/jp2",9) == 0) {
            /* JPEG2000 image */
            data_is_image = true;
            data += 9;
        }
        else { /* unrecognized option; skip it */
            while (*data) {
                if (((*data) == ';') || ((*data) == ',')) {
                    break;
                }
                data++;
            }
        }
        if ((*data) == ';') {
            data++;
            continue;
        }
        if ((*data) == ',') {
            data++;
            break;
        }
    }

    if ((*data) && data_is_image && data_is_base64) {
        GdkPixbufLoader *loader = gdk_pixbuf_loader_new();

        if (!loader) return NULL;

        gsize decoded_len = 0;
        guchar *decoded = g_base64_decode(data, &decoded_len);

        if (gdk_pixbuf_loader_write(loader, decoded, decoded_len, NULL)) {
            gdk_pixbuf_loader_close(loader, NULL);
            GdkPixbuf *buf = gdk_pixbuf_loader_get_pixbuf(loader);
            if (buf) {
                g_object_ref(buf);
                pixbuf = new Pixbuf(buf);

                GdkPixbufFormat *fmt = gdk_pixbuf_loader_get_format(loader);
                gchar *fmt_name = gdk_pixbuf_format_get_name(fmt);
                pixbuf->_setMimeData(decoded, decoded_len, fmt_name);
                g_free(fmt_name);
            } else {
                g_free(decoded);
            }
        } else {
            g_free(decoded);
        }
        g_object_unref(loader);
    }

    return pixbuf;
}

Pixbuf *Pixbuf::create_from_file(std::string const &fn)
{
    Pixbuf *pb = NULL;
    // test correctness of filename
    if (!g_file_test(fn.c_str(), G_FILE_TEST_EXISTS)) { 
        return NULL;
    }
    struct stat stdir;
    int val = g_stat(fn.c_str(), &stdir);
    if (val == 0 && stdir.st_mode & S_IFDIR){
        return NULL;
    }

    // we need to load the entire file into memory,
    // since we'll store it as MIME data
    gchar *data = NULL;
    gsize len = 0;
    GError *error;

    if (g_file_get_contents(fn.c_str(), &data, &len, &error)) {

        GdkPixbufLoader *loader = gdk_pixbuf_loader_new();
        gdk_pixbuf_loader_write(loader, (guchar *) data, len, NULL);
        gdk_pixbuf_loader_close(loader, NULL);

        GdkPixbuf *buf = gdk_pixbuf_loader_get_pixbuf(loader);
        if (buf) {
            g_object_ref(buf);
            pb = new Pixbuf(buf);
            pb->_mod_time = stdir.st_mtime;
            pb->_path = fn;

            GdkPixbufFormat *fmt = gdk_pixbuf_loader_get_format(loader);
            gchar *fmt_name = gdk_pixbuf_format_get_name(fmt);
            pb->_setMimeData((guchar *) data, len, fmt_name);
            g_free(fmt_name);
        } else {
            g_free(data);
        }
        g_object_unref(loader);

        // TODO: we could also read DPI, ICC profile, gamma correction, and other information
        // from the file. This can be done by using format-specific libraries e.g. libpng.
    } else {
        return NULL;
    }

    return pb;
}

/**
 * Converts the pixbuf to GdkPixbuf pixel format.
 * The returned pixbuf can be used e.g. in calls to gdk_pixbuf_save().
 */
GdkPixbuf *Pixbuf::getPixbufRaw(bool convert_format)
{
    if (convert_format) {
        ensurePixelFormat(PF_GDK);
    }
    return _pixbuf;
}

/**
 * Converts the pixbuf to Cairo pixel format and returns an image surface
 * which can be used as a source.
 *
 * The returned surface is owned by the GdkPixbuf and should not be freed.
 * Calling this function causes the pixbuf to be unsuitable for use
 * with GTK drawing functions until ensurePixelFormat(Pixbuf::PIXEL_FORMAT_PIXBUF) is called.
 */
cairo_surface_t *Pixbuf::getSurfaceRaw(bool convert_format)
{
    if (convert_format) {
        ensurePixelFormat(PF_CAIRO);
    }
    return _surface;
}

/* Declaring this function in the header requires including <gdkmm/pixbuf.h>,
 * which stupidly includes <glibmm.h> which in turn pulls in <glibmm/threads.h>.
 * However, since glib 2.32, <glibmm/threads.h> has to be included before <glib.h>
 * when compiling with G_DISABLE_DEPRECATED, as we do in non-release builds.
 * This necessitates spamming a lot of files with #include <glibmm/threads.h>
 * at the top.
 *
 * Since we don't really use gdkmm, do not define this function for now. */

/*
Glib::RefPtr<Gdk::Pixbuf> Pixbuf::getPixbuf(bool convert_format = true)
{
    g_object_ref(_pixbuf);
    Glib::RefPtr<Gdk::Pixbuf> p(getPixbuf(convert_format));
    return p;
}
*/

Cairo::RefPtr<Cairo::Surface> Pixbuf::getSurface(bool convert_format)
{
    Cairo::RefPtr<Cairo::Surface> p(new Cairo::Surface(getSurfaceRaw(convert_format), false));
    return p;
}

/** Retrieves the original compressed data for the surface, if any.
 * The returned data belongs to the object and should not be freed. */
guchar const *Pixbuf::getMimeData(gsize &len, std::string &mimetype) const
{
    static gchar const *mimetypes[] = {
        CAIRO_MIME_TYPE_JPEG, CAIRO_MIME_TYPE_JP2, CAIRO_MIME_TYPE_PNG, NULL };
    static guint mimetypes_len = g_strv_length(const_cast<gchar**>(mimetypes));

    guchar const *data = NULL;

    for (guint i = 0; i < mimetypes_len; ++i) {
        unsigned long len_long = 0;
        cairo_surface_get_mime_data(const_cast<cairo_surface_t*>(_surface), mimetypes[i], &data, &len_long);
        if (data != NULL) {
			len = len_long;
            mimetype = mimetypes[i];
            break;
        }
    }

    return data;
}

int Pixbuf::width() const {
    return gdk_pixbuf_get_width(const_cast<GdkPixbuf*>(_pixbuf));
}
int Pixbuf::height() const {
    return gdk_pixbuf_get_height(const_cast<GdkPixbuf*>(_pixbuf));
}
int Pixbuf::rowstride() const {
    return gdk_pixbuf_get_rowstride(const_cast<GdkPixbuf*>(_pixbuf));
}
guchar const *Pixbuf::pixels() const {
    return gdk_pixbuf_get_pixels(const_cast<GdkPixbuf*>(_pixbuf));
}
guchar *Pixbuf::pixels() {
    return gdk_pixbuf_get_pixels(_pixbuf);
}
void Pixbuf::markDirty() {
    cairo_surface_mark_dirty(_surface);
}

void Pixbuf::_forceAlpha()
{
    if (gdk_pixbuf_get_has_alpha(_pixbuf)) return;

    GdkPixbuf *old = _pixbuf;
    _pixbuf = gdk_pixbuf_add_alpha(old, FALSE, 0, 0, 0);
    g_object_unref(old);
}

void Pixbuf::_setMimeData(guchar *data, gsize len, Glib::ustring const &format)
{
    gchar const *mimetype = NULL;

    if (format == "jpeg") {
        mimetype = CAIRO_MIME_TYPE_JPEG;
    } else if (format == "jpeg2000") {
        mimetype = CAIRO_MIME_TYPE_JP2;
    } else if (format == "png") {
        mimetype = CAIRO_MIME_TYPE_PNG;
    }

    if (mimetype != NULL) {
        cairo_surface_set_mime_data(_surface, mimetype, data, len, g_free, data);
        //g_message("Setting Cairo MIME data: %s", mimetype);
    } else {
        g_free(data);
        //g_message("Not setting Cairo MIME data: unknown format %s", name.c_str());
    }
}

void Pixbuf::ensurePixelFormat(PixelFormat fmt)
{
    if (_pixel_format == PF_GDK) {
        if (fmt == PF_GDK) {
            return;
        }
        if (fmt == PF_CAIRO) {
            convert_pixels_pixbuf_to_argb32(
                gdk_pixbuf_get_pixels(_pixbuf),
                gdk_pixbuf_get_width(_pixbuf),
                gdk_pixbuf_get_height(_pixbuf),
                gdk_pixbuf_get_rowstride(_pixbuf));
            _pixel_format = fmt;
            return;
        }
        g_assert_not_reached();
    }
    if (_pixel_format == PF_CAIRO) {
        if (fmt == PF_GDK) {
            convert_pixels_argb32_to_pixbuf(
                gdk_pixbuf_get_pixels(_pixbuf),
                gdk_pixbuf_get_width(_pixbuf),
                gdk_pixbuf_get_height(_pixbuf),
                gdk_pixbuf_get_rowstride(_pixbuf));
            _pixel_format = fmt;
            return;
        }
        if (fmt == PF_CAIRO) {
            return;
        }
        g_assert_not_reached();
    }
    g_assert_not_reached();
}

} // namespace Inkscape

/*
 * Can be called recursively.
 * If optimize_stroke == false, the view Rect is not used.
 */
static void
feed_curve_to_cairo(cairo_t *cr, Geom::Curve const &c, Geom::Affine const & trans, Geom::Rect view, bool optimize_stroke)
{
    using Geom::X;
    using Geom::Y;

    unsigned order = 0;
    if (Geom::BezierCurve const* b = dynamic_cast<Geom::BezierCurve const*>(&c)) {
        order = b->order();
    }

    // handle the three typical curve cases
    switch (order) {
    case 1:
    {
        Geom::Point end_tr = c.finalPoint() * trans;
        if (!optimize_stroke) {
            cairo_line_to(cr, end_tr[0], end_tr[1]);
        } else {
            Geom::Rect swept(c.initialPoint()*trans, end_tr);
            if (swept.intersects(view)) {
                cairo_line_to(cr, end_tr[0], end_tr[1]);
            } else {
                cairo_move_to(cr, end_tr[0], end_tr[1]);
            }
        }
    }
    break;
    case 2:
    {
        Geom::QuadraticBezier const *quadratic_bezier = static_cast<Geom::QuadraticBezier const*>(&c);
        std::vector<Geom::Point> points = quadratic_bezier->controlPoints();
        points[0] *= trans;
        points[1] *= trans;
        points[2] *= trans;
        // degree-elevate to cubic Bezier, since Cairo doesn't do quadratic Beziers
        Geom::Point b1 = points[0] + (2./3) * (points[1] - points[0]);
        Geom::Point b2 = b1 + (1./3) * (points[2] - points[0]);
        if (!optimize_stroke) {
            cairo_curve_to(cr, b1[X], b1[Y], b2[X], b2[Y], points[2][X], points[2][Y]);
        } else {
            Geom::Rect swept(points[0], points[2]);
            swept.expandTo(points[1]);
            if (swept.intersects(view)) {
                cairo_curve_to(cr, b1[X], b1[Y], b2[X], b2[Y], points[2][X], points[2][Y]);
            } else {
                cairo_move_to(cr, points[2][X], points[2][Y]);
            }
        }
    }
    break;
    case 3:
    {
        Geom::CubicBezier const *cubic_bezier = static_cast<Geom::CubicBezier const*>(&c);
        std::vector<Geom::Point> points = cubic_bezier->controlPoints();
        //points[0] *= trans; // don't do this one here for fun: it is only needed for optimized strokes
        points[1] *= trans;
        points[2] *= trans;
        points[3] *= trans;
        if (!optimize_stroke) {
            cairo_curve_to(cr, points[1][X], points[1][Y], points[2][X], points[2][Y], points[3][X], points[3][Y]);
        } else {
            points[0] *= trans;  // didn't transform this point yet
            Geom::Rect swept(points[0], points[3]);
            swept.expandTo(points[1]);
            swept.expandTo(points[2]);
            if (swept.intersects(view)) {
                cairo_curve_to(cr, points[1][X], points[1][Y], points[2][X], points[2][Y], points[3][X], points[3][Y]);
            } else {
                cairo_move_to(cr, points[3][X], points[3][Y]);
            }
        }
    }
    break;
    default:
    {
        if (Geom::EllipticalArc const *a = dynamic_cast<Geom::EllipticalArc const*>(&c)) {
            //if (!optimize_stroke || a->boundsFast().intersects(view)) {
                Geom::Affine xform = a->unitCircleTransform() * trans;
                Geom::Point ang(a->initialAngle().radians(), a->finalAngle().radians());

                // Apply the transformation to the current context
                cairo_matrix_t cm;
                cm.xx = xform[0];
                cm.xy = xform[2];
                cm.x0 = xform[4];
                cm.yx = xform[1];
                cm.yy = xform[3];
                cm.y0 = xform[5];

                cairo_save(cr);
                cairo_transform(cr, &cm);

                // Draw the circle
                if (a->sweep()) {
                    cairo_arc(cr, 0, 0, 1, ang[0], ang[1]);
                } else {
                    cairo_arc_negative(cr, 0, 0, 1, ang[0], ang[1]);
                }
                // Revert the current context
                cairo_restore(cr);
            //} else {
            //    Geom::Point f = a->finalPoint() * trans;
            //    cairo_move_to(cr, f[X], f[Y]);
            //}
        } else {
            // handles sbasis as well as all other curve types
            // this is very slow
            Geom::Path sbasis_path = Geom::cubicbezierpath_from_sbasis(c.toSBasis(), 0.1);

            // recurse to convert the new path resulting from the sbasis to svgd
            for (Geom::Path::iterator iter = sbasis_path.begin(); iter != sbasis_path.end(); ++iter) {
                feed_curve_to_cairo(cr, *iter, trans, view, optimize_stroke);
            }
        }
    }
    break;
    }
}


/** Feeds path-creating calls to the cairo context translating them from the Path */
static void
feed_path_to_cairo (cairo_t *ct, Geom::Path const &path)
{
    if (path.empty())
        return;

    cairo_move_to(ct, path.initialPoint()[0], path.initialPoint()[1] );

    for (Geom::Path::const_iterator cit = path.begin(); cit != path.end_open(); ++cit) {
        feed_curve_to_cairo(ct, *cit, Geom::identity(), Geom::Rect(), false); // optimize_stroke is false, so the view rect is not used
    }

    if (path.closed()) {
        cairo_close_path(ct);
    }
}

/** Feeds path-creating calls to the cairo context translating them from the Path, with the given transform and shift */
static void
feed_path_to_cairo (cairo_t *ct, Geom::Path const &path, Geom::Affine trans, Geom::OptRect area, bool optimize_stroke, double stroke_width)
{
    if (!area)
        return;
    if (path.empty())
        return;

    // Transform all coordinates to coords within "area"
    Geom::Point shift = area->min();
    Geom::Rect view = *area;
    view.expandBy (stroke_width);
    view = view * (Geom::Affine)Geom::Translate(-shift);
    //  Pass transformation to feed_curve, so that we don't need to create a whole new path.
    Geom::Affine transshift(trans * Geom::Translate(-shift));

    Geom::Point initial = path.initialPoint() * transshift;
    cairo_move_to(ct, initial[0], initial[1] );

    for(Geom::Path::const_iterator cit = path.begin(); cit != path.end_open(); ++cit) {
        feed_curve_to_cairo(ct, *cit, transshift, view, optimize_stroke);
    }

    if (path.closed()) {
        if (!optimize_stroke) {
            cairo_close_path(ct);
        } else {
            cairo_line_to(ct, initial[0], initial[1]);
            /* We cannot use cairo_close_path(ct) here because some parts of the path may have been
               clipped and not drawn (maybe the before last segment was outside view area), which 
               would result in closing the "subpath" after the last interruption, not the entire path.

               However, according to cairo documentation:
               The behavior of cairo_close_path() is distinct from simply calling cairo_line_to() with the equivalent coordinate
               in the case of stroking. When a closed sub-path is stroked, there are no caps on the ends of the sub-path. Instead,
               there is a line join connecting the final and initial segments of the sub-path. 

               The correct fix will be possible when cairo introduces methods for moving without
               ending/starting subpaths, which we will use for skipping invisible segments; then we
               will be able to use cairo_close_path here. This issue also affects ps/eps/pdf export,
               see bug 168129
            */
        }
    }
}

/** Feeds path-creating calls to the cairo context translating them from the PathVector, with the given transform and shift
 *  One must have done cairo_new_path(ct); before calling this function. */
void
feed_pathvector_to_cairo (cairo_t *ct, Geom::PathVector const &pathv, Geom::Affine trans, Geom::OptRect area, bool optimize_stroke, double stroke_width)
{
    if (!area)
        return;
    if (pathv.empty())
        return;

    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {
        feed_path_to_cairo(ct, *it, trans, area, optimize_stroke, stroke_width);
    }
}

/** Feeds path-creating calls to the cairo context translating them from the PathVector
 *  One must have done cairo_new_path(ct); before calling this function. */
void
feed_pathvector_to_cairo (cairo_t *ct, Geom::PathVector const &pathv)
{
    if (pathv.empty())
        return;

    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {
        feed_path_to_cairo(ct, *it);
    }
}

SPColorInterpolation
get_cairo_surface_ci(cairo_surface_t *surface) {
    void* data = cairo_surface_get_user_data( surface, &ink_color_interpolation_key );
    if( data != NULL ) {
        return (SPColorInterpolation)GPOINTER_TO_INT( data );
    } else {
        return SP_CSS_COLOR_INTERPOLATION_AUTO;
    }
}

/** Set the color_interpolation_value for a Cairo surface.
 *  Transform the surface between sRGB and linearRGB if necessary. */
void
set_cairo_surface_ci(cairo_surface_t *surface, SPColorInterpolation ci) {

    if( cairo_surface_get_content( surface ) != CAIRO_CONTENT_ALPHA ) {

        SPColorInterpolation ci_in = get_cairo_surface_ci( surface );

        if( ci_in == SP_CSS_COLOR_INTERPOLATION_SRGB &&
            ci    == SP_CSS_COLOR_INTERPOLATION_LINEARRGB ) {
            ink_cairo_surface_srgb_to_linear( surface );
        }
        if( ci_in == SP_CSS_COLOR_INTERPOLATION_LINEARRGB &&
            ci    == SP_CSS_COLOR_INTERPOLATION_SRGB ) {
            ink_cairo_surface_linear_to_srgb( surface );
        }

        cairo_surface_set_user_data(surface, &ink_color_interpolation_key, GINT_TO_POINTER (ci), NULL);
    }
}

void
copy_cairo_surface_ci(cairo_surface_t *in, cairo_surface_t *out) {
    cairo_surface_set_user_data(out, &ink_color_interpolation_key, cairo_surface_get_user_data(in, &ink_color_interpolation_key), NULL);
}

void
ink_cairo_set_source_rgba32(cairo_t *ct, guint32 rgba)
{
    cairo_set_source_rgba(ct, SP_RGBA32_R_F(rgba), SP_RGBA32_G_F(rgba), SP_RGBA32_B_F(rgba), SP_RGBA32_A_F(rgba));
}

void
ink_cairo_set_source_color(cairo_t *ct, SPColor const &c, double opacity)
{
    cairo_set_source_rgba(ct, c.v.c[0], c.v.c[1], c.v.c[2], opacity);
}

void ink_matrix_to_2geom(Geom::Affine &m, cairo_matrix_t const &cm)
{
    m[0] = cm.xx;
    m[2] = cm.xy;
    m[4] = cm.x0;
    m[1] = cm.yx;
    m[3] = cm.yy;
    m[5] = cm.y0;
}

void ink_matrix_to_cairo(cairo_matrix_t &cm, Geom::Affine const &m)
{
    cm.xx = m[0];
    cm.xy = m[2];
    cm.x0 = m[4];
    cm.yx = m[1];
    cm.yy = m[3];
    cm.y0 = m[5];
}

void
ink_cairo_transform(cairo_t *ct, Geom::Affine const &m)
{
    cairo_matrix_t cm;
    ink_matrix_to_cairo(cm, m);
    cairo_transform(ct, &cm);
}

void
ink_cairo_pattern_set_matrix(cairo_pattern_t *cp, Geom::Affine const &m)
{
    cairo_matrix_t cm;
    ink_matrix_to_cairo(cm, m);
    cairo_pattern_set_matrix(cp, &cm);
}

/**
 * Create an exact copy of a surface.
 * Creates a surface that has the same type, content type, dimensions and contents
 * as the specified surface.
 */
cairo_surface_t *
ink_cairo_surface_copy(cairo_surface_t *s)
{
    cairo_surface_t *ns = ink_cairo_surface_create_identical(s);

    if (cairo_surface_get_type(s) == CAIRO_SURFACE_TYPE_IMAGE) {
        // use memory copy instead of using a Cairo context
        cairo_surface_flush(s);
        int stride = cairo_image_surface_get_stride(s);
        int h = cairo_image_surface_get_height(s);
        memcpy(cairo_image_surface_get_data(ns), cairo_image_surface_get_data(s), stride * h);
        cairo_surface_mark_dirty(ns);
    } else {
        // generic implementation
        cairo_t *ct = cairo_create(ns);
        cairo_set_source_surface(ct, s, 0, 0);
        cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
        cairo_paint(ct);
        cairo_destroy(ct);
    }

    return ns;
}

/**
 * Create a surface that differs only in pixel content.
 * Creates a surface that has the same type, content type and dimensions
 * as the specified surface. Pixel contents are not copied.
 */
cairo_surface_t *
ink_cairo_surface_create_identical(cairo_surface_t *s)
{
    cairo_surface_t *ns = ink_cairo_surface_create_same_size(s, cairo_surface_get_content(s));
    cairo_surface_set_user_data(ns, &ink_color_interpolation_key, cairo_surface_get_user_data(s, &ink_color_interpolation_key), NULL);
    return ns;
}

cairo_surface_t *
ink_cairo_surface_create_same_size(cairo_surface_t *s, cairo_content_t c)
{
    cairo_surface_t *ns = cairo_surface_create_similar(s, c,
        ink_cairo_surface_get_width(s), ink_cairo_surface_get_height(s));
    return ns;
}

/**
 * Extract the alpha channel into a new surface.
 * Creates a surface with a content type of CAIRO_CONTENT_ALPHA that contains
 * the alpha values of pixels from @a s.
 */
cairo_surface_t *
ink_cairo_extract_alpha(cairo_surface_t *s)
{
    cairo_surface_t *alpha = ink_cairo_surface_create_same_size(s, CAIRO_CONTENT_ALPHA);

    cairo_t *ct = cairo_create(alpha);
    cairo_set_source_surface(ct, s, 0, 0);
    cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
    cairo_paint(ct);
    cairo_destroy(ct);

    return alpha;
}

cairo_surface_t *
ink_cairo_surface_create_output(cairo_surface_t *image, cairo_surface_t *bg)
{
    cairo_content_t imgt = cairo_surface_get_content(image);
    cairo_content_t bgt = cairo_surface_get_content(bg);
    cairo_surface_t *out = NULL;

    if (bgt == CAIRO_CONTENT_ALPHA && imgt == CAIRO_CONTENT_ALPHA) {
        out = ink_cairo_surface_create_identical(bg);
    } else {
        out = ink_cairo_surface_create_same_size(bg, CAIRO_CONTENT_COLOR_ALPHA);
    }

    return out;
}

void
ink_cairo_surface_blit(cairo_surface_t *src, cairo_surface_t *dest)
{
    if (cairo_surface_get_type(src) == CAIRO_SURFACE_TYPE_IMAGE &&
        cairo_surface_get_type(dest) == CAIRO_SURFACE_TYPE_IMAGE &&
        cairo_image_surface_get_format(src) == cairo_image_surface_get_format(dest) &&
        cairo_image_surface_get_height(src) == cairo_image_surface_get_height(dest) &&
        cairo_image_surface_get_width(src) == cairo_image_surface_get_width(dest) &&
        cairo_image_surface_get_stride(src) == cairo_image_surface_get_stride(dest))
    {
        // use memory copy instead of using a Cairo context
        cairo_surface_flush(src);
        int stride = cairo_image_surface_get_stride(src);
        int h = cairo_image_surface_get_height(src);
        memcpy(cairo_image_surface_get_data(dest), cairo_image_surface_get_data(src), stride * h);
        cairo_surface_mark_dirty(dest);
    } else {
        // generic implementation
        cairo_t *ct = cairo_create(dest);
        cairo_set_source_surface(ct, src, 0, 0);
        cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
        cairo_paint(ct);
        cairo_destroy(ct);
    }
}

int
ink_cairo_surface_get_width(cairo_surface_t *surface)
{
    // For now only image surface is handled.
    // Later add others, e.g. cairo-gl
    assert(cairo_surface_get_type(surface) == CAIRO_SURFACE_TYPE_IMAGE);
    return cairo_image_surface_get_width(surface);
}
int
ink_cairo_surface_get_height(cairo_surface_t *surface)
{
    assert(cairo_surface_get_type(surface) == CAIRO_SURFACE_TYPE_IMAGE);
    return cairo_image_surface_get_height(surface);
}

static int ink_cairo_surface_average_color_internal(cairo_surface_t *surface, double &rf, double &gf, double &bf, double &af)
{
    rf = gf = bf = af = 0.0;
    cairo_surface_flush(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    int stride = cairo_image_surface_get_stride(surface);
    unsigned char *data = cairo_image_surface_get_data(surface);

    /* TODO convert this to OpenMP somehow */
    for (int y = 0; y < height; ++y, data += stride) {
        for (int x = 0; x < width; ++x) {
            guint32 px = *reinterpret_cast<guint32*>(data + 4*x);
            EXTRACT_ARGB32(px, a,r,g,b)
            rf += r / 255.0;
            gf += g / 255.0;
            bf += b / 255.0;
            af += a / 255.0;
        }
    }
    return width * height;
}

guint32 ink_cairo_surface_average_color(cairo_surface_t *surface)
{
    double rf,gf,bf,af;
    ink_cairo_surface_average_color_premul(surface, rf,gf,bf,af);
    guint32 r = round(rf * 255);
    guint32 g = round(gf * 255);
    guint32 b = round(bf * 255);
    guint32 a = round(af * 255);
    ASSEMBLE_ARGB32(px, a,r,g,b);
    return px;
}

void ink_cairo_surface_average_color(cairo_surface_t *surface, double &r, double &g, double &b, double &a)
{
    int count = ink_cairo_surface_average_color_internal(surface, r,g,b,a);

    r /= a;
    g /= a;
    b /= a;
    a /= count;

    r = CLAMP(r, 0.0, 1.0);
    g = CLAMP(g, 0.0, 1.0);
    b = CLAMP(b, 0.0, 1.0);
    a = CLAMP(a, 0.0, 1.0);
}

void ink_cairo_surface_average_color_premul(cairo_surface_t *surface, double &r, double &g, double &b, double &a)
{
    int count = ink_cairo_surface_average_color_internal(surface, r,g,b,a);

    r /= count;
    g /= count;
    b /= count;
    a /= count;

    r = CLAMP(r, 0.0, 1.0);
    g = CLAMP(g, 0.0, 1.0);
    b = CLAMP(b, 0.0, 1.0);
    a = CLAMP(a, 0.0, 1.0);
}

static guint32 srgb_to_linear( const guint32 c, const guint32 a ) {

    const guint32 c1 = unpremul_alpha( c, a );

    double cc = c1/255.0;

    if( cc < 0.04045 ) {
        cc /= 12.92;
    } else {
        cc = pow( (cc+0.055)/1.055, 2.4 );
    }
    cc *= 255.0;

    const guint32 c2 = (int)cc;

    return premul_alpha( c2, a );
}

static guint32 linear_to_srgb( const guint32 c, const guint32 a ) {

    const guint32 c1 = unpremul_alpha( c, a );

    double cc = c1/255.0;

    if( cc < 0.0031308 ) {
        cc *= 12.92;
    } else {
        cc = pow( cc, 1.0/2.4 )*1.055-0.055;
    }
    cc *= 255.0;

    const guint32 c2 = (int)cc;

    return premul_alpha( c2, a );
}

struct SurfaceSrgbToLinear {

    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a,r,g,b)    ; // Unneeded semi-colon for indenting
        if( a != 0 ) {
            r = srgb_to_linear( r, a );
            g = srgb_to_linear( g, a );
            b = srgb_to_linear( b, a );
        }
        ASSEMBLE_ARGB32(out, a,r,g,b);
        return out;
    }
private:
    /* None */
};

int ink_cairo_surface_srgb_to_linear(cairo_surface_t *surface)
{
    cairo_surface_flush(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    // int stride = cairo_image_surface_get_stride(surface);
    // unsigned char *data = cairo_image_surface_get_data(surface);

    ink_cairo_surface_filter( surface, surface, SurfaceSrgbToLinear() );

    /* TODO convert this to OpenMP somehow */
    // for (int y = 0; y < height; ++y, data += stride) {
    //     for (int x = 0; x < width; ++x) {
    //         guint32 px = *reinterpret_cast<guint32*>(data + 4*x);
    //         EXTRACT_ARGB32(px, a,r,g,b)    ; // Unneeded semi-colon for indenting
    //         if( a != 0 ) {
    //             r = srgb_to_linear( r, a );
    //             g = srgb_to_linear( g, a );
    //             b = srgb_to_linear( b, a );
    //         }
    //         ASSEMBLE_ARGB32(px2, a,r,g,b);
    //         *reinterpret_cast<guint32*>(data + 4*x) = px2;
    //     }
    // }
    return width * height;
}

struct SurfaceLinearToSrgb {

    guint32 operator()(guint32 in) {
        EXTRACT_ARGB32(in, a,r,g,b)    ; // Unneeded semi-colon for indenting
        if( a != 0 ) {
            r = linear_to_srgb( r, a );
            g = linear_to_srgb( g, a );
            b = linear_to_srgb( b, a );
        }
        ASSEMBLE_ARGB32(out, a,r,g,b);
        return out;
    }
private:
    /* None */
};

int ink_cairo_surface_linear_to_srgb(cairo_surface_t *surface)
{
    cairo_surface_flush(surface);
    int width = cairo_image_surface_get_width(surface);
    int height = cairo_image_surface_get_height(surface);
    // int stride = cairo_image_surface_get_stride(surface);
    // unsigned char *data = cairo_image_surface_get_data(surface);

    ink_cairo_surface_filter( surface, surface, SurfaceLinearToSrgb() );

    // /* TODO convert this to OpenMP somehow */
    // for (int y = 0; y < height; ++y, data += stride) {
    //     for (int x = 0; x < width; ++x) {
    //         guint32 px = *reinterpret_cast<guint32*>(data + 4*x);
    //         EXTRACT_ARGB32(px, a,r,g,b)    ; // Unneeded semi-colon for indenting
    //         if( a != 0 ) {
    //             r = linear_to_srgb( r, a );
    //             g = linear_to_srgb( g, a );
    //             b = linear_to_srgb( b, a );
    //         }
    //         ASSEMBLE_ARGB32(px2, a,r,g,b);
    //         *reinterpret_cast<guint32*>(data + 4*x) = px2;
    //     }
    // }
    return width * height;
}

cairo_pattern_t *
ink_cairo_pattern_create_checkerboard()
{
    int const w = 6;
    int const h = 6;

    cairo_surface_t *s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2*w, 2*h);

    cairo_t *ct = cairo_create(s);
    cairo_set_operator(ct, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(ct, 0.75, 0.75, 0.75);
    cairo_paint(ct);
    cairo_set_source_rgb(ct, 0.5, 0.5, 0.5);
    cairo_rectangle(ct, 0, 0, w, h);
    cairo_rectangle(ct, w, h, w, h);
    cairo_fill(ct);
    cairo_destroy(ct);

    cairo_pattern_t *p = cairo_pattern_create_for_surface(s);
    cairo_pattern_set_extend(p, CAIRO_EXTEND_REPEAT);
    cairo_pattern_set_filter(p, CAIRO_FILTER_NEAREST);

    cairo_surface_destroy(s);
    return p;
}

/**
 * Converts the Cairo surface to a GdkPixbuf pixel format,
 * without allocating extra memory.
 *
 * This function is intended mainly for creating previews displayed by GTK.
 * For loading images for display on the canvas, use the Inkscape::Pixbuf object.
 *
 * The returned GdkPixbuf takes ownership of the passed surface reference,
 * so it should NOT be freed after calling this function.
 */
GdkPixbuf *ink_pixbuf_create_from_cairo_surface(cairo_surface_t *s)
{
    guchar *pixels = cairo_image_surface_get_data(s);
    int w = cairo_image_surface_get_width(s);
    int h = cairo_image_surface_get_height(s);
    int rs = cairo_image_surface_get_stride(s);

    convert_pixels_argb32_to_pixbuf(pixels, w, h, rs);

    GdkPixbuf *pb = gdk_pixbuf_new_from_data(
        pixels, GDK_COLORSPACE_RGB, TRUE, 8,
        w, h, rs, ink_cairo_pixbuf_cleanup, s);

    return pb;
}

/**
 * Cleanup function for GdkPixbuf.
 * This function should be passed as the GdkPixbufDestroyNotify parameter
 * to gdk_pixbuf_new_from_data when creating a GdkPixbuf backed by
 * a Cairo surface.
 */
void ink_cairo_pixbuf_cleanup(guchar * /*pixels*/, void *data)
{
    cairo_surface_t *surface = static_cast<cairo_surface_t*>(data);
    cairo_surface_destroy(surface);
}

/* The following two functions use "from" instead of "to", because when you write:
   val1 = argb32_from_pixbuf(val1);
   the name of the format is closer to the value in that format. */

guint32 argb32_from_pixbuf(guint32 c)
{
    guint32 o = 0;
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    guint32 a = (c & 0xff000000) >> 24;
#else
    guint32 a = (c & 0x000000ff);
#endif
    if (a != 0) {
        // extract color components
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
        guint32 r = (c & 0x000000ff);
        guint32 g = (c & 0x0000ff00) >> 8;
        guint32 b = (c & 0x00ff0000) >> 16;
#else
        guint32 r = (c & 0xff000000) >> 24;
        guint32 g = (c & 0x00ff0000) >> 16;
        guint32 b = (c & 0x0000ff00) >> 8;
#endif
        // premultiply
        r = premul_alpha(r, a);
        b = premul_alpha(b, a);
        g = premul_alpha(g, a);
        // combine into output
        o = (a << 24) | (r << 16) | (g << 8) | (b);
    }
    return o;
}

guint32 pixbuf_from_argb32(guint32 c)
{
    guint32 a = (c & 0xff000000) >> 24;
    if (a == 0) return 0;

    // extract color components
    guint32 r = (c & 0x00ff0000) >> 16;
    guint32 g = (c & 0x0000ff00) >> 8;
    guint32 b = (c & 0x000000ff);
    // unpremultiply; adding a/2 gives correct rounding
    // (taken from Cairo sources)
    r = (r * 255 + a/2) / a;
    b = (b * 255 + a/2) / a;
    g = (g * 255 + a/2) / a;
    // combine into output
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
    guint32 o = (r) | (g << 8) | (b << 16) | (a << 24);
#else
    guint32 o = (r << 24) | (g << 16) | (b << 8) | (a);
#endif
    return o;
}

/**
 * Convert pixel data from GdkPixbuf format to ARGB.
 * This will convert pixel data from GdkPixbuf format to Cairo's native pixel format.
 * This involves premultiplying alpha and shuffling around the channels.
 * Pixbuf data must have an alpha channel, otherwise the results are undefined
 * (usually a segfault).
 */
void
convert_pixels_pixbuf_to_argb32(guchar *data, int w, int h, int stride)
{
    for (int i = 0; i < h; ++i) {
        guint32 *px = reinterpret_cast<guint32*>(data + i*stride);
        for (int j = 0; j < w; ++j) {
            *px = argb32_from_pixbuf(*px);
            ++px;
        }
    }
}

/**
 * Convert pixel data from ARGB to GdkPixbuf format.
 * This will convert pixel data from GdkPixbuf format to Cairo's native pixel format.
 * This involves premultiplying alpha and shuffling around the channels.
 */
void
convert_pixels_argb32_to_pixbuf(guchar *data, int w, int h, int stride)
{
    for (int i = 0; i < h; ++i) {
        guint32 *px = reinterpret_cast<guint32*>(data + i*stride);
        for (int j = 0; j < w; ++j) {
            *px = pixbuf_from_argb32(*px);
            ++px;
        }
    }
}

/**
 * Converts GdkPixbuf's data to premultiplied ARGB.
 * This function will convert a GdkPixbuf in place into Cairo's native pixel format.
 * Note that this is a hack intended to save memory. When the pixbuf is in Cairo's format,
 * using it with GTK will result in corrupted drawings.
 */
void
ink_pixbuf_ensure_argb32(GdkPixbuf *pb)
{
    gchar *pixel_format = reinterpret_cast<gchar*>(g_object_get_data(G_OBJECT(pb), "pixel_format"));
    if (pixel_format != NULL && strcmp(pixel_format, "argb32") == 0) {
        // nothing to do
        return;
    }

    convert_pixels_pixbuf_to_argb32(
        gdk_pixbuf_get_pixels(pb),
        gdk_pixbuf_get_width(pb),
        gdk_pixbuf_get_height(pb),
        gdk_pixbuf_get_rowstride(pb));
    g_object_set_data_full(G_OBJECT(pb), "pixel_format", g_strdup("argb32"), g_free);
}

/**
 * Converts GdkPixbuf's data back to its native format.
 * Once this is done, the pixbuf can be used with GTK again.
 */
void
ink_pixbuf_ensure_normal(GdkPixbuf *pb)
{
    gchar *pixel_format = reinterpret_cast<gchar*>(g_object_get_data(G_OBJECT(pb), "pixel_format"));
    if (pixel_format == NULL || strcmp(pixel_format, "pixbuf") == 0) {
        // nothing to do
        return;
    }

    convert_pixels_argb32_to_pixbuf(
        gdk_pixbuf_get_pixels(pb),
        gdk_pixbuf_get_width(pb),
        gdk_pixbuf_get_height(pb),
        gdk_pixbuf_get_rowstride(pb));
    g_object_set_data_full(G_OBJECT(pb), "pixel_format", g_strdup("pixbuf"), g_free);
}

guint32 argb32_from_rgba(guint32 in)
{
    guint32 r, g, b, a;
    a = (in & 0x000000ff);
    r = premul_alpha((in & 0xff000000) >> 24, a);
    g = premul_alpha((in & 0x00ff0000) >> 16, a);
    b = premul_alpha((in & 0x0000ff00) >> 8,  a);
    ASSEMBLE_ARGB32(px, a, r, g, b)
    return px;
}

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
