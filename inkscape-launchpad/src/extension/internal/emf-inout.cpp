/** @file
 * @brief Windows-only Enhanced Metafile input and output.
 */
/* Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *   Jon A. Cruz <jon@joncruz.org>
 *   David Mathog
 *   Abhishek Sharma
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 * References:
 *  - How to Create & Play Enhanced Metafiles in Win32
 *      http://support.microsoft.com/kb/q145999/
 *  - INFO: Windows Metafile Functions & Aldus Placeable Metafiles
 *      http://support.microsoft.com/kb/q66949/
 *  - Metafile Functions
 *      http://msdn.microsoft.com/library/en-us/gdi/metafile_0whf.asp
 *  - Metafile Structures
 *      http://msdn.microsoft.com/library/en-us/gdi/metafile_5hkj.asp
 */


#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <libuemf/symbol_convert.h>

#include "document.h"
#include "sp-root.h" // even though it is included indirectly by wmf-inout.h
#include "sp-path.h"
#include "print.h"
#include "extension/system.h"
#include "extension/print.h"
#include "extension/db.h"
#include "extension/input.h"
#include "extension/output.h"
#include "display/drawing.h"
#include "display/drawing-item.h"
#include "clear-n_.h"
#include "svg/svg.h"
#include "util/units.h" // even though it is included indirectly by emf-inout.h
#include "inkscape.h"   // even though it is included indirectly by emf-inout.h

#include "emf-print.h"
#include "emf-inout.h"

#define PRINT_EMF "org.inkscape.print.emf"

#ifndef U_PS_JOIN_MASK
#define U_PS_JOIN_MASK (U_PS_JOIN_BEVEL|U_PS_JOIN_MITER|U_PS_JOIN_ROUND)
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {

static uint32_t ICMmode = 0;  // not used yet, but code to read it from EMF implemented
static uint32_t BLTmode = 0;
float           faraway = 10000000; // used in "exclude" clips, hopefully well outside any real drawing!

Emf::Emf (void) // The null constructor
{
    return;
}


Emf::~Emf (void) //The destructor
{
    return;
}


bool
Emf::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(PRINT_EMF))
        return FALSE;
    return TRUE;
}


void
Emf::print_document_to_file(SPDocument *doc, const gchar *filename)
{
    Inkscape::Extension::Print *mod;
    SPPrintContext context;
    const gchar *oldconst;
    gchar *oldoutput;
    unsigned int ret;

    doc->ensureUpToDate();

    mod = Inkscape::Extension::get_print(PRINT_EMF);
    oldconst = mod->get_param_string("destination");
    oldoutput = g_strdup(oldconst);
    mod->set_param_string("destination", filename);

/* Start */
    context.module = mod;
    /* fixme: This has to go into module constructor somehow */
    /* Create new arena */
    mod->base = doc->getRoot();
    Inkscape::Drawing drawing;
    mod->dkey = SPItem::display_key_new(1);
    mod->root = mod->base->invoke_show(drawing, mod->dkey, SP_ITEM_SHOW_DISPLAY);
    drawing.setRoot(mod->root);
    /* Print document */
    ret = mod->begin(doc);
    if (ret) {
        g_free(oldoutput);
        throw Inkscape::Extension::Output::save_failed();
    }
    mod->base->invoke_print(&context);
    (void) mod->finish();
    /* Release arena */
    mod->base->invoke_hide(mod->dkey);
    mod->base = NULL;
    mod->root = NULL; // deleted by invoke_hide
/* end */

    mod->set_param_string("destination", oldoutput);
    g_free(oldoutput);

    return;
}


void
Emf::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::db.get(PRINT_EMF);
    if (ext == NULL)
        return;

    bool new_val                  = mod->get_param_bool("textToPath");
    bool new_FixPPTCharPos        = mod->get_param_bool("FixPPTCharPos");  // character position bug
    // reserve FixPPT2 for opacity bug.  Currently EMF does not export opacity values
    bool new_FixPPTDashLine       = mod->get_param_bool("FixPPTDashLine");  // dashed line bug
    bool new_FixPPTGrad2Polys     = mod->get_param_bool("FixPPTGrad2Polys");  // gradient bug
    bool new_FixPPTLinGrad        = mod->get_param_bool("FixPPTLinGrad");     // allow native rectangular linear gradient
    bool new_FixPPTPatternAsHatch = mod->get_param_bool("FixPPTPatternAsHatch");  // force all patterns as standard EMF hatch
    bool new_FixImageRot          = mod->get_param_bool("FixImageRot");  // remove rotations on images

    TableGen(                  //possibly regenerate the unicode-convert tables
        mod->get_param_bool("TnrToSymbol"),
        mod->get_param_bool("TnrToWingdings"),
        mod->get_param_bool("TnrToZapfDingbats"),
        mod->get_param_bool("UsePUA")
    );

    ext->set_param_bool("FixPPTCharPos",new_FixPPTCharPos);   // Remember to add any new ones to PrintEmf::init or a mysterious failure will result!
    ext->set_param_bool("FixPPTDashLine",new_FixPPTDashLine);
    ext->set_param_bool("FixPPTGrad2Polys",new_FixPPTGrad2Polys);
    ext->set_param_bool("FixPPTLinGrad",new_FixPPTLinGrad);
    ext->set_param_bool("FixPPTPatternAsHatch",new_FixPPTPatternAsHatch);
    ext->set_param_bool("FixImageRot",new_FixImageRot);
    ext->set_param_bool("textToPath", new_val);

    print_document_to_file(doc, filename);

    return;
}


/*  given the transformation matrix from worldTransform return the scale in the matrix part.  Assumes that the
    matrix is not used to skew, invert, or make another distorting transformation.  */
double Emf::current_scale(PEMF_CALLBACK_DATA d){
    double scale =
        d->dc[d->level].worldTransform.eM11 * d->dc[d->level].worldTransform.eM22 -
        d->dc[d->level].worldTransform.eM12 * d->dc[d->level].worldTransform.eM21;
    if(scale <= 0.0)scale=1.0;  /* something is dreadfully wrong with the matrix, but do not crash over it */
    scale=sqrt(scale);
    return(scale);
}

/*  given the transformation matrix from worldTransform and the current x,y position in inkscape coordinates,
    generate an SVG transform that gives the same amount of rotation, no scaling, and maps x,y back onto x,y.  This is used for
    rotating objects when the location of at least one point in that object is known. Returns:
    "matrix(a,b,c,d,e,f)"  (WITH the double quotes)
*/
std::string Emf::current_matrix(PEMF_CALLBACK_DATA d, double x, double y, int useoffset){
    SVGOStringStream cxform;
    double scale = current_scale(d);
    cxform << "\"matrix(";
    cxform << d->dc[d->level].worldTransform.eM11/scale;   cxform << ",";
    cxform << d->dc[d->level].worldTransform.eM12/scale;   cxform << ",";
    cxform << d->dc[d->level].worldTransform.eM21/scale;   cxform << ",";
    cxform << d->dc[d->level].worldTransform.eM22/scale;   cxform << ",";
    if(useoffset){
        /* for the "new" coordinates drop the worldtransform translations, not used here */
        double newx    = x * d->dc[d->level].worldTransform.eM11/scale + y * d->dc[d->level].worldTransform.eM21/scale;
        double newy    = x * d->dc[d->level].worldTransform.eM12/scale + y * d->dc[d->level].worldTransform.eM22/scale;
        cxform << x - newx;                                  cxform << ",";
        cxform << y - newy;
    }
    else {
        cxform << "0,0";
    }
    cxform << ")\"";
    return(cxform.str());
}

/*  given the transformation matrix from worldTransform return the rotation angle in radians.
    counter clocwise from the x axis.  */
double Emf::current_rotation(PEMF_CALLBACK_DATA d){
    return -std::atan2(d->dc[d->level].worldTransform.eM12, d->dc[d->level].worldTransform.eM11);
}

/*  Add another 100 blank slots to the hatches array.
*/
void Emf::enlarge_hatches(PEMF_CALLBACK_DATA d){
    d->hatches.size += 100;
    d->hatches.strings = (char **) realloc(d->hatches.strings,d->hatches.size * sizeof(char *));
}

/*  See if the pattern name is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Emf::in_hatches(PEMF_CALLBACK_DATA d, char *test){
    int i;
    for(i=0; i<d->hatches.count; i++){
        if(strcmp(test,d->hatches.strings[i])==0)return(i+1);
    }
    return(0);
}

/*  (Conditionally) add a hatch.  If a matching hatch already exists nothing happens.  If one
    does not exist it is added to the hatches list and also entered into <defs>.
    This is also used to add the path part of the hatches, which they reference with a xlink:href
*/
uint32_t Emf::add_hatch(PEMF_CALLBACK_DATA d, uint32_t hatchType, U_COLORREF hatchColor){
    char hatchname[64]; // big enough
    char hpathname[64]; // big enough
    char hbkname[64];   // big enough
    char tmpcolor[8];
    char bkcolor[8];
    uint32_t idx;

    switch(hatchType){
        case U_HS_SOLIDTEXTCLR:
        case U_HS_DITHEREDTEXTCLR:
            sprintf(tmpcolor,"%6.6X",sethexcolor(d->dc[d->level].textColor));
            break;
        case U_HS_SOLIDBKCLR:
        case U_HS_DITHEREDBKCLR:
            sprintf(tmpcolor,"%6.6X",sethexcolor(d->dc[d->level].bkColor));
            break;
        default:
            sprintf(tmpcolor,"%6.6X",sethexcolor(hatchColor));
            break;
    }

    /*  For both bkMode types set the PATH + FOREGROUND COLOR for the indicated standard hatch.
        This will be used late to compose, or recompose  the transparent or opaque final hatch.*/

    std::string refpath; // used to reference later the path pieces which are about to be created
    sprintf(hpathname,"EMFhpath%d_%s",hatchType,tmpcolor);
    idx = in_hatches(d,hpathname);
    if(!idx){  // add path/color if not already present
        if(d->hatches.count == d->hatches.size){  enlarge_hatches(d); }
        d->hatches.strings[d->hatches.count++]=strdup(hpathname);

        d->defs += "\n";
        switch(hatchType){
            case U_HS_HORIZONTAL:
                d->defs += "   <path id=\"";
                d->defs += hpathname;
                d->defs += "\" d=\"M 0 0 6 0\" style=\"fill:none;stroke:#";
                d->defs += tmpcolor;
                d->defs += "\" />\n";
                break;
            case U_HS_VERTICAL:
                d->defs += "   <path id=\"";
                d->defs += hpathname;
                d->defs += "\" d=\"M 0 0 0 6\" style=\"fill:none;stroke:#";
                d->defs += tmpcolor;
                d->defs += "\" />\n";
                break;
            case U_HS_FDIAGONAL:
                d->defs += "   <line  id=\"sub";
                d->defs += hpathname;
                d->defs += "\" x1=\"-1\" y1=\"-1\" x2=\"7\" y2=\"7\" stroke=\"#";
                d->defs += tmpcolor;
                d->defs += "\"/>\n";
                break;
            case U_HS_BDIAGONAL:
                d->defs += "   <line  id=\"sub";
                d->defs += hpathname;
                d->defs += "\" x1=\"-1\" y1=\"7\" x2=\"7\" y2=\"-1\" stroke=\"#";
                d->defs += tmpcolor;
                d->defs += "\"/>\n";
                break;
            case U_HS_CROSS:
                d->defs += "   <path   id=\"";
                d->defs += hpathname;
                d->defs += "\" d=\"M 0 0 6 0 M 0 0 0 6\" style=\"fill:none;stroke:#";
                d->defs += tmpcolor;
                d->defs += "\" />\n";
                 break;
            case U_HS_DIAGCROSS:
                d->defs += "   <line   id=\"subfd";
                d->defs += hpathname;
                d->defs += "\" x1=\"-1\" y1=\"-1\" x2=\"7\" y2=\"7\" stroke=\"#";
                d->defs += tmpcolor;
                d->defs += "\"/>\n";
                d->defs += "   <line   id=\"subbd";
                d->defs += hpathname;
                d->defs += "\" x1=\"-1\" y1=\"7\" x2=\"7\" y2=\"-1\" stroke=\"#";
                d->defs += tmpcolor;
                d->defs += "\"/>\n";
                break;
            case U_HS_SOLIDCLR:
            case U_HS_DITHEREDCLR:
            case U_HS_SOLIDTEXTCLR:
            case U_HS_DITHEREDTEXTCLR:
            case U_HS_SOLIDBKCLR:
            case U_HS_DITHEREDBKCLR:
            default:
                d->defs += "   <path   id=\"";
                d->defs += hpathname;
                d->defs += "\" d=\"M 0 0 6 0 6 6 0 6 z\" style=\"fill:#";
                d->defs += tmpcolor;
                d->defs += ";stroke:none";
                d->defs += "\" />\n";
                break;
        }
    }

    // References to paths possibly just created above.  These will be used in the actual patterns.
    switch(hatchType){
        case U_HS_HORIZONTAL:
        case U_HS_VERTICAL:
        case U_HS_CROSS:
        case U_HS_SOLIDCLR:
        case U_HS_DITHEREDCLR:
        case U_HS_SOLIDTEXTCLR:
        case U_HS_DITHEREDTEXTCLR:
        case U_HS_SOLIDBKCLR:
        case U_HS_DITHEREDBKCLR:
        default:
            refpath    += "      <use xlink:href=\"#";
            refpath    += hpathname;
            refpath    += "\" />\n";
            break;
        case U_HS_FDIAGONAL:
        case U_HS_BDIAGONAL:
            refpath    += "      <use xlink:href=\"#sub";
            refpath    += hpathname;
            refpath    += "\" />\n";
            refpath    += "      <use xlink:href=\"#sub";
            refpath    += hpathname;
            refpath    += "\"  transform=\"translate(6,0)\" />\n";
            refpath    += "      <use xlink:href=\"#sub";
            refpath    += hpathname;
            refpath    += "\"  transform=\"translate(-6,0)\" />\n";
            break;
        case U_HS_DIAGCROSS:
            refpath    += "      <use xlink:href=\"#subfd";
            refpath    += hpathname;
            refpath    += "\" />\n";
            refpath    += "      <use xlink:href=\"#subfd";
            refpath    += hpathname;
            refpath    += "\" transform=\"translate(6,0)\"/>\n";
            refpath    += "      <use xlink:href=\"#subfd";
            refpath    += hpathname;
            refpath    += "\" transform=\"translate(-6,0)\"/>\n";
            refpath    += "      <use xlink:href=\"#subbd";
            refpath    += hpathname;
            refpath    += "\" />\n";
            refpath    += "      <use xlink:href=\"#subbd";
            refpath    += hpathname;
            refpath    += "\" transform=\"translate(6,0)\"/>\n";
            refpath    += "      <use xlink:href=\"#subbd";
            refpath    += hpathname;
            refpath    += "\" transform=\"translate(-6,0)\"/>\n";
            break;
    }

    if(d->dc[d->level].bkMode == U_TRANSPARENT || hatchType >= U_HS_SOLIDCLR){
        sprintf(hatchname,"EMFhatch%d_%s",hatchType,tmpcolor);
        sprintf(hpathname,"EMFhpath%d_%s",hatchType,tmpcolor);
        idx = in_hatches(d,hatchname);
        if(!idx){  // add it if not already present
            if(d->hatches.count == d->hatches.size){  enlarge_hatches(d); }
            d->hatches.strings[d->hatches.count++]=strdup(hatchname);
            d->defs += "\n";
            d->defs += "   <pattern id=\"";
            d->defs += hatchname;
            d->defs += "\"  xlink:href=\"#EMFhbasepattern\">\n";
            d->defs += refpath;
            d->defs += "   </pattern>\n";
            idx = d->hatches.count;
        }
    }
    else { //  bkMode==U_OPAQUE
        /* Set up an object in the defs for this background, if there is not one already there */
        sprintf(bkcolor,"%6.6X",sethexcolor(d->dc[d->level].bkColor));
        sprintf(hbkname,"EMFhbkclr_%s",bkcolor);
        idx = in_hatches(d,hbkname);
        if(!idx){  // add path/color if not already present.  Hatchtype is not needed in the name.
            if(d->hatches.count == d->hatches.size){  enlarge_hatches(d); }
            d->hatches.strings[d->hatches.count++]=strdup(hbkname);

            d->defs += "\n";
            d->defs += "   <rect id=\"";
            d->defs += hbkname;
            d->defs += "\" x=\"0\" y=\"0\" width=\"6\" height=\"6\" fill=\"#";
            d->defs += bkcolor;
            d->defs += "\" />\n";
        }

        // this is the pattern, its name will show up in Inkscape's pattern selector
        sprintf(hatchname,"EMFhatch%d_%s_%s",hatchType,tmpcolor,bkcolor);
        idx = in_hatches(d,hatchname);
        if(!idx){  // add it if not already present
            if(d->hatches.count == d->hatches.size){  enlarge_hatches(d); }
            d->hatches.strings[d->hatches.count++]=strdup(hatchname);
            d->defs += "\n";
            d->defs += "   <pattern id=\"";
            d->defs += hatchname;
            d->defs += "\"  xlink:href=\"#EMFhbasepattern\">\n";
            d->defs += "      <use xlink:href=\"#";
            d->defs += hbkname;
            d->defs += "\" />\n";
            d->defs += refpath;
            d->defs += "   </pattern>\n";
            idx = d->hatches.count;
        }
    }
    return(idx-1);
}

/*  Add another 100 blank slots to the images array.
*/
void Emf::enlarge_images(PEMF_CALLBACK_DATA d){
    d->images.size += 100;
    d->images.strings = (char **) realloc(d->images.strings,d->images.size * sizeof(char *));
}

/*  See if the image string is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Emf::in_images(PEMF_CALLBACK_DATA d, const char *test){
    int i;
    for(i=0; i<d->images.count; i++){
        if(strcmp(test,d->images.strings[i])==0)return(i+1);
    }
    return(0);
}

/*  (Conditionally) add an image.  If a matching image already exists nothing happens.  If one
    does not exist it is added to the images list and also entered into <defs>.

    U_EMRCREATEMONOBRUSH records only work when the bitmap is monochrome.  If we hit one that isn't
      set idx to 2^32-1 and let the caller handle it.
*/
uint32_t Emf::add_image(PEMF_CALLBACK_DATA d,  void *pEmr, uint32_t cbBits, uint32_t cbBmi,
    uint32_t iUsage, uint32_t offBits, uint32_t offBmi){

    uint32_t idx;
    char imagename[64];             // big enough
    char imrotname[64];             // big enough
    char xywh[64];                  // big enough
    int  dibparams = U_BI_UNKNOWN;  // type of image not yet determined

    MEMPNG mempng; // PNG in memory comes back in this
    mempng.buffer = NULL;

    char            *rgba_px = NULL;     // RGBA pixels
    const char      *px      = NULL;     // DIB pixels
    const U_RGBQUAD *ct      = NULL;     // DIB color table
    U_RGBQUAD        ct2[2];
    uint32_t width, height, colortype, numCt, invert; // if needed these values will be set in get_DIB_params
    if(cbBits && cbBmi  && (iUsage == U_DIB_RGB_COLORS)){
        // next call returns pointers and values, but allocates no memory
        dibparams = get_DIB_params((const char *)pEmr, offBits, offBmi, &px, (const U_RGBQUAD **) &ct,
            &numCt, &width, &height, &colortype, &invert);
        if(dibparams ==U_BI_RGB){
            // U_EMRCREATEMONOBRUSH uses text/bk colors instead of what is in the color map.
            if(((PU_EMR)pEmr)->iType == U_EMR_CREATEMONOBRUSH){
                if(numCt==2){
                    ct2[0] =  U_RGB2BGR(d->dc[d->level].textColor);
                    ct2[1] =  U_RGB2BGR(d->dc[d->level].bkColor);
                    ct     =  &ct2[0];
                }
                else {  // This record is invalid, nothing more to do here, let caller handle it
                    return(U_EMR_INVALID);
                }
            }

            if(!DIB_to_RGBA(
                px,         // DIB pixel array
                ct,         // DIB color table
                numCt,      // DIB color table number of entries
                &rgba_px,   // U_RGBA pixel array (32 bits), created by this routine, caller must free.
                width,      // Width of pixel array in record
                height,     // Height of pixel array in record
                colortype,  // DIB BitCount Enumeration
                numCt,      // Color table used if not 0
                invert      // If DIB rows are in opposite order from RGBA rows
            )){
                toPNG(         // Get the image from the RGBA px into mempng
                    &mempng,
                    width, height,    // of the SRC bitmap
                    rgba_px
                );
                free(rgba_px);
            }
        }
    }

    gchar *base64String=NULL;
    if(dibparams == U_BI_JPEG || dibparams==U_BI_PNG){  // image was binary png or jpg in source file
        base64String = g_base64_encode((guchar*) px, numCt );
    }
    else if(mempng.buffer){                             // image was DIB in source file, converted to png in this routine
        base64String = g_base64_encode((guchar*) mempng.buffer, mempng.size );
        free(mempng.buffer);
    }
    else {                                              // unknown or unsupported image type or failed conversion, insert the common bad image picture
        width  = 3;
        height = 4;
        base64String = bad_image_png();
    }

    idx = in_images(d, (char *) base64String);
    if(!idx){  // add it if not already present - we looked at the actual data for comparison
        if(d->images.count == d->images.size){  enlarge_images(d); }
        idx = d->images.count;
        d->images.strings[d->images.count++]=strdup(base64String);

        sprintf(imagename,"EMFimage%d",idx++);
        sprintf(xywh," x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" ",width,height); // reuse this buffer

        d->defs += "\n";
        d->defs += "   <image id=\"";
        d->defs += imagename;
        d->defs += "\"\n      ";
        d->defs += xywh;
        d->defs += "\n";
        if(dibparams == U_BI_JPEG){    d->defs += "       xlink:href=\"data:image/jpeg;base64,"; }
        else {                         d->defs += "       xlink:href=\"data:image/png;base64,";  }
        d->defs += base64String;
        d->defs += "\"\n";
        d->defs += "    preserveAspectRatio=\"none\"\n";
        d->defs += "    />\n";


        d->defs += "\n";
        d->defs += "   <pattern id=\"";
        d->defs += imagename;
        d->defs += "_ref\"\n      ";
        d->defs += xywh;
        d->defs += "\n       patternUnits=\"userSpaceOnUse\"";
        d->defs += " >\n";
        d->defs += "      <use id=\"";
        d->defs += imagename;
        d->defs += "_ign\" ";
        d->defs += " xlink:href=\"#";
        d->defs += imagename;
        d->defs += "\" />\n";
        d->defs += "    ";
        d->defs += "   </pattern>\n";
    }
    g_free(base64String);//wait until this point to free because it might be a duplicate image

    /*  image allows the inner image to be rotated nicely, load this one second only if needed
        imagename retained from above
        Here comes a dreadful hack.  How do we determine if this rotation of the base image has already
        been loaded?  The image names contain no identifying information, they are just numbered sequentially.
        So the rotated name is EMFrotimage###_XXXXXX, where ### is the number of the referred to image, and
        XXXX is the rotation in radians x 1000000 and truncated.  That is then stored in BASE64 as the "image".
        The corresponding SVG generated though is not for an image, but a reference to an image.
        The name of the pattern MUST stil be EMFimage###_ref or output_style() will not be able to use it.
    */
    if(current_rotation(d) >= 0.00001 || current_rotation(d) <= -0.00001){ /* some rotation, allow a little rounding error around 0 degrees */
        int tangle = round(current_rotation(d)*1000000.0);
        sprintf(imrotname,"EMFrotimage%d_%d",idx-1,tangle);
        base64String = g_base64_encode((guchar*) imrotname, strlen(imrotname) );
        idx = in_images(d, (char *) base64String); // scan for this "image"
        if(!idx){
            if(d->images.count == d->images.size){  enlarge_images(d); }
            idx = d->images.count;
            d->images.strings[d->images.count++]=strdup(base64String);
            sprintf(imrotname,"EMFimage%d",idx++);

            d->defs += "\n";
            d->defs += "   <pattern\n";
            d->defs += "       id=\"";
            d->defs += imrotname;
            d->defs += "_ref\"\n";
            d->defs += "       xlink:href=\"#";
            d->defs += imagename;
            d->defs += "_ref\"\n";
            d->defs += "       patternTransform=";
            d->defs += current_matrix(d, 0.0, 0.0, 0); //j use offset 0,0
            d->defs += " />\n";
        }
        g_free(base64String);
    }

    return(idx-1);
}

/*  Add another 100 blank slots to the gradients array.
*/
void Emf::enlarge_gradients(PEMF_CALLBACK_DATA d){
    d->gradients.size += 100;
    d->gradients.strings = (char **) realloc(d->gradients.strings,d->gradients.size * sizeof(char *));
}

/*  See if the gradient name is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Emf::in_gradients(PEMF_CALLBACK_DATA d, const char *test){
    int i;
    for(i=0; i<d->gradients.count; i++){
        if(strcmp(test,d->gradients.strings[i])==0)return(i+1);
    }
    return(0);
}

U_COLORREF trivertex_to_colorref(U_TRIVERTEX tv){
    U_COLORREF uc;
    uc.Red       = tv.Red   >> 8;            
    uc.Green     = tv.Green >> 8;          
    uc.Blue      = tv.Blue  >> 8;           
    uc.Reserved  = tv.Alpha >> 8;          // Not used
    return(uc);
}

/*  (Conditionally) add a gradient.  If a matching gradient already exists nothing happens.  If one
    does not exist it is added to the gradients list and also entered into <defs>.
    Only call this with H or V gradient, not a triangle.
*/
uint32_t Emf::add_gradient(PEMF_CALLBACK_DATA d, uint32_t gradientType, U_TRIVERTEX tv1, U_TRIVERTEX tv2){
    char hgradname[64];    // big enough
    char tmpcolor1[8];
    char tmpcolor2[8];
    char gradc;
    uint32_t idx;
    std::string x2,y2;
    
    U_COLORREF gradientColor1 = trivertex_to_colorref(tv1);
    U_COLORREF gradientColor2 = trivertex_to_colorref(tv2);


    sprintf(tmpcolor1,"%6.6X",sethexcolor(gradientColor1));
    sprintf(tmpcolor2,"%6.6X",sethexcolor(gradientColor2));
    switch(gradientType){
        case U_GRADIENT_FILL_RECT_H:
            gradc='H';
            x2="100";
            y2="0";
            break;
        case U_GRADIENT_FILL_RECT_V:
            gradc='V';
            x2="0";
            y2="100";
            break;
        default: // this should never happen, but fill these in to avoid compiler warnings
            gradc='!';
            x2="0";
            y2="0";
            break;
    }

    /*  Even though the gradient was defined as Horizontal or Vertical if the rectangle is rotated it needs to
        be at some other alignment, and that needs gradienttransform.   Set the name using the same sort of hack
        as for add_image.
    */
    int tangle = round(current_rotation(d)*1000000.0);
    sprintf(hgradname,"LinGrd%c_%s_%s_%d",gradc,tmpcolor1,tmpcolor2,tangle);
    
    idx = in_gradients(d,hgradname);
    if(!idx){ // gradient does not yet exist
        if(d->gradients.count == d->gradients.size){  enlarge_gradients(d); }
        d->gradients.strings[d->gradients.count++]=strdup(hgradname);
        idx = d->gradients.count;
        SVGOStringStream stmp;
        stmp <<  "   <linearGradient id=\"";
        stmp <<  hgradname;
        stmp << "\" x1=\"";
        stmp << pix_to_x_point(d, tv1.x , tv1.y);
        stmp << "\" y1=\"";
        stmp << pix_to_y_point(d, tv1.x , tv1.y);
        stmp << "\" x2=\"";
        if(gradc=='H'){  // UR corner
            stmp << pix_to_x_point(d, tv2.x , tv1.y);
            stmp << "\" y2=\"";
            stmp << pix_to_y_point(d, tv2.x , tv1.y);
        }
        else {  // LL corner
            stmp << pix_to_x_point(d, tv1.x , tv2.y);
            stmp << "\" y2=\"";
            stmp << pix_to_y_point(d, tv1.x , tv2.y);
        }
        stmp << "\" gradientTransform=\"(1,0,0,1,0,0)\"";
        stmp << " gradientUnits=\"userSpaceOnUse\"\n";
        stmp << ">\n";
        stmp << "      <stop offset=\"0\" style=\"stop-color:#";
        stmp << tmpcolor1;
        stmp << ";stop-opacity:1\" />\n";
        stmp << "      <stop offset=\"1\" style=\"stop-color:#";
        stmp << tmpcolor2;
        stmp << ";stop-opacity:1\" />\n";
        stmp << "   </linearGradient>\n";
        d->defs += stmp.str().c_str();
    }

    return(idx-1);
}

/*  Add another 100 blank slots to the clips array.
*/
void Emf::enlarge_clips(PEMF_CALLBACK_DATA d){
    d->clips.size += 100;
    d->clips.strings = (char **) realloc(d->clips.strings,d->clips.size * sizeof(char *));
}

/*  See if the pattern name is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Emf::in_clips(PEMF_CALLBACK_DATA d, const char *test){
    int i;
    for(i=0; i<d->clips.count; i++){
        if(strcmp(test,d->clips.strings[i])==0)return(i+1);
    }
    return(0);
}

/*  (Conditionally) add a clip.  
    If a matching clip already exists nothing happens  
    If one does exist it is added to the clips list, entered into <defs>.
*/
void Emf::add_clips(PEMF_CALLBACK_DATA d, const char *clippath, unsigned int logic){
    int op = combine_ops_to_livarot(logic);
    Geom::PathVector combined_vect;
    char *combined = NULL;
    if (op >= 0 && d->dc[d->level].clip_id) {
        unsigned int real_idx = d->dc[d->level].clip_id - 1;
        Geom::PathVector old_vect = sp_svg_read_pathv(d->clips.strings[real_idx]);
        Geom::PathVector new_vect = sp_svg_read_pathv(clippath);
        combined_vect = sp_pathvector_boolop(new_vect, old_vect, (bool_op) op , (FillRule) fill_oddEven, (FillRule) fill_oddEven);
        combined = sp_svg_write_path(combined_vect);
    }
    else {
        combined = strdup(clippath);  // COPY operation, erases everything and starts a new one
    }

    uint32_t  idx = in_clips(d, combined);
    if(!idx){  // add clip if not already present
        if(d->clips.count == d->clips.size){  enlarge_clips(d); }
        d->clips.strings[d->clips.count++]=strdup(combined);
        d->dc[d->level].clip_id = d->clips.count;  // one more than the slot where it is actually stored
        SVGOStringStream tmp_clippath;
        tmp_clippath << "\n<clipPath";
        tmp_clippath << "\n\tclipPathUnits=\"userSpaceOnUse\" ";
        tmp_clippath << "\n\tid=\"clipEmfPath" << d->dc[d->level].clip_id << "\"";
        tmp_clippath << " >";
        tmp_clippath << "\n\t<path d=\"";
        tmp_clippath << combined;
        tmp_clippath << "\"";
        tmp_clippath << "\n\t/>";
        tmp_clippath << "\n</clipPath>";
        d->outdef += tmp_clippath.str().c_str();
    }
    else {
        d->dc[d->level].clip_id = idx;
    }
    free(combined);
}



void
Emf::output_style(PEMF_CALLBACK_DATA d, int iType)
{
//    SVGOStringStream tmp_id;
    SVGOStringStream tmp_style;
    char tmp[1024] = {0};

    float fill_rgb[3];
    sp_color_get_rgb_floatv( &(d->dc[d->level].style.fill.value.color), fill_rgb );
    float stroke_rgb[3];
    sp_color_get_rgb_floatv(&(d->dc[d->level].style.stroke.value.color), stroke_rgb);

    // for U_EMR_BITBLT with no image, try to approximate some of these operations/
    // Assume src color is "white"
    if(d->dwRop3){
        switch(d->dwRop3){
            case U_PATINVERT: // invert pattern
                fill_rgb[0] = 1.0 - fill_rgb[0];
                fill_rgb[1] = 1.0 - fill_rgb[1];
                fill_rgb[2] = 1.0 - fill_rgb[2];
                break;
            case U_SRCINVERT: // treat all of these as black
            case U_DSTINVERT:
            case U_BLACKNESS:
            case U_SRCERASE:
            case U_NOTSRCCOPY:
                fill_rgb[0]=fill_rgb[1]=fill_rgb[2]=0.0;
                break;
            case U_SRCCOPY:    // treat all of these as white
            case U_NOTSRCERASE:
            case U_WHITENESS:
                fill_rgb[0]=fill_rgb[1]=fill_rgb[2]=1.0;
                break;
            case U_SRCPAINT:  // use the existing color
            case U_SRCAND:
            case U_MERGECOPY:
            case U_MERGEPAINT:
            case U_PATPAINT:
            case U_PATCOPY:
            default:
                break;
        }
        d->dwRop3 = 0;  // might as well reset it here, it must be set for each BITBLT
    }

    // Implement some of these, the ones where the original screen color does not matter.
    // The options that merge screen and pen colors cannot be done correctly because we
    // have no way of knowing what color is already on the screen. For those just pass the
    // pen color through.
    switch(d->dwRop2){
        case U_R2_BLACK:
            fill_rgb[0]  = fill_rgb[1]  = fill_rgb[2]   = 0.0;
            stroke_rgb[0]= stroke_rgb[1]= stroke_rgb[2] = 0.0;
            break;
        case U_R2_NOTMERGEPEN:
        case U_R2_MASKNOTPEN:
            break;
        case U_R2_NOTCOPYPEN:
            fill_rgb[0]    =  1.0 - fill_rgb[0];
            fill_rgb[1]    =  1.0 - fill_rgb[1];
            fill_rgb[2]    =  1.0 - fill_rgb[2];
            stroke_rgb[0]  =  1.0 - stroke_rgb[0];
            stroke_rgb[1]  =  1.0 - stroke_rgb[1];
            stroke_rgb[2]  =  1.0 - stroke_rgb[2];
            break;
        case U_R2_MASKPENNOT:
        case U_R2_NOT:
        case U_R2_XORPEN:
        case U_R2_NOTMASKPEN:
        case U_R2_NOTXORPEN:
        case U_R2_NOP:
        case U_R2_MERGENOTPEN:
        case U_R2_COPYPEN:
        case U_R2_MASKPEN:
        case U_R2_MERGEPENNOT:
        case U_R2_MERGEPEN:
            break;
        case U_R2_WHITE:
            fill_rgb[0]  = fill_rgb[1]  = fill_rgb[2]   = 1.0;
            stroke_rgb[0]= stroke_rgb[1]= stroke_rgb[2] = 1.0;
            break;
        default:
            break;
    }


//    tmp_id << "\n\tid=\"" << (d->id++) << "\"";
//    d->outsvg += tmp_id.str().c_str();
    d->outsvg += "\n\tstyle=\"";
    if (iType == U_EMR_STROKEPATH || !d->dc[d->level].fill_set) {
        tmp_style << "fill:none;";
    } else {
        switch(d->dc[d->level].fill_mode){
            // both of these use the url(#) method
            case DRAW_PATTERN:
                snprintf(tmp, 1023, "fill:url(#%s); ",d->hatches.strings[d->dc[d->level].fill_idx]);
                tmp_style << tmp;
                break;
             case DRAW_IMAGE:
                snprintf(tmp, 1023, "fill:url(#EMFimage%d_ref); ",d->dc[d->level].fill_idx);
                tmp_style << tmp;
                break;
            case DRAW_LINEAR_GRADIENT:
            case DRAW_PAINT:
            default:  // <--  this should never happen, but just in case...
                snprintf(
                    tmp, 1023,
                    "fill:#%02x%02x%02x;",
                    SP_COLOR_F_TO_U(fill_rgb[0]),
                    SP_COLOR_F_TO_U(fill_rgb[1]),
                    SP_COLOR_F_TO_U(fill_rgb[2])
                );
                tmp_style << tmp;
                break;
        }
        snprintf(
            tmp, 1023,
            "fill-rule:%s;",
            (d->dc[d->level].style.fill_rule.value == 0 ? "evenodd" : "nonzero")
        );
        tmp_style << tmp;
        tmp_style << "fill-opacity:1;";

        // if the stroke is the same as the fill, and the right size not to change the end size of the object, do not do it separately
        if(
            (d->dc[d->level].fill_set                                )  &&
            (d->dc[d->level].stroke_set                              )  &&
            (d->dc[d->level].style.stroke_width.value == 1           )  &&
            (d->dc[d->level].fill_mode == d->dc[d->level].stroke_mode)  &&
            (
                (d->dc[d->level].fill_mode != DRAW_PAINT)               ||
                (
                    (fill_rgb[0]==stroke_rgb[0])                        &&
                    (fill_rgb[1]==stroke_rgb[1])                        &&
                    (fill_rgb[2]==stroke_rgb[2])
                )
            )
        ){
            d->dc[d->level].stroke_set = false;
        }
    }

    if (iType == U_EMR_FILLPATH || !d->dc[d->level].stroke_set) {
        tmp_style << "stroke:none;";
    } else {
        switch(d->dc[d->level].stroke_mode){
            // both of these use the url(#) method
            case DRAW_PATTERN:
                snprintf(tmp, 1023, "stroke:url(#%s); ",d->hatches.strings[d->dc[d->level].stroke_idx]);
                tmp_style << tmp;
                break;
            case DRAW_IMAGE:
                snprintf(tmp, 1023, "stroke:url(#EMFimage%d_ref); ",d->dc[d->level].stroke_idx);
                tmp_style << tmp;
                break;
            case DRAW_LINEAR_GRADIENT:
            case DRAW_PAINT:
            default:  // <--  this should never happen, but just in case...
                snprintf(
                    tmp, 1023,
                    "stroke:#%02x%02x%02x;",
                    SP_COLOR_F_TO_U(stroke_rgb[0]),
                    SP_COLOR_F_TO_U(stroke_rgb[1]),
                    SP_COLOR_F_TO_U(stroke_rgb[2])
                );
                tmp_style << tmp;
                break;
        }
        tmp_style << "stroke-width:" <<
            MAX( 0.001, d->dc[d->level].style.stroke_width.value ) << "px;";

        tmp_style << "stroke-linecap:" <<
            (
                d->dc[d->level].style.stroke_linecap.computed == 0 ? "butt" :
                d->dc[d->level].style.stroke_linecap.computed == 1 ? "round" :
                d->dc[d->level].style.stroke_linecap.computed == 2 ? "square" :
                "unknown"
            ) << ";";

        tmp_style << "stroke-linejoin:" <<
            (
                d->dc[d->level].style.stroke_linejoin.computed == 0 ? "miter" :
                d->dc[d->level].style.stroke_linejoin.computed == 1 ? "round" :
                d->dc[d->level].style.stroke_linejoin.computed == 2 ? "bevel" :
                "unknown"
            ) << ";";

        // Set miter limit if known, even if it is not needed immediately (not miter)
        tmp_style << "stroke-miterlimit:" <<
            MAX( 2.0, d->dc[d->level].style.stroke_miterlimit.value ) << ";";

        if (d->dc[d->level].style.stroke_dasharray.set &&
            !d->dc[d->level].style.stroke_dasharray.values.empty() )
        {
            tmp_style << "stroke-dasharray:";
            for (unsigned i=0; i<d->dc[d->level].style.stroke_dasharray.values.size(); i++) {
                if (i)
                    tmp_style << ",";
                tmp_style << d->dc[d->level].style.stroke_dasharray.values[i];
            }
            tmp_style << ";";
            tmp_style << "stroke-dashoffset:0;";
        } else {
            tmp_style << "stroke-dasharray:none;";
        }
        tmp_style << "stroke-opacity:1;";
    }
    tmp_style << "\" ";
    if (d->dc[d->level].clip_id)
        tmp_style << "\n\tclip-path=\"url(#clipEmfPath" << d->dc[d->level].clip_id << ")\" ";

    d->outsvg += tmp_style.str().c_str();
}


double
Emf::_pix_x_to_point(PEMF_CALLBACK_DATA d, double px)
{
    double scale = (d->dc[d->level].ScaleInX ? d->dc[d->level].ScaleInX : 1.0);
    double tmp;
    tmp = ((((double) (px - d->dc[d->level].winorg.x))*scale) + d->dc[d->level].vieworg.x) * d->D2PscaleX;
    tmp -= d->ulCornerOutX; //The EMF boundary rectangle can be anywhere, place its upper left corner in the Inkscape upper left corner
    return(tmp);
}

double
Emf::_pix_y_to_point(PEMF_CALLBACK_DATA d, double py)
{
    double scale = (d->dc[d->level].ScaleInY ? d->dc[d->level].ScaleInY : 1.0);
    double tmp;
    tmp = ((((double) (py - d->dc[d->level].winorg.y))*scale) * d->E2IdirY + d->dc[d->level].vieworg.y) * d->D2PscaleY;
    tmp -= d->ulCornerOutY; //The EMF boundary rectangle can be anywhere, place its upper left corner in the Inkscape upper left corner
    return(tmp);
}


double
Emf::pix_to_x_point(PEMF_CALLBACK_DATA d, double px, double py)
{
    double wpx = px * d->dc[d->level].worldTransform.eM11 + py * d->dc[d->level].worldTransform.eM21 + d->dc[d->level].worldTransform.eDx;
    double x   = _pix_x_to_point(d, wpx);

    return x;
}

double
Emf::pix_to_y_point(PEMF_CALLBACK_DATA d, double px, double py)
{

    double wpy = px * d->dc[d->level].worldTransform.eM12 + py * d->dc[d->level].worldTransform.eM22 + d->dc[d->level].worldTransform.eDy;
    double y   = _pix_y_to_point(d, wpy);

    return y;

}

double
Emf::pix_to_abs_size(PEMF_CALLBACK_DATA d, double px)
{
    double  ppx = fabs(px * (d->dc[d->level].ScaleInX ? d->dc[d->level].ScaleInX : 1.0) * d->D2PscaleX * current_scale(d));
    return ppx;
}

/* snaps coordinate pairs made up of values near +/-faraway, +/-faraway to exactly faraway.
   This eliminates coordinate drift on repeated clipping cycles which use exclude.  
   It should not affect internals of normal drawings because the value of faraway is so large.
*/
void
Emf::snap_to_faraway_pair(double *x, double *y)
{
    if((abs(abs(*x) - faraway)/faraway <= 1e-4) && (abs(abs(*y) - faraway)/faraway <= 1e-4)){
        *x = (*x > 0 ? faraway : -faraway);
        *y = (*y > 0 ? faraway : -faraway);
    }
}

/* returns "x,y" (without the quotes) in inkscape coordinates for a pair of EMF x,y coordinates.
   Since exclude clip can go through here, it calls snap_to_faraway_pair for numerical stability.
*/
std::string Emf::pix_to_xy(PEMF_CALLBACK_DATA d, double x, double y){
    SVGOStringStream cxform;
    double tx = pix_to_x_point(d,x,y);
    double ty = pix_to_y_point(d,x,y);
    snap_to_faraway_pair(&tx,&ty);
    cxform << tx;
    cxform << ",";
    cxform << ty;
    return(cxform.str());
}


void
Emf::select_pen(PEMF_CALLBACK_DATA d, int index)
{
    PU_EMRCREATEPEN pEmr = NULL;

    if (index >= 0 && index < d->n_obj){
        pEmr = (PU_EMRCREATEPEN) d->emf_obj[index].lpEMFR;
    }

    if (!pEmr){ return; }

    switch (pEmr->lopn.lopnStyle & U_PS_STYLE_MASK) {
        case U_PS_DASH:
        case U_PS_DOT:
        case U_PS_DASHDOT:
        case U_PS_DASHDOTDOT:
        {
            int penstyle = (pEmr->lopn.lopnStyle & U_PS_STYLE_MASK);
            if (!d->dc[d->level].style.stroke_dasharray.values.empty() && (d->level==0 || (d->level>0 && d->dc[d->level].style.stroke_dasharray.values!=d->dc[d->level-1].style.stroke_dasharray.values)))
                d->dc[d->level].style.stroke_dasharray.values.clear();
            if (penstyle==U_PS_DASH || penstyle==U_PS_DASHDOT || penstyle==U_PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dasharray.values.push_back( 3 );
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
            }
            if (penstyle==U_PS_DOT || penstyle==U_PS_DASHDOT || penstyle==U_PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
            }
            if (penstyle==U_PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
            }

            d->dc[d->level].style.stroke_dasharray.set = 1;
            break;
        }

        case U_PS_SOLID:
        default:
        {
            d->dc[d->level].style.stroke_dasharray.set = 0;
            break;
        }
    }

    switch (pEmr->lopn.lopnStyle & U_PS_ENDCAP_MASK) {
        case U_PS_ENDCAP_ROUND: {  d->dc[d->level].style.stroke_linecap.computed = 1;   break; }
        case U_PS_ENDCAP_SQUARE: { d->dc[d->level].style.stroke_linecap.computed = 2;   break; }
        case U_PS_ENDCAP_FLAT:
        default: {                 d->dc[d->level].style.stroke_linecap.computed = 0;   break; }
    }

    switch (pEmr->lopn.lopnStyle & U_PS_JOIN_MASK) {
        case U_PS_JOIN_BEVEL: {    d->dc[d->level].style.stroke_linejoin.computed = 2;  break; }
        case U_PS_JOIN_MITER: {    d->dc[d->level].style.stroke_linejoin.computed = 0;  break; }
        case U_PS_JOIN_ROUND:
        default: {                 d->dc[d->level].style.stroke_linejoin.computed = 1;  break; }
    }

    d->dc[d->level].stroke_set = true;

    if (pEmr->lopn.lopnStyle == U_PS_NULL) {
        d->dc[d->level].style.stroke_width.value = 0;
        d->dc[d->level].stroke_set = false;
    } else if (pEmr->lopn.lopnWidth.x) {
        int cur_level = d->level;
        d->level = d->emf_obj[index].level;
        double pen_width = pix_to_abs_size( d, pEmr->lopn.lopnWidth.x );
        d->level = cur_level;
        d->dc[d->level].style.stroke_width.value = pen_width;
    } else { // this stroke should always be rendered as 1 pixel wide, independent of zoom level (can that be done in SVG?)
        //d->dc[d->level].style.stroke_width.value = 1.0;
        int cur_level = d->level;
        d->level = d->emf_obj[index].level;
        double pen_width = pix_to_abs_size( d, 1 );
        d->level = cur_level;
        d->dc[d->level].style.stroke_width.value = pen_width;
    }

    double r, g, b;
    r = SP_COLOR_U_TO_F( U_RGBAGetR(pEmr->lopn.lopnColor) );
    g = SP_COLOR_U_TO_F( U_RGBAGetG(pEmr->lopn.lopnColor) );
    b = SP_COLOR_U_TO_F( U_RGBAGetB(pEmr->lopn.lopnColor) );
    d->dc[d->level].style.stroke.value.color.set( r, g, b );
}


void
Emf::select_extpen(PEMF_CALLBACK_DATA d, int index)
{
    PU_EMREXTCREATEPEN pEmr = NULL;

    if (index >= 0 && index < d->n_obj)
        pEmr = (PU_EMREXTCREATEPEN) d->emf_obj[index].lpEMFR;

    if (!pEmr)
        return;

    switch (pEmr->elp.elpPenStyle & U_PS_STYLE_MASK) {
        case U_PS_USERSTYLE:
        {
            if (pEmr->elp.elpNumEntries) {
                if (!d->dc[d->level].style.stroke_dasharray.values.empty() && (d->level==0 || (d->level>0 && d->dc[d->level].style.stroke_dasharray.values!=d->dc[d->level-1].style.stroke_dasharray.values)))
                    d->dc[d->level].style.stroke_dasharray.values.clear();
                for (unsigned int i=0; i<pEmr->elp.elpNumEntries; i++) {
//  Doing it this way typically results in a pattern that is tiny, better to assume the array
//  is the same scale as for dot/dash below, that is, no scaling should be applied
//                    double dash_length = pix_to_abs_size( d, pEmr->elp.elpStyleEntry[i] );
                    double dash_length = pEmr->elp.elpStyleEntry[i];
                    d->dc[d->level].style.stroke_dasharray.values.push_back(dash_length);
                }
                d->dc[d->level].style.stroke_dasharray.set = 1;
            } else {
                d->dc[d->level].style.stroke_dasharray.set = 0;
            }
            break;
        }

        case U_PS_DASH:
        case U_PS_DOT:
        case U_PS_DASHDOT:
        case U_PS_DASHDOTDOT:
        {
            int penstyle = (pEmr->elp.elpPenStyle & U_PS_STYLE_MASK);
            if (!d->dc[d->level].style.stroke_dasharray.values.empty() && (d->level==0 || (d->level>0 && d->dc[d->level].style.stroke_dasharray.values!=d->dc[d->level-1].style.stroke_dasharray.values)))
                d->dc[d->level].style.stroke_dasharray.values.clear();
            if (penstyle==U_PS_DASH || penstyle==U_PS_DASHDOT || penstyle==U_PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dasharray.values.push_back( 3 );
                d->dc[d->level].style.stroke_dasharray.values.push_back( 2 );
            }
            if (penstyle==U_PS_DOT || penstyle==U_PS_DASHDOT || penstyle==U_PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
                d->dc[d->level].style.stroke_dasharray.values.push_back( 2 );
            }
            if (penstyle==U_PS_DASHDOTDOT) {
                d->dc[d->level].style.stroke_dasharray.values.push_back( 1 );
                d->dc[d->level].style.stroke_dasharray.values.push_back( 2 );
            }

            d->dc[d->level].style.stroke_dasharray.set = 1;
            break;
        }
        case U_PS_SOLID:
/* includes these for now, some should maybe not be in here
        case U_PS_NULL:
        case  U_PS_INSIDEFRAME:
        case  U_PS_ALTERNATE:
        case  U_PS_STYLE_MASK:
*/
        default:
        {
            d->dc[d->level].style.stroke_dasharray.set = 0;
            break;
        }
    }

    switch (pEmr->elp.elpPenStyle & U_PS_ENDCAP_MASK) {
        case U_PS_ENDCAP_ROUND:
        {
            d->dc[d->level].style.stroke_linecap.computed = 1;
            break;
        }
        case U_PS_ENDCAP_SQUARE:
        {
            d->dc[d->level].style.stroke_linecap.computed = 2;
            break;
        }
        case U_PS_ENDCAP_FLAT:
        default:
        {
            d->dc[d->level].style.stroke_linecap.computed = 0;
            break;
        }
    }

    switch (pEmr->elp.elpPenStyle & U_PS_JOIN_MASK) {
        case U_PS_JOIN_BEVEL:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 2;
            break;
        }
        case U_PS_JOIN_MITER:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 0;
            break;
        }
        case U_PS_JOIN_ROUND:
        default:
        {
            d->dc[d->level].style.stroke_linejoin.computed = 1;
            break;
        }
    }

    d->dc[d->level].stroke_set = true;

    if (pEmr->elp.elpPenStyle == U_PS_NULL) { // draw nothing, but fill out all the values with something
        double r, g, b;
        r = SP_COLOR_U_TO_F( U_RGBAGetR(d->dc[d->level].textColor));
        g = SP_COLOR_U_TO_F( U_RGBAGetG(d->dc[d->level].textColor));
        b = SP_COLOR_U_TO_F( U_RGBAGetB(d->dc[d->level].textColor));
        d->dc[d->level].style.stroke.value.color.set( r, g, b );
        d->dc[d->level].style.stroke_width.value = 0;
        d->dc[d->level].stroke_set = false;
        d->dc[d->level].stroke_mode = DRAW_PAINT;
    }
    else {
        if (pEmr->elp.elpWidth) {
            int cur_level = d->level;
            d->level = d->emf_obj[index].level;
            double pen_width = pix_to_abs_size( d, pEmr->elp.elpWidth );
            d->level = cur_level;
            d->dc[d->level].style.stroke_width.value = pen_width;
        } else { // this stroke should always be rendered as 1 pixel wide, independent of zoom level (can that be done in SVG?)
            //d->dc[d->level].style.stroke_width.value = 1.0;
            int cur_level = d->level;
            d->level = d->emf_obj[index].level;
            double pen_width = pix_to_abs_size( d, 1 );
            d->level = cur_level;
            d->dc[d->level].style.stroke_width.value = pen_width;
        }

        if(     pEmr->elp.elpBrushStyle == U_BS_SOLID){
            double r, g, b;
            r = SP_COLOR_U_TO_F( U_RGBAGetR(pEmr->elp.elpColor) );
            g = SP_COLOR_U_TO_F( U_RGBAGetG(pEmr->elp.elpColor) );
            b = SP_COLOR_U_TO_F( U_RGBAGetB(pEmr->elp.elpColor) );
            d->dc[d->level].style.stroke.value.color.set( r, g, b );
            d->dc[d->level].stroke_mode = DRAW_PAINT;
            d->dc[d->level].stroke_set  = true;
        }
        else if(pEmr->elp.elpBrushStyle == U_BS_HATCHED){
            d->dc[d->level].stroke_idx    = add_hatch(d, pEmr->elp.elpHatch, pEmr->elp.elpColor);
            d->dc[d->level].stroke_recidx = index; // used if the hatch needs to be redone due to bkMode, textmode, etc. changes
            d->dc[d->level].stroke_mode   = DRAW_PATTERN;
            d->dc[d->level].stroke_set    = true;
        }
        else if(pEmr->elp.elpBrushStyle == U_BS_DIBPATTERN || pEmr->elp.elpBrushStyle == U_BS_DIBPATTERNPT){
            d->dc[d->level].stroke_idx  = add_image(d, (void *)pEmr, pEmr->cbBits, pEmr->cbBmi, *(uint32_t *) &(pEmr->elp.elpColor), pEmr->offBits, pEmr->offBmi);
            d->dc[d->level].stroke_mode = DRAW_IMAGE;
            d->dc[d->level].stroke_set  = true;
        }
        else { // U_BS_PATTERN and anything strange that falls in, stroke is solid textColor
            double r, g, b;
            r = SP_COLOR_U_TO_F( U_RGBAGetR(d->dc[d->level].textColor));
            g = SP_COLOR_U_TO_F( U_RGBAGetG(d->dc[d->level].textColor));
            b = SP_COLOR_U_TO_F( U_RGBAGetB(d->dc[d->level].textColor));
            d->dc[d->level].style.stroke.value.color.set( r, g, b );
            d->dc[d->level].stroke_mode = DRAW_PAINT;
            d->dc[d->level].stroke_set  = true;
        }
    }
}


void
Emf::select_brush(PEMF_CALLBACK_DATA d, int index)
{
    uint32_t                          tidx;
    uint32_t                          iType;

    if (index >= 0 && index < d->n_obj){
        iType = ((PU_EMR) (d->emf_obj[index].lpEMFR))->iType;
        if(iType == U_EMR_CREATEBRUSHINDIRECT){
            PU_EMRCREATEBRUSHINDIRECT pEmr = (PU_EMRCREATEBRUSHINDIRECT) d->emf_obj[index].lpEMFR;
            if(     pEmr->lb.lbStyle == U_BS_SOLID){
                double r, g, b;
                r = SP_COLOR_U_TO_F( U_RGBAGetR(pEmr->lb.lbColor) );
                g = SP_COLOR_U_TO_F( U_RGBAGetG(pEmr->lb.lbColor) );
                b = SP_COLOR_U_TO_F( U_RGBAGetB(pEmr->lb.lbColor) );
                d->dc[d->level].style.fill.value.color.set( r, g, b );
                d->dc[d->level].fill_mode    = DRAW_PAINT;
                d->dc[d->level].fill_set     = true;
            }
            else if(pEmr->lb.lbStyle == U_BS_HATCHED){
                d->dc[d->level].fill_idx     = add_hatch(d, pEmr->lb.lbHatch, pEmr->lb.lbColor);
                d->dc[d->level].fill_recidx  = index; // used if the hatch needs to be redone due to bkMode, textmode, etc. changes
                d->dc[d->level].fill_mode    = DRAW_PATTERN;
                d->dc[d->level].fill_set     = true;
            }
        }
        else if(iType == U_EMR_CREATEDIBPATTERNBRUSHPT || iType == U_EMR_CREATEMONOBRUSH){
            PU_EMRCREATEDIBPATTERNBRUSHPT pEmr = (PU_EMRCREATEDIBPATTERNBRUSHPT) d->emf_obj[index].lpEMFR;
            tidx = add_image(d, (void *) pEmr, pEmr->cbBits, pEmr->cbBmi, pEmr->iUsage, pEmr->offBits, pEmr->offBmi);
            if(tidx == U_EMR_INVALID){  // This happens if createmonobrush has a DIB that isn't monochrome
                double r, g, b;
                r = SP_COLOR_U_TO_F( U_RGBAGetR(d->dc[d->level].textColor));
                g = SP_COLOR_U_TO_F( U_RGBAGetG(d->dc[d->level].textColor));
                b = SP_COLOR_U_TO_F( U_RGBAGetB(d->dc[d->level].textColor));
                d->dc[d->level].style.fill.value.color.set( r, g, b );
                d->dc[d->level].fill_mode = DRAW_PAINT;
            }
            else {
                d->dc[d->level].fill_idx  = tidx;
                d->dc[d->level].fill_mode = DRAW_IMAGE;
            }
            d->dc[d->level].fill_set = true;
        }
    }
}


void
Emf::select_font(PEMF_CALLBACK_DATA d, int index)
{
    PU_EMREXTCREATEFONTINDIRECTW pEmr = NULL;

    if (index >= 0 && index < d->n_obj)
        pEmr = (PU_EMREXTCREATEFONTINDIRECTW) d->emf_obj[index].lpEMFR;

    if (!pEmr)return;


    /*  The logfont information always starts with a U_LOGFONT structure but the U_EMREXTCREATEFONTINDIRECTW
        is defined as U_LOGFONT_PANOSE so it can handle one of those if that is actually present. Currently only logfont
        is supported, and the remainder, it it really is a U_LOGFONT_PANOSE record, is ignored
    */
    int cur_level = d->level;
    d->level = d->emf_obj[index].level;
    double font_size = pix_to_abs_size( d, pEmr->elfw.elfLogFont.lfHeight );
    /*  snap the font_size to the nearest 1/32nd of a point.
        (The size is converted from Pixels to points, snapped, and converted back.)
        See the notes where d->D2Pscale[XY] are set for the reason why.
        Typically this will set the font to the desired exact size.  If some peculiar size
        was intended this will, at worst, make it .03125 off, which is unlikely to be a problem. */
    font_size = round(20.0 * 0.8 * font_size)/(20.0 * 0.8);
    d->level = cur_level;
    d->dc[d->level].style.font_size.computed = font_size;
    d->dc[d->level].style.font_weight.value =
        pEmr->elfw.elfLogFont.lfWeight == U_FW_THIN ? SP_CSS_FONT_WEIGHT_100 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_EXTRALIGHT ? SP_CSS_FONT_WEIGHT_200 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_LIGHT ? SP_CSS_FONT_WEIGHT_300 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_NORMAL ? SP_CSS_FONT_WEIGHT_400 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_MEDIUM ? SP_CSS_FONT_WEIGHT_500 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_SEMIBOLD ? SP_CSS_FONT_WEIGHT_600 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_BOLD ? SP_CSS_FONT_WEIGHT_700 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_EXTRABOLD ? SP_CSS_FONT_WEIGHT_800 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_HEAVY ? SP_CSS_FONT_WEIGHT_900 :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_NORMAL ? SP_CSS_FONT_WEIGHT_NORMAL :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_BOLD ? SP_CSS_FONT_WEIGHT_BOLD :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_EXTRALIGHT ? SP_CSS_FONT_WEIGHT_LIGHTER :
        pEmr->elfw.elfLogFont.lfWeight == U_FW_EXTRABOLD ? SP_CSS_FONT_WEIGHT_BOLDER :
        U_FW_NORMAL;
    d->dc[d->level].style.font_style.value = (pEmr->elfw.elfLogFont.lfItalic ? SP_CSS_FONT_STYLE_ITALIC : SP_CSS_FONT_STYLE_NORMAL);
    d->dc[d->level].style.text_decoration_line.underline    = pEmr->elfw.elfLogFont.lfUnderline;
    d->dc[d->level].style.text_decoration_line.line_through = pEmr->elfw.elfLogFont.lfStrikeOut;
    d->dc[d->level].style.text_decoration_line.set          = true;
    d->dc[d->level].style.text_decoration_line.inherit      = false;
    // malformed  EMF with empty filename may exist, ignore font change if encountered
    char *ctmp = U_Utf16leToUtf8((uint16_t *) (pEmr->elfw.elfLogFont.lfFaceName), U_LF_FACESIZE, NULL);
    if(ctmp){
        if (d->dc[d->level].font_name){ free(d->dc[d->level].font_name); }
        if(*ctmp){
            d->dc[d->level].font_name = ctmp;
        }
        else {  // Malformed EMF might specify an empty font name
            free(ctmp);
            d->dc[d->level].font_name = strdup("Arial");  // Default font, EMF spec says device can pick whatever it wants
        }
    }
    d->dc[d->level].style.baseline_shift.value = round((double)((pEmr->elfw.elfLogFont.lfEscapement + 3600) % 3600)) / 10.0;   // use baseline_shift instead of text_transform to avoid overflow
}

void
Emf::delete_object(PEMF_CALLBACK_DATA d, int index)
{
    if (index >= 0 && index < d->n_obj) {
        d->emf_obj[index].type = 0;
// We are keeping a copy of the EMR rather than just a structure.  Currently that is not necessary as the entire
// EMF is read in at once and is stored in a big malloc.  However, in past versions it was handled
// reord by record, and we might need to do that again at some point in the future if we start running into EMF
// files too big to fit into memory.
        if (d->emf_obj[index].lpEMFR)
            free(d->emf_obj[index].lpEMFR);
        d->emf_obj[index].lpEMFR = NULL;
    }
}


void
Emf::insert_object(PEMF_CALLBACK_DATA d, int index, int type, PU_ENHMETARECORD pObj)
{
    if (index >= 0 && index < d->n_obj) {
        delete_object(d, index);
        d->emf_obj[index].type = type;
        d->emf_obj[index].level = d->level;
        d->emf_obj[index].lpEMFR = emr_dup((char *) pObj);
    }
}

/*  Identify probable Adobe Illustrator produced EMF files, which do strange things with the scaling.
    The few so far observed all had this format.
*/
int Emf::AI_hack(PU_EMRHEADER pEmr){
  int ret=0;
  char *ptr;
  ptr = (char *)pEmr;
  PU_EMRSETMAPMODE nEmr = (PU_EMRSETMAPMODE) (ptr + pEmr->emr.nSize);
  char *string = NULL;
  if(pEmr->nDescription)string = U_Utf16leToUtf8((uint16_t *)((char *) pEmr + pEmr->offDescription), pEmr->nDescription, NULL);
  if(string){
     if((pEmr->nDescription >= 13) &&
        (0==strcmp("Adobe Systems",string)) &&
        (nEmr->emr.iType == U_EMR_SETMAPMODE) &&
        (nEmr->iMode == U_MM_ANISOTROPIC)){ ret=1; }
     free(string);
  }
  return(ret);
}

/**
    \fn create a UTF-32LE buffer and fill it with UNICODE unknown character
    \param count number of copies of the Unicode unknown character to fill with
*/
uint32_t *Emf::unknown_chars(size_t count){
    uint32_t *res = (uint32_t *) malloc(sizeof(uint32_t) * (count + 1));
    if(!res)throw "Inkscape fatal memory allocation error - cannot continue";
    for(uint32_t i=0; i<count; i++){ res[i] = 0xFFFD; }
    res[count]=0;
    return res;
}

/**
    \fn store SVG for an image given the pixmap and various coordinate information
    \param d
    \param pEmr
    \param dx       (double) destination x      in inkscape pixels
    \param dy       (double) destination y      in inkscape pixels
    \param dw       (double) destination width  in inkscape pixels
    \param dh       (double) destination height in inkscape pixels
    \param sx       (int)    source      x      in src image pixels
    \param sy       (int)    source      y      in src image pixels
    \param iUsage
    \param offBits
    \param cbBits
    \param offBmi
    \param cbBmi
*/
void Emf::common_image_extraction(PEMF_CALLBACK_DATA d, void *pEmr,
        double dx, double dy, double dw, double dh, int sx, int sy, int sw, int sh,
        uint32_t iUsage, uint32_t offBits, uint32_t cbBits, uint32_t offBmi, uint32_t cbBmi){

    SVGOStringStream tmp_image;
    int  dibparams = U_BI_UNKNOWN;  // type of image not yet determined

    tmp_image << "\n\t <image\n";
    if (d->dc[d->level].clip_id){
        tmp_image << "\tclip-path=\"url(#clipEmfPath" << d->dc[d->level].clip_id << ")\"\n";
    }
    tmp_image << " y=\"" << dy << "\"\n x=\"" << dx <<"\"\n ";

    MEMPNG mempng; // PNG in memory comes back in this
    mempng.buffer = NULL;

    char             *rgba_px = NULL;     // RGBA pixels
    char             *sub_px  = NULL;     // RGBA pixels, subarray
    const char       *px      = NULL;     // DIB pixels
    const U_RGBQUAD  *ct      = NULL;     // DIB color table
    uint32_t width, height, colortype, numCt, invert; // if needed these values will be set in get_DIB_params
    if(cbBits && cbBmi && (iUsage == U_DIB_RGB_COLORS)){
        // next call returns pointers and values, but allocates no memory
        dibparams = get_DIB_params((const char *)pEmr, offBits, offBmi, &px, (const U_RGBQUAD **) &ct,
            &numCt, &width, &height, &colortype, &invert);
        if(dibparams ==U_BI_RGB){
            if(sw == 0 || sh == 0){
                sw = width;
                sh = height;
            }

            if(!DIB_to_RGBA(
                px,         // DIB pixel array
                ct,         // DIB color table
                numCt,      // DIB color table number of entries
                &rgba_px,   // U_RGBA pixel array (32 bits), created by this routine, caller must free.
                width,      // Width of pixel array
                height,     // Height of pixel array
                colortype,  // DIB BitCount Enumeration
                numCt,      // Color table used if not 0
                invert      // If DIB rows are in opposite order from RGBA rows
            )){
                sub_px = RGBA_to_RGBA( // returns either a subset (side effect: frees rgba_px) or NULL (for subset == entire image)
                    rgba_px,           // full pixel array from DIB
                    width,             // Width of pixel array
                    height,            // Height of pixel array
                    sx,sy,             // starting point in pixel array
                    &sw,&sh            // columns/rows to extract from the pixel array (output array size)
                );

                if(!sub_px)sub_px=rgba_px;
                toPNG(         // Get the image from the RGBA px into mempng
                    &mempng,
                    sw, sh,    // size of the extracted pixel array
                    sub_px
                );
                free(sub_px);
            }
        }
    }

    gchar *base64String=NULL;
    if(dibparams == U_BI_JPEG){    // image was binary jpg in source file
        tmp_image << " xlink:href=\"data:image/jpeg;base64,";
        base64String = g_base64_encode((guchar*) px, numCt );
    }
    else if(dibparams==U_BI_PNG){  // image was binary png in source file
        tmp_image << " xlink:href=\"data:image/png;base64,";
        base64String = g_base64_encode((guchar*) px, numCt );
    }
    else if(mempng.buffer){        // image was DIB in source file, converted to png in this routine
        tmp_image << " xlink:href=\"data:image/png;base64,";
        base64String = g_base64_encode((guchar*) mempng.buffer, mempng.size );
        free(mempng.buffer);
    }
    else {                         // unknown or unsupported image type or failed conversion, insert the common bad image picture
        tmp_image << " xlink:href=\"data:image/png;base64,";
        base64String = bad_image_png();
    }
    tmp_image << base64String;
    g_free(base64String);

    tmp_image << "\"\n height=\"" << dh << "\"\n width=\"" << dw << "\"\n";

    tmp_image << " transform=" << current_matrix(d, dx, dy, 1); // calculate appropriate offset
    tmp_image << " preserveAspectRatio=\"none\"\n";
    tmp_image <<  "/> \n";

    d->outsvg += tmp_image.str().c_str();
    d->path = "";
}

/**
    \fn myEnhMetaFileProc(char *contents, unsigned int length, PEMF_CALLBACK_DATA lpData)
    \param contents binary contents of an EMF file
    \param length   length in bytes of contents
    \param d   Inkscape data structures returned by this call
*/
//THis was a callback, just build it into a normal function
int Emf::myEnhMetaFileProc(char *contents, unsigned int length, PEMF_CALLBACK_DATA d)
{
    uint32_t         off=0;
    uint32_t         emr_mask;
    int              OK =1;
    int              file_status=1;
    uint32_t         nSize;
    uint32_t         iType;
    const char      *blimit = contents + length;
    PU_ENHMETARECORD lpEMFR;
    TCHUNK_SPECS     tsp;
    uint32_t         tbkMode  = U_TRANSPARENT;          // holds proposed change to bkMode, if text is involved saving these to the DC must wait until the text is written
    U_COLORREF       tbkColor = U_RGB(255, 255, 255);   // holds proposed change to bkColor

    /* initialize the tsp for text reassembly */
    tsp.string     = NULL;
    tsp.ori        = 0.0;  /* degrees */
    tsp.fs         = 12.0; /* font size */
    tsp.x          = 0.0;
    tsp.y          = 0.0;
    tsp.boff       = 0.0;  /* offset to baseline from LL corner of bounding rectangle, changes with fs and taln*/
    tsp.vadvance   = 0.0;  /* meaningful only when a complex contains two or more lines */
    tsp.taln       = ALILEFT + ALIBASE;
    tsp.ldir       = LDIR_LR;
    tsp.spaces     = 0; // this field is only used for debugging
    tsp.color.Red       = 0;    /* RGB Black */
    tsp.color.Green     = 0;    /* RGB Black */
    tsp.color.Blue      = 0;    /* RGB Black */
    tsp.color.Reserved  = 0;    /* not used  */
    tsp.italics    = 0;
    tsp.weight     = 80;
    tsp.decoration = TXTDECOR_NONE;
    tsp.condensed  = 100;
    tsp.co         = 0;
    tsp.fi_idx     = -1;  /* set to an invalid */

    while(OK){
    if(off>=length)return(0);  //normally should exit from while after EMREOF sets OK to false.

    // check record sizes and types thoroughly 
    int badrec = 0;
    if (!U_emf_record_sizeok(contents + off, blimit, &nSize, &iType, 1) || 
        !U_emf_record_safe(contents + off)){
        badrec = 1;
    }
    else {
        emr_mask = emr_properties(iType);
        if (emr_mask == U_EMR_INVALID) { badrec = 1; }
    }
    if (badrec) {
        file_status = 0;
        break;
    }

    lpEMFR = (PU_ENHMETARECORD)(contents + off);

//  Uncomment the following to track down toxic records
// std::cout << "record type: " << iType  << " name " << U_emr_names(iType) << " length: " << nSize << " offset: " << off <<std::endl;
    off += nSize;

    SVGOStringStream tmp_outsvg;
    SVGOStringStream tmp_path;
    SVGOStringStream tmp_str;
    SVGOStringStream dbg_str;

/* Uncomment the following to track down text problems */
//std::cout << "tri->dirty:"<< d->tri->dirty << " emr_mask: " << std::hex << emr_mask << std::dec << std::endl;

    // incompatible change to text drawing detected (color or background change) forces out existing text
    //    OR
    // next record is valid type and forces pending text to be drawn immediately
    if ((d->dc[d->level].dirty & DIRTY_TEXT) || ((emr_mask != U_EMR_INVALID)   &&   (emr_mask & U_DRAW_TEXT) && d->tri->dirty)){
        TR_layout_analyze(d->tri);
        if (d->dc[d->level].clip_id){
           SVGOStringStream tmp_clip;
           tmp_clip << "\n<g\n\tclip-path=\"url(#clipEmfPath" << d->dc[d->level].clip_id << ")\"\n>";
           d->outsvg += tmp_clip.str().c_str();
        }
        TR_layout_2_svg(d->tri);
        SVGOStringStream ts;
        ts << d->tri->out;
        d->outsvg += ts.str().c_str();
        d->tri = trinfo_clear(d->tri);
        if (d->dc[d->level].clip_id){
           d->outsvg += "\n</g>\n";
        }        
    }
    if(d->dc[d->level].dirty){  //Apply the delayed background changes, clear the flag
        d->dc[d->level].bkMode = tbkMode;
        memcpy(&(d->dc[d->level].bkColor),&tbkColor, sizeof(U_COLORREF));

        if(d->dc[d->level].dirty & DIRTY_TEXT){ 
            // U_COLORREF and TRCOLORREF are exactly the same in memory, but the compiler needs some convincing...
            if(tbkMode == U_TRANSPARENT){ (void) trinfo_load_bk(d->tri, BKCLR_NONE, *(TRCOLORREF *) &tbkColor); }
            else {                        (void) trinfo_load_bk(d->tri, BKCLR_LINE, *(TRCOLORREF *) &tbkColor); } // Opaque
        }

        /*  It is possible to have a series of EMF records that would result in
            the following creating hash patterns which are never used.  For instance, if 
            there were a series of records that changed the background color but did nothing
            else.
        */
        if((d->dc[d->level].stroke_mode == DRAW_PATTERN) && (d->dc[d->level].dirty & DIRTY_STROKE)){ 
            select_extpen(d, d->dc[d->level].stroke_recidx);
        }
  
        if((d->dc[d->level].fill_mode   == DRAW_PATTERN) && (d->dc[d->level].dirty & DIRTY_FILL)){
            select_brush(d, d->dc[d->level].fill_recidx);
        }
        
        d->dc[d->level].dirty = 0;
    }

// std::cout << "BEFORE DRAW logic d->mask: " << std::hex << d->mask << " emr_mask: " << emr_mask << std::dec << std::endl;
/*
std::cout << "BEFORE DRAW"
 << " test0 " << ( d->mask & U_DRAW_VISIBLE)
 << " test1 " << ( d->mask & U_DRAW_FORCE)
 << " test2 " << (emr_mask & U_DRAW_ALTERS)
 << " test3 " << (emr_mask & U_DRAW_VISIBLE)
 << " test4 " << !(d->mask & U_DRAW_ONLYTO)
 << " test5 " << ((d->mask & U_DRAW_ONLYTO) && !(emr_mask & U_DRAW_ONLYTO)  )
 << std::endl;
*/

    if(
        (emr_mask != U_EMR_INVALID)                             &&              // next record is valid type
        (d->mask & U_DRAW_VISIBLE)                              &&              // Current set of objects are drawable
        (
            (d->mask & U_DRAW_FORCE)                            ||              // This draw is forced by STROKE/FILL/STROKEANDFILL PATH
            (emr_mask & U_DRAW_ALTERS)                          ||              // Next record would alter the drawing environment in some way
            (
                (emr_mask & U_DRAW_VISIBLE)                     &&              // Next record is visible...
                (
                    ( !(d->mask & U_DRAW_ONLYTO) )              ||              //   Non *TO records cannot be followed by any Visible
                    ((d->mask & U_DRAW_ONLYTO) && !(emr_mask & U_DRAW_ONLYTO)  )//   *TO records can only be followed by other *TO records
                )
            )
        )
    ){
// std::cout << "PATH DRAW at TOP path" << *(d->path) << std::endl;
        if(!(d->path.empty())){
            d->outsvg += "   <path ";     // this is the ONLY place <path should be used!!!  One exception, gradientfill.
            if(d->drawtype){                 // explicit draw type EMR record
                output_style(d, d->drawtype);
            }
            else if(d->mask & U_DRAW_CLOSED){              // implicit draw type
                output_style(d, U_EMR_STROKEANDFILLPATH);
            }
            else {
                output_style(d, U_EMR_STROKEPATH);
            }
            d->outsvg += "\n\t";
            d->outsvg += "\n\td=\"";      // this is the ONLY place d=" should be used!!!!  One exception, gradientfill.
            d->outsvg += d->path;
            d->outsvg += " \" /> \n";
            d->path = "";
        }
        // reset the flags
        d->mask = 0;
        d->drawtype = 0;
    }
// std::cout << "AFTER DRAW logic d->mask: " << std::hex << d->mask << " emr_mask: " << emr_mask << std::dec << std::endl;

    switch (iType)
    {
        case U_EMR_HEADER:
        {
            dbg_str << "<!-- U_EMR_HEADER -->\n";

            d->outdef += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";

            if (d->pDesc) {
                d->outdef += "<!-- ";
                d->outdef += d->pDesc;
                d->outdef += " -->\n";
            }

            PU_EMRHEADER pEmr = (PU_EMRHEADER) lpEMFR;
            SVGOStringStream tmp_outdef;
            tmp_outdef << "<svg\n";
            tmp_outdef << "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
            tmp_outdef << "  xmlns=\"http://www.w3.org/2000/svg\"\n";
            tmp_outdef << "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
            tmp_outdef << "  xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"; // needed for sodipodi:role
            tmp_outdef << "  version=\"1.0\"\n";


            /* inclusive-inclusive, so the size is 1 more than the difference */
            d->MM100InX  = pEmr->rclFrame.right    - pEmr->rclFrame.left + 1;
            d->MM100InY  = pEmr->rclFrame.bottom   - pEmr->rclFrame.top  + 1;
            d->PixelsInX = pEmr->rclBounds.right   - pEmr->rclBounds.left + 1;
            d->PixelsInY = pEmr->rclBounds.bottom  - pEmr->rclBounds.top + 1;

            /*
                calculate ratio of Inkscape dpi/EMF device dpi
                This can cause problems later due to accuracy limits in the EMF.  A high resolution
                EMF might have a final D2Pscale[XY] of 0.074998, and adjusting the (integer) device size
                by 1 will still not get it exactly to 0.075.  Later when the font size is calculated it
                can end up as 29.9992 or 22.4994 instead of the intended 30 or 22.5.  This is handled by
                snapping font sizes to the nearest .01.  The best estimate is made by using both values.
            */
            if ((pEmr->szlMillimeters.cx + pEmr->szlMillimeters.cy) && ( pEmr->szlDevice.cx + pEmr->szlDevice.cy)){
                d->E2IdirY = 1.0;  // assume MM_TEXT, if not, this will be changed later
                d->D2PscaleX = d->D2PscaleY = Inkscape::Util::Quantity::convert(1, "mm", "px") *
                    (double)(pEmr->szlMillimeters.cx + pEmr->szlMillimeters.cy)/
                    (double)( pEmr->szlDevice.cx + pEmr->szlDevice.cy);
            }
            trinfo_load_qe(d->tri, d->D2PscaleX);  /* quantization error that will affect text positions */

            /*  Adobe Illustrator files set mapmode to MM_ANISOTROPIC and somehow or other this
                converts the rclFrame values from MM_HIMETRIC to MM_HIENGLISH, with another factor of 3 thrown
                in for good measure.  Ours not to question why...
            */
            if(AI_hack(pEmr)){
                d->MM100InX  *= 25.4/(10.0*3.0);
                d->MM100InY  *= 25.4/(10.0*3.0);
                d->D2PscaleX *= 25.4/(10.0*3.0);
                d->D2PscaleY *= 25.4/(10.0*3.0);
            }

            d->MMX = d->MM100InX / 100.0;
            d->MMY = d->MM100InY / 100.0;

            d->PixelsOutX = Inkscape::Util::Quantity::convert(d->MMX, "mm", "px");
            d->PixelsOutY = Inkscape::Util::Quantity::convert(d->MMY, "mm", "px");

            // Upper left corner, from header rclBounds, in device units, usually both 0, but not always
            d->ulCornerInX = pEmr->rclBounds.left;
            d->ulCornerInY = pEmr->rclBounds.top;
            d->ulCornerOutX = d->ulCornerInX              * d->D2PscaleX;
            d->ulCornerOutY = d->ulCornerInY * d->E2IdirY * d->D2PscaleY;

            tmp_outdef <<
                "  width=\"" << d->MMX << "mm\"\n" <<
                "  height=\"" << d->MMY << "mm\">\n";
            d->outdef += tmp_outdef.str().c_str();
            d->outdef += "<defs>";                           // temporary end of header

            // d->defs holds any defines which are read in.

            tmp_outsvg << "\n</defs>\n\n";                   // start of main body

            if (pEmr->nHandles) {
                d->n_obj = pEmr->nHandles;
                d->emf_obj = new EMF_OBJECT[d->n_obj];

                // Init the new emf_obj list elements to null, provided the
                // dynamic allocation succeeded.
                if ( d->emf_obj != NULL )
                {
                    for( int i=0; i < d->n_obj; ++i )
                        d->emf_obj[i].lpEMFR = NULL;
                } //if

            } else {
                d->emf_obj = NULL;
            }

            break;
        }
        case U_EMR_POLYBEZIER:
        {
            dbg_str << "<!-- U_EMR_POLYBEZIER -->\n";

            PU_EMRPOLYBEZIER pEmr = (PU_EMRPOLYBEZIER) lpEMFR;
            uint32_t i,j;

            if (pEmr->cptl<4)
                break;

            d->mask |= emr_mask;

            tmp_str <<
                "\n\tM " <<
                pix_to_xy( d, pEmr->aptl[0].x, pEmr->aptl[0].y) << " ";

            for (i=1; i<pEmr->cptl; ) {
                tmp_str << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cptl; j++,i++) {
                    tmp_str << pix_to_xy( d, pEmr->aptl[i].x, pEmr->aptl[i].y) << " ";
                }
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_EMR_POLYGON:
        {
            dbg_str << "<!-- U_EMR_POLYGON -->\n";

            PU_EMRPOLYGON pEmr = (PU_EMRPOLYGON) lpEMFR;
            uint32_t i;

            if (pEmr->cptl < 2)
                break;

            d->mask |= emr_mask;

            tmp_str <<
                "\n\tM " <<
                pix_to_xy( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " ";

            for (i=1; i<pEmr->cptl; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_to_xy( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
            }

            tmp_path << tmp_str.str().c_str();
            tmp_path << " z";

            break;
        }
        case U_EMR_POLYLINE:
        {
            dbg_str << "<!-- U_EMR_POLYLINE -->\n";

            PU_EMRPOLYLINE pEmr = (PU_EMRPOLYLINE) lpEMFR;
            uint32_t i;

            if (pEmr->cptl<2)
                break;

            d->mask |= emr_mask;

            tmp_str <<
                "\n\tM " <<
                pix_to_xy( d, pEmr->aptl[0].x, pEmr->aptl[0].y ) << " ";

            for (i=1; i<pEmr->cptl; i++) {
                tmp_str <<
                    "\n\tL " <<
                    pix_to_xy( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_EMR_POLYBEZIERTO:
        {
            dbg_str << "<!-- U_EMR_POLYBEZIERTO -->\n";

            PU_EMRPOLYBEZIERTO pEmr = (PU_EMRPOLYBEZIERTO) lpEMFR;
            uint32_t i,j;

            d->mask |= emr_mask;

            for (i=0; i<pEmr->cptl;) {
                tmp_path << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cptl; j++,i++) {
                    tmp_path <<
                        pix_to_xy( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
                }
            }

            break;
        }
        case U_EMR_POLYLINETO:
        {
            dbg_str << "<!-- U_EMR_POLYLINETO -->\n";

            PU_EMRPOLYLINETO pEmr = (PU_EMRPOLYLINETO) lpEMFR;
            uint32_t i;

            d->mask |= emr_mask;

            for (i=0; i<pEmr->cptl;i++) {
                tmp_path <<
                    "\n\tL " <<
                    pix_to_xy( d, pEmr->aptl[i].x, pEmr->aptl[i].y ) << " ";
            }

            break;
        }
        case U_EMR_POLYPOLYLINE:
        case U_EMR_POLYPOLYGON:
        {
            if (lpEMFR->iType == U_EMR_POLYPOLYLINE)
                dbg_str << "<!-- U_EMR_POLYPOLYLINE -->\n";
            if (lpEMFR->iType == U_EMR_POLYPOLYGON)
                dbg_str << "<!-- U_EMR_POLYPOLYGON -->\n";

            PU_EMRPOLYPOLYGON pEmr = (PU_EMRPOLYPOLYGON) lpEMFR;
            unsigned int n, i, j;

            d->mask |= emr_mask;

            U_POINTL *aptl = (PU_POINTL) &pEmr->aPolyCounts[pEmr->nPolys];

            i = 0;
            for (n=0; n<pEmr->nPolys && i<pEmr->cptl; n++) {
                SVGOStringStream poly_path;

                poly_path << "\n\tM " << pix_to_xy( d, aptl[i].x, aptl[i].y) << " ";
                i++;

                for (j=1; j<pEmr->aPolyCounts[n] && i<pEmr->cptl; j++) {
                    poly_path << "\n\tL " << pix_to_xy( d, aptl[i].x, aptl[i].y) << " ";
                    i++;
                }

                tmp_str << poly_path.str().c_str();
                if (lpEMFR->iType == U_EMR_POLYPOLYGON)
                    tmp_str << " z";
                tmp_str << " \n";
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_EMR_SETWINDOWEXTEX:
        {
            dbg_str << "<!-- U_EMR_SETWINDOWEXTEX -->\n";

            PU_EMRSETWINDOWEXTEX pEmr = (PU_EMRSETWINDOWEXTEX) lpEMFR;

            d->dc[d->level].sizeWnd = pEmr->szlExtent;

            if (!d->dc[d->level].sizeWnd.cx || !d->dc[d->level].sizeWnd.cy) {
                d->dc[d->level].sizeWnd = d->dc[d->level].sizeView;
                if (!d->dc[d->level].sizeWnd.cx || !d->dc[d->level].sizeWnd.cy) {
                    d->dc[d->level].sizeWnd.cx = d->PixelsOutX;
                    d->dc[d->level].sizeWnd.cy = d->PixelsOutY;
                }
            }

            if (!d->dc[d->level].sizeView.cx || !d->dc[d->level].sizeView.cy) {
                d->dc[d->level].sizeView = d->dc[d->level].sizeWnd;
            }

            /* scales logical to EMF pixels, transfer a negative sign on Y, if any */
            if (d->dc[d->level].sizeWnd.cx && d->dc[d->level].sizeWnd.cy) {
                d->dc[d->level].ScaleInX = (double) d->dc[d->level].sizeView.cx / (double) d->dc[d->level].sizeWnd.cx;
                d->dc[d->level].ScaleInY = (double) d->dc[d->level].sizeView.cy / (double) d->dc[d->level].sizeWnd.cy;
                if(d->dc[d->level].ScaleInY < 0){
                    d->dc[d->level].ScaleInY *= -1.0;
                    d->E2IdirY = -1.0;
                }
            }
            else {
                d->dc[d->level].ScaleInX = 1;
                d->dc[d->level].ScaleInY = 1;
            }
            break;
        }
        case U_EMR_SETWINDOWORGEX:
        {
            dbg_str << "<!-- U_EMR_SETWINDOWORGEX -->\n";

            PU_EMRSETWINDOWORGEX pEmr = (PU_EMRSETWINDOWORGEX) lpEMFR;
            d->dc[d->level].winorg = pEmr->ptlOrigin;
            break;
        }
        case U_EMR_SETVIEWPORTEXTEX:
        {
            dbg_str << "<!-- U_EMR_SETVIEWPORTEXTEX -->\n";

            PU_EMRSETVIEWPORTEXTEX pEmr = (PU_EMRSETVIEWPORTEXTEX) lpEMFR;

            d->dc[d->level].sizeView = pEmr->szlExtent;

            if (!d->dc[d->level].sizeView.cx || !d->dc[d->level].sizeView.cy) {
                d->dc[d->level].sizeView = d->dc[d->level].sizeWnd;
                if (!d->dc[d->level].sizeView.cx || !d->dc[d->level].sizeView.cy) {
                    d->dc[d->level].sizeView.cx = d->PixelsOutX;
                    d->dc[d->level].sizeView.cy = d->PixelsOutY;
                }
            }

            if (!d->dc[d->level].sizeWnd.cx || !d->dc[d->level].sizeWnd.cy) {
                d->dc[d->level].sizeWnd = d->dc[d->level].sizeView;
            }

            /* scales logical to EMF pixels, transfer a negative sign on Y, if any */
            if (d->dc[d->level].sizeWnd.cx && d->dc[d->level].sizeWnd.cy) {
                d->dc[d->level].ScaleInX = (double) d->dc[d->level].sizeView.cx / (double) d->dc[d->level].sizeWnd.cx;
                d->dc[d->level].ScaleInY = (double) d->dc[d->level].sizeView.cy / (double) d->dc[d->level].sizeWnd.cy;
                if( d->dc[d->level].ScaleInY < 0){
                    d->dc[d->level].ScaleInY *= -1.0;
                    d->E2IdirY = -1.0;
                }
            }
            else {
                d->dc[d->level].ScaleInX = 1;
                d->dc[d->level].ScaleInY = 1;
            }
            break;
        }
        case U_EMR_SETVIEWPORTORGEX:
        {
            dbg_str << "<!-- U_EMR_SETVIEWPORTORGEX -->\n";

            PU_EMRSETVIEWPORTORGEX pEmr = (PU_EMRSETVIEWPORTORGEX) lpEMFR;
            d->dc[d->level].vieworg = pEmr->ptlOrigin;
            break;
        }
        case U_EMR_SETBRUSHORGEX:        dbg_str << "<!-- U_EMR_SETBRUSHORGEX -->\n";      break;
        case U_EMR_EOF:
        {
            dbg_str << "<!-- U_EMR_EOF -->\n";

            tmp_outsvg << "</svg>\n";
            d->outsvg = d->outdef + d->defs + d->outsvg;
            OK=0;
            break;
        }
        case U_EMR_SETPIXELV:            dbg_str << "<!-- U_EMR_SETPIXELV -->\n";          break;
        case U_EMR_SETMAPPERFLAGS:       dbg_str << "<!-- U_EMR_SETMAPPERFLAGS -->\n";     break;
        case U_EMR_SETMAPMODE:
        {
            dbg_str << "<!-- U_EMR_SETMAPMODE -->\n";
            PU_EMRSETMAPMODE pEmr = (PU_EMRSETMAPMODE) lpEMFR;
            switch (pEmr->iMode){
                case U_MM_TEXT:
                default:
                    // Use all values from the header.
                    break;
                /*  For all of the following the indicated scale this will be encoded in WindowExtEx/ViewportExtex
                    and show up in ScaleIn[XY]
                */
                case U_MM_LOMETRIC:  // 1 LU = 0.1 mm,
                case U_MM_HIMETRIC:  // 1 LU = 0.01 mm
                case U_MM_LOENGLISH:  // 1 LU = 0.1 in
                case U_MM_HIENGLISH:  // 1 LU = 0.01 in
                case U_MM_TWIPS:  // 1 LU = 1/1440  in
                    d->E2IdirY = -1.0;
                    // Use d->D2Pscale[XY] values from the header.
                    break;
                case U_MM_ISOTROPIC: // ScaleIn[XY] should be set elsewhere by SETVIEWPORTEXTEX and SETWINDOWEXTEX
                case U_MM_ANISOTROPIC:
                    break;
            }
            break;
        }
        case U_EMR_SETBKMODE:
        {
            dbg_str << "<!-- U_EMR_SETBKMODE -->\n";
            PU_EMRSETBKMODE pEmr = (PU_EMRSETBKMODE) lpEMFR;
            tbkMode = pEmr->iMode;
            if(tbkMode != d->dc[d->level].bkMode){
                d->dc[d->level].dirty  |= DIRTY_TEXT;
                if(tbkMode != d->dc[d->level].bkMode){
                    if(d->dc[d->level].fill_mode   == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_FILL;   }
                    if(d->dc[d->level].stroke_mode == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_STROKE; }
                }
                memcpy(&tbkColor,&(d->dc[d->level].bkColor),sizeof(U_COLORREF));
            }
            break;
        }
        case U_EMR_SETPOLYFILLMODE:
        {
            dbg_str << "<!-- U_EMR_SETPOLYFILLMODE -->\n";

            PU_EMRSETPOLYFILLMODE pEmr = (PU_EMRSETPOLYFILLMODE) lpEMFR;
            d->dc[d->level].style.fill_rule.value =
                (pEmr->iMode == U_ALTERNATE ? 0 : (pEmr->iMode == U_WINDING ? 1 : 0));
            break;
        }
        case U_EMR_SETROP2:
        {
            dbg_str << "<!-- U_EMR_SETROP2 -->\n";
            PU_EMRSETROP2 pEmr = (PU_EMRSETROP2) lpEMFR;
            d->dwRop2 = pEmr->iMode;
            break;
        }
        case U_EMR_SETSTRETCHBLTMODE:
        {
            PU_EMRSETSTRETCHBLTMODE pEmr = (PU_EMRSETSTRETCHBLTMODE) lpEMFR; // from wingdi.h
            BLTmode = pEmr->iMode;
            dbg_str << "<!-- U_EMR_SETSTRETCHBLTMODE -->\n";
            break;
        }
        case U_EMR_SETTEXTALIGN:
        {
            dbg_str << "<!-- U_EMR_SETTEXTALIGN -->\n";

            PU_EMRSETTEXTALIGN pEmr = (PU_EMRSETTEXTALIGN) lpEMFR;
            d->dc[d->level].textAlign = pEmr->iMode;
            break;
        }
        case U_EMR_SETCOLORADJUSTMENT:
            dbg_str << "<!-- U_EMR_SETCOLORADJUSTMENT -->\n";
            break;
        case U_EMR_SETTEXTCOLOR:
        {
            dbg_str << "<!-- U_EMR_SETTEXTCOLOR -->\n";

            PU_EMRSETTEXTCOLOR pEmr = (PU_EMRSETTEXTCOLOR) lpEMFR;
            d->dc[d->level].textColor = pEmr->crColor;
            if(tbkMode != d->dc[d->level].bkMode){
                if(d->dc[d->level].fill_mode   == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_FILL;   }
                if(d->dc[d->level].stroke_mode == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_STROKE; }
            }
            // not text_dirty, because multicolored complex text is supported in libTERE
            break;
        }
        case U_EMR_SETBKCOLOR:
        {
            dbg_str << "<!-- U_EMR_SETBKCOLOR -->\n";

            PU_EMRSETBKCOLOR pEmr = (PU_EMRSETBKCOLOR) lpEMFR;
            tbkColor = pEmr->crColor;
            if(memcmp(&tbkColor, &(d->dc[d->level].bkColor), sizeof(U_COLORREF))){
                d->dc[d->level].dirty  |= DIRTY_TEXT;
                if(d->dc[d->level].fill_mode   == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_FILL;   }
                if(d->dc[d->level].stroke_mode == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_STROKE; }
                tbkMode = d->dc[d->level].bkMode;
            }
            break;
        }
        case U_EMR_OFFSETCLIPRGN:
        {
            dbg_str << "<!-- U_EMR_OFFSETCLIPRGN -->\n"; 
            if (d->dc[d->level].clip_id) { // can only offsetan existing clipping path
                PU_EMROFFSETCLIPRGN pEmr = (PU_EMROFFSETCLIPRGN) lpEMFR;
                U_POINTL off = pEmr->ptlOffset;
                unsigned int real_idx = d->dc[d->level].clip_id - 1;
                Geom::PathVector tmp_vect = sp_svg_read_pathv(d->clips.strings[real_idx]);
                double ox = pix_to_x_point(d, off.x, off.y) - pix_to_x_point(d, 0, 0); // take into account all active transforms
                double oy = pix_to_y_point(d, off.x, off.y) - pix_to_y_point(d, 0, 0);
                Geom::Affine tf = Geom::Translate(ox,oy);
                tmp_vect *= tf;
                char *tmp_path = sp_svg_write_path(tmp_vect);
                add_clips(d, tmp_path, U_RGN_COPY);
                free(tmp_path);
            }
            break;
        }
        case U_EMR_MOVETOEX:
        {
            dbg_str << "<!-- U_EMR_MOVETOEX -->\n";

            PU_EMRMOVETOEX pEmr = (PU_EMRMOVETOEX) lpEMFR;

            d->mask |= emr_mask;

            d->dc[d->level].cur = pEmr->ptl;

            tmp_path <<
                "\n\tM " << pix_to_xy( d, pEmr->ptl.x, pEmr->ptl.y ) << " ";
            break;
        }
        case U_EMR_SETMETARGN:           dbg_str << "<!-- U_EMR_SETMETARGN -->\n";         break;
        case U_EMR_EXCLUDECLIPRECT:
        {
            dbg_str << "<!-- U_EMR_EXCLUDECLIPRECT -->\n";

            PU_EMREXCLUDECLIPRECT pEmr = (PU_EMREXCLUDECLIPRECT) lpEMFR;
            U_RECTL rc = pEmr->rclClip;

            SVGOStringStream tmp_path;
            //outer rect, clockwise 
            tmp_path << "M " <<  faraway << "," <<  faraway << " ";
            tmp_path << "L " <<  faraway << "," << -faraway << " ";
            tmp_path << "L " << -faraway << "," << -faraway << " ";
            tmp_path << "L " << -faraway << "," <<  faraway << " ";
            tmp_path << "z ";
            //inner rect, counterclockwise (sign of Y is reversed)
            tmp_path << "M " << pix_to_xy( d, rc.left , rc.top )     << " ";
            tmp_path << "L " << pix_to_xy( d, rc.right, rc.top )     << " ";
            tmp_path << "L " << pix_to_xy( d, rc.right, rc.bottom )  << " ";
            tmp_path << "L " << pix_to_xy( d, rc.left,  rc.bottom )  << " ";
            tmp_path << "z";
            
            add_clips(d, tmp_path.str().c_str(), U_RGN_AND);

            d->path = "";
            d->drawtype = 0;
            break;
        }
        case U_EMR_INTERSECTCLIPRECT:
        {
            dbg_str << "<!-- U_EMR_INTERSECTCLIPRECT -->\n";

            PU_EMRINTERSECTCLIPRECT pEmr = (PU_EMRINTERSECTCLIPRECT) lpEMFR;
            U_RECTL rc = pEmr->rclClip;

            SVGOStringStream tmp_path;
            tmp_path << "M " << pix_to_xy( d, rc.left , rc.top )     << " ";
            tmp_path << "L " << pix_to_xy( d, rc.right, rc.top )     << " ";
            tmp_path << "L " << pix_to_xy( d, rc.right, rc.bottom )  << " ";
            tmp_path << "L " << pix_to_xy( d, rc.left,  rc.bottom )  << " ";
            tmp_path << "z";
            
            add_clips(d, tmp_path.str().c_str(), U_RGN_AND);

            d->path = "";
            d->drawtype = 0;
            break;
        }
        case U_EMR_SCALEVIEWPORTEXTEX:   dbg_str << "<!-- U_EMR_SCALEVIEWPORTEXTEX -->\n"; break;
        case U_EMR_SCALEWINDOWEXTEX:     dbg_str << "<!-- U_EMR_SCALEWINDOWEXTEX -->\n";   break;
        case U_EMR_SAVEDC:
            dbg_str << "<!-- U_EMR_SAVEDC -->\n";

            if (d->level < EMF_MAX_DC) {
                d->dc[d->level + 1] = d->dc[d->level];
                if(d->dc[d->level].font_name){
                    d->dc[d->level + 1].font_name = strdup(d->dc[d->level].font_name); // or memory access problems because font name pointer duplicated
                }
                d->level = d->level + 1;
            }
            break;
        case U_EMR_RESTOREDC:
        {
            dbg_str << "<!-- U_EMR_RESTOREDC -->\n";

            PU_EMRRESTOREDC pEmr = (PU_EMRRESTOREDC) lpEMFR;
            int old_level = d->level;
            if (pEmr->iRelative >= 0) {
                if (pEmr->iRelative < d->level)
                    d->level = pEmr->iRelative;
            }
            else {
                if (d->level + pEmr->iRelative >= 0)
                    d->level = d->level + pEmr->iRelative;
            }
            while (old_level > d->level) {
                if (!d->dc[old_level].style.stroke_dasharray.values.empty() && (old_level==0 || (old_level>0 && d->dc[old_level].style.stroke_dasharray.values!=d->dc[old_level-1].style.stroke_dasharray.values))){
                    d->dc[old_level].style.stroke_dasharray.values.clear();
                }
                if(d->dc[old_level].font_name){
                    free(d->dc[old_level].font_name); // else memory leak
                    d->dc[old_level].font_name = NULL;
                }
                old_level--;
            }
            break;
        }
        case U_EMR_SETWORLDTRANSFORM:
        {
            dbg_str << "<!-- U_EMR_SETWORLDTRANSFORM -->\n";

            PU_EMRSETWORLDTRANSFORM pEmr = (PU_EMRSETWORLDTRANSFORM) lpEMFR;
            d->dc[d->level].worldTransform = pEmr->xform;
            break;
        }
        case U_EMR_MODIFYWORLDTRANSFORM:
        {
            dbg_str << "<!-- U_EMR_MODIFYWORLDTRANSFORM -->\n";

            PU_EMRMODIFYWORLDTRANSFORM pEmr = (PU_EMRMODIFYWORLDTRANSFORM) lpEMFR;
            switch (pEmr->iMode)
            {
                case U_MWT_IDENTITY:
                    d->dc[d->level].worldTransform.eM11 = 1.0;
                    d->dc[d->level].worldTransform.eM12 = 0.0;
                    d->dc[d->level].worldTransform.eM21 = 0.0;
                    d->dc[d->level].worldTransform.eM22 = 1.0;
                    d->dc[d->level].worldTransform.eDx  = 0.0;
                    d->dc[d->level].worldTransform.eDy  = 0.0;
                    break;
                case U_MWT_LEFTMULTIPLY:
                {
//                    d->dc[d->level].worldTransform = pEmr->xform * worldTransform;

                    float a11 = pEmr->xform.eM11;
                    float a12 = pEmr->xform.eM12;
                    float a13 = 0.0;
                    float a21 = pEmr->xform.eM21;
                    float a22 = pEmr->xform.eM22;
                    float a23 = 0.0;
                    float a31 = pEmr->xform.eDx;
                    float a32 = pEmr->xform.eDy;
                    float a33 = 1.0;

                    float b11 = d->dc[d->level].worldTransform.eM11;
                    float b12 = d->dc[d->level].worldTransform.eM12;
                    //float b13 = 0.0;
                    float b21 = d->dc[d->level].worldTransform.eM21;
                    float b22 = d->dc[d->level].worldTransform.eM22;
                    //float b23 = 0.0;
                    float b31 = d->dc[d->level].worldTransform.eDx;
                    float b32 = d->dc[d->level].worldTransform.eDy;
                    //float b33 = 1.0;

                    float c11 = a11*b11 + a12*b21 + a13*b31;;
                    float c12 = a11*b12 + a12*b22 + a13*b32;;
                    //float c13 = a11*b13 + a12*b23 + a13*b33;;
                    float c21 = a21*b11 + a22*b21 + a23*b31;;
                    float c22 = a21*b12 + a22*b22 + a23*b32;;
                    //float c23 = a21*b13 + a22*b23 + a23*b33;;
                    float c31 = a31*b11 + a32*b21 + a33*b31;;
                    float c32 = a31*b12 + a32*b22 + a33*b32;;
                    //float c33 = a31*b13 + a32*b23 + a33*b33;;

                    d->dc[d->level].worldTransform.eM11 = c11;;
                    d->dc[d->level].worldTransform.eM12 = c12;;
                    d->dc[d->level].worldTransform.eM21 = c21;;
                    d->dc[d->level].worldTransform.eM22 = c22;;
                    d->dc[d->level].worldTransform.eDx = c31;
                    d->dc[d->level].worldTransform.eDy = c32;

                    break;
                }
                case U_MWT_RIGHTMULTIPLY:
                {
//                    d->dc[d->level].worldTransform = worldTransform * pEmr->xform;

                    float a11 = d->dc[d->level].worldTransform.eM11;
                    float a12 = d->dc[d->level].worldTransform.eM12;
                    float a13 = 0.0;
                    float a21 = d->dc[d->level].worldTransform.eM21;
                    float a22 = d->dc[d->level].worldTransform.eM22;
                    float a23 = 0.0;
                    float a31 = d->dc[d->level].worldTransform.eDx;
                    float a32 = d->dc[d->level].worldTransform.eDy;
                    float a33 = 1.0;

                    float b11 = pEmr->xform.eM11;
                    float b12 = pEmr->xform.eM12;
                    //float b13 = 0.0;
                    float b21 = pEmr->xform.eM21;
                    float b22 = pEmr->xform.eM22;
                    //float b23 = 0.0;
                    float b31 = pEmr->xform.eDx;
                    float b32 = pEmr->xform.eDy;
                    //float b33 = 1.0;

                    float c11 = a11*b11 + a12*b21 + a13*b31;;
                    float c12 = a11*b12 + a12*b22 + a13*b32;;
                    //float c13 = a11*b13 + a12*b23 + a13*b33;;
                    float c21 = a21*b11 + a22*b21 + a23*b31;;
                    float c22 = a21*b12 + a22*b22 + a23*b32;;
                    //float c23 = a21*b13 + a22*b23 + a23*b33;;
                    float c31 = a31*b11 + a32*b21 + a33*b31;;
                    float c32 = a31*b12 + a32*b22 + a33*b32;;
                    //float c33 = a31*b13 + a32*b23 + a33*b33;;

                    d->dc[d->level].worldTransform.eM11 = c11;;
                    d->dc[d->level].worldTransform.eM12 = c12;;
                    d->dc[d->level].worldTransform.eM21 = c21;;
                    d->dc[d->level].worldTransform.eM22 = c22;;
                    d->dc[d->level].worldTransform.eDx = c31;
                    d->dc[d->level].worldTransform.eDy = c32;

                    break;
                }
//                case MWT_SET:
                default:
                    d->dc[d->level].worldTransform = pEmr->xform;
                    break;
            }
            break;
        }
        case U_EMR_SELECTOBJECT:
        {
            dbg_str << "<!-- U_EMR_SELECTOBJECT -->\n";

            PU_EMRSELECTOBJECT pEmr = (PU_EMRSELECTOBJECT) lpEMFR;
            unsigned int index = pEmr->ihObject;

            if (index & U_STOCK_OBJECT) {
                switch (index) {
                    case U_NULL_BRUSH:
                        d->dc[d->level].fill_mode = DRAW_PAINT;
                        d->dc[d->level].fill_set = false;
                        break;
                    case U_BLACK_BRUSH:
                    case U_DKGRAY_BRUSH:
                    case U_GRAY_BRUSH:
                    case U_LTGRAY_BRUSH:
                    case U_WHITE_BRUSH:
                    {
                        float val = 0;
                        switch (index) {
                            case U_BLACK_BRUSH:
                                val = 0.0 / 255.0;
                                break;
                            case U_DKGRAY_BRUSH:
                                val = 64.0 / 255.0;
                                break;
                            case U_GRAY_BRUSH:
                                val = 128.0 / 255.0;
                                break;
                            case U_LTGRAY_BRUSH:
                                val = 192.0 / 255.0;
                                break;
                            case U_WHITE_BRUSH:
                                val = 255.0 / 255.0;
                                break;
                        }
                        d->dc[d->level].style.fill.value.color.set( val, val, val );

                        d->dc[d->level].fill_mode = DRAW_PAINT;
                        d->dc[d->level].fill_set = true;
                        break;
                    }
                    case U_NULL_PEN:
                        d->dc[d->level].stroke_mode = DRAW_PAINT;
                        d->dc[d->level].stroke_set = false;
                        break;
                    case U_BLACK_PEN:
                    case U_WHITE_PEN:
                    {
                        float val = index == U_BLACK_PEN ? 0 : 1;
                        d->dc[d->level].style.stroke_dasharray.set = 0;
                        d->dc[d->level].style.stroke_width.value = 1.0;
                        d->dc[d->level].style.stroke.value.color.set( val, val, val );

                        d->dc[d->level].stroke_mode = DRAW_PAINT;
                        d->dc[d->level].stroke_set = true;

                        break;
                    }
                }
            } else {
                if ( /*index >= 0 &&*/ index < (unsigned int) d->n_obj) {
                    switch (d->emf_obj[index].type)
                    {
                        case U_EMR_CREATEPEN:
                            select_pen(d, index);
                            break;
                        case U_EMR_CREATEBRUSHINDIRECT:
                        case U_EMR_CREATEDIBPATTERNBRUSHPT:
                        case U_EMR_CREATEMONOBRUSH:
                            select_brush(d, index);
                            break;
                        case U_EMR_EXTCREATEPEN:
                            select_extpen(d, index);
                            break;
                        case U_EMR_EXTCREATEFONTINDIRECTW:
                            select_font(d, index);
                            break;
                    }
                }
            }
            break;
        }
        case U_EMR_CREATEPEN:
        {
            dbg_str << "<!-- U_EMR_CREATEPEN -->\n";

            PU_EMRCREATEPEN pEmr = (PU_EMRCREATEPEN) lpEMFR;
            insert_object(d, pEmr->ihPen, U_EMR_CREATEPEN, lpEMFR);
            break;
        }
        case U_EMR_CREATEBRUSHINDIRECT:
        {
            dbg_str << "<!-- U_EMR_CREATEBRUSHINDIRECT -->\n";

            PU_EMRCREATEBRUSHINDIRECT pEmr = (PU_EMRCREATEBRUSHINDIRECT) lpEMFR;
            insert_object(d, pEmr->ihBrush, U_EMR_CREATEBRUSHINDIRECT, lpEMFR);
            break;
        }
        case U_EMR_DELETEOBJECT:
            dbg_str << "<!-- U_EMR_DELETEOBJECT -->\n";
            // Objects here are not deleted until the draw completes, new ones may write over an existing one.
            break;
        case U_EMR_ANGLEARC:
            dbg_str << "<!-- U_EMR_ANGLEARC -->\n";
            break;
        case U_EMR_ELLIPSE:
        {
            dbg_str << "<!-- U_EMR_ELLIPSE -->\n";

            PU_EMRELLIPSE pEmr = (PU_EMRELLIPSE) lpEMFR;
            U_RECTL rclBox = pEmr->rclBox;

            double cx = pix_to_x_point( d, (rclBox.left + rclBox.right)/2.0, (rclBox.bottom + rclBox.top)/2.0 );
            double cy = pix_to_y_point( d, (rclBox.left + rclBox.right)/2.0, (rclBox.bottom + rclBox.top)/2.0 );
            double rx = pix_to_abs_size( d, std::abs(rclBox.right - rclBox.left  )/2.0 );
            double ry = pix_to_abs_size( d, std::abs(rclBox.top   - rclBox.bottom)/2.0 );

            SVGOStringStream tmp_ellipse;
            tmp_ellipse << "cx=\"" << cx << "\" ";
            tmp_ellipse << "cy=\"" << cy << "\" ";
            tmp_ellipse << "rx=\"" << rx << "\" ";
            tmp_ellipse << "ry=\"" << ry << "\" ";

            d->mask |= emr_mask;

            d->outsvg += "   <ellipse ";
            output_style(d, lpEMFR->iType);  //
            d->outsvg += "\n\t";
            d->outsvg += tmp_ellipse.str().c_str();
            d->outsvg += "/> \n";
            d->path = "";
            break;
        }
        case U_EMR_RECTANGLE:
        {
            dbg_str << "<!-- U_EMR_RECTANGLE -->\n";

            PU_EMRRECTANGLE pEmr = (PU_EMRRECTANGLE) lpEMFR;
            U_RECTL rc = pEmr->rclBox;

            SVGOStringStream tmp_rectangle;
            tmp_rectangle << "\n\tM " << pix_to_xy( d, rc.left , rc.top )     << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, rc.right, rc.top )     << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, rc.right, rc.bottom )  << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, rc.left,  rc.bottom )  << " ";
            tmp_rectangle << "\n\tz";

            d->mask |= emr_mask;

            tmp_path << tmp_rectangle.str().c_str();
            break;
        }
        case U_EMR_ROUNDRECT:
        {
            dbg_str << "<!-- U_EMR_ROUNDRECT -->\n";

            PU_EMRROUNDRECT pEmr = (PU_EMRROUNDRECT) lpEMFR;
            U_RECTL rc = pEmr->rclBox;
            U_SIZEL corner = pEmr->szlCorner;
            double f = 4.*(sqrt(2) - 1)/3;
            double f1 = 1.0 - f;
            double cnx = corner.cx/2;
            double cny = corner.cy/2;

            SVGOStringStream tmp_rectangle;
            tmp_rectangle << "\n"
                          << "    M "
                          << pix_to_xy(d,    rc.left            ,        rc.top    + cny    )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d,    rc.left            ,        rc.top    + cny*f1 )
                          << " "
                          << pix_to_xy(d,    rc.left  + cnx*f1  ,        rc.top             )
                          << " "
                          << pix_to_xy(d,    rc.left  + cnx     ,        rc.top             )
                          << "\n";
            tmp_rectangle << "   L "
                          << pix_to_xy(d,    rc.right - cnx     ,        rc.top             )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d,    rc.right - cnx*f1  ,        rc.top             )
                          << " "
                          << pix_to_xy(d,    rc.right           ,        rc.top    + cny*f1 )
                          << " "
                          << pix_to_xy(d,    rc.right           ,        rc.top    + cny    )
                          << "\n";
            tmp_rectangle << "   L "
                          << pix_to_xy(d,    rc.right           ,        rc.bottom - cny    )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d,    rc.right           ,        rc.bottom - cny*f1 )
                          << " "
                          << pix_to_xy(d,    rc.right - cnx*f1  ,        rc.bottom          )
                          << " "
                          << pix_to_xy(d,    rc.right - cnx     ,        rc.bottom          )
                          << "\n";
            tmp_rectangle << "   L "
                          << pix_to_xy(d,    rc.left  + cnx     ,        rc.bottom          )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d,    rc.left  + cnx*f1  ,        rc.bottom          )
                          << " "
                          << pix_to_xy(d,    rc.left            ,        rc.bottom - cny*f1 )
                          << " "
                          << pix_to_xy(d,    rc.left            ,        rc.bottom - cny    )
                          << "\n";
            tmp_rectangle << "   z\n";


            d->mask |= emr_mask;

            tmp_path <<   tmp_rectangle.str().c_str();
            break;
        }
        case U_EMR_ARC:
        {
            dbg_str << "<!-- U_EMR_ARC -->\n";
            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            int stat = emr_arc_points( lpEMFR, &f1, f2, &center, &start, &end, &size);
            if(!stat){
                tmp_path <<  "\n\tM " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y) << " \n";
                d->mask |= emr_mask;
            }
            else {
                dbg_str << "<!-- ARC record is invalid -->\n";
            }
            break;
        }
        case U_EMR_CHORD:
        {
            dbg_str << "<!-- U_EMR_CHORD -->\n";
            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            if(!emr_arc_points( lpEMFR, &f1, f2, &center, &start, &end, &size)){
                tmp_path <<  "\n\tM " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y) << " \n";
                tmp_path << " z ";
                d->mask |= emr_mask;
            }
            else {
                dbg_str << "<!-- CHORD record is invalid -->\n";
            }
            break;
        }
        case U_EMR_PIE:
        {
            dbg_str << "<!-- U_EMR_PIE -->\n";
            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            if(!emr_arc_points( lpEMFR, &f1, f2, &center, &start, &end, &size)){
                tmp_path <<  "\n\tM " << pix_to_xy(d, center.x, center.y);
                tmp_path <<  "\n\tL " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y) << " \n";
                tmp_path << " z ";
                d->mask |= emr_mask;
            }
            else {
                dbg_str << "<!-- PIE record is invalid -->\n";
            }
            break;
        }
        case U_EMR_SELECTPALETTE:        dbg_str << "<!-- U_EMR_SELECTPALETTE -->\n";        break;
        case U_EMR_CREATEPALETTE:        dbg_str << "<!-- U_EMR_CREATEPALETTE -->\n";        break;
        case U_EMR_SETPALETTEENTRIES:    dbg_str << "<!-- U_EMR_SETPALETTEENTRIES -->\n";    break;
        case U_EMR_RESIZEPALETTE:        dbg_str << "<!-- U_EMR_RESIZEPALETTE -->\n";        break;
        case U_EMR_REALIZEPALETTE:       dbg_str << "<!-- U_EMR_REALIZEPALETTE -->\n";       break;
        case U_EMR_EXTFLOODFILL:         dbg_str << "<!-- U_EMR_EXTFLOODFILL -->\n";         break;
        case U_EMR_LINETO:
        {
            dbg_str << "<!-- U_EMR_LINETO -->\n";

            PU_EMRLINETO pEmr = (PU_EMRLINETO) lpEMFR;

            d->mask |= emr_mask;

            tmp_path <<
                "\n\tL " << pix_to_xy( d, pEmr->ptl.x, pEmr->ptl.y) << " ";
            break;
        }
        case U_EMR_ARCTO:
        {
            dbg_str << "<!-- U_EMR_ARCTO -->\n";
            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            if(!emr_arc_points( lpEMFR, &f1, f2, &center, &start, &end, &size)){
                // draw a line from current position to start, arc from there
                tmp_path <<  "\n\tL " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y)<< " ";

                d->mask |= emr_mask;
            }
            else {
                dbg_str << "<!-- ARCTO record is invalid -->\n";
            }
            break;
        }
        case U_EMR_POLYDRAW:             dbg_str << "<!-- U_EMR_POLYDRAW -->\n";             break;
        case U_EMR_SETARCDIRECTION:
        {
              dbg_str << "<!-- U_EMR_SETARCDIRECTION -->\n";
              PU_EMRSETARCDIRECTION pEmr = (PU_EMRSETARCDIRECTION) lpEMFR;
              if(d->arcdir == U_AD_CLOCKWISE || d->arcdir == U_AD_COUNTERCLOCKWISE){ // EMF file could be corrupt
                 d->arcdir = pEmr->iArcDirection;
              }
              break;
        }
        case U_EMR_SETMITERLIMIT:
        {
            dbg_str << "<!-- U_EMR_SETMITERLIMIT -->\n";

            PU_EMRSETMITERLIMIT pEmr = (PU_EMRSETMITERLIMIT) lpEMFR;

            //The function takes a float but saves a 32 bit int in the U_EMR_SETMITERLIMIT record.
            float miterlimit = *((int32_t *) &(pEmr->eMiterLimit));
            d->dc[d->level].style.stroke_miterlimit.value = miterlimit;   //ratio, not a pt size
            if (d->dc[d->level].style.stroke_miterlimit.value < 2)
                d->dc[d->level].style.stroke_miterlimit.value = 2.0;
            break;
        }
        case U_EMR_BEGINPATH:
        {
            dbg_str << "<!-- U_EMR_BEGINPATH -->\n";
            // The next line should never be needed, should have been handled before main switch
            // qualifier added because EMF's encountered where moveto preceded beginpath followed by lineto
            if(d->mask & U_DRAW_VISIBLE){
               d->path = "";
            }
            d->mask |= emr_mask;
            break;
        }
        case U_EMR_ENDPATH:
        {
            dbg_str << "<!-- U_EMR_ENDPATH -->\n";
            d->mask &= (0xFFFFFFFF - U_DRAW_ONLYTO);  // clear the OnlyTo bit (it might not have been set), prevents any further path extension
            break;
        }
        case U_EMR_CLOSEFIGURE:
        {
            dbg_str << "<!-- U_EMR_CLOSEFIGURE -->\n";
            // EMF may contain multiple closefigures on one path
            tmp_path << "\n\tz";
            d->mask |= U_DRAW_CLOSED;
            break;
        }
        case U_EMR_FILLPATH:
        {
            dbg_str << "<!-- U_EMR_FILLPATH -->\n";
            if(d->mask & U_DRAW_PATH){          // Operation only effects declared paths
                if(!(d->mask & U_DRAW_CLOSED)){  // Close a path not explicitly closed by an EMRCLOSEFIGURE, otherwise fill makes no sense
                    tmp_path << "\n\tz";
                    d->mask |= U_DRAW_CLOSED;
                }
                d->mask |= emr_mask;
                d->drawtype = U_EMR_FILLPATH;
            }
            break;
        }
        case U_EMR_STROKEANDFILLPATH:
        {
            dbg_str << "<!-- U_EMR_STROKEANDFILLPATH -->\n";
            if(d->mask & U_DRAW_PATH){          // Operation only effects declared paths
                if(!(d->mask & U_DRAW_CLOSED)){  // Close a path not explicitly closed by an EMRCLOSEFIGURE, otherwise fill makes no sense
                    tmp_path << "\n\tz";
                    d->mask |= U_DRAW_CLOSED;
                }
                d->mask |= emr_mask;
                d->drawtype = U_EMR_STROKEANDFILLPATH;
            }
            break;
        }
        case U_EMR_STROKEPATH:
        {
            dbg_str << "<!-- U_EMR_STROKEPATH -->\n";
            if(d->mask & U_DRAW_PATH){          // Operation only effects declared paths
                d->mask |= emr_mask;
                d->drawtype = U_EMR_STROKEPATH;
            }
            break;
        }
        case U_EMR_FLATTENPATH:          dbg_str << "<!-- U_EMR_FLATTENPATH -->\n";          break;
        case U_EMR_WIDENPATH:            dbg_str << "<!-- U_EMR_WIDENPATH -->\n";            break;
        case U_EMR_SELECTCLIPPATH:
        {
            dbg_str << "<!-- U_EMR_SELECTCLIPPATH -->\n";
            PU_EMRSELECTCLIPPATH pEmr = (PU_EMRSELECTCLIPPATH) lpEMFR;
            int logic = pEmr->iMode;

            if ((logic < U_RGN_MIN) || (logic > U_RGN_MAX)){ break; }
            add_clips(d, d->path.c_str(), logic); // finds an existing one or stores this, sets clip_id
            d->path = "";
            d->drawtype = 0;
            break;
        }
        case U_EMR_ABORTPATH:
        {
            dbg_str << "<!-- U_EMR_ABORTPATH -->\n";
            d->path = "";
            d->drawtype = 0;
            break;
        }
        case U_EMR_UNDEF69:              dbg_str << "<!-- U_EMR_UNDEF69 -->\n";              break;
        case U_EMR_COMMENT:
        {
            dbg_str << "<!-- U_EMR_COMMENT -->\n";

            PU_EMRCOMMENT pEmr = (PU_EMRCOMMENT) lpEMFR;

            char *szTxt = (char *) pEmr->Data;

            for (uint32_t i = 0; i < pEmr->cbData; i++) {
                if ( *szTxt) {
                    if ( *szTxt >= ' ' && *szTxt < 'z' && *szTxt != '<' && *szTxt != '>' ) {
                        tmp_str << *szTxt;
                    }
                    szTxt++;
                }
            }

            if (0 && strlen(tmp_str.str().c_str())) {
                tmp_outsvg << "   <!-- \"";
                tmp_outsvg << tmp_str.str().c_str();
                tmp_outsvg << "\" -->\n";
            }

            break;
        }
        case U_EMR_FILLRGN:              dbg_str << "<!-- U_EMR_FILLRGN -->\n";              break;
        case U_EMR_FRAMERGN:             dbg_str << "<!-- U_EMR_FRAMERGN -->\n";             break;
        case U_EMR_INVERTRGN:            dbg_str << "<!-- U_EMR_INVERTRGN -->\n";            break;
        case U_EMR_PAINTRGN:             dbg_str << "<!-- U_EMR_PAINTRGN -->\n";             break;
        case U_EMR_EXTSELECTCLIPRGN:
        {
            dbg_str << "<!-- U_EMR_EXTSELECTCLIPRGN -->\n";

            PU_EMREXTSELECTCLIPRGN pEmr = (PU_EMREXTSELECTCLIPRGN) lpEMFR;
            // the only mode we implement - this clears the clipping region
            if (pEmr->iMode == U_RGN_COPY) {
                d->dc[d->level].clip_id = 0;
            }
            break;
        }
        case U_EMR_BITBLT:
        {
            dbg_str << "<!-- U_EMR_BITBLT -->\n";

            PU_EMRBITBLT pEmr = (PU_EMRBITBLT) lpEMFR;
            //  Treat all nonImage bitblts as a rectangular write.  Definitely not correct, but at
            //  least it leaves objects where the operations should have been.
            if (!pEmr->cbBmiSrc) {
                // should be an application of a DIBPATTERNBRUSHPT, use a solid color instead

                if(pEmr->dwRop == U_NOOP)break; /* GDI applications apparently often end with this as a sort of flush(), nothing should be drawn */
                int32_t dx = pEmr->Dest.x;
                int32_t dy = pEmr->Dest.y;
                int32_t dw = pEmr->cDest.x;
                int32_t dh = pEmr->cDest.y;
                SVGOStringStream tmp_rectangle;
                tmp_rectangle << "\n\tM " << pix_to_xy( d, dx,      dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy + dh )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx,      dy + dh )    << " ";
                tmp_rectangle << "\n\tz";

                d->mask |= emr_mask;
                d->dwRop3 = pEmr->dwRop;   // we will try to approximate SOME of these
                d->mask |= U_DRAW_CLOSED; // Bitblit is not really open or closed, but we need it to fill, and this is the flag for that

                tmp_path <<   tmp_rectangle.str().c_str();
            }
            else {
                 double dx = pix_to_x_point( d, pEmr->Dest.x, pEmr->Dest.y);
                 double dy = pix_to_y_point( d, pEmr->Dest.x, pEmr->Dest.y);
                 double dw = pix_to_abs_size( d, pEmr->cDest.x);
                 double dh = pix_to_abs_size( d, pEmr->cDest.y);
                 //source position within the bitmap, in pixels
                 int sx = pEmr->Src.x + pEmr->xformSrc.eDx;
                 int sy = pEmr->Src.y + pEmr->xformSrc.eDy;
                 int sw = 0; // extract all of the image
                 int sh = 0;
                 if(sx<0)sx=0;
                 if(sy<0)sy=0;
                 common_image_extraction(d,pEmr,dx,dy,dw,dh,sx,sy,sw,sh,
                    pEmr->iUsageSrc, pEmr->offBitsSrc, pEmr->cbBitsSrc, pEmr->offBmiSrc, pEmr->cbBmiSrc);
            }
            break;
        }
        case U_EMR_STRETCHBLT:
        {
            dbg_str << "<!-- U_EMR_STRETCHBLT -->\n";
            PU_EMRSTRETCHBLT pEmr = (PU_EMRSTRETCHBLT) lpEMFR;
            // Always grab image, ignore modes.
            if (pEmr->cbBmiSrc) {
                double dx = pix_to_x_point( d, pEmr->Dest.x, pEmr->Dest.y);
                double dy = pix_to_y_point( d, pEmr->Dest.x, pEmr->Dest.y);
                double dw = pix_to_abs_size( d, pEmr->cDest.x);
                double dh = pix_to_abs_size( d, pEmr->cDest.y);
                //source position within the bitmap, in pixels
                int sx = pEmr->Src.x + pEmr->xformSrc.eDx;
                int sy = pEmr->Src.y + pEmr->xformSrc.eDy;
                int sw = pEmr->cSrc.x; // extract the specified amount of the image
                int sh = pEmr->cSrc.y;
                common_image_extraction(d,pEmr,dx,dy,dw,dh,sx,sy,sw,sh,
                    pEmr->iUsageSrc, pEmr->offBitsSrc, pEmr->cbBitsSrc, pEmr->offBmiSrc, pEmr->cbBmiSrc);
            }
            break;
        }
        case U_EMR_MASKBLT:
        {
            dbg_str << "<!-- U_EMR_MASKBLT -->\n";
            PU_EMRMASKBLT pEmr = (PU_EMRMASKBLT) lpEMFR;
            // Always grab image, ignore masks and modes.
            if (pEmr->cbBmiSrc) {
                double dx = pix_to_x_point( d, pEmr->Dest.x, pEmr->Dest.y);
                double dy = pix_to_y_point( d, pEmr->Dest.x, pEmr->Dest.y);
                double dw = pix_to_abs_size( d, pEmr->cDest.x);
                double dh = pix_to_abs_size( d, pEmr->cDest.y);
                int sx = pEmr->Src.x + pEmr->xformSrc.eDx;  //source position within the bitmap, in pixels
                int sy = pEmr->Src.y + pEmr->xformSrc.eDy;
                int sw = 0; // extract all of the image
                int sh = 0;
                common_image_extraction(d,pEmr,dx,dy,dw,dh,sx,sy,sw,sh,
                    pEmr->iUsageSrc, pEmr->offBitsSrc, pEmr->cbBitsSrc, pEmr->offBmiSrc, pEmr->cbBmiSrc);
            }
            break;
        }
        case U_EMR_PLGBLT:               dbg_str << "<!-- U_EMR_PLGBLT -->\n";               break;
        case U_EMR_SETDIBITSTODEVICE:    dbg_str << "<!-- U_EMR_SETDIBITSTODEVICE -->\n";    break;
        case U_EMR_STRETCHDIBITS:
        {
            // Some applications use multiple EMF operations, including multiple STRETCHDIBITS to create
            // images with transparent regions.  PowerPoint does this with rotated images, for instance.
            // Parsing all of that to derive a single resultant image object is left for a later version
            // of this code.  In the meantime, every STRETCHDIBITS goes directly to an image.  The Inkscape
            // user can sort out transparency later using Gimp, if need be.

            PU_EMRSTRETCHDIBITS pEmr = (PU_EMRSTRETCHDIBITS) lpEMFR;
            double dx = pix_to_x_point( d, pEmr->Dest.x, pEmr->Dest.y );
            double dy = pix_to_y_point( d, pEmr->Dest.x, pEmr->Dest.y );
            double dw = pix_to_abs_size( d, pEmr->cDest.x);
            double dh = pix_to_abs_size( d, pEmr->cDest.y);
            int sx = pEmr->Src.x;  //source position within the bitmap, in pixels
            int sy = pEmr->Src.y;
            int sw = pEmr->cSrc.x; // extract the specified amount of the image
            int sh = pEmr->cSrc.y;
            common_image_extraction(d,pEmr,dx,dy,dw,dh,sx,sy,sw,sh,
                pEmr->iUsageSrc, pEmr->offBitsSrc, pEmr->cbBitsSrc, pEmr->offBmiSrc, pEmr->cbBmiSrc);

            dbg_str << "<!-- U_EMR_STRETCHDIBITS -->\n";
            break;
        }
        case U_EMR_EXTCREATEFONTINDIRECTW:
        {
            dbg_str << "<!-- U_EMR_EXTCREATEFONTINDIRECTW -->\n";

            PU_EMREXTCREATEFONTINDIRECTW pEmr = (PU_EMREXTCREATEFONTINDIRECTW) lpEMFR;
            insert_object(d, pEmr->ihFont, U_EMR_EXTCREATEFONTINDIRECTW, lpEMFR);
            break;
        }
        case U_EMR_EXTTEXTOUTA:
        case U_EMR_EXTTEXTOUTW:
        case U_EMR_SMALLTEXTOUT:
        {
            dbg_str << "<!-- U_EMR_EXTTEXTOUTA/W -->\n";

            PU_EMREXTTEXTOUTW  pEmr  = (PU_EMREXTTEXTOUTW) lpEMFR;
            PU_EMRSMALLTEXTOUT pEmrS = (PU_EMRSMALLTEXTOUT) lpEMFR;

            double x1,y1;
            int roff   = sizeof(U_EMRSMALLTEXTOUT);  //offset to the start of the variable fields, only used with U_EMR_SMALLTEXTOUT
            int cChars;
            if(lpEMFR->iType==U_EMR_SMALLTEXTOUT){
                x1 = pEmrS->Dest.x;
                y1 = pEmrS->Dest.y;
                cChars = pEmrS->cChars;
                if(!(pEmrS->fuOptions & U_ETO_NO_RECT)){ roff += sizeof(U_RECTL); }
            }
            else {
                x1 = pEmr->emrtext.ptlReference.x;
                y1 = pEmr->emrtext.ptlReference.y;
                cChars = 0;
            }
            uint32_t fOptions = pEmr->emrtext.fOptions;

            if (d->dc[d->level].textAlign & U_TA_UPDATECP) {
                x1 = d->dc[d->level].cur.x;
                y1 = d->dc[d->level].cur.y;
            }

            double x = pix_to_x_point(d, x1, y1);
            double y = pix_to_y_point(d, x1, y1);

            /* Rotation issues are handled entirely in libTERE now */

            uint32_t *dup_wt = NULL;

            if(       lpEMFR->iType==U_EMR_EXTTEXTOUTA){
                /*  These should be JUST ASCII, but they might not be...
                    If it holds Utf-8 or plain ASCII the first call will succeed.
                    If not, assume that it holds Latin1.
                    If that fails then someting is really screwed up!
                */
                dup_wt = U_Utf8ToUtf32le((char *) pEmr + pEmr->emrtext.offString, pEmr->emrtext.nChars, NULL);
                if(!dup_wt)dup_wt = U_Latin1ToUtf32le((char *) pEmr + pEmr->emrtext.offString, pEmr->emrtext.nChars, NULL);
                if(!dup_wt)dup_wt = unknown_chars(pEmr->emrtext.nChars);
            }
            else if(  lpEMFR->iType==U_EMR_EXTTEXTOUTW){
                dup_wt = U_Utf16leToUtf32le((uint16_t *)((char *) pEmr + pEmr->emrtext.offString), pEmr->emrtext.nChars, NULL);
                if(!dup_wt)dup_wt = unknown_chars(pEmr->emrtext.nChars);
            }
            else { // U_EMR_SMALLTEXTOUT
                if(pEmrS->fuOptions & U_ETO_SMALL_CHARS){
                    dup_wt = U_Utf8ToUtf32le((char *) pEmrS + roff, cChars, NULL);
                }
                else {
                    dup_wt = U_Utf16leToUtf32le((uint16_t *)((char *) pEmrS + roff), cChars, NULL);
                }
                if(!dup_wt)dup_wt = unknown_chars(cChars);
            }

            msdepua(dup_wt); //convert everything in Microsoft's private use area.  For Symbol, Wingdings, Dingbats

            if(NonToUnicode(dup_wt, d->dc[d->level].font_name)){
                free(d->dc[d->level].font_name);
                d->dc[d->level].font_name =  strdup("Times New Roman");
            }

            char *ansi_text;
            ansi_text = (char *) U_Utf32leToUtf8((uint32_t *)dup_wt, 0, NULL);
            free(dup_wt);
            // Empty string or starts with an invalid escape/control sequence, which is bogus text.  Throw it out before g_markup_escape_text can make things worse
            if(*((uint8_t *)ansi_text) <= 0x1F){
                free(ansi_text);
                ansi_text=NULL;
            }

            if (ansi_text) {

                SVGOStringStream ts;

                gchar *escaped_text = g_markup_escape_text(ansi_text, -1);

                tsp.x              = x*0.8;  // TERE expects sizes in points.
                tsp.y              = y*0.8;
                tsp.color.Red      = d->dc[d->level].textColor.Red;
                tsp.color.Green    = d->dc[d->level].textColor.Green;
                tsp.color.Blue     = d->dc[d->level].textColor.Blue;
                tsp.color.Reserved = 0;
                switch(d->dc[d->level].style.font_style.value){
                  case SP_CSS_FONT_STYLE_OBLIQUE:
                     tsp.italics = FC_SLANT_OBLIQUE; break;
                  case SP_CSS_FONT_STYLE_ITALIC:
                     tsp.italics = FC_SLANT_ITALIC;  break;
                  default:
                  case SP_CSS_FONT_STYLE_NORMAL:
                     tsp.italics = FC_SLANT_ROMAN;   break;
                }
                switch(d->dc[d->level].style.font_weight.value){
                    case SP_CSS_FONT_WEIGHT_100:       tsp.weight =  FC_WEIGHT_THIN       ; break;
                    case SP_CSS_FONT_WEIGHT_200:       tsp.weight =  FC_WEIGHT_EXTRALIGHT ; break;
                    case SP_CSS_FONT_WEIGHT_300:       tsp.weight =  FC_WEIGHT_LIGHT      ; break;
                    case SP_CSS_FONT_WEIGHT_400:       tsp.weight =  FC_WEIGHT_NORMAL     ; break;
                    case SP_CSS_FONT_WEIGHT_500:       tsp.weight =  FC_WEIGHT_MEDIUM     ; break;
                    case SP_CSS_FONT_WEIGHT_600:       tsp.weight =  FC_WEIGHT_SEMIBOLD   ; break;
                    case SP_CSS_FONT_WEIGHT_700:       tsp.weight =  FC_WEIGHT_BOLD       ; break;
                    case SP_CSS_FONT_WEIGHT_800:       tsp.weight =  FC_WEIGHT_EXTRABOLD  ; break;
                    case SP_CSS_FONT_WEIGHT_900:       tsp.weight =  FC_WEIGHT_HEAVY      ; break;
                    case SP_CSS_FONT_WEIGHT_NORMAL:    tsp.weight =  FC_WEIGHT_NORMAL     ; break;
                    case SP_CSS_FONT_WEIGHT_BOLD:      tsp.weight =  FC_WEIGHT_BOLD       ; break;
                    case SP_CSS_FONT_WEIGHT_LIGHTER:   tsp.weight =  FC_WEIGHT_EXTRALIGHT ; break;
                    case SP_CSS_FONT_WEIGHT_BOLDER:    tsp.weight =  FC_WEIGHT_EXTRABOLD  ; break;
                    default:                           tsp.weight =  FC_WEIGHT_NORMAL     ; break;
                }
                // EMF only supports two types of text decoration
                tsp.decoration = TXTDECOR_NONE;
                if(d->dc[d->level].style.text_decoration_line.underline){    tsp.decoration |= TXTDECOR_UNDER; }
                if(d->dc[d->level].style.text_decoration_line.line_through){ tsp.decoration |= TXTDECOR_STRIKE;}

                // EMF textalignment is a bit strange: 0x6 is center, 0x2 is right, 0x0 is left, the value 0x4 is also drawn left
                tsp.taln  = ((d->dc[d->level].textAlign & U_TA_CENTER)  == U_TA_CENTER)  ?  ALICENTER :
                            (((d->dc[d->level].textAlign & U_TA_CENTER) == U_TA_LEFT)    ?  ALILEFT   :
                                                                                            ALIRIGHT);
                tsp.taln |= ((d->dc[d->level].textAlign & U_TA_BASEBIT) ?   ALIBASE :
                            ((d->dc[d->level].textAlign & U_TA_BOTTOM)  ?   ALIBOT  :
                                                                            ALITOP));

                // language direction can be encoded two ways, U_TA_RTLREADING is preferred 
                if( (fOptions & U_ETO_RTLREADING) || (d->dc[d->level].textAlign & U_TA_RTLREADING) ){ tsp.ldir = LDIR_RL; }
                else{                                                                                 tsp.ldir = LDIR_LR; }

                tsp.condensed = FC_WIDTH_NORMAL; // Not implemented well in libTERE (yet)
                tsp.ori = d->dc[d->level].style.baseline_shift.value;            // For now orientation is always the same as escapement
                tsp.ori += 180.0 * current_rotation(d)/ M_PI;                    // radians to degrees
                tsp.string = (uint8_t *) U_strdup(escaped_text);                 // this will be free'd much later at a trinfo_clear().
                tsp.fs = d->dc[d->level].style.font_size.computed * 0.8;         // Font size in points
                char *fontspec = TR_construct_fontspec(&tsp, d->dc[d->level].font_name);
                tsp.fi_idx = ftinfo_load_fontname(d->tri->fti,fontspec);
                free(fontspec);
                // when font name includes narrow it may not be set to "condensed".  Narrow fonts do not work well anyway though
                // as the metrics from fontconfig may not match, or the font may not be present.
                if(0<= TR_findcasesub(d->dc[d->level].font_name, (char *) "Narrow")){ tsp.co=1; }
                else {                                                                tsp.co=0; }

                int status = trinfo_load_textrec(d->tri, &tsp, tsp.ori,TR_EMFBOT);  // ori is actually escapement
                if(status==-1){ // change of escapement, emit what we have and reset
                    TR_layout_analyze(d->tri);
                    if (d->dc[d->level].clip_id){
                       SVGOStringStream tmp_clip;
                       tmp_clip << "\n<g\n\tclip-path=\"url(#clipEmfPath" << d->dc[d->level].clip_id << ")\"\n>";
                       d->outsvg += tmp_clip.str().c_str();
                    }
                    TR_layout_2_svg(d->tri);
                    ts << d->tri->out;
                    d->outsvg += ts.str().c_str();
                    d->tri = trinfo_clear(d->tri);
                    (void) trinfo_load_textrec(d->tri, &tsp, tsp.ori,TR_EMFBOT); // ignore return status, it must work
                    if (d->dc[d->level].clip_id){
                       d->outsvg += "\n</g>\n";
                    }        
                }

                g_free(escaped_text);
                free(ansi_text);
            }

            break;
        }
        case U_EMR_POLYBEZIER16:
        {
            dbg_str << "<!-- U_EMR_POLYBEZIER16 -->\n";

            PU_EMRPOLYBEZIER16 pEmr = (PU_EMRPOLYBEZIER16) lpEMFR;
            PU_POINT16 apts = (PU_POINT16) pEmr->apts; // Bug in MinGW wingdi.h ?
            uint32_t i,j;

            if (pEmr->cpts<4)
                break;

            d->mask |= emr_mask;

            tmp_str << "\n\tM " << pix_to_xy( d, apts[0].x, apts[0].y ) << " ";

            for (i=1; i<pEmr->cpts; ) {
                tmp_str << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cpts; j++,i++) {
                    tmp_str << pix_to_xy( d, apts[i].x, apts[i].y ) << " ";
                }
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_EMR_POLYGON16:
        {
            dbg_str << "<!-- U_EMR_POLYGON16 -->\n";

            PU_EMRPOLYGON16 pEmr = (PU_EMRPOLYGON16) lpEMFR;
            PU_POINT16 apts = (PU_POINT16) pEmr->apts; // Bug in MinGW wingdi.h ?
            SVGOStringStream tmp_poly;
            unsigned int i;
            unsigned int first = 0;

            d->mask |= emr_mask;

            // skip the first point?
            tmp_poly << "\n\tM " << pix_to_xy( d, apts[first].x, apts[first].y ) << " ";

            for (i=first+1; i<pEmr->cpts; i++) {
                tmp_poly << "\n\tL " << pix_to_xy( d, apts[i].x, apts[i].y ) << " ";
            }

            tmp_path <<  tmp_poly.str().c_str();
            tmp_path << "\n\tz";
            d->mask |= U_DRAW_CLOSED;

            break;
        }
        case U_EMR_POLYLINE16:
        {
            dbg_str << "<!-- U_EMR_POLYLINE16 -->\n";

            PU_EMRPOLYLINE16 pEmr = (PU_EMRPOLYLINE16) lpEMFR;
            PU_POINT16 apts = (PU_POINT16) pEmr->apts; // Bug in MinGW wingdi.h ?
            uint32_t i;

            if (pEmr->cpts<2)
                break;

            d->mask |= emr_mask;

            tmp_str << "\n\tM " << pix_to_xy( d, apts[0].x, apts[0].y ) << " ";

            for (i=1; i<pEmr->cpts; i++) {
                tmp_str << "\n\tL " << pix_to_xy( d, apts[i].x, apts[i].y ) << " ";
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_EMR_POLYBEZIERTO16:
        {
            dbg_str << "<!-- U_EMR_POLYBEZIERTO16 -->\n";

            PU_EMRPOLYBEZIERTO16 pEmr = (PU_EMRPOLYBEZIERTO16) lpEMFR;
            PU_POINT16 apts = (PU_POINT16) pEmr->apts; // Bug in MinGW wingdi.h ?
            uint32_t i,j;

            d->mask |= emr_mask;

            for (i=0; i<pEmr->cpts;) {
                tmp_path << "\n\tC ";
                for (j=0; j<3 && i<pEmr->cpts; j++,i++) {
                    tmp_path << pix_to_xy( d, apts[i].x, apts[i].y) << " ";
                }
            }

            break;
        }
        case U_EMR_POLYLINETO16:
        {
            dbg_str << "<!-- U_EMR_POLYLINETO16 -->\n";

            PU_EMRPOLYLINETO16 pEmr = (PU_EMRPOLYLINETO16) lpEMFR;
            PU_POINT16 apts = (PU_POINT16) pEmr->apts; // Bug in MinGW wingdi.h ?
            uint32_t i;

            d->mask |= emr_mask;

            for (i=0; i<pEmr->cpts;i++) {
                tmp_path << "\n\tL " << pix_to_xy( d, apts[i].x, apts[i].y) << " ";
            }

            break;
        }
        case U_EMR_POLYPOLYLINE16:
        case U_EMR_POLYPOLYGON16:
        {
            if (lpEMFR->iType == U_EMR_POLYPOLYLINE16)
                dbg_str << "<!-- U_EMR_POLYPOLYLINE16 -->\n";
            if (lpEMFR->iType == U_EMR_POLYPOLYGON16)
                dbg_str << "<!-- U_EMR_POLYPOLYGON16 -->\n";

            PU_EMRPOLYPOLYGON16 pEmr = (PU_EMRPOLYPOLYGON16) lpEMFR;
            unsigned int n, i, j;

            d->mask |= emr_mask;

            PU_POINT16 apts = (PU_POINT16) &pEmr->aPolyCounts[pEmr->nPolys];

            i = 0;
            for (n=0; n<pEmr->nPolys && i<pEmr->cpts; n++) {
                SVGOStringStream poly_path;

                poly_path << "\n\tM " << pix_to_xy( d, apts[i].x, apts[i].y) << " ";
                i++;

                for (j=1; j<pEmr->aPolyCounts[n] && i<pEmr->cpts; j++) {
                    poly_path << "\n\tL " << pix_to_xy( d, apts[i].x, apts[i].y) << " ";
                    i++;
                }

                tmp_str << poly_path.str().c_str();
                if (lpEMFR->iType == U_EMR_POLYPOLYGON16)
                    tmp_str << " z";
                tmp_str << " \n";
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_EMR_POLYDRAW16:           dbg_str << "<!-- U_EMR_POLYDRAW16 -->\n";           break;
        case U_EMR_CREATEMONOBRUSH:
        {
            dbg_str << "<!-- U_EMR_CREATEDIBPATTERNBRUSHPT -->\n";

            PU_EMRCREATEMONOBRUSH pEmr = (PU_EMRCREATEMONOBRUSH) lpEMFR;
            insert_object(d, pEmr->ihBrush, U_EMR_CREATEMONOBRUSH, lpEMFR);
            break;
        }
        case U_EMR_CREATEDIBPATTERNBRUSHPT:
        {
            dbg_str << "<!-- U_EMR_CREATEDIBPATTERNBRUSHPT -->\n";

            PU_EMRCREATEDIBPATTERNBRUSHPT pEmr = (PU_EMRCREATEDIBPATTERNBRUSHPT) lpEMFR;
            insert_object(d, pEmr->ihBrush, U_EMR_CREATEDIBPATTERNBRUSHPT, lpEMFR);
            break;
        }
        case U_EMR_EXTCREATEPEN:
        {
            dbg_str << "<!-- U_EMR_EXTCREATEPEN -->\n";

            PU_EMREXTCREATEPEN pEmr = (PU_EMREXTCREATEPEN) lpEMFR;
            insert_object(d, pEmr->ihPen, U_EMR_EXTCREATEPEN, lpEMFR);
            break;
        }
        case U_EMR_POLYTEXTOUTA:         dbg_str << "<!-- U_EMR_POLYTEXTOUTA -->\n";         break;
        case U_EMR_POLYTEXTOUTW:         dbg_str << "<!-- U_EMR_POLYTEXTOUTW -->\n";         break;
        case U_EMR_SETICMMODE:
        {
            dbg_str << "<!-- U_EMR_SETICMMODE -->\n";
            PU_EMRSETICMMODE pEmr = (PU_EMRSETICMMODE) lpEMFR;
            ICMmode= pEmr->iMode;
            break;
        }
        case U_EMR_CREATECOLORSPACE:     dbg_str << "<!-- U_EMR_CREATECOLORSPACE -->\n";     break;
        case U_EMR_SETCOLORSPACE:        dbg_str << "<!-- U_EMR_SETCOLORSPACE -->\n";        break;
        case U_EMR_DELETECOLORSPACE:     dbg_str << "<!-- U_EMR_DELETECOLORSPACE -->\n";     break;
        case U_EMR_GLSRECORD:            dbg_str << "<!-- U_EMR_GLSRECORD -->\n";            break;
        case U_EMR_GLSBOUNDEDRECORD:     dbg_str << "<!-- U_EMR_GLSBOUNDEDRECORD -->\n";     break;
        case U_EMR_PIXELFORMAT:          dbg_str << "<!-- U_EMR_PIXELFORMAT -->\n";          break;
        case U_EMR_DRAWESCAPE:           dbg_str << "<!-- U_EMR_DRAWESCAPE -->\n";           break;
        case U_EMR_EXTESCAPE:            dbg_str << "<!-- U_EMR_EXTESCAPE -->\n";            break;
        case U_EMR_UNDEF107:             dbg_str << "<!-- U_EMR_UNDEF107 -->\n";             break;
        // U_EMR_SMALLTEXTOUT is handled with U_EMR_EXTTEXTOUTA/W above
        case U_EMR_FORCEUFIMAPPING:      dbg_str << "<!-- U_EMR_FORCEUFIMAPPING -->\n";      break;
        case U_EMR_NAMEDESCAPE:          dbg_str << "<!-- U_EMR_NAMEDESCAPE -->\n";          break;
        case U_EMR_COLORCORRECTPALETTE:  dbg_str << "<!-- U_EMR_COLORCORRECTPALETTE -->\n";  break;
        case U_EMR_SETICMPROFILEA:       dbg_str << "<!-- U_EMR_SETICMPROFILEA -->\n";       break;
        case U_EMR_SETICMPROFILEW:       dbg_str << "<!-- U_EMR_SETICMPROFILEW -->\n";       break;
        case U_EMR_ALPHABLEND:           dbg_str << "<!-- U_EMR_ALPHABLEND -->\n";           break;
        case U_EMR_SETLAYOUT:            dbg_str << "<!-- U_EMR_SETLAYOUT -->\n";            break;
        case U_EMR_TRANSPARENTBLT:       dbg_str << "<!-- U_EMR_TRANSPARENTBLT -->\n";       break;
        case U_EMR_UNDEF117:             dbg_str << "<!-- U_EMR_UNDEF117 -->\n";             break;
        case U_EMR_GRADIENTFILL:
        {
            /*  Gradient fill is doable for rectangles because those correspond to linear gradients.  However,
                the general case for the triangle fill, with a different color in each corner of the triangle,
                has no SVG equivalent and cannot be easily emulated with SVG gradients. So the linear gradient
                is implemented, and the triangle fill just paints with the color of the first corner.
                
                This record can hold a series of gradients so we are forced to add path elements directly here, 
                it cannot wait for the top of the main loop.  Any existing path is erased.
            
            */
            dbg_str << "<!-- U_EMR_GRADIENTFILL -->\n";
            PU_EMRGRADIENTFILL pEmr = (PU_EMRGRADIENTFILL) lpEMFR;
            int     nV = pEmr->nTriVert;  //  Number of TriVertex objects
            int     nG = pEmr->nGradObj;  //  Number of gradient triangle/rectangle objects
            U_TRIVERTEX *tv  = (U_TRIVERTEX *)(((char *)lpEMFR) + sizeof(U_EMRGRADIENTFILL));
            if( pEmr->ulMode == U_GRADIENT_FILL_RECT_H ||
                pEmr->ulMode == U_GRADIENT_FILL_RECT_V
            ){
                 SVGOStringStream tmp_rectangle;
                 int i,fill_idx;
                 U_GRADIENT4 *rcs = (U_GRADIENT4 *)(((char *)lpEMFR) + sizeof(U_EMRGRADIENTFILL) + sizeof(U_TRIVERTEX)*nV);
                 for(i=0;i<nG;i++){
                     tmp_rectangle << "\n<path d=\"";
                     fill_idx = add_gradient(d, pEmr->ulMode, tv[rcs[i].UpperLeft], tv[rcs[i].LowerRight]);
                     tmp_rectangle << "\n\tM " << pix_to_xy( d, tv[rcs[i].UpperLeft ].x , tv[rcs[i].UpperLeft ].y )  << " ";
                     tmp_rectangle << "\n\tL " << pix_to_xy( d, tv[rcs[i].LowerRight].x , tv[rcs[i].UpperLeft ].y )  << " ";
                     tmp_rectangle << "\n\tL " << pix_to_xy( d, tv[rcs[i].LowerRight].x , tv[rcs[i].LowerRight].y )  << " ";
                     tmp_rectangle << "\n\tL " << pix_to_xy( d, tv[rcs[i].UpperLeft ].x , tv[rcs[i].LowerRight].y )  << " ";
                     tmp_rectangle << "\n\tz\"";
                     tmp_rectangle << "\n\tstyle=\"stroke:none;fill:url(#";
                     tmp_rectangle << d->gradients.strings[fill_idx];
                     tmp_rectangle << ");\"\n";
                     if (d->dc[d->level].clip_id){
                        tmp_rectangle << "\tclip-path=\"url(#clipEmfPath" << d->dc[d->level].clip_id << ")\"\n";
                     }
                     tmp_rectangle << "/>\n";
                 }
                 d->outsvg += tmp_rectangle.str().c_str();
            }
            else if(pEmr->ulMode == U_GRADIENT_FILL_TRIANGLE){
                 SVGOStringStream tmp_triangle;
                 char tmpcolor[8];
                 int i;
                 U_GRADIENT3 *tris = (U_GRADIENT3 *)(((char *)lpEMFR) + sizeof(U_EMRGRADIENTFILL) + sizeof(U_TRIVERTEX)*nV);
                 for(i=0;i<nG;i++){
                     tmp_triangle << "\n<path d=\"";
                     sprintf(tmpcolor,"%6.6X",sethexcolor(trivertex_to_colorref(tv[tris[i].Vertex1])));
                     tmp_triangle << "\n\tM " << pix_to_xy( d, tv[tris[i].Vertex1].x , tv[tris[i].Vertex1].y )  << " ";
                     tmp_triangle << "\n\tL " << pix_to_xy( d, tv[tris[i].Vertex2].x , tv[tris[i].Vertex2].y )  << " ";
                     tmp_triangle << "\n\tL " << pix_to_xy( d, tv[tris[i].Vertex3].x , tv[tris[i].Vertex3].y )  << " ";
                     tmp_triangle << "\n\tz\"";
                     tmp_triangle << "\n\tstyle=\"stroke:none;fill:#";
                     tmp_triangle << tmpcolor;
                     tmp_triangle << ";\"\n/>\n";
                 }
                 d->outsvg += tmp_triangle.str().c_str();
            }
            d->path = "";            
            // if it is anything else the record is bogus, so ignore it
            break;
        }
        case U_EMR_SETLINKEDUFIS:        dbg_str << "<!-- U_EMR_SETLINKEDUFIS -->\n";        break;
        case U_EMR_SETTEXTJUSTIFICATION: dbg_str << "<!-- U_EMR_SETTEXTJUSTIFICATION -->\n"; break;
        case U_EMR_COLORMATCHTOTARGETW:  dbg_str << "<!-- U_EMR_COLORMATCHTOTARGETW -->\n";  break;
        case U_EMR_CREATECOLORSPACEW:    dbg_str << "<!-- U_EMR_CREATECOLORSPACEW -->\n";    break;
        default:
            dbg_str << "<!-- U_EMR_??? -->\n";
            break;
    }  //end of switch
// When testing, uncomment the following to place a comment for each processed EMR record in the SVG
//    d->outsvg += dbg_str.str().c_str();
    d->outsvg += tmp_outsvg.str().c_str();
    d->path += tmp_path.str().c_str();

    }  //end of while
// When testing, uncomment the following to show the final SVG derived from the EMF
// std::cout << d->outsvg << std::endl;
    (void) emr_properties(U_EMR_INVALID);  // force the release of the lookup table memory, returned value is irrelevant

    return(file_status);
}

void Emf::free_emf_strings(EMF_STRINGS name){
    if(name.count){
        for(int i=0; i< name.count; i++){ free(name.strings[i]); }
        free(name.strings);
    }
    name.count = 0;
    name.size = 0;
}

SPDocument *
Emf::open( Inkscape::Extension::Input * /*mod*/, const gchar *uri )
{
    if (uri == NULL) {
        return NULL;
    }

    EMF_CALLBACK_DATA d;

    d.n_obj   = 0;     //these might not be set otherwise if the input file is corrupt
    d.emf_obj = NULL; 
    d.dc[0].font_name = strdup("Arial"); // Default font, set only on lowest level, it copies up from there EMF spec says device can pick whatever it wants

    // set up the size default for patterns in defs.  This might not be referenced if there are no patterns defined in the drawing.

    d.defs += "\n";
    d.defs += "   <pattern id=\"EMFhbasepattern\"     \n";
    d.defs += "        patternUnits=\"userSpaceOnUse\"\n";
    d.defs += "        width=\"6\"                    \n";
    d.defs += "        height=\"6\"                   \n";
    d.defs += "        x=\"0\"                        \n";
    d.defs += "        y=\"0\">                       \n";
    d.defs += "   </pattern>                          \n";


    size_t length;
    char *contents;
    if(emf_readdata(uri, &contents, &length))return(NULL);

    d.pDesc = NULL;

    // set up the text reassembly system
    if(!(d.tri = trinfo_init(NULL)))return(NULL);
    (void) trinfo_load_ft_opts(d.tri, 1,
      FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING  | FT_LOAD_NO_BITMAP,
      FT_KERNING_UNSCALED);

    int good = myEnhMetaFileProc(contents,length, &d);
    free(contents);

    if (d.pDesc){ free( d.pDesc ); }

//    std::cout << "SVG Output: " << std::endl << d.outsvg << std::endl;

    SPDocument *doc = NULL;
    if (good) {
        doc = SPDocument::createNewDocFromMem(d.outsvg.c_str(), strlen(d.outsvg.c_str()), TRUE);
    }

    free_emf_strings(d.hatches);
    free_emf_strings(d.images);
    free_emf_strings(d.gradients);
    free_emf_strings(d.clips);

    if (d.emf_obj) {
        int i;
        for (i=0; i<d.n_obj; i++)
            delete_object(&d, i);
        delete[] d.emf_obj;
    }

    d.dc[0].style.stroke_dasharray.values.clear();

    for(int i=0; i<=EMF_MAX_DC; i++){
      if(d.dc[i].font_name)free(d.dc[i].font_name);
    }

    d.tri = trinfo_release_except_FC(d.tri);

    // in earlier versions no viewbox was generated and a call to setViewBoxIfMissing() was needed here.

    return doc;
}


void
Emf::init (void)
{
    /* EMF in */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("EMF Input") "</name>\n"
            "<id>org.inkscape.input.emf</id>\n"
            "<input>\n"
                "<extension>.emf</extension>\n"
                "<mimetype>image/x-emf</mimetype>\n"
                "<filetypename>" N_("Enhanced Metafiles (*.emf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Enhanced Metafiles") "</filetypetooltip>\n"
                "<output_extension>org.inkscape.output.emf</output_extension>\n"
            "</input>\n"
        "</inkscape-extension>", new Emf());

    /* EMF out */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("EMF Output") "</name>\n"
            "<id>org.inkscape.output.emf</id>\n"
            "<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">true</param>\n"
            "<param name=\"TnrToSymbol\" gui-text=\"" N_("Map Unicode to Symbol font") "\" type=\"boolean\">true</param>\n"
            "<param name=\"TnrToWingdings\" gui-text=\"" N_("Map Unicode to Wingdings") "\" type=\"boolean\">true</param>\n"
            "<param name=\"TnrToZapfDingbats\" gui-text=\"" N_("Map Unicode to Zapf Dingbats") "\" type=\"boolean\">true</param>\n"
            "<param name=\"UsePUA\" gui-text=\"" N_("Use MS Unicode PUA (0xF020-0xF0FF) for converted characters") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTCharPos\" gui-text=\"" N_("Compensate for PPT font bug") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTDashLine\" gui-text=\"" N_("Convert dashed/dotted lines to single lines") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTGrad2Polys\" gui-text=\"" N_("Convert gradients to colored polygon series") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTLinGrad\" gui-text=\"" N_("Use native rectangular linear gradients") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTPatternAsHatch\" gui-text=\"" N_("Map all fill patterns to standard EMF hatches") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixImageRot\" gui-text=\"" N_("Ignore image rotations") "\" type=\"boolean\">false</param>\n"
            "<output>\n"
                "<extension>.emf</extension>\n"
                "<mimetype>image/x-emf</mimetype>\n"
                "<filetypename>" N_("Enhanced Metafile (*.emf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Enhanced Metafile") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new Emf());

    return;
}


} } }  /* namespace Inkscape, Extension, Implementation */

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
