/*
 * A simple utility for exporting Inkscape svg Shapes as JavaFX paths.
 *
 *  For information on the JavaFX file format, see:
 *      https://openjfx.dev.java.net/
 *
 * Authors:
 *   Bob Jamison <ishmal@inkscape.org>
 *   Silveira Neto <silveiraneto@gmail.com>
 *   Jim Clarke <Jim.Clarke@sun.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 2008,2009 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glibmm/miscutils.h>
#include "javafx-out.h"
#include <inkscape.h>
#include <inkscape-version.h>
#include <sp-path.h>
#include <sp-linear-gradient.h>
#include <sp-radial-gradient.h>
#include <style.h>
#include <display/curve.h>
#include <display/canvas-bpath.h>
#include <svg/svg.h>
#include <extension/system.h>
#include <2geom/pathvector.h>
#include <2geom/rect.h>
#include <2geom/curves.h>
#include "helper/geom.h"
#include "helper/geom-curves.h"
#include <io/sys.h>
#include "sp-root.h"

#include <string>
#include <stdio.h>
#include <stdarg.h>
#include "extension/extension.h"


namespace Inkscape
{
namespace Extension
{
namespace Internal
{




//########################################################################
//# M E S S A G E S
//########################################################################

static void err(const char *fmt, ...)
{
    va_list args;
    g_log(NULL,  G_LOG_LEVEL_WARNING, "javafx-out err: ");
    va_start(args, fmt);
    g_logv(NULL, G_LOG_LEVEL_WARNING, fmt, args);
    va_end(args);
    g_log(NULL,  G_LOG_LEVEL_WARNING, "\n");
}


//########################################################################
//# U T I L I T Y
//########################################################################

/**
 * Got this method from Bulia, and modified it a bit.  It basically
 * starts with this style, gets its SPObject parent, walks up the object
 * tree and finds all of the opacities and multiplies them.
 *
 * We use this for our "flat" object output.  If the code is modified
 * to reflect a tree of <groups>, then this will be unneccessary.
 */
static double effective_opacity(const SPStyle *style)
{
    double val = 1.0;
    for (SPObject const *obj = style->object; obj ; obj = obj->parent)
    {
        if (obj->style) {
            val *= SP_SCALE24_TO_FLOAT(obj->style->opacity.value);
        }
    }
    return val;
}

//########################################################################
//# OUTPUT FORMATTING
//########################################################################

JavaFXOutput::JavaFXOutput(void) :
    name(),
    outbuf(),
    foutbuf(),
    nrNodes(0),
    nrShapes(0),
    idindex(0),
    minx(0),
    miny(0),
    maxx(0),
    maxy(0)
{
}

/**
 * We want to control floating output format.
 * Especially to avoid localization. (decimal ',' for example)
 */
static JavaFXOutput::String dstr(double d)
{
    char dbuf[G_ASCII_DTOSTR_BUF_SIZE+1];
    g_ascii_formatd(dbuf, G_ASCII_DTOSTR_BUF_SIZE,
                  "%.8f", (gdouble)d);
    JavaFXOutput::String s = dbuf;
    return s;
}

#define DSTR(d) (dstr(d).c_str())


/**
 * Format an rgba() string
 */
static JavaFXOutput::String rgba(guint32 rgba)
{
    unsigned int r = SP_RGBA32_R_U(rgba);
    unsigned int g = SP_RGBA32_G_U(rgba);
    unsigned int b = SP_RGBA32_B_U(rgba);
    unsigned int a = SP_RGBA32_A_U(rgba);
    char buf[80];
    snprintf(buf, 79, "Color.rgb(0x%02x, 0x%02x, 0x%02x, %s)",
                           r, g, b, DSTR((double)a/255.0));
    JavaFXOutput::String s = buf;
    return s;
}


/**
 * Format an rgba() string for a color and a 0.0-1.0 alpha
 */
static JavaFXOutput::String rgba(SPColor color, gdouble alpha)
{
    return rgba(color.toRGBA32(alpha));
}

/**
 * Map Inkscape linecap styles to JavaFX
 */
static JavaFXOutput::String getStrokeLineCap(unsigned value) {
    switch(value) {
        case SP_STROKE_LINECAP_BUTT:
            return "StrokeLineCap.BUTT";
        case SP_STROKE_LINECAP_ROUND:
            return "StrokeLineCap.ROUND";
        case SP_STROKE_LINECAP_SQUARE:
            return "StrokeLineCap.SQUARE";
        default:
            return "INVALID LINE CAP";
    }
}


/**
 * Map Inkscape linejoin styles to JavaFX
 */
static JavaFXOutput::String getStrokeLineJoin(unsigned value) {
    switch(value) {
        case SP_STROKE_LINEJOIN_MITER:
            return "StrokeLineJoin.MITER";
        case SP_STROKE_LINEJOIN_ROUND:
            return "StrokeLineJoin.ROUND";
        case SP_STROKE_LINEJOIN_BEVEL:
            return "StrokeLineJoin.BEVEL";
        default:
            return "INVALID LINE JOIN";
    }
}


/**
 * Replace illegal characters for JavaFX for a underscore.
 */
static JavaFXOutput::String sanatize(const JavaFXOutput::String &badstr){
    JavaFXOutput::String good(badstr);
    for (int pos = 0; pos < static_cast<int>(badstr.length()); ++pos )
        if ((badstr.at(pos)=='-')||(badstr.at(pos)==' ')) {
            good.replace(pos, 1, "_");
        }
    return good;
}

/**
 *  Output data to the buffer, printf()-style
 */
void JavaFXOutput::out(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    gchar *output = g_strdup_vprintf(fmt, args);
    va_end(args);
    outbuf.append(output);
    g_free(output);
}



/**
 * Output the file header
 */
bool JavaFXOutput::doHeader()
{
    time_t tim = time(NULL);
    out("/*###################################################################\n");
    out("### This JavaFX document was generated by Inkscape\n");
    out("### http://www.inkscape.org\n");
    out("### Created: %s",   ctime(&tim));
    out("### Version: %s\n", Inkscape::version_string);
    out("#####################################################################\n");
    out("### NOTES:\n");
    out("### ============\n");
    out("### JavaFX information can be found at\n");
    out("### http://www.javafx.com/\n");
    out("###\n");
    out("### If you have any problems with this output, please see the\n");
    out("### Inkscape project at http://www.inkscape.org, or visit\n");
    out("### the #inkscape channel on irc.freenode.net . \n");
    out("###\n");
    out("###################################################################*/\n");
    out("\n\n");
    out("/*###################################################################\n");
    out("##   Exports in this file\n");
    out("##==========================\n");
    out("##    Shapes   : %d\n", nrShapes);
    out("##    Nodes    : %d\n", nrNodes);
    out("###################################################################*/\n");
    out("\n\n");

    // import javafx libraries we can need
    out("import javafx.scene.*;\n");
    out("import javafx.scene.shape.*;\n");
    out("import javafx.scene.transform.*;\n");
    out("import javafx.scene.paint.*;\n");
    out("\n");

    out("\n\n");

    // Creates a class extended from CustomNode
    out("public class %s extends CustomNode {\n", name.c_str());

    return true;
}



/**
 *  Output the file footer
 */
bool JavaFXOutput::doTail()
{
    float border = 25.0;

    // Write the tail of CustomNode
    out("           ] // content\n");
    out("           transforms: Translate { x : %s, y : %s }\n",
        DSTR((-minx) + border), DSTR((-miny) + border) );
    out("       } // Group\n");
    out("   } // function create()\n");
    out("} // class %s\n", name.c_str());
    out("\n");

    // Stage
//     out("    stage: Stage {\n");
//     out("        content: %s{}\n", name.c_str());
//     out("    } // Stage\n");


    out("\n");

    out("/*###################################################################\n");
    out("### E N D   C L A S S    %s\n", name.c_str());
    out("###################################################################*/\n");
    out("\n\n");
    return true;
}



/**
 *  Output gradient information to the buffer
 */
bool JavaFXOutput::doGradient(SPGradient *grad, const String &id)
{
    String jfxid = sanatize(id);

    if (SP_IS_LINEARGRADIENT(grad))
        {
        SPLinearGradient *g = SP_LINEARGRADIENT(grad);
        out("    /* create LinearGradient for %s */\n", jfxid.c_str());
        out("    function %s(): LinearGradient {\n",  jfxid.c_str());
        out("        LinearGradient {\n");
        std::vector<SPGradientStop> stops = g->vector.stops;
        if (!stops.empty())
            {
            out("            stops:\n");
            out("                [\n");
            for (unsigned int i = 0 ; i<stops.size() ; i++)
                {
                SPGradientStop stop = stops[i];
                out("                Stop {\n");
                out("                    offset: %s\n", DSTR(stop.offset));
                out("                    color: %s\n",  rgba(stop.color, stop.opacity).c_str());
                out("                },\n");
                }
            out("            ]\n");
            }
        out("        };\n");
        out("    } // end LinearGradient: %s\n", jfxid.c_str());
        out("\n\n");
        }
    else if (SP_IS_RADIALGRADIENT(grad))
        {
        SPRadialGradient *g = SP_RADIALGRADIENT(grad);
        out("    /* create RadialGradient for %s */\n", jfxid.c_str());
        out("    function %s() {\n", jfxid.c_str());
        out("        RadialGradient {\n");
        out("            centerX: %s\n", DSTR(g->cx.value));
        out("            centerY: %s\n", DSTR(g->cy.value));
        out("            focusX: %s\n",  DSTR(g->fx.value));
        out("            focusY: %s\n",  DSTR(g->fy.value));
        out("            radius: %s\n",  DSTR(g->r.value ));
        std::vector<SPGradientStop> stops = g->vector.stops;
        if (!stops.empty())
            {
            out("            stops:\n");
            out("            [\n");
            for (unsigned int i = 0 ; i<stops.size() ; i++)
                {
                SPGradientStop stop = stops[i];
                out("                Stop {\n");
                out("                    offset: %s\n", DSTR(stop.offset));
                out("                    color: %s\n",  rgba(stop.color, stop.opacity).c_str());
                out("                },\n");
                }
            out("            ]\n");
            }
        out("        };\n");
        out("    } // end RadialGradient: %s\n", jfxid.c_str());
        out("\n\n");
        }
    else
        {
        err("Unknown gradient type for '%s'\n", jfxid.c_str());
        return false;
        }


    return true;
}




/**
 *  Output an element's style attribute
 */
bool JavaFXOutput::doStyle(SPStyle *style)
{
    if (!style) {
        return true;
    }

    out("            opacity: %s\n", DSTR(effective_opacity(style)));

    /**
     * Fill
     */
    SPIPaint const &fill = style->fill;
    if (fill.isColor())
        {
        // see color.h for how to parse SPColor
        out("            fill: %s\n",
            rgba(fill.value.color, SP_SCALE24_TO_FLOAT(style->fill_opacity.value)).c_str());
        }
    else if (fill.isPaintserver()){
        if (fill.value.href && fill.value.href->getURI() ){
            gchar *str = fill.value.href->getURI()->toString();
            String uri = (str ? str : "");
            /* trim the anchor '#' from the front */
            if (uri.size() > 0 && uri[0]=='#') {
                uri = uri.substr(1);
            }
            out("            fill: %s()\n", sanatize(uri).c_str());
            g_free(str);
        }
    }


    /**
     * Stroke
     */
    /**
     *NOTE:  Things in style we can use:
     * SPIPaint stroke;
     * SPILength stroke_width;
     * SPIEnum stroke_linecap;
     * SPIEnum stroke_linejoin;
     * SPIFloat stroke_miterlimit;
     * SPIDashArray stroke_dasharray;
     * SPILength stroke_dashoffset;
     * SPIScale24 stroke_opacity;
     */
    if (style->stroke_opacity.value > 0)
        {
        SPIPaint const &stroke = style->stroke;
        out("            stroke: %s\n",
            rgba(stroke.value.color, SP_SCALE24_TO_FLOAT(style->stroke_opacity.value)).c_str());
        double strokewidth = style->stroke_width.value;
        unsigned linecap   = style->stroke_linecap.value;
        unsigned linejoin  = style->stroke_linejoin.value;
        out("            strokeWidth: %s\n",      DSTR(strokewidth));
        out("            strokeLineCap: %s\n",    getStrokeLineCap(linecap).c_str());
        out("            strokeLineJoin: %s\n",   getStrokeLineJoin(linejoin).c_str());
        out("            strokeMiterLimit: %s\n", DSTR(style->stroke_miterlimit.value));
        if (style->stroke_dasharray.set) {
           if (style->stroke_dashoffset.set) {
               out("            strokeDashOffset: %s\n", DSTR(style->stroke_dashoffset.value));
           }
           out("            strokeDashArray: [ ");
           for(unsigned i = 0; i < style->stroke_dasharray.values.size(); i++ ) {
               if (i > 0) {
                   out(", %.2lf", style->stroke_dasharray.values[i]);
               }else {
                   out(" %.2lf", style->stroke_dasharray.values[i]);
               }
           }
           out(" ]\n");
        }

        }

    return true;
}


#if 1

/**
 *  Output the curve data to buffer
 */
bool JavaFXOutput::doCurve(SPItem *item, const String &id)
{
    using Geom::X;
    using Geom::Y;

    String jfxid = sanatize(id);

    //### Get the Shape
    if (!SP_IS_SHAPE(item)) { //Bulia's suggestion.  Allow all shapes
        return true;
    }

    SPShape *shape = SP_SHAPE(item);
    if (shape->_curve->is_empty()) {
        return true;
    }

    nrShapes++;

    out("    /** path %s */\n", jfxid.c_str());
    out("    function %s() : Path {\n",jfxid.c_str());
    out("        Path {\n");
    out("            id: \"%s\"\n", jfxid.c_str());

    /**
     * Output the style information
     */
    if (!doStyle(shape->style)) {
        return false;
    }

    // convert the path to only lineto's and cubic curveto's:
    Geom::Scale yflip(1.0, -1.0); /// @fixme  hardcoded desktop transform!
    Geom::Affine tf = item->i2dt_affine() * yflip;
    Geom::PathVector pathv = pathv_to_linear_and_cubic_beziers( shape->_curve->get_pathvector() * tf );

    //Count the NR_CURVETOs/LINETOs (including closing line segment)
    guint segmentCount = 0;
    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {
        segmentCount += (*it).size();
        if (it->closed()) {
            segmentCount += 1;
        }
    }

    out("            elements: [\n");

    unsigned int segmentNr = 0;

    nrNodes += segmentCount;

    Geom::Rect cminmax( pathv.front().initialPoint(), pathv.front().initialPoint() );

    /**
     * For all Subpaths in the <path>
     */
    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit)
        {
        Geom::Point p = pit->front().initialPoint();
        cminmax.expandTo(p);
        out("                MoveTo {\n");
        out("                    x: %s\n", DSTR(p[X]));
        out("                    y: %s\n", DSTR(p[Y]));
        out("                },\n");

        /**
         * For all segments in the subpath
         */
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_closed(); ++cit)
            {
            //### LINE
            if ( dynamic_cast<Geom::LineSegment  const *> (&*cit) )
                {
                Geom::Point p = cit->finalPoint();
                out("                LineTo {\n");
                out("                    x: %s\n", DSTR(p[X]));
                out("                    y: %s\n", DSTR(p[Y]));
                out("                },\n");
                nrNodes++;
                }
            //### BEZIER
            else if (Geom::CubicBezier const *cubic = dynamic_cast<Geom::CubicBezier const*>(&*cit))
                {
                std::vector<Geom::Point> points = cubic->controlPoints();
                Geom::Point p1 = points[1];
                Geom::Point p2 = points[2];
                Geom::Point p3 = points[3];
                out("                CubicCurveTo {\n");
                out("                    controlX1: %s\n", DSTR(p1[X]));
                out("                    controlY1: %s\n", DSTR(p1[Y]));
                out("                    controlX2: %s\n", DSTR(p2[X]));
                out("                    controlY2: %s\n", DSTR(p2[Y]));
                out("                    x: %s\n",         DSTR(p3[X]));
                out("                    y: %s\n",         DSTR(p3[Y]));
                out("                },\n");
                nrNodes++;
                }
            else
                {
                g_error ("logical error, because pathv_to_linear_and_cubic_beziers was used");
                }
            segmentNr++;
            cminmax.expandTo(cit->finalPoint());
            }
        if (pit->closed())
            {
            out("                ClosePath {},\n");
            }
        }

    out("            ] // elements\n");
    out("        }; // Path\n");
    out("    } // end path %s\n\n", jfxid.c_str());

    double cminx = cminmax.min()[X];
    double cmaxx = cminmax.max()[X];
    double cminy = cminmax.min()[Y];
    double cmaxy = cminmax.max()[Y];

    if (cminx < minx) {
        minx = cminx;
    }
    if (cmaxx > maxx) {
        maxx = cmaxx;
    }
    if (cminy < miny) {
        miny = cminy;
    }
    if (cmaxy > maxy) {
        maxy = cmaxy;
    }

    return true;
}



#else

/**
 *  Output the curve data to buffer
 */
bool JavaFXOutput::doCurve(SPItem *item, const String &id)
{
    using Geom::X;
    using Geom::Y;

    //### Get the Shape
    if (!SP_IS_SHAPE(item)) { //Bulia's suggestion.  Allow all shapes
        return true;
    }

    SPShape *shape = SP_SHAPE(item);
    SPCurve *curve = shape->curve;
    if (curve->is_empty()) {
        return true;
    }

    nrShapes++;

    out("        SVGPath \n");
    out("        {\n");
    out("        id: \"%s\"\n", id.c_str());

    /**
     * Output the style information
     */
    if (!doStyle(shape->style)) {
        return false;
    }

    // convert the path to only lineto's and cubic curveto's:
    Geom::Scale yflip(1.0, -1.0); /// @fixme hardcoded desktop transform
    Geom::Affine tf = item->i2dt_affine() * yflip;
    Geom::PathVector pathv = pathv_to_linear_and_cubic_beziers( curve->get_pathvector() * tf );

    //Count the NR_CURVETOs/LINETOs (including closing line segment)
    nrNodes = 0;
    for(Geom::PathVector::const_iterator it = pathv.begin(); it != pathv.end(); ++it) {
        nrNodes += (*it).size();
        if (it->closed()) {
            nrNodes += 1;
        }
    }

    char *dataStr = sp_svg_write_path(pathv);
    out("        content: \"%s\"\n", dataStr);
    free(dataStr);

    Geom::Rect cminmax( pathv.front().initialPoint(), pathv.front().initialPoint() );

    /**
     * Get the Min and Max X and Y extends for the Path.
     * ....For all Subpaths in the <path>
     */
    for (Geom::PathVector::const_iterator pit = pathv.begin(); pit != pathv.end(); ++pit)
        {
        cminmax.expandTo(pit->front().initialPoint());
        /**
         * For all segments in the subpath
         */
        for (Geom::Path::const_iterator cit = pit->begin(); cit != pit->end_closed(); ++cit)
            {
            cminmax.expandTo(cit->finalPoint());
            }
        }

    out("        },\n");

    double cminx = cminmax.min()[X];
    double cmaxx = cminmax.max()[X];
    double cminy = cminmax.min()[Y];
    double cmaxy = cminmax.max()[Y];

    if (cminx < minx) {
        minx = cminx;
    }
    if (cmaxx > maxx) {
        maxx = cmaxx;
    }
    if (cminy < miny) {
        miny = cminy;
    }
    if (cmaxy > maxy) {
        maxy = cmaxy;
    }

    return true;
}



#endif  /* #if o */



/**
 *  Output the tree data to buffer
 */
bool JavaFXOutput::doTreeRecursive(SPDocument *doc, SPObject *obj)
{
    /**
     * Check the type of node and process
     */
    String id;
    if (!obj->getId())
        {
        char buf[16];
        sprintf(buf, "id%d", idindex++);
        id = buf;
        }
    else
        {
            id = obj->getId();
        }
    if (SP_IS_ITEM(obj))
        {
        SPItem *item = SP_ITEM(obj);
        if (!doCurve(item, id)) {
            return false;
        }
        }
    else if (SP_IS_GRADIENT(obj))
        {
        SPGradient *grad = SP_GRADIENT(obj);
        if (!doGradient(grad, id)) {
            return false;
        }
        }

    /**
     * Descend into children
     */
    for (SPObject *child = obj->firstChild() ; child ; child = child->next)
        {
            if (!doTreeRecursive(doc, child)) {
                return false;
            }
        }

    return true;
}


/**
 *  Output the curve data to buffer
 */
bool JavaFXOutput::doTree(SPDocument *doc)
{

    double bignum = 1000000.0;
    minx  =  bignum;
    maxx  = -bignum;
    miny  =  bignum;
    maxy  = -bignum;

    if (!doTreeRecursive(doc, doc->getRoot())) {
        return false;
    }

    return true;

}


bool JavaFXOutput::doBody(SPDocument *doc, SPObject *obj)
{
    /**
     * Check the type of node and process
     */
    String id;
    if (!obj->getId())
        {
        char buf[16];
        sprintf(buf, "id%d", idindex++);
        id = buf;
        }
    else
        {
            id = obj->getId();
        }

    if (SP_IS_ITEM(obj)) {
        SPItem *item = SP_ITEM(obj);
        //### Get the Shape
        if (SP_IS_SHAPE(item)) {//Bulia's suggestion.  Allow all shapes
            SPShape *shape = SP_SHAPE(item);
            SPCurve *curve = shape->_curve;
            if (!curve->is_empty()) {
                String jfxid = sanatize(id);
                out("               %s(),\n", jfxid.c_str());
            }
        }
    }
    else if (SP_IS_GRADIENT(obj)) {
        //TODO: what to do with Gradient within body?????
        //SPGradient *grad = SP_GRADIENT(reprobj);
        //if (!doGradient(grad, id)) {
        //    return false;
        //}
    }

    /**
     * Descend into children
     */
    for (SPObject *child = obj->firstChild() ; child ; child = child->next)
        {
            if (!doBody(doc, child)) {
                return false;
            }
        }

    return true;
}



//########################################################################
//# M A I N    O U T P U T
//########################################################################



/**
 *  Set values back to initial state
 */
void JavaFXOutput::reset()
{
    nrNodes    = 0;
    nrShapes   = 0;
    idindex    = 0;
    name.clear();
    outbuf.clear();
    foutbuf.clear();
}



/**
 * Saves the <paths> of an Inkscape SVG file as JavaFX spline definitions
 */
bool JavaFXOutput::saveDocument(SPDocument *doc, gchar const *filename_utf8)
{
    reset();


    name = Glib::path_get_basename(filename_utf8);
    int pos = name.find('.');
    if (pos > 0) {
        name = name.substr(0, pos);
    }


    //###### SAVE IN JAVAFX FORMAT TO BUFFER
    //# Lets do the curves first, to get the stats

    if (!doTree(doc)) {
        return false;
    }
    String curveBuf = outbuf;
    outbuf.clear();

    if (!doHeader()) {
        return false;
    }

    outbuf.append(curveBuf);

    out("   override function create(): Node {\n");
    out("       Group {\n");
    out("           content: [\n");
    idindex    = 0;

    doBody(doc, doc->getRoot());

    if (!doTail()) {
        return false;
    }



    //###### WRITE TO FILE
    FILE *f = Inkscape::IO::fopen_utf8name(filename_utf8, "w");
    if (!f)
        {
        err("Could open JavaFX file '%s' for writing", filename_utf8);
        return false;
        }

    for (String::iterator iter = outbuf.begin() ; iter!=outbuf.end(); ++iter)
        {
        fputc(*iter, f);
        }

    fclose(f);

    return true;
}




//########################################################################
//# EXTENSION API
//########################################################################



#include "clear-n_.h"



/**
 * API call to save document
*/
void
JavaFXOutput::save(Inkscape::Extension::Output */*mod*/,
                        SPDocument *doc, gchar const *filename_utf8)
{
    /* N.B. The name `filename_utf8' represents the fact that we want it to be in utf8; whereas in
     * fact we know that some callers of Extension::save pass something in the filesystem's
     * encoding, while others do g_filename_to_utf8 before calling.
     *
     * In terms of safety, it's best to make all callers pass actual filenames, since in general
     * one can't round-trip from filename to utf8 back to the same filename.  Whereas the argument
     * for passing utf8 filenames is one of convenience: we often want to pass to g_warning or
     * store as a string (rather than a byte stream) in XML, or the like. */
    if (!saveDocument(doc, filename_utf8))
        {
        g_warning("Could not save JavaFX file '%s'", filename_utf8);
        }
}



/**
 * Make sure that we are in the database
 */
bool JavaFXOutput::check (Inkscape::Extension::Extension */*module*/)
{
    /* We don't need a Key
    if (NULL == Inkscape::Extension::db.get(SP_MODULE_KEY_OUTPUT_JFX)) {
        return FALSE;
    }
    */

    return true;
}



/**
 * This is the definition of JavaFX output.  This function just
 * calls the extension system with the memory allocated XML that
 * describes the data.
*/
void
JavaFXOutput::init()
{
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("JavaFX Output") "</name>\n"
            "<id>org.inkscape.output.jfx</id>\n"
            "<output>\n"
                "<extension>.fx</extension>\n"
                "<mimetype>text/x-javafx-script</mimetype>\n"
                "<filetypename>" N_("JavaFX (*.fx)") "</filetypename>\n"
                "<filetypetooltip>" N_("JavaFX Raytracer File") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>",
        new JavaFXOutput());
}





}  // namespace Internal
}  // namespace Extension
}  // namespace Inkscape


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
