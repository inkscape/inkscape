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

//#include <png.h>   //This must precede text_reassemble.h or it blows up in pngconf.h when compiling
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
#include "util/units.h" // even though it is included indirectly by wmf-inout.h
#include "inkscape.h" // even though it is included indirectly by wmf-inout.h


#include "wmf-inout.h"
#include "wmf-print.h"

#define PRINT_WMF "org.inkscape.print.wmf"

#ifndef U_PS_JOIN_MASK
#define U_PS_JOIN_MASK (U_PS_JOIN_BEVEL|U_PS_JOIN_MITER|U_PS_JOIN_ROUND)
#endif

namespace Inkscape {
namespace Extension {
namespace Internal {


static bool clipset = false;
static uint32_t BLTmode=0;

Wmf::Wmf (void) // The null constructor
{
    return;
}


Wmf::~Wmf (void) //The destructor
{
    return;
}


bool
Wmf::check (Inkscape::Extension::Extension * /*module*/)
{
    if (NULL == Inkscape::Extension::db.get(PRINT_WMF))
        return FALSE;
    return TRUE;
}


void
Wmf::print_document_to_file(SPDocument *doc, const gchar *filename)
{
    Inkscape::Extension::Print *mod;
    SPPrintContext context;
    const gchar *oldconst;
    gchar *oldoutput;

    doc->ensureUpToDate();

    mod = Inkscape::Extension::get_print(PRINT_WMF);
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
    if (mod->begin(doc)) {
        g_free(oldoutput);
        throw Inkscape::Extension::Output::save_failed();
    }
    mod->base->invoke_print(&context);
    mod->finish();
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
Wmf::save(Inkscape::Extension::Output *mod, SPDocument *doc, gchar const *filename)
{
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::db.get(PRINT_WMF);
    if (ext == NULL)
        return;

    bool new_val         = mod->get_param_bool("textToPath");
    bool new_FixPPTCharPos     = mod->get_param_bool("FixPPTCharPos");  // character position bug
    // reserve FixPPT2 for opacity bug.  Currently WMF does not export opacity values
    bool new_FixPPTDashLine       = mod->get_param_bool("FixPPTDashLine");  // dashed line bug
    bool new_FixPPTGrad2Polys     = mod->get_param_bool("FixPPTGrad2Polys");  // gradient bug
    bool new_FixPPTPatternAsHatch = mod->get_param_bool("FixPPTPatternAsHatch");  // force all patterns as standard WMF hatch

    TableGen(                  //possibly regenerate the unicode-convert tables
        mod->get_param_bool("TnrToSymbol"),
        mod->get_param_bool("TnrToWingdings"),
        mod->get_param_bool("TnrToZapfDingbats"),
        mod->get_param_bool("UsePUA")
    );

    ext->set_param_bool("FixPPTCharPos",new_FixPPTCharPos);   // Remember to add any new ones to PrintWmf::init or a mysterious failure will result!
    ext->set_param_bool("FixPPTDashLine",new_FixPPTDashLine);
    ext->set_param_bool("FixPPTGrad2Polys",new_FixPPTGrad2Polys);
    ext->set_param_bool("FixPPTPatternAsHatch",new_FixPPTPatternAsHatch);
    ext->set_param_bool("textToPath", new_val);

    print_document_to_file(doc, filename);

    return;
}


/* WMF has no worldTransform, so this always returns 1.0.  Retain it to keep WMF and WMF in sync as much as possible.*/
double Wmf::current_scale(PWMF_CALLBACK_DATA /*d*/){
    return 1.0;
}

/* WMF has no worldTransform, so this always returns an Identity rotation matrix, but the offsets may have values.*/
std::string Wmf::current_matrix(PWMF_CALLBACK_DATA d, double x, double y, int useoffset){
    SVGOStringStream cxform;
    double scale = current_scale(d);
    cxform << "\"matrix(";
    cxform << 1.0/scale;   cxform << ",";
    cxform << 0.0;         cxform << ",";
    cxform << 0.0;         cxform << ",";
    cxform << 1.0/scale;   cxform << ",";
    if(useoffset){  cxform << x;  cxform << ","; cxform << y; }
    else {          cxform << "0,0"; }
    cxform << ")\"";
    return(cxform.str());
}

/* WMF has no worldTransform, so this always returns 0.  Retain it to keep WMF and WMF in sync as much as possible.*/
double Wmf::current_rotation(PWMF_CALLBACK_DATA /*d*/){
    return 0.0;
}

/*  Add another 100 blank slots to the hatches array.
*/
void Wmf::enlarge_hatches(PWMF_CALLBACK_DATA d){
    d->hatches.size += 100;
    d->hatches.strings = (char **) realloc(d->hatches.strings,d->hatches.size * sizeof(char *));
}

/*  See if the pattern name is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Wmf::in_hatches(PWMF_CALLBACK_DATA d, char *test){
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
uint32_t Wmf::add_hatch(PWMF_CALLBACK_DATA d, uint32_t hatchType, U_COLORREF hatchColor){
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
    sprintf(hpathname,"WMFhpath%d_%s",hatchType,tmpcolor);
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
        sprintf(hatchname,"WMFhatch%d_%s",hatchType,tmpcolor);
        sprintf(hpathname,"WMFhpath%d_%s",hatchType,tmpcolor);
        idx = in_hatches(d,hatchname);
        if(!idx){  // add it if not already present
            if(d->hatches.count == d->hatches.size){  enlarge_hatches(d); }
            d->hatches.strings[d->hatches.count++]=strdup(hatchname);
            d->defs += "\n";
            d->defs += "   <pattern id=\"";
            d->defs += hatchname;
            d->defs += "\"  xlink:href=\"#WMFhbasepattern\">\n";
            d->defs += refpath;
            d->defs += "   </pattern>\n";
            idx = d->hatches.count;
        }
    }
    else { //  bkMode==U_OPAQUE
        /* Set up an object in the defs for this background, if there is not one already there */
        sprintf(bkcolor,"%6.6X",sethexcolor(d->dc[d->level].bkColor));
        sprintf(hbkname,"WMFhbkclr_%s",bkcolor);
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
        sprintf(hatchname,"WMFhatch%d_%s_%s",hatchType,tmpcolor,bkcolor);
        idx = in_hatches(d,hatchname);
        if(!idx){  // add it if not already present
            if(d->hatches.count == d->hatches.size){  enlarge_hatches(d); }
            d->hatches.strings[d->hatches.count++]=strdup(hatchname);
            d->defs += "\n";
            d->defs += "   <pattern id=\"";
            d->defs += hatchname;
            d->defs += "\"  xlink:href=\"#WMFhbasepattern\">\n";
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
void Wmf::enlarge_images(PWMF_CALLBACK_DATA d){
    d->images.size += 100;
    d->images.strings = (char **) realloc(d->images.strings,d->images.size * sizeof(char *));
}

/*  See if the image string is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Wmf::in_images(PWMF_CALLBACK_DATA d, char *test){
    int i;
    for(i=0; i<d->images.count; i++){
        if(strcmp(test,d->images.strings[i])==0)return(i+1);
    }
    return(0);
}

/*  (Conditionally) add an image from a DIB.  If a matching image already exists nothing happens.  If one
    does not exist it is added to the images list and also entered into <defs>.

*/
uint32_t Wmf::add_dib_image(PWMF_CALLBACK_DATA d, const char *dib, uint32_t iUsage){

    uint32_t idx;
    char imagename[64];             // big enough
    char xywh[64];                  // big enough
    int  dibparams = U_BI_UNKNOWN;  // type of image not yet determined

    MEMPNG mempng; // PNG in memory comes back in this
    mempng.buffer = NULL;

    char            *rgba_px = NULL;     // RGBA pixels
    const char      *px      = NULL;     // DIB pixels
    const U_RGBQUAD *ct      = NULL;     // DIB color table
    uint32_t numCt;
    int32_t  width, height, colortype, invert; // if needed these values will be set by wget_DIB_params
    if(iUsage == U_DIB_RGB_COLORS){
        // next call returns pointers and values, but allocates no memory
        dibparams = wget_DIB_params(dib, &px, &ct, &numCt, &width, &height, &colortype, &invert);
        if(dibparams == U_BI_RGB){
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
    else {                         // failed conversion, insert the common bad image picture
        width  = 3;
        height = 4;
        base64String = bad_image_png();
    }
    idx = in_images(d, (char *) base64String);
    if(!idx){  // add it if not already present - we looked at the actual data for comparison
        if(d->images.count == d->images.size){  enlarge_images(d); }
        idx = d->images.count;
        d->images.strings[d->images.count++]=strdup(base64String);

        sprintf(imagename,"WMFimage%d",idx++);
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
        d->defs += " preserveAspectRatio=\"none\"\n";
        d->defs += "   />\n";


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
    g_free(base64String); //wait until this point to free because it might be a duplicate image
    return(idx-1);
}

/*  (Conditionally) add an image from a Bitmap16.  If a matching image already exists nothing happens.  If one
    does not exist it is added to the images list and also entered into <defs>.

*/
uint32_t Wmf::add_bm16_image(PWMF_CALLBACK_DATA d, U_BITMAP16 Bm16, const char *px){

    uint32_t idx;
    char imagename[64]; // big enough
    char xywh[64]; // big enough

    MEMPNG mempng; // PNG in memory comes back in this
    mempng.buffer = NULL;

    char            *rgba_px = NULL;     // RGBA pixels
    const U_RGBQUAD *ct      = NULL;     // color table, always NULL here
    int32_t    width, height, colortype, numCt, invert;
    numCt     = 0;
    width     = Bm16.Width;              //  bitmap width in pixels.
    height    = Bm16.Height;             //  bitmap height in scan lines.
    colortype = Bm16.BitsPixel;          //  seems to be BitCount Enumeration
    invert    = 0;
    if(colortype < 16)return(U_WMR_INVALID);  // these would need a colortable if they were a dib, no idea what bm16 is supposed to do instead.

    if(!DIB_to_RGBA(// This is not really a dib, but close enough so that it still works.
        px,         // DIB pixel array
        ct,         // DIB color table (always NULL here)
        numCt,      // DIB color table number of entries (always 0)
        &rgba_px,   // U_RGBA pixel array (32 bits), created by this routine, caller must free.
        width,      // Width of pixel array
        height,     // Height of pixel array
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

    gchar *base64String=NULL;
    if(mempng.buffer){             // image was Bm16 in source file, converted to png in this routine
        base64String = g_base64_encode((guchar*) mempng.buffer, mempng.size );
        free(mempng.buffer);
    }
    else {                         // failed conversion, insert the common bad image picture
        width  = 3;
        height = 4;
        base64String = bad_image_png();
    }

    idx = in_images(d, (char *) base64String);
    if(!idx){  // add it if not already present - we looked at the actual data for comparison
        if(d->images.count == d->images.size){  enlarge_images(d); }
        idx = d->images.count;
        d->images.strings[d->images.count++]=g_strdup(base64String);

        sprintf(imagename,"WMFimage%d",idx++);
        sprintf(xywh," x=\"0\" y=\"0\" width=\"%d\" height=\"%d\" ",width,height); // reuse this buffer

        d->defs += "\n";
        d->defs += "   <image id=\"";
        d->defs += imagename;
        d->defs += "\"\n      ";
        d->defs += xywh;
        d->defs += "\n";
        d->defs += "       xlink:href=\"data:image/png;base64,";
        d->defs += base64String;
        d->defs += "\"\n";
        d->defs += " preserveAspectRatio=\"none\"\n";
        d->defs += "   />\n";


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
        d->defs += "   </pattern>\n";
    }
    g_free(base64String); //wait until this point to free because it might be a duplicate image
    return(idx-1);
}

/*  Add another 100 blank slots to the clips array.
*/
void Wmf::enlarge_clips(PWMF_CALLBACK_DATA d){
    d->clips.size += 100;
    d->clips.strings = (char **) realloc(d->clips.strings,d->clips.size * sizeof(char *));
}

/*  See if the pattern name is already in the list.  If it is return its position (1->n, not 1-n-1)
*/
int Wmf::in_clips(PWMF_CALLBACK_DATA d, const char *test){
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
void Wmf::add_clips(PWMF_CALLBACK_DATA d, const char *clippath, unsigned int logic){
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
        tmp_clippath << "\n\tid=\"clipWmfPath" << d->dc[d->level].clip_id << "\"";
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
Wmf::output_style(PWMF_CALLBACK_DATA d)
{
//    SVGOStringStream tmp_id;
    SVGOStringStream tmp_style;
    char tmp[1024] = {0};

    float fill_rgb[3];
    sp_color_get_rgb_floatv( &(d->dc[d->level].style.fill.value.color), fill_rgb );
    float stroke_rgb[3];
    sp_color_get_rgb_floatv(&(d->dc[d->level].style.stroke.value.color), stroke_rgb);

    // for U_WMR_BITBLT with no image, try to approximate some of these operations/
    // Assume src color is "white"
    if(d->dwRop3){
        switch(d->dwRop3){
            case U_PATINVERT: // treat all of these as black
            case U_SRCINVERT:
            case U_DSTINVERT:
            case U_BLACKNESS:
            case U_SRCERASE:
            case U_NOTSRCCOPY:
                fill_rgb[0]=fill_rgb[1]=fill_rgb[2]=0.0;
                break;
            case U_SRCCOPY:    // treat all of these as white
            case U_NOTSRCERASE:
            case U_PATCOPY:
            case U_WHITENESS:
                fill_rgb[0]=fill_rgb[1]=fill_rgb[2]=1.0;
                break;
            case U_SRCPAINT:  // use the existing color
            case U_SRCAND:
            case U_MERGECOPY:
            case U_MERGEPAINT:
            case U_PATPAINT:
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
    if (!d->dc[d->level].fill_set ||  ( d->mask & U_DRAW_NOFILL)) { // nofill are lines and arcs
        tmp_style << "fill:none;";
    } else {
        switch(d->dc[d->level].fill_mode){
            // both of these use the url(#) method
            case DRAW_PATTERN:
                snprintf(tmp, 1023, "fill:url(#%s); ",d->hatches.strings[d->dc[d->level].fill_idx]);
                tmp_style << tmp;
                break;
            case DRAW_IMAGE:
                snprintf(tmp, 1023, "fill:url(#WMFimage%d_ref); ",d->dc[d->level].fill_idx);
                tmp_style << tmp;
                break;
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

    if (!d->dc[d->level].stroke_set) {
        tmp_style << "stroke:none;";
    } else {
        switch(d->dc[d->level].stroke_mode){
            // both of these use the url(#) method
            case DRAW_PATTERN:
                snprintf(tmp, 1023, "stroke:url(#%s); ",d->hatches.strings[d->dc[d->level].stroke_idx]);
                tmp_style << tmp;
                break;
            case DRAW_IMAGE:
                snprintf(tmp, 1023, "stroke:url(#WMFimage%d_ref); ",d->dc[d->level].stroke_idx);
                tmp_style << tmp;
                break;
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
        if(d->dc[d->level].style.stroke_width.value){
            tmp_style << "stroke-width:" <<
                MAX( 0.001, d->dc[d->level].style.stroke_width.value ) << "px;";
        }
        else { // In a WMF a 0 width pixel means "1 pixel"
            tmp_style << "stroke-width:" << pix_to_abs_size( d, 1 ) << "px;";
        }

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
            !d->dc[d->level].style.stroke_dasharray.values.empty())
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
        tmp_style << "\n\tclip-path=\"url(#clipWmfPath" << d->dc[d->level].clip_id << ")\" ";

    d->outsvg += tmp_style.str().c_str();
}


double
Wmf::_pix_x_to_point(PWMF_CALLBACK_DATA d, double px)
{
    double scale = (d->dc[d->level].ScaleInX ? d->dc[d->level].ScaleInX : 1.0);
    double tmp;
    tmp = ((((double) (px - d->dc[d->level].winorg.x))*scale) + d->dc[d->level].vieworg.x) * d->D2PscaleX;
    tmp -= d->ulCornerOutX; //The WMF boundary rectangle can be anywhere, place its upper left corner in the Inkscape upper left corner
    return(tmp);
}

double
Wmf::_pix_y_to_point(PWMF_CALLBACK_DATA d, double py)
{
    double scale = (d->dc[d->level].ScaleInY ? d->dc[d->level].ScaleInY : 1.0);
    double tmp;
    tmp = ((((double) (py - d->dc[d->level].winorg.y))*scale) * d->E2IdirY + d->dc[d->level].vieworg.y) * d->D2PscaleY;
    tmp -= d->ulCornerOutY; //The WMF boundary rectangle can be anywhere, place its upper left corner in the Inkscape upper left corner
    return(tmp);
}


double
Wmf::pix_to_x_point(PWMF_CALLBACK_DATA d, double px, double /*py*/)
{
    double x   = _pix_x_to_point(d, px);
    return x;
}

double
Wmf::pix_to_y_point(PWMF_CALLBACK_DATA d, double /*px*/, double py)
{
    double y   = _pix_y_to_point(d, py);
    return y;

}

double
Wmf::pix_to_abs_size(PWMF_CALLBACK_DATA d, double px)
{
    double  ppx = fabs(px * (d->dc[d->level].ScaleInX ? d->dc[d->level].ScaleInX : 1.0) * d->D2PscaleX * current_scale(d));
    return ppx;
}

/* returns "x,y" (without the quotes) in inkscape coordinates for a pair of WMF x,y coordinates
*/
std::string Wmf::pix_to_xy(PWMF_CALLBACK_DATA d, double x, double y){
    SVGOStringStream cxform;
    cxform << pix_to_x_point(d,x,y);
    cxform << ",";
    cxform << pix_to_y_point(d,x,y);
    return(cxform.str());
}


void
Wmf::select_pen(PWMF_CALLBACK_DATA d, int index)
{
    int width;
    char *record = NULL;
    U_PEN up;

    if (index < 0 && index >= d->n_obj){ return; }
    record = d->wmf_obj[index].record;
    if(!record){ return; }
    d->dc[d->level].active_pen = index;

    (void) U_WMRCREATEPENINDIRECT_get(record, &up);
    width = up.Widthw[0];  // width is stored in the first 16 bits of the 32.

    switch (up.Style & U_PS_STYLE_MASK) {
        case U_PS_DASH:
        case U_PS_DOT:
        case U_PS_DASHDOT:
        case U_PS_DASHDOTDOT:
        {
            int penstyle = (up.Style & U_PS_STYLE_MASK);
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

    switch (up.Style & U_PS_ENDCAP_MASK) {
        case U_PS_ENDCAP_ROUND: {  d->dc[d->level].style.stroke_linecap.computed = 1;   break; }
        case U_PS_ENDCAP_SQUARE: { d->dc[d->level].style.stroke_linecap.computed = 2;   break; }
        case U_PS_ENDCAP_FLAT:
        default: {                 d->dc[d->level].style.stroke_linecap.computed = 0;   break; }
    }

    switch (up.Style & U_PS_JOIN_MASK) {
        case U_PS_JOIN_BEVEL: {    d->dc[d->level].style.stroke_linejoin.computed = 2;  break; }
        case U_PS_JOIN_MITER: {    d->dc[d->level].style.stroke_linejoin.computed = 0;  break; }
        case U_PS_JOIN_ROUND:
        default: {                 d->dc[d->level].style.stroke_linejoin.computed = 1;  break; }
    }


    double pen_width;
    if (up.Style == U_PS_NULL) {
        d->dc[d->level].stroke_set = false;
        pen_width =0.0;
    } else if (width) {
        d->dc[d->level].stroke_set = true;
        int cur_level = d->level;
        d->level = d->wmf_obj[index].level;   // this object may have been defined in some other DC.
        pen_width = pix_to_abs_size( d, width );
        d->level = cur_level;
    } else { // this stroke should always be rendered as 1 pixel wide, independent of zoom level (can that be done in SVG?)
        d->dc[d->level].stroke_set = true;
        int cur_level = d->level;
        d->level = d->wmf_obj[index].level;   // this object may have been defined in some other DC.
        pen_width = pix_to_abs_size( d, 1 );
        d->level = cur_level;
    }
    d->dc[d->level].style.stroke_width.value = pen_width;

    double r, g, b;
    r = SP_COLOR_U_TO_F( U_RGBAGetR(up.Color) );
    g = SP_COLOR_U_TO_F( U_RGBAGetG(up.Color) );
    b = SP_COLOR_U_TO_F( U_RGBAGetB(up.Color) );
    d->dc[d->level].style.stroke.value.color.set( r, g, b );
}


void
Wmf::select_brush(PWMF_CALLBACK_DATA d, int index)
{
    uint8_t      iType;
    char        *record;
    const char  *membrush;

    if (index < 0 || index >= d->n_obj)return;
    record = d->wmf_obj[index].record;
    if(!record)return;
    d->dc[d->level].active_brush = index;

    iType     = *(uint8_t *)(record + offsetof(U_METARECORD, iType )  );
    if(iType == U_WMR_CREATEBRUSHINDIRECT){
        U_WLOGBRUSH  lb;
        (void) U_WMRCREATEBRUSHINDIRECT_get(record, &membrush);
        memcpy(&lb, membrush, U_SIZE_WLOGBRUSH);
        if(lb.Style == U_BS_SOLID){
            double r, g, b;
            r = SP_COLOR_U_TO_F( U_RGBAGetR(lb.Color) );
            g = SP_COLOR_U_TO_F( U_RGBAGetG(lb.Color) );
            b = SP_COLOR_U_TO_F( U_RGBAGetB(lb.Color) );
            d->dc[d->level].style.fill.value.color.set( r, g, b );
            d->dc[d->level].fill_mode    = DRAW_PAINT;
            d->dc[d->level].fill_set     = true;
        }
        else if(lb.Style == U_BS_HATCHED){
            d->dc[d->level].fill_idx     = add_hatch(d, lb.Hatch, lb.Color);
            d->dc[d->level].fill_recidx  = index; // used if the hatch needs to be redone due to bkMode, textmode, etc. changes
            d->dc[d->level].fill_mode    = DRAW_PATTERN;
            d->dc[d->level].fill_set     = true;
        }
        else if(lb.Style == U_BS_NULL){
            d->dc[d->level].fill_mode    = DRAW_PAINT;  // set it to something
            d->dc[d->level].fill_set     = false;
        }
    }
    else if(iType == U_WMR_DIBCREATEPATTERNBRUSH){
        uint32_t    tidx;
        uint16_t    Style;
        uint16_t    cUsage;
        const char *Bm16h;  // Pointer to Bitmap16 header (px follows)
        const char *dib;    // Pointer to DIB
        (void) U_WMRDIBCREATEPATTERNBRUSH_get(record, &Style, &cUsage, &Bm16h, &dib);
        if(dib || Bm16h){
            if(dib){ tidx = add_dib_image(d, dib, cUsage); }
            else if(Bm16h){
                U_BITMAP16  Bm16;
                const char *px;
                memcpy(&Bm16, Bm16h, U_SIZE_BITMAP16);
                px = Bm16h + U_SIZE_BITMAP16;
                tidx = add_bm16_image(d, Bm16, px);
            }
            if(tidx == U_WMR_INVALID){  // Problem with the image, for instance, an unsupported bitmap16 type
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
        else {
            g_message("Please send WMF file to developers - select_brush U_WMR_DIBCREATEPATTERNBRUSH not bm16 or dib, not handled");
        }
    }
    else if(iType == U_WMR_CREATEPATTERNBRUSH){
        uint32_t    tidx;
        int         cbPx;
        U_BITMAP16  Bm16h;
        const char *px;
        if(U_WMRCREATEPATTERNBRUSH_get(record, &Bm16h, &cbPx, &px)){
            tidx = add_bm16_image(d, Bm16h, px);
            if(tidx == 0xFFFFFFFF){  // Problem with the image, for instance, an unsupported bitmap16 type
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
Wmf::select_font(PWMF_CALLBACK_DATA d, int index)
{
    char         *record = NULL;
    const char   *memfont;
    const char   *facename;
    U_FONT        font;

    if (index < 0 || index >= d->n_obj)return;
    record = d->wmf_obj[index].record;
    if (!record)return;
    d->dc[d->level].active_font = index;


    (void) U_WMRCREATEFONTINDIRECT_get(record, &memfont);
    memcpy(&font,memfont,U_SIZE_FONT_CORE);  //make sure it is in a properly aligned structure before touching it
    facename = memfont + U_SIZE_FONT_CORE;

    /*  The logfont information always starts with a U_LOGFONT structure but the U_WMRCREATEFONTINDIRECT
        is defined as U_LOGFONT_PANOSE so it can handle one of those if that is actually present. Currently only logfont
        is supported, and the remainder, it it really is a U_LOGFONT_PANOSE record, is ignored
    */
    int cur_level = d->level;
    d->level = d->wmf_obj[index].level;
    double font_size = pix_to_abs_size( d, font.Height );
    /*  snap the font_size to the nearest 1/32nd of a point.
        (The size is converted from Pixels to points, snapped, and converted back.)
        See the notes where d->D2Pscale[XY] are set for the reason why.
        Typically this will set the font to the desired exact size.  If some peculiar size
        was intended this will, at worst, make it .03125 off, which is unlikely to be a problem. */
    font_size = round(20.0 * 0.8 * font_size)/(20.0 * 0.8);
    d->level = cur_level;
    d->dc[d->level].style.font_size.computed = font_size;
    d->dc[d->level].style.font_weight.value =
        font.Weight == U_FW_THIN ? SP_CSS_FONT_WEIGHT_100 :
        font.Weight == U_FW_EXTRALIGHT ? SP_CSS_FONT_WEIGHT_200 :
        font.Weight == U_FW_LIGHT ? SP_CSS_FONT_WEIGHT_300 :
        font.Weight == U_FW_NORMAL ? SP_CSS_FONT_WEIGHT_400 :
        font.Weight == U_FW_MEDIUM ? SP_CSS_FONT_WEIGHT_500 :
        font.Weight == U_FW_SEMIBOLD ? SP_CSS_FONT_WEIGHT_600 :
        font.Weight == U_FW_BOLD ? SP_CSS_FONT_WEIGHT_700 :
        font.Weight == U_FW_EXTRABOLD ? SP_CSS_FONT_WEIGHT_800 :
        font.Weight == U_FW_HEAVY ? SP_CSS_FONT_WEIGHT_900 :
        font.Weight == U_FW_NORMAL ? SP_CSS_FONT_WEIGHT_NORMAL :
        font.Weight == U_FW_BOLD ? SP_CSS_FONT_WEIGHT_BOLD :
        font.Weight == U_FW_EXTRALIGHT ? SP_CSS_FONT_WEIGHT_LIGHTER :
        font.Weight == U_FW_EXTRABOLD ? SP_CSS_FONT_WEIGHT_BOLDER :
        U_FW_NORMAL;
    d->dc[d->level].style.font_style.value = (font.Italic ? SP_CSS_FONT_STYLE_ITALIC : SP_CSS_FONT_STYLE_NORMAL);
    d->dc[d->level].style.text_decoration_line.underline    = font.Underline;
    d->dc[d->level].style.text_decoration_line.line_through = font.StrikeOut;
    d->dc[d->level].style.text_decoration_line.set          = true;
    d->dc[d->level].style.text_decoration_line.inherit      = false;

    // malformed  WMF with empty filename may exist, ignore font change if encountered
    if(d->dc[d->level].font_name)free(d->dc[d->level].font_name);
    if(*facename){
        d->dc[d->level].font_name = strdup(facename);
    }
    else {  // Malformed WMF might specify an empty font name
        d->dc[d->level].font_name = strdup("Arial");  // Default font, WMF spec says device can pick whatever it wants
    }
    d->dc[d->level].style.baseline_shift.value = round((double)((font.Escapement + 3600) % 3600) / 10.0);   // use baseline_shift instead of text_transform to avoid overflow
}

/*  Find the first free hole where an object may be stored.
    If there are not any return -1.  This is a big error, possibly from a corrupt WMF file.
*/
int Wmf::insertable_object(PWMF_CALLBACK_DATA d)
{
    int index = d->low_water;  // Start looking from here, it may already have been filled
    while(index < d->n_obj &&  d->wmf_obj[index].record != NULL){ index++; }
    if(index >= d->n_obj)return(-1);  // this is a big problem, percolate it back up so the program can get out of this gracefully
    d->low_water = index; // Could probably be index+1
    return(index);
}

void
Wmf::delete_object(PWMF_CALLBACK_DATA d, int index)
{
    if (index >= 0 && index < d->n_obj) {
        // If the active object is deleted set default draw values
        if(index == d->dc[d->level].active_pen){  // Use default pen: solid, black, 1 pixel wide
            d->dc[d->level].active_pen                     = -1;
            d->dc[d->level].style.stroke_dasharray.set     = 0;
            d->dc[d->level].style.stroke_linecap.computed  = 2; // U_PS_ENDCAP_SQUARE
            d->dc[d->level].style.stroke_linejoin.computed = 0; // U_PS_JOIN_MITER;
            d->dc[d->level].stroke_set                     = true;
            d->dc[d->level].style.stroke_width.value       = 1.0;
            d->dc[d->level].style.stroke.value.color.set( 0, 0, 0 );
        }
        else if(index == d->dc[d->level].active_brush){
            d->dc[d->level].active_brush                   = -1;
            d->dc[d->level].fill_set                       = false;
        }
        else if(index == d->dc[d->level].active_font){
            d->dc[d->level].active_font                         = -1;
            if(d->dc[d->level].font_name){ free(d->dc[d->level].font_name);}
            d->dc[d->level].font_name = strdup("Arial");            // Default font, WMF spec says device can pick whatever it wants
            d->dc[d->level].style.font_size.computed                = 16.0;
            d->dc[d->level].style.font_weight.value                 = SP_CSS_FONT_WEIGHT_400;
            d->dc[d->level].style.font_style.value                  = SP_CSS_FONT_STYLE_NORMAL;
            d->dc[d->level].style.text_decoration_line.underline    = 0;
            d->dc[d->level].style.text_decoration_line.line_through = 0;
            d->dc[d->level].style.baseline_shift.value              = 0;
        }


        d->wmf_obj[index].type = 0;
// We are keeping a copy of the WMR rather than just a structure.  Currently that is not necessary as the entire
// WMF is read in at once and is stored in a big malloc.  However, in past versions it was handled
// reord by record, and we might need to do that again at some point in the future if we start running into WMF
// files too big to fit into memory.
        if (d->wmf_obj[index].record)
            free(d->wmf_obj[index].record);
        d->wmf_obj[index].record = NULL;
        if(index < d->low_water)d->low_water = index;
    }
}


// returns the new index, or -1 on error.
int Wmf::insert_object(PWMF_CALLBACK_DATA d, int type, const char *record)
{
    int index = insertable_object(d);
    if(index>=0){
        d->wmf_obj[index].type = type;
        d->wmf_obj[index].level = d->level;
        d->wmf_obj[index].record = wmr_dup(record);
    }
    return(index);
}


/**
    \fn create a UTF-32LE buffer and fill it with UNICODE unknown character
    \param count number of copies of the Unicode unknown character to fill with
*/
uint32_t *Wmf::unknown_chars(size_t count){
    uint32_t *res = (uint32_t *) malloc(sizeof(uint32_t) * (count + 1));
    if(!res)throw "Inkscape fatal memory allocation error - cannot continue";
    for(uint32_t i=0; i<count; i++){ res[i] = 0xFFFD; }
    res[count]=0;
    return res;
}

/**
    \brief store SVG for an image given the pixmap and various coordinate information
    \param d
    \param dib      packed DIB in memory
    \param dx       (double) destination x      in inkscape pixels
    \param dy       (double) destination y      in inkscape pixels
    \param dw       (double) destination width  in inkscape pixels
    \param dh       (double) destination height in inkscape pixels
    \param sx       (int)    source      x      in src image pixels
    \param sy       (int)    source      y      in src image pixels
    \param iUsage
*/
void Wmf::common_dib_to_image(PWMF_CALLBACK_DATA d, const char *dib,
        double dx, double dy, double dw, double dh, int sx, int sy, int sw, int sh, uint32_t iUsage){

    SVGOStringStream tmp_image;
    int  dibparams = U_BI_UNKNOWN;  // type of image not yet determined

    tmp_image << "\n\t <image\n";
    if (d->dc[d->level].clip_id){
        tmp_image << "\tclip-path=\"url(#clipWmfPath" << d->dc[d->level].clip_id << ")\"\n";
    }
    tmp_image << " y=\"" << dy << "\"\n x=\"" << dx <<"\"\n ";

    MEMPNG mempng; // PNG in memory comes back in this
    mempng.buffer = NULL;

    char            *rgba_px = NULL;        // RGBA pixels
    char            *sub_px  = NULL;        // RGBA pixels, subarray
    const char      *px      = NULL;        // DIB pixels
    const U_RGBQUAD *ct      = NULL;        // color table
    uint32_t numCt;
    int32_t width, height, colortype, invert;  // if needed these values will be set in wget_DIB_params
    if(iUsage == U_DIB_RGB_COLORS){
        // next call returns pointers and values, but allocates no memory
        dibparams = wget_DIB_params(dib, &px, &ct, &numCt, &width, &height, &colortype, &invert);
        if(dibparams == U_BI_RGB){
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
    tmp_image << " transform=" << current_matrix(d, 0.0, 0.0, 0); // returns an identity matrix, no offsets.
    tmp_image << " preserveAspectRatio=\"none\"\n";
    tmp_image << "/> \n";

    d->outsvg += tmp_image.str().c_str();
    d->path = "";
}

/**
  \brief store SVG for an image given the pixmap and various coordinate information
    \param d
    \param Bm16     core Bitmap16 header
    \param px       pointer to Bitmap16 image data
    \param dx       (double) destination x      in inkscape pixels
    \param dy       (double) destination y      in inkscape pixels
    \param dw       (double) destination width  in inkscape pixels
    \param dh       (double) destination height in inkscape pixels
    \param sx       (int)    source      x      in src image pixels
    \param sy       (int)    source      y      in src image pixels
    \param iUsage
*/
void Wmf::common_bm16_to_image(PWMF_CALLBACK_DATA d, U_BITMAP16 Bm16, const char *px,
        double dx, double dy, double dw, double dh, int sx, int sy, int sw, int sh){

    SVGOStringStream tmp_image;

    tmp_image << "\n\t <image\n";
    if (d->dc[d->level].clip_id){
        tmp_image << "\tclip-path=\"url(#clipWmfPath" << d->dc[d->level].clip_id << ")\"\n";
    }
    tmp_image << " y=\"" << dy << "\"\n x=\"" << dx <<"\"\n ";

    MEMPNG mempng; // PNG in memory comes back in this
    mempng.buffer = NULL;

    char            *rgba_px = NULL;        // RGBA pixels
    char            *sub_px  = NULL;        // RGBA pixels, subarray
    const U_RGBQUAD *ct      = NULL;        // color table
    int32_t width, height, colortype, numCt, invert;

    numCt     = 0;
    width     = Bm16.Width;             //  bitmap width in pixels.
    height    = Bm16.Height;            //  bitmap height in scan lines.
    colortype = Bm16.BitsPixel;         //  seems to be BitCount Enumeration
    invert    = 0;

    if(sw == 0 || sh == 0){
        sw = width;
        sh = height;
    }

    if(colortype < 16)return;  // these would need a colortable if they were a dib, no idea what bm16 is supposed to do instead.

    if(!DIB_to_RGBA(// This is not really a dib, but close enough so that it still works.
        px,         // DIB pixel array
        ct,         // DIB color table (always NULL here)
        numCt,      // DIB color table number of entries (always 0)
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

    gchar *base64String=NULL;
    if(mempng.buffer){             // image was Bm16 in source file, converted to png in this routine
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
    tmp_image << " transform=" << current_matrix(d, 0.0, 0.0, 0); // returns an identity matrix, no offsets.
    tmp_image << " preserveAspectRatio=\"none\"\n";
    tmp_image << "/> \n";

    d->outsvg += tmp_image.str().c_str();
    d->path = "";
}

/**
    \fn myMetaFileProc(char *contents, unsigned int length, PWMF_CALLBACK_DATA lpData)
    \returns 1 on success, 0 on error
    \param contents binary contents of an WMF file
    \param length   length in bytes of contents
    \param d   Inkscape data structures returned by this call
*/
//THis was a callback, just build it into a normal function
int Wmf::myMetaFileProc(const char *contents, unsigned int length, PWMF_CALLBACK_DATA d)
{
    uint32_t         off=0;
    uint32_t         wmr_mask;
    int              OK =1;
    int              file_status=1;
    TCHUNK_SPECS     tsp;
    uint8_t          iType;
    int              nSize;   // size of the current record, in bytes, or an error value if <=0
    const char      *blimit = contents + length;  // 1 byte past the end of the last record

    /* variables used to retrieve data from WMF records */
    uint16_t         utmp16;
    U_POINT16        pt16;   // any point
    U_RECT16         rc;     // any rectangle, usually a bounding rectangle
    U_POINT16        Dst;    // Destination coordinates
    U_POINT16        cDst;   // Destination w,h, if different from Src
    U_POINT16        Src;    // Source coordinates
    U_POINT16        cSrc;   // Source w,h, if different from Dst
    U_POINT16        cwh;    // w,h, if Src and Dst use the same values
    uint16_t         cUsage; // colorusage enumeration
    uint32_t         dwRop3; // raster operations, these are only barely supported here
    const char      *dib;    // DIB style image structure
    U_BITMAP16       Bm16;   // Bitmap16 style image structure
    const char      *px;     // Image for Bm16
    uint16_t         cPts;   // number of points in the next variable
    const char      *points; // any list of U_POINT16, may not be aligned
    int16_t          tlen;   // length of returned text, in bytes
    const char      *text;   // returned text, Latin1 encoded
    uint16_t         Opts;
    const int16_t   *dx;     // character spacing for one text mode,  inkscape ignores this
    double           left, right, top, bottom; // values used, because a bounding rect can have values reversed L<->R, T<->B

    uint16_t         tbkMode  = U_TRANSPARENT;          // holds proposed change to bkMode, if text is involved saving these to the DC must wait until the text is written
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
    tsp.rt_tidx    = -1;  /* set to an invalid */

    SVGOStringStream dbg_str;

    /*  There is very little information in WMF headers, get what is there.  In many cases pretty much everything will have to
        default.  If there is no placeable header we know pretty much nothing about the size of the page, in which case
        assume that it is 1440 WMF pixels/inch and make the page A4 landscape.  That is almost certainly the wrong page size
        but it has to be set to something, and nothing horrible happens if the drawing goes off the page.  */
    {

        U_WMRPLACEABLE Placeable;
        U_WMRHEADER Header;
        off = 0;
        nSize = wmfheader_get(contents, blimit, &Placeable, &Header);
        if (!nSize) {
            return(0);
        }
        if(!Header.nObjects){  Header.nObjects = 256; }// there _may_ be WMF files with no objects, more likely it is corrupt.  Try to use it anyway.
        d->n_obj     = Header.nObjects;
        d->wmf_obj   = new WMF_OBJECT[d->n_obj];
        d->low_water = 0;  // completely empty at this point, so start searches at 0

        // Init the new wmf_obj list elements to null, provided the
        // dynamic allocation succeeded.
        if ( d->wmf_obj != NULL )
        {
            for( int i=0; i < d->n_obj; ++i )
                d->wmf_obj[i].record = NULL;
        } //if

        if(!Placeable.Inch){ Placeable.Inch= 1440; }
        if(!Placeable.Dst.right && !Placeable.Dst.left){  // no page size has been supplied
            // This is gross, scan forward looking for a SETWINDOWEXT record, use the first one found to
            // define the page size
            int hold_nSize = off = nSize;
            Placeable.Dst.left   = 0;
            Placeable.Dst.top    = 0;
            while(OK){
                nSize = U_WMRRECSAFE_get(contents + off, blimit);
                if(nSize){
                    iType = *(uint8_t *)(contents + off + offsetof(U_METARECORD, iType )  );
                    if(iType ==  U_WMR_SETWINDOWEXT){
                        OK=0;
                        (void) U_WMRSETWINDOWEXT_get(contents + off, &Dst);
                        Placeable.Dst.right  = Dst.x;
                        Placeable.Dst.bottom = Dst.y;
                    }
                    else if(iType == U_WMR_EOF){
                        OK=0;
                        // Really messed up WMF, have to set the page to something, make it A4 horizontal
                        Placeable.Dst.right  = round(((double) Placeable.Inch) * 297.0/25.4);
                        Placeable.Dst.bottom = round(((double) Placeable.Inch) * 210.0/25.4);
                    }
                    else {
                        off += nSize;
                    }
                }
                else {
                    return(0);
                }
            }
            off=0;
            nSize = hold_nSize;
            OK=1;
        }

        // drawing size in WMF pixels
        d->PixelsInX = Placeable.Dst.right   - Placeable.Dst.left + 1;
        d->PixelsInY = Placeable.Dst.bottom  - Placeable.Dst.top  + 1;

        /*
            Set values for Window and ViewPort extents to 0 - not defined yet.
        */
        d->dc[d->level].sizeView.x = d->dc[d->level].sizeWnd.x = 0;
        d->dc[d->level].sizeView.y = d->dc[d->level].sizeWnd.y = 0;

        /* Upper left corner in device units, usually both 0, but not always.
        If a placeable header is used, and later a windoworg/windowext are found, then
        the placeable information will be ignored.
        */
        d->ulCornerInX  = Placeable.Dst.left;
        d->ulCornerInY  = Placeable.Dst.top;

        d->E2IdirY = 1.0;  // assume MM_ANISOTROPIC, if not, this will be changed later
        d->D2PscaleX = d->D2PscaleY = Inkscape::Util::Quantity::convert(1, "in", "px")/(double) Placeable.Inch;
        trinfo_load_qe(d->tri, d->D2PscaleX);  /* quantization error that will affect text positions */

        // drawing size in Inkscape pixels
        d->PixelsOutX = d->PixelsInX * d->D2PscaleX;
        d->PixelsOutY = d->PixelsInY * d->D2PscaleY;

        // Upper left corner in Inkscape units
        d->ulCornerOutX = d->ulCornerInX              * d->D2PscaleX;
        d->ulCornerOutY = d->ulCornerInY * d->E2IdirY * d->D2PscaleY;

        d->dc[0].style.stroke_width.value =  pix_to_abs_size( d, 1 ); // This could not be set until the size of the WMF was known
        dbg_str << "<!-- U_WMR_HEADER -->\n";

        d->outdef += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";

        SVGOStringStream tmp_outdef;
        tmp_outdef << "<svg\n";
        tmp_outdef << "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
        tmp_outdef << "  xmlns=\"http://www.w3.org/2000/svg\"\n";
        tmp_outdef << "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
        tmp_outdef << "  xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\"\n"; // needed for sodipodi:role
        tmp_outdef << "  version=\"1.0\"\n";

        tmp_outdef <<
            "  width=\"" << Inkscape::Util::Quantity::convert(d->PixelsOutX, "px", "mm") << "mm\"\n" <<
            "  height=\"" << Inkscape::Util::Quantity::convert(d->PixelsOutY, "px", "mm")  << "mm\">\n";
        d->outdef += tmp_outdef.str().c_str();
        d->outdef += "<defs>";                           // temporary end of header

        // d->defs holds any defines which are read in.


    }



    while(OK){
    if (off>=length) {
        return(0);  //normally should exit from while after WMREOF sets OK to false.
    }
    contents += nSize;         // pointer to the start of the next record
    off      += nSize;         // offset from beginning of buffer to the start of the next record

    /*  Currently this is a weaker check than for EMF, it only checks the size of the constant part
        of the record */
    nSize = U_WMRRECSAFE_get(contents, blimit);
    if(!nSize) {
        file_status = 0;
        break;
    }

    iType     = *(uint8_t *)(contents + offsetof(U_METARECORD, iType )  );

    wmr_mask = U_wmr_properties(iType);
    if (wmr_mask == U_WMR_INVALID) { 
        file_status = 0;
        break;
    }
//  Uncomment the following to track down toxic records
// std::cout << "record type: " << (int) iType << " name " << U_wmr_names(iType) << " length: " << nSize << " offset: " << off <<std::endl;

    SVGOStringStream tmp_path;
    SVGOStringStream tmp_str;

/* Uncomment the following to track down text problems */
//std::cout << "tri->dirty:"<< d->tri->dirty << " wmr_mask: " << std::hex << wmr_mask << std::dec << std::endl;

    // incompatible change to text drawing detected (color or background change) forces out existing text
    //    OR
    // next record is valid type and forces pending text to be drawn immediately
    if ((d->dc[d->level].dirty & DIRTY_TEXT) || ((wmr_mask != U_WMR_INVALID) && (wmr_mask & U_DRAW_TEXT) && d->tri->dirty)){
        TR_layout_analyze(d->tri);
        if (d->dc[d->level].clip_id){
           SVGOStringStream tmp_clip;
           tmp_clip << "\n<g\n\tclip-path=\"url(#clipWmfPath" << d->dc[d->level].clip_id << ")\"\n>";
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
        if((d->dc[d->level].fill_mode   == DRAW_PATTERN) && (d->dc[d->level].dirty & DIRTY_FILL)){
            select_brush(d, d->dc[d->level].fill_recidx);
        }
        
        d->dc[d->level].dirty = 0;
    }

//std::cout << "BEFORE DRAW logic d->mask: " << std::hex << d->mask << " wmr_mask: " << wmr_mask << std::dec << std::endl;
/*
std::cout << "BEFORE DRAW"
 << " test0 " << ( d->mask & U_DRAW_VISIBLE)
 << " test1 " << ( d->mask & U_DRAW_FORCE)
 << " test2 " << (wmr_mask & U_DRAW_ALTERS)
 << " test3 " << (wmr_mask & U_DRAW_VISIBLE)
 << " test4 " << !(d->mask & U_DRAW_ONLYTO)
 << " test5 " << ((d->mask & U_DRAW_ONLYTO) && !(wmr_mask & U_DRAW_ONLYTO)  )
 << std::endl;
*/

    if(
        (wmr_mask != U_WMR_INVALID)                             &&              // next record is valid type
        (d->mask & U_DRAW_VISIBLE)                              &&              // This record is drawable
        (
            (d->mask & U_DRAW_FORCE)                            ||              // This draw is forced by STROKE/FILL/STROKEANDFILL PATH
            (wmr_mask & U_DRAW_ALTERS)                          ||              // Next record would alter the drawing environment in some way
            (  (wmr_mask & U_DRAW_VISIBLE)                      &&              // Next record is visible...
                (
                    ( !(d->mask & U_DRAW_ONLYTO) )              ||              //   Non *TO records cannot be followed by any Visible
                    ((d->mask & U_DRAW_ONLYTO) && !(wmr_mask & U_DRAW_ONLYTO)  )//   *TO records can only be followed by other *TO records
                )
            )
        )
    ){
//  std::cout << "PATH DRAW at TOP <<+++++++++++++++++++++++++++++++++++++" << std::endl;
        if(!(d->path.empty())){
            d->outsvg += "   <path ";    // this is the ONLY place <path should be used!!!!
            output_style(d);
            d->outsvg += "\n\t";
            d->outsvg += "\n\td=\"";      // this is the ONLY place d=" should be used!!!!
            d->outsvg += d->path;
            d->outsvg += " \" /> \n";
            d->path = ""; //reset the path
        }
        // reset the flags
        d->mask = 0;
        d->drawtype = 0;
    }
// std::cout << "AFTER DRAW logic d->mask: " << std::hex << d->mask << " wmr_mask: " << wmr_mask << std::dec << std::endl;
    switch (iType)
    {
        case U_WMR_EOF:
        {
            dbg_str << "<!-- U_WMR_EOF -->\n";

            d->outsvg = d->outdef + d->defs + "\n</defs>\n\n" + d->outsvg + "</svg>\n";
            OK=0;
            break;
        }
        case U_WMR_SETBKCOLOR:
        {
            dbg_str << "<!-- U_WMR_SETBKCOLOR -->\n";
            nSize = U_WMRSETBKCOLOR_get(contents, &tbkColor);
            if(memcmp(&tbkColor, &(d->dc[d->level].bkColor), sizeof(U_COLORREF))){
                d->dc[d->level].dirty  |= DIRTY_TEXT;
                if(d->dc[d->level].fill_mode   == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_FILL;   }
                tbkMode = d->dc[d->level].bkMode;
            }
            break;
        }
        case U_WMR_SETBKMODE:{
            dbg_str << "<!-- U_WMR_SETBKMODE -->\n";
            nSize = U_WMRSETBKMODE_get(contents, &tbkMode);
            if(tbkMode != d->dc[d->level].bkMode){
                d->dc[d->level].dirty  |= DIRTY_TEXT;
                if(tbkMode != d->dc[d->level].bkMode){
                    if(d->dc[d->level].fill_mode   == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_FILL;   }
                }
                memcpy(&tbkColor,&(d->dc[d->level].bkColor),sizeof(U_COLORREF));
            }
            break;
        }
        case U_WMR_SETMAPMODE:
        {
            dbg_str << "<!-- U_WMR_SETMAPMODE -->\n";
            nSize = U_WMRSETMAPMODE_get(contents, &utmp16);
            switch (utmp16){
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
        case U_WMR_SETROP2:
        {
            dbg_str << "<!-- U_WMR_SETROP2 -->\n";
            nSize = U_WMRSETROP2_get(contents, &utmp16);
            d->dwRop2 = utmp16;
            break;
        }
        case U_WMR_SETRELABS:             dbg_str << "<!-- U_WMR_SETRELABS -->\n";  break;
        case U_WMR_SETPOLYFILLMODE:
        {
            dbg_str << "<!-- U_WMR_SETPOLYFILLMODE -->\n";
            nSize = U_WMRSETPOLYFILLMODE_get(contents, &utmp16);
            d->dc[d->level].style.fill_rule.value = (utmp16 == U_ALTERNATE ? 0 :  utmp16 == U_WINDING ? 1 : 0);
            break;
        }
        case U_WMR_SETSTRETCHBLTMODE:
        {
            dbg_str << "<!-- U_WMR_SETSTRETCHBLTMODE -->\n";
            nSize  = U_WMRSETSTRETCHBLTMODE_get(contents, &utmp16);
            BLTmode = utmp16;
            break;
        }
        case U_WMR_SETTEXTCHAREXTRA:      dbg_str << "<!-- U_WMR_SETTEXTCHAREXTRA -->\n";  break;
        case U_WMR_SETTEXTCOLOR:
        {
            dbg_str << "<!-- U_WMR_SETTEXTCOLOR -->\n";
            nSize = U_WMRSETTEXTCOLOR_get(contents, &(d->dc[d->level].textColor));
            if(tbkMode != d->dc[d->level].bkMode){
                if(d->dc[d->level].fill_mode   == DRAW_PATTERN){ d->dc[d->level].dirty  |= DIRTY_FILL;   }
            }
            // not text_dirty, because multicolored complex text is supported in libTERE
            break;
        }
        case U_WMR_SETTEXTJUSTIFICATION:  dbg_str << "<!-- U_WMR_SETTEXTJUSTIFICATION -->\n"; break;
        case U_WMR_SETWINDOWORG:
        {
            dbg_str << "<!-- U_WMR_SETWINDOWORG -->\n";
            nSize = U_WMRSETWINDOWORG_get(contents, &d->dc[d->level].winorg);
            d->ulCornerOutX = 0.0; // In the examples seen to date if this record is used with a placeable header, that header is ignored
            d->ulCornerOutY = 0.0;
            break;
        }
        case U_WMR_SETWINDOWEXT:
        {
            dbg_str << "<!-- U_WMR_SETWINDOWEXT -->\n";

            nSize = U_WMRSETWINDOWEXT_get(contents, &d->dc[d->level].sizeWnd);

            if (!d->dc[d->level].sizeWnd.x || !d->dc[d->level].sizeWnd.y) {
                d->dc[d->level].sizeWnd = d->dc[d->level].sizeView;
                if (!d->dc[d->level].sizeWnd.x || !d->dc[d->level].sizeWnd.y) {
                    d->dc[d->level].sizeWnd.x = d->PixelsOutX;
                    d->dc[d->level].sizeWnd.y = d->PixelsOutY;
                }
            }
            else {
                /* There are a lot WMF files in circulation with the x,y values in the  setwindowext reversed.  If this is detected, swap them.
                   There is a remote possibility that the strange scaling this implies was intended, and those will be rendered incorrectly */
                double Ox = d->PixelsOutX;
                double Oy = d->PixelsOutY;
                double Wx = d->dc[d->level].sizeWnd.x;
                double Wy = d->dc[d->level].sizeWnd.y;
                if(Wx != Wy && Geom::are_near(Ox/Wy, Oy/Wx, 1.01/MIN(Wx,Wy)) ){
                    int tmp;
                    tmp = d->dc[d->level].sizeWnd.x;
                    d->dc[d->level].sizeWnd.x = d->dc[d->level].sizeWnd.y;
                    d->dc[d->level].sizeWnd.y = tmp;
                }
            }

            if (!d->dc[d->level].sizeView.x || !d->dc[d->level].sizeView.y) {
                /* Previously it used sizeWnd, but that always resulted in scale = 1 if no viewport ever appeared, and in most files, it did not */
                d->dc[d->level].sizeView.x = d->PixelsInX - 1;
                d->dc[d->level].sizeView.y = d->PixelsInY - 1;
            }

            /* scales logical to WMF pixels, transfer a negative sign on Y, if any */
            if (d->dc[d->level].sizeWnd.x && d->dc[d->level].sizeWnd.y) {
                d->dc[d->level].ScaleInX = (double) d->dc[d->level].sizeView.x / (double) d->dc[d->level].sizeWnd.x;
                d->dc[d->level].ScaleInY = (double) d->dc[d->level].sizeView.y / (double) d->dc[d->level].sizeWnd.y;
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
        case U_WMR_SETVIEWPORTORG:
        {
            dbg_str << "<!-- U_WMR_SETWINDOWORG -->\n";
            nSize = U_WMRSETVIEWPORTORG_get(contents, &d->dc[d->level].vieworg);
            break;
        }
        case U_WMR_SETVIEWPORTEXT:
        {
            dbg_str << "<!-- U_WMR_SETVIEWPORTEXTEX -->\n";

            nSize = U_WMRSETVIEWPORTEXT_get(contents, &d->dc[d->level].sizeView);

            if (!d->dc[d->level].sizeView.x || !d->dc[d->level].sizeView.y) {
                d->dc[d->level].sizeView = d->dc[d->level].sizeWnd;
                if (!d->dc[d->level].sizeView.x || !d->dc[d->level].sizeView.y) {
                    d->dc[d->level].sizeView.x = d->PixelsOutX;
                    d->dc[d->level].sizeView.y = d->PixelsOutY;
                }
            }

            if (!d->dc[d->level].sizeWnd.x || !d->dc[d->level].sizeWnd.y) {
                d->dc[d->level].sizeWnd = d->dc[d->level].sizeView;
            }

            /* scales logical to WMF pixels, transfer a negative sign on Y, if any */
            if (d->dc[d->level].sizeWnd.x && d->dc[d->level].sizeWnd.y) {
                d->dc[d->level].ScaleInX = (double) d->dc[d->level].sizeView.x / (double) d->dc[d->level].sizeWnd.x;
                d->dc[d->level].ScaleInY = (double) d->dc[d->level].sizeView.y / (double) d->dc[d->level].sizeWnd.y;
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
        case U_WMR_OFFSETWINDOWORG:       dbg_str << "<!-- U_WMR_OFFSETWINDOWORG -->\n";   break;
        case U_WMR_SCALEWINDOWEXT:        dbg_str << "<!-- U_WMR_SCALEWINDOWEXT -->\n";    break;
        case U_WMR_OFFSETVIEWPORTORG:     dbg_str << "<!-- U_WMR_OFFSETVIEWPORTORG -->\n"; break;
        case U_WMR_SCALEVIEWPORTEXT:      dbg_str << "<!-- U_WMR_SCALEVIEWPORTEXT -->\n";  break;
        case U_WMR_LINETO:
        {
            dbg_str << "<!-- U_WMR_LINETO -->\n";

            nSize = U_WMRLINETO_get(contents, &pt16);

            d->mask |= wmr_mask;

            tmp_path << "\n\tL " << pix_to_xy( d, pt16.x, pt16.y) << " ";
            break;
        }
        case U_WMR_MOVETO:
        {
            dbg_str << "<!-- U_WMR_MOVETO -->\n";

            nSize = U_WMRLINETO_get(contents, &pt16);

            d->mask |= wmr_mask;

            d->dc[d->level].cur = pt16;

            tmp_path <<
                "\n\tM " << pix_to_xy( d, pt16.x, pt16.y ) << " ";
            break;
        }
        case U_WMR_EXCLUDECLIPRECT:
        {
            dbg_str << "<!-- U_WMR_EXCLUDECLIPRECT -->\n";

            U_RECT16   rc;
            nSize = U_WMREXCLUDECLIPRECT_get(contents, &rc);

            SVGOStringStream tmp_path;
            float faraway = 10000000; // hopefully well outside any real drawing!
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
        case U_WMR_INTERSECTCLIPRECT:
        {
            dbg_str << "<!-- U_WMR_INTERSECTCLIPRECT -->\n";

            nSize = U_WMRINTERSECTCLIPRECT_get(contents, &rc);

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
        case U_WMR_ARC:
        {
            dbg_str << "<!-- U_WMR_ARC -->\n";
            U_POINT16 ArcStart, ArcEnd;
            nSize = U_WMRARC_get(contents, &ArcStart, &ArcEnd, &rc);

            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            int stat = wmr_arc_points(rc, ArcStart, ArcEnd,&f1, f2, &center, &start, &end, &size);
            if(!stat){
                tmp_path <<  "\n\tM " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y) << " \n";
                d->mask |= wmr_mask;
            }
            else {
                dbg_str << "<!-- ARC record is invalid -->\n";
            }
            break;
        }
        case U_WMR_ELLIPSE:
        {
            dbg_str << "<!-- U_WMR_ELLIPSE -->\n";

            nSize = U_WMRELLIPSE_get(contents, &rc);

            double cx = pix_to_x_point( d, (rc.left + rc.right)/2.0, (rc.bottom + rc.top)/2.0 );
            double cy = pix_to_y_point( d, (rc.left + rc.right)/2.0, (rc.bottom + rc.top)/2.0 );
            double rx = pix_to_abs_size( d, std::abs(rc.right - rc.left  )/2.0 );
            double ry = pix_to_abs_size( d, std::abs(rc.top   - rc.bottom)/2.0 );

            SVGOStringStream tmp_ellipse;
            tmp_ellipse << "cx=\"" << cx << "\" ";
            tmp_ellipse << "cy=\"" << cy << "\" ";
            tmp_ellipse << "rx=\"" << rx << "\" ";
            tmp_ellipse << "ry=\"" << ry << "\" ";

            d->mask |= wmr_mask;

            d->outsvg += "   <ellipse ";
            output_style(d);
            d->outsvg += "\n\t";
            d->outsvg += tmp_ellipse.str().c_str();
            d->outsvg += "/> \n";
            d->path = "";
            break;
        }
        case U_WMR_FLOODFILL:             dbg_str << "<!-- U_WMR_EXTFLOODFILL -->\n";         break;
        case U_WMR_PIE:
        {
            dbg_str << "<!-- U_WMR_PIE -->\n";
            U_POINT16 ArcStart, ArcEnd;
            nSize = U_WMRPIE_get(contents, &ArcStart, &ArcEnd, &rc);
            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            if(!wmr_arc_points(rc, ArcStart, ArcEnd, &f1, f2, &center, &start, &end, &size)){
                tmp_path <<  "\n\tM " << pix_to_xy(d, center.x, center.y);
                tmp_path <<  "\n\tL " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y) << " \n";
                tmp_path << " z ";
                d->mask |= wmr_mask;
            }
            else {
                dbg_str << "<!-- PIE record is invalid -->\n";
            }
            break;
        }
        case U_WMR_RECTANGLE:
        {
            dbg_str << "<!-- U_WMR_RECTANGLE -->\n";

            nSize = U_WMRRECTANGLE_get(contents, &rc);
            U_sanerect16(rc, &left, &top, &right, &bottom);

            SVGOStringStream tmp_rectangle;
            tmp_rectangle << "\n\tM " << pix_to_xy( d, left , top )     << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, right, top )     << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, right, bottom )  << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, left,  bottom )  << " ";
            tmp_rectangle << "\n\tz";

            d->mask |= wmr_mask;

            tmp_path << tmp_rectangle.str().c_str();
            break;
        }
        case U_WMR_ROUNDRECT:
        {
            dbg_str << "<!-- U_WMR_ROUNDRECT -->\n";

            int16_t Height,Width;
            nSize = U_WMRROUNDRECT_get(contents, &Width, &Height, &rc);
            U_sanerect16(rc, &left, &top, &right, &bottom);
            double f = 4.*(sqrt(2) - 1)/3;
            double f1 = 1.0 - f;
            double cnx = Width/2;
            double cny = Height/2;


            SVGOStringStream tmp_rectangle;
            tmp_rectangle << "\n"
                          << "    M "
                          << pix_to_xy(d, left            ,  top    + cny    )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d, left            ,  top    + cny*f1 )
                          << " "
                          << pix_to_xy(d, left  + cnx*f1  ,  top             )
                          << " "
                          << pix_to_xy(d, left  + cnx     ,  top             )
                          << "\n";
            tmp_rectangle << "   L "
                          << pix_to_xy(d, right - cnx     ,  top             )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d, right - cnx*f1  ,  top             )
                          << " "
                          << pix_to_xy(d, right           ,  top    + cny*f1 )
                          << " "
                          << pix_to_xy(d, right           ,  top    + cny    )
                          << "\n";
            tmp_rectangle << "   L "
                          << pix_to_xy(d, right           ,  bottom - cny    )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d, right           ,  bottom - cny*f1 )
                          << " "
                          << pix_to_xy(d, right - cnx*f1  ,  bottom          )
                          << " "
                          << pix_to_xy(d, right - cnx     ,  bottom          )
                          << "\n";
            tmp_rectangle << "   L "
                          << pix_to_xy(d, left  + cnx     ,  bottom          )
                          << "\n";
            tmp_rectangle << "   C "
                          << pix_to_xy(d, left  + cnx*f1  ,  bottom          )
                          << " "
                          << pix_to_xy(d, left            ,  bottom - cny*f1 )
                          << " "
                          << pix_to_xy(d, left            ,  bottom - cny    )
                          << "\n";
            tmp_rectangle << "   z\n";


            d->mask |= wmr_mask;

            tmp_path <<   tmp_rectangle.str().c_str();
            break;
        }
        case U_WMR_PATBLT:
        {
            dbg_str << "<!-- U_WMR_PATBLT -->\n";
            // Treat this like any other rectangle, ie, ignore the dwRop3
            nSize = U_WMRPATBLT_get(contents, &Dst, &cwh, &dwRop3);
            SVGOStringStream tmp_rectangle;
            tmp_rectangle << "\n\tM " << pix_to_xy( d, Dst.x ,        Dst.y )         << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, Dst.x + cwh.x, Dst.y )         << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, Dst.x + cwh.x, Dst.y + cwh.y )  << " ";
            tmp_rectangle << "\n\tL " << pix_to_xy( d, Dst.x,         Dst.y + cwh.y )  << " ";
            tmp_rectangle << "\n\tz";

            d->mask |= wmr_mask;

            tmp_path << tmp_rectangle.str().c_str();
            break;
        }
        case U_WMR_SAVEDC:
        {
            dbg_str << "<!-- U_WMR_SAVEDC -->\n";

            if (d->level < WMF_MAX_DC) {
                d->dc[d->level + 1] = d->dc[d->level];
                if(d->dc[d->level].font_name){
                  d->dc[d->level + 1].font_name = strdup(d->dc[d->level].font_name); // or memory access problems because font name pointer duplicated
                }
                d->level = d->level + 1;
            }
            break;
        }
        case U_WMR_SETPIXEL:              dbg_str << "<!-- U_WMR_SETPIXEL -->\n";                break;
        case U_WMR_OFFSETCLIPRGN:
        {
            dbg_str << "<!-- U_EMR_OFFSETCLIPRGN -->\n"; 
            U_POINT16  off;
            nSize = U_WMROFFSETCLIPRGN_get(contents,&off);
            if (d->dc[d->level].clip_id) { // can only offset an existing clipping path
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
        // U_WMR_TEXTOUT should be here, but has been moved down to merge with U_WMR_EXTTEXTOUT
        case U_WMR_BITBLT:
        {
            dbg_str << "<!-- U_WMR_BITBLT -->\n";
            nSize = U_WMRBITBLT_get(contents,&Dst,&cwh,&Src,&dwRop3,&Bm16,&px);
            if(!px){
                if(dwRop3 == U_NOOP)break; /* GDI applications apparently often end with this as a sort of flush(), nothing should be drawn */
                int32_t dx = Dst.x;
                int32_t dy = Dst.y;
                int32_t dw = cwh.x;
                int32_t dh = cwh.y;
                SVGOStringStream tmp_rectangle;
                tmp_rectangle << "\n\tM " << pix_to_xy( d, dx,      dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy + dh )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx,      dy + dh )    << " ";
                tmp_rectangle << "\n\tz";

                d->mask |= wmr_mask;
                d->dwRop3 = dwRop3;   // we will try to approximate SOME of these
                d->mask |= U_DRAW_CLOSED; // Bitblit is not really open or closed, but we need it to fill, and this is the flag for that

                tmp_path <<   tmp_rectangle.str().c_str();
            }
            else { /* Not done yet, Bm16 image present */ }
                 double dx = pix_to_x_point( d, Dst.x, Dst.y);
                 double dy = pix_to_y_point( d, Dst.x, Dst.y);
                 double dw = pix_to_abs_size( d, cwh.x);
                 double dh = pix_to_abs_size( d, cwh.y);
                 //source position within the bitmap, in pixels
                 int sx = Src.x;
                 int sy = Src.y;
                 int sw = 0; // extract all of the image
                 int sh = 0;
                 if(sx<0)sx=0;
                 if(sy<0)sy=0;
                 common_bm16_to_image(d,Bm16,px,dx,dy,dw,dh,sx,sy,sw,sh);
             break;
        }
        case U_WMR_STRETCHBLT:
        {
            dbg_str << "<!-- U_WMR_STRETCHBLT -->\n";
            nSize = U_WMRSTRETCHBLT_get(contents,&Dst,&cDst,&Src,&cSrc,&dwRop3,&Bm16,&px);
            if(!px){
                if(dwRop3 == U_NOOP)break; /* GDI applications apparently often end with this as a sort of flush(), nothing should be drawn */
                int32_t dx = Dst.x;
                int32_t dy = Dst.y;
                int32_t dw = cDst.x;
                int32_t dh = cDst.y;
                SVGOStringStream tmp_rectangle;
                tmp_rectangle << "\n\tM " << pix_to_xy( d, dx,      dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy + dh )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx,      dy + dh )    << " ";
                tmp_rectangle << "\n\tz";

                d->mask |= wmr_mask;
                d->dwRop3 = dwRop3;   // we will try to approximate SOME of these
                d->mask |= U_DRAW_CLOSED; // Bitblit is not really open or closed, but we need it to fill, and this is the flag for that

                tmp_path <<   tmp_rectangle.str().c_str();
            }
            else { /* Not done yet, Bm16 image present */ }
                 double dx = pix_to_x_point( d, Dst.x, Dst.y);
                 double dy = pix_to_y_point( d, Dst.x, Dst.y);
                 double dw = pix_to_abs_size( d, cDst.x);
                 double dh = pix_to_abs_size( d, cDst.y);
                 //source position within the bitmap, in pixels
                 int sx = Src.x;
                 int sy = Src.y;
                 int sw = cSrc.x; // extract the specified amount of the image
                 int sh = cSrc.y;
                 if(sx<0)sx=0;
                 if(sy<0)sy=0;
                 common_bm16_to_image(d,Bm16,px,dx,dy,dw,dh,sx,sy,sw,sh);
            break;
        }
        case U_WMR_POLYGON:
        case U_WMR_POLYLINE:
        {
            dbg_str << "<!-- U_WMR_POLYGON/POLYLINE -->\n";
            nSize = U_WMRPOLYGON_get(contents, &cPts, &points);
            uint32_t i;

            if (cPts < 2)break;

            d->mask |= wmr_mask;
            memcpy(&pt16,points,U_SIZE_POINT16); points += U_SIZE_POINT16;

            tmp_str << "\n\tM " << pix_to_xy( d, pt16.x, pt16.y) << " ";

            for (i=1; i<cPts; i++) {
                memcpy(&pt16,points,U_SIZE_POINT16); points+=U_SIZE_POINT16;
                tmp_str << "\n\tL " << pix_to_xy( d, pt16.x, pt16.y) << " ";
            }

            tmp_path << tmp_str.str().c_str();
            if(iType==U_WMR_POLYGON){ tmp_path << " z"; }

            break;
        }
        case U_WMR_ESCAPE:  // only 3 types of escape are implemented
        {
            dbg_str << "<!-- U_WMR_ESCAPE -->\n";
            uint16_t Escape, elen;
            nSize = U_WMRESCAPE_get(contents, &Escape, &elen, &text);
            if(elen>=4){
                uint32_t utmp4;
                memcpy(&utmp4, text ,4);
                if(Escape == U_MFE_SETLINECAP){
                    switch (utmp4 & U_PS_ENDCAP_MASK) {
                        case U_PS_ENDCAP_ROUND: {  d->dc[d->level].style.stroke_linecap.computed = 1;   break; }
                        case U_PS_ENDCAP_SQUARE: { d->dc[d->level].style.stroke_linecap.computed = 2;   break; }
                        case U_PS_ENDCAP_FLAT:
                        default: {                 d->dc[d->level].style.stroke_linecap.computed = 0;   break; }
                    }
                }
                else if(Escape == U_MFE_SETLINEJOIN){
                    switch (utmp4 & U_PS_JOIN_MASK) {
                        case U_PS_JOIN_BEVEL: {    d->dc[d->level].style.stroke_linejoin.computed = 2;  break; }
                        case U_PS_JOIN_MITER: {    d->dc[d->level].style.stroke_linejoin.computed = 0;  break; }
                        case U_PS_JOIN_ROUND:
                        default: {                 d->dc[d->level].style.stroke_linejoin.computed = 1;  break; }
                    }
                }
                else if(Escape == U_MFE_SETMITERLIMIT){
                    //The function takes a float but uses a 32 bit int in the record.
                    float miterlimit = utmp4;
                    d->dc[d->level].style.stroke_miterlimit.value = miterlimit;   //ratio, not a pt size
                    if (d->dc[d->level].style.stroke_miterlimit.value < 2)
                        d->dc[d->level].style.stroke_miterlimit.value = 2.0;
                }
            }
            break;
        }
        case U_WMR_RESTOREDC:
        {
            dbg_str << "<!-- U_WMR_RESTOREDC -->\n";

            int16_t DC;
            nSize = U_WMRRESTOREDC_get(contents, &DC);
            int old_level = d->level;
            if (DC >= 0) {
                if (DC < d->level)
                    d->level = DC;
            }
            else {
                if (d->level + DC >= 0)
                    d->level = d->level + DC;
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
        case U_WMR_FILLREGION:            dbg_str << "<!-- U_WMR_FILLREGION -->\n"; break;
        case U_WMR_FRAMEREGION:           dbg_str << "<!-- U_WMR_FRAMEREGION -->\n"; break;
        case U_WMR_INVERTREGION:          dbg_str << "<!-- U_WMR_INVERTREGION -->\n"; break;
        case U_WMR_PAINTREGION:           dbg_str << "<!-- U_WMR_PAINTREGION -->\n"; break;
        case U_WMR_SELECTCLIPREGION:
        {
            dbg_str << "<!-- U_WMR_EXTSELECTCLIPRGN -->\n";
            nSize = U_WMRSELECTCLIPREGION_get(contents, &utmp16);
            if (utmp16 == U_RGN_COPY)
                clipset = false;
            break;
        }
        case U_WMR_SELECTOBJECT:
        {
            dbg_str << "<!-- U_WMR_SELECTOBJECT -->\n";

            nSize = U_WMRSELECTOBJECT_get(contents, &utmp16);
            unsigned int index = utmp16;

            // WMF has no stock objects
            if ( /*index >= 0 &&*/ index < (unsigned int) d->n_obj) {
                switch (d->wmf_obj[index].type)
                {
                     case U_WMR_CREATEPENINDIRECT:
                        select_pen(d, index);
                        break;
                    case U_WMR_CREATEBRUSHINDIRECT:
                    case U_WMR_DIBCREATEPATTERNBRUSH:
                    case U_WMR_CREATEPATTERNBRUSH: // <- this one did not display properly on XP, DIBCREATEPATTERNBRUSH works
                        select_brush(d, index);
                        break;
                    case U_WMR_CREATEFONTINDIRECT:
                        select_font(d, index);
                        break;
                    case U_WMR_CREATEPALETTE:
                    case U_WMR_CREATEBITMAPINDIRECT:
                    case U_WMR_CREATEBITMAP:
                    case U_WMR_CREATEREGION:
                        /* these do not do anything, but their objects must be kept in the count */
                        break;
                }
            }
            break;
        }
        case U_WMR_SETTEXTALIGN:
        {
            dbg_str << "<!-- U_WMR_SETTEXTALIGN -->\n";
            nSize = U_WMRSETTEXTALIGN_get(contents, &(d->dc[d->level].textAlign));
            break;
        }
        case U_WMR_DRAWTEXT:              dbg_str << "<!-- U_WMR_DRAWTEXT -->\n"; break;
        case U_WMR_CHORD:
        {
            dbg_str << "<!-- U_WMR_CHORD -->\n";
            U_POINT16 ArcStart, ArcEnd;
            nSize = U_WMRCHORD_get(contents, &ArcStart, &ArcEnd, &rc);
            U_PAIRF center,start,end,size;
            int f1;
            int f2 = (d->arcdir == U_AD_COUNTERCLOCKWISE ? 0 : 1);
            if(!wmr_arc_points(rc, ArcStart, ArcEnd, &f1, f2, &center, &start, &end, &size)){
                tmp_path <<  "\n\tM " << pix_to_xy(d, start.x, start.y);
                tmp_path <<  " A "    << pix_to_abs_size(d, size.x)/2.0      << ","  << pix_to_abs_size(d, size.y)/2.0 ;
                tmp_path <<  " ";
                tmp_path <<  180.0 * current_rotation(d)/M_PI;
                tmp_path <<  " ";
                tmp_path <<  " " << f1 << "," << f2 << " ";
                tmp_path <<              pix_to_xy(d, end.x, end.y) << " \n";
                tmp_path << " z ";
                d->mask |= wmr_mask;
            }
            else {
                dbg_str << "<!-- CHORD record is invalid -->\n";
            }
            break;
        }
        case U_WMR_SETMAPPERFLAGS:       dbg_str << "<!-- U_WMR_SETMAPPERFLAGS -->\n";     break;
        case U_WMR_TEXTOUT:
        case U_WMR_EXTTEXTOUT:
        {
            if(iType == U_WMR_TEXTOUT){
                dbg_str << "<!-- U_WMR_TEXTOUT -->\n";
                nSize = U_WMRTEXTOUT_get(contents, &Dst, &tlen, &text);
                Opts=0;
            }
            else {
                dbg_str << "<!-- U_WMR_EXTTEXTOUT -->\n";
                nSize = U_WMREXTTEXTOUT_get(contents, &Dst, &tlen, &Opts, &text, &dx, &rc );
            }
            uint32_t fOptions = Opts;

            double x1,y1;
            int cChars;
            x1 = Dst.x;
            y1 = Dst.y;
            cChars = tlen;

            if (d->dc[d->level].textAlign & U_TA_UPDATECP) {
                x1 = d->dc[d->level].cur.x;
                y1 = d->dc[d->level].cur.y;
            }

            double x = pix_to_x_point(d, x1, y1);
            double y = pix_to_y_point(d, x1, y1);

            /* Rotation issues are handled entirely in libTERE now */

            uint32_t *dup_wt = NULL;

            dup_wt = U_Latin1ToUtf32le(text, cChars, NULL);
            if(!dup_wt)dup_wt = unknown_chars(cChars);

            msdepua(dup_wt); //convert everything in Microsoft's private use area.  For Symbol, Wingdings, Dingbats

            if(NonToUnicode(dup_wt, d->dc[d->level].font_name)){
                free(d->dc[d->level].font_name);
                d->dc[d->level].font_name =  strdup("Times New Roman");
            }

            char *ansi_text;
            ansi_text = (char *) U_Utf32leToUtf8((uint32_t *)dup_wt, 0, NULL);
            free(dup_wt);
            // Empty text or starts with an invalid escape/control sequence, which is bogus text.  Throw it out before g_markup_escape_text can make things worse
            if(*((uint8_t *)ansi_text) <= 0x1F){
                free(ansi_text);
                ansi_text=NULL;
            }

            if (ansi_text) {

                SVGOStringStream ts;

                gchar *escaped_text = g_markup_escape_text(ansi_text, -1);

                tsp.x              = x*0.8;  // TERE expects sizes in points
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
                // WMF only supports two types of text decoration
                tsp.decoration = TXTDECOR_NONE;
                if(d->dc[d->level].style.text_decoration_line.underline){    tsp.decoration |= TXTDECOR_UNDER; }
                if(d->dc[d->level].style.text_decoration_line.line_through){ tsp.decoration |= TXTDECOR_STRIKE;}

                // WMF textalignment is a bit strange: 0x6 is center, 0x2 is right, 0x0 is left, the value 0x4 is also drawn left
                tsp.taln  = ((d->dc[d->level].textAlign & U_TA_CENTER)  == U_TA_CENTER)  ? ALICENTER :
                           (((d->dc[d->level].textAlign & U_TA_CENTER)  == U_TA_LEFT)    ? ALILEFT   :
                                                                                           ALIRIGHT);
                tsp.taln |= ((d->dc[d->level].textAlign & U_TA_BASEBIT) ? ALIBASE :
                            ((d->dc[d->level].textAlign & U_TA_BOTTOM)  ? ALIBOT  :
                                                                          ALITOP));

                // language direction can be encoded two ways, U_TA_RTLREADING is preferred 
                if( (fOptions & U_ETO_RTLREADING) || (d->dc[d->level].textAlign & U_TA_RTLREADING) ){ tsp.ldir = LDIR_RL; }
                else{                                                                                 tsp.ldir = LDIR_LR; }

                tsp.condensed = FC_WIDTH_NORMAL; // Not implemented well in libTERE (yet)
                tsp.ori = d->dc[d->level].style.baseline_shift.value;            // For now orientation is always the same as escapement
                // There is no world transform, so ori need not be further rotated
                tsp.string = (uint8_t *) U_strdup(escaped_text);                 // this will be free'd much later at a trinfo_clear().
                tsp.fs = d->dc[d->level].style.font_size.computed * 0.8;         // Font size in points
                char *fontspec = TR_construct_fontspec(&tsp, d->dc[d->level].font_name);
                tsp.fi_idx = ftinfo_load_fontname(d->tri->fti,fontspec);
                free(fontspec);
                // when font name includes narrow it may not be set to "condensed".  Narrow fonts do not work well anyway though
                // as the metrics from fontconfig may not match, or the font may not be present.
                if(0<= TR_findcasesub(d->dc[d->level].font_name, (char *) "Narrow")){ tsp.co=1; }
                else {                                                                tsp.co=0; }

                int status;

                status = trinfo_load_textrec(d->tri, &tsp, tsp.ori,TR_EMFBOT);  // ori is actually escapement
                if(status==-1){ // change of escapement, emit what we have and reset
                    TR_layout_analyze(d->tri);
                    if (d->dc[d->level].clip_id){
                        SVGOStringStream tmp_clip;
                        tmp_clip << "\n<g\n\tclip-path=\"url(#clipWmfPath" << d->dc[d->level].clip_id << ")\"\n>";
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
        case U_WMR_SETDIBTODEV:          dbg_str << "<!-- U_WMR_EXTTEXTOUT -->\n";           break;
        case U_WMR_SELECTPALETTE:        dbg_str << "<!-- U_WMR_SELECTPALETTE -->\n";        break;
        case U_WMR_REALIZEPALETTE:       dbg_str << "<!-- U_WMR_REALIZEPALETTE -->\n";       break;
        case U_WMR_ANIMATEPALETTE:       dbg_str << "<!-- U_WMR_ANIMATEPALETTE -->\n";       break;
        case U_WMR_SETPALENTRIES:        dbg_str << "<!-- U_WMR_SETPALENTRIES -->\n";        break;
        case U_WMR_POLYPOLYGON:
        {
            dbg_str << "<!-- U_WMR_POLYPOLYGON16 -->\n";
            uint16_t          nPolys;
            const uint16_t   *aPolyCounts;
            const char       *Points;
            int               cpts; /* total number of points in Points*/
            nSize = U_WMRPOLYPOLYGON_get(contents, &nPolys, &aPolyCounts, &Points);
            int n, i, j;

            d->mask |= wmr_mask;

            U_POINT16 apt;
            for (n=cpts=0; n < nPolys; n++) { cpts += aPolyCounts[n]; }
            i = 0; // offset in BYTES
            cpts *= U_SIZE_POINT16;  // limit for offset i, in BYTES

            for (n=0; n < nPolys && i<cpts; n++) {
                SVGOStringStream poly_path;

                memcpy(&apt, Points + i, U_SIZE_POINT16); // points may not be aligned, copy them this way

                poly_path << "\n\tM " << pix_to_xy( d, apt.x, apt.y) << " ";
                i += U_SIZE_POINT16;

                for (j=1; j < aPolyCounts[n] && i < cpts; j++) {
                    memcpy(&apt, Points + i, U_SIZE_POINT16); // points may not be aligned, copy them this way
                    poly_path << "\n\tL " << pix_to_xy( d, apt.x, apt.y) << " ";
                    i += U_SIZE_POINT16;
                }

                tmp_str << poly_path.str().c_str();
                tmp_str << " z";
                tmp_str << " \n";
            }

            tmp_path << tmp_str.str().c_str();

            break;
        }
        case U_WMR_RESIZEPALETTE:        dbg_str << "<!-- U_WMR_RESIZEPALETTE -->\n";        break;
        case U_WMR_3A:
        case U_WMR_3B:
        case U_WMR_3C:
        case U_WMR_3D:
        case U_WMR_3E:
        case U_WMR_3F:
        {
            dbg_str << "<!-- U_WMR_3A..3F -->\n";
            break;
        }
        case U_WMR_DIBBITBLT:
        {
            dbg_str << "<!-- U_WMR_DIBBITBLT -->\n";
            nSize = U_WMRDIBBITBLT_get(contents, &Dst, &cwh, &Src, &dwRop3, &dib);

            //  Treat all nonImage bitblts as a rectangular write.  Definitely not correct, but at
            //  least it leaves objects where the operations should have been.
            if (!dib) {
                // should be an application of a DIBPATTERNBRUSHPT, use a solid color instead

                if(dwRop3 == U_NOOP)break; /* GDI applications apparently often end with this as a sort of flush(), nothing should be drawn */
                int32_t dx = Dst.x;
                int32_t dy = Dst.y;
                int32_t dw = cwh.x;
                int32_t dh = cwh.y;
                SVGOStringStream tmp_rectangle;
                tmp_rectangle << "\n\tM " << pix_to_xy( d, dx,      dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy      )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx + dw, dy + dh )    << " ";
                tmp_rectangle << "\n\tL " << pix_to_xy( d, dx,      dy + dh )    << " ";
                tmp_rectangle << "\n\tz";

                d->mask |= wmr_mask;
                d->dwRop3 = dwRop3;   // we will try to approximate SOME of these
                d->mask |= U_DRAW_CLOSED; // Bitblit is not really open or closed, but we need it to fill, and this is the flag for that

                tmp_path <<   tmp_rectangle.str().c_str();
            }
            else {
                 double dx = pix_to_x_point( d, Dst.x, Dst.y);
                 double dy = pix_to_y_point( d, Dst.x, Dst.y);
                 double dw = pix_to_abs_size( d, cDst.x);
                 double dh = pix_to_abs_size( d, cDst.y);
                 //source position within the bitmap, in pixels
                 int sx = Src.x;
                 int sy = Src.y;
                 int sw = 0; // extract all of the image
                 int sh = 0;
                 if(sx<0)sx=0;
                 if(sy<0)sy=0;
                 // usageSrc not defined, implicitly it must be U_DIB_RGB_COLORS
                 common_dib_to_image(d,dib,dx,dy,dw,dh,sx,sy,sw,sh,U_DIB_RGB_COLORS);
            }
            break;
        }
        case U_WMR_DIBSTRETCHBLT:
        {
            dbg_str << "<!-- U_WMR_DIBSTRETCHBLT -->\n";
            nSize = U_WMRDIBSTRETCHBLT_get(contents, &Dst, &cDst, &Src, &cSrc, &dwRop3, &dib);
            // Always grab image, ignore modes.
            if (dib) {
                double dx = pix_to_x_point( d, Dst.x, Dst.y);
                double dy = pix_to_y_point( d, Dst.x, Dst.y);
                double dw = pix_to_abs_size( d, cDst.x);
                double dh = pix_to_abs_size( d, cDst.y);
                //source position within the bitmap, in pixels
                int sx = Src.x;
                int sy = Src.y;
                int sw = cSrc.x; // extract the specified amount of the image
                int sh = cSrc.y;
                // usageSrc not defined, implicitly it must be U_DIB_RGB_COLORS
                common_dib_to_image(d,dib,dx,dy,dw,dh,sx,sy,sw,sh, U_DIB_RGB_COLORS);
            }
            break;
        }
        case U_WMR_DIBCREATEPATTERNBRUSH:
        {
            dbg_str << "<!-- U_WMR_DIBCREATEPATTERNBRUSH -->\n";
            insert_object(d, U_WMR_DIBCREATEPATTERNBRUSH, contents);
            break;
        }
        case U_WMR_STRETCHDIB:
        {
            dbg_str << "<!-- U_WMR_STRETCHDIB -->\n";
            nSize = U_WMRSTRETCHDIB_get(contents, &Dst, &cDst, &Src, &cSrc, &cUsage, &dwRop3, &dib);
            double dx = pix_to_x_point( d, Dst.x, Dst.y );
            double dy = pix_to_y_point( d, Dst.x, Dst.y );
            double dw = pix_to_abs_size( d, cDst.x);
            double dh = pix_to_abs_size( d, cDst.y);
            int sx = Src.x;  //source position within the bitmap, in pixels
            int sy = Src.y;
            int sw = cSrc.x; // extract the specified amount of the image
            int sh = cSrc.y;
            uint32_t iUsageSrc;
            iUsageSrc = cUsage;
            common_dib_to_image(d,dib,dx,dy,dw,dh,sx,sy,sw,sh,iUsageSrc);

            break;
        }
        case U_WMR_44:
        case U_WMR_45:
        case U_WMR_46:
        case U_WMR_47:
        {
            dbg_str << "<!-- U_WMR_44..47 -->\n";
            break;
        }
        case U_WMR_EXTFLOODFILL:          dbg_str << "<!-- U_WMR_EXTFLOODFILL -->\n";  break;
        case U_WMR_49:
        case U_WMR_4A:
        case U_WMR_4B:
        case U_WMR_4C:
        case U_WMR_4D:
        case U_WMR_4E:
        case U_WMR_4F:
        case U_WMR_50:
        case U_WMR_51:
        case U_WMR_52:
        case U_WMR_53:
        case U_WMR_54:
        case U_WMR_55:
        case U_WMR_56:
        case U_WMR_57:
        case U_WMR_58:
        case U_WMR_59:
        case U_WMR_5A:
        case U_WMR_5B:
        case U_WMR_5C:
        case U_WMR_5D:
        case U_WMR_5E:
        case U_WMR_5F:
        case U_WMR_60:
        case U_WMR_61:
        case U_WMR_62:
        case U_WMR_63:
        case U_WMR_64:
        case U_WMR_65:
        case U_WMR_66:
        case U_WMR_67:
        case U_WMR_68:
        case U_WMR_69:
        case U_WMR_6A:
        case U_WMR_6B:
        case U_WMR_6C:
        case U_WMR_6D:
        case U_WMR_6E:
        case U_WMR_6F:
        case U_WMR_70:
        case U_WMR_71:
        case U_WMR_72:
        case U_WMR_73:
        case U_WMR_74:
        case U_WMR_75:
        case U_WMR_76:
        case U_WMR_77:
        case U_WMR_78:
        case U_WMR_79:
        case U_WMR_7A:
        case U_WMR_7B:
        case U_WMR_7C:
        case U_WMR_7D:
        case U_WMR_7E:
        case U_WMR_7F:
        case U_WMR_80:
        case U_WMR_81:
        case U_WMR_82:
        case U_WMR_83:
        case U_WMR_84:
        case U_WMR_85:
        case U_WMR_86:
        case U_WMR_87:
        case U_WMR_88:
        case U_WMR_89:
        case U_WMR_8A:
        case U_WMR_8B:
        case U_WMR_8C:
        case U_WMR_8D:
        case U_WMR_8E:
        case U_WMR_8F:
        case U_WMR_90:
        case U_WMR_91:
        case U_WMR_92:
        case U_WMR_93:
        case U_WMR_94:
        case U_WMR_95:
        case U_WMR_96:
        case U_WMR_97:
        case U_WMR_98:
        case U_WMR_99:
        case U_WMR_9A:
        case U_WMR_9B:
        case U_WMR_9C:
        case U_WMR_9D:
        case U_WMR_9E:
        case U_WMR_9F:
        case U_WMR_A0:
        case U_WMR_A1:
        case U_WMR_A2:
        case U_WMR_A3:
        case U_WMR_A4:
        case U_WMR_A5:
        case U_WMR_A6:
        case U_WMR_A7:
        case U_WMR_A8:
        case U_WMR_A9:
        case U_WMR_AA:
        case U_WMR_AB:
        case U_WMR_AC:
        case U_WMR_AD:
        case U_WMR_AE:
        case U_WMR_AF:
        case U_WMR_B0:
        case U_WMR_B1:
        case U_WMR_B2:
        case U_WMR_B3:
        case U_WMR_B4:
        case U_WMR_B5:
        case U_WMR_B6:
        case U_WMR_B7:
        case U_WMR_B8:
        case U_WMR_B9:
        case U_WMR_BA:
        case U_WMR_BB:
        case U_WMR_BC:
        case U_WMR_BD:
        case U_WMR_BE:
        case U_WMR_BF:
        case U_WMR_C0:
        case U_WMR_C1:
        case U_WMR_C2:
        case U_WMR_C3:
        case U_WMR_C4:
        case U_WMR_C5:
        case U_WMR_C6:
        case U_WMR_C7:
        case U_WMR_C8:
        case U_WMR_C9:
        case U_WMR_CA:
        case U_WMR_CB:
        case U_WMR_CC:
        case U_WMR_CD:
        case U_WMR_CE:
        case U_WMR_CF:
        case U_WMR_D0:
        case U_WMR_D1:
        case U_WMR_D2:
        case U_WMR_D3:
        case U_WMR_D4:
        case U_WMR_D5:
        case U_WMR_D6:
        case U_WMR_D7:
        case U_WMR_D8:
        case U_WMR_D9:
        case U_WMR_DA:
        case U_WMR_DB:
        case U_WMR_DC:
        case U_WMR_DD:
        case U_WMR_DE:
        case U_WMR_DF:
        case U_WMR_E0:
        case U_WMR_E1:
        case U_WMR_E2:
        case U_WMR_E3:
        case U_WMR_E4:
        case U_WMR_E5:
        case U_WMR_E6:
        case U_WMR_E7:
        case U_WMR_E8:
        case U_WMR_E9:
        case U_WMR_EA:
        case U_WMR_EB:
        case U_WMR_EC:
        case U_WMR_ED:
        case U_WMR_EE:
        case U_WMR_EF:
        {
            dbg_str << "<!-- U_WMR_EXTFLOODFILL..EF -->\n";
            break;
        }
        case U_WMR_DELETEOBJECT:
        {
            dbg_str << "<!-- U_WMR_DELETEOBJECT -->\n";
            nSize = U_WMRDELETEOBJECT_get(contents, &utmp16);
            delete_object(d, utmp16);
            break;
        }
        case U_WMR_F1:
        case U_WMR_F2:
        case U_WMR_F3:
        case U_WMR_F4:
        case U_WMR_F5:
        case U_WMR_F6:
        {
            dbg_str << "<!-- F1..F6 -->\n";
            break;
        }
        case U_WMR_CREATEPALETTE:
        {
            dbg_str << "<!-- U_WMR_CREATEPALETTE -->\n";
            insert_object(d, U_WMR_CREATEPALETTE, contents);
            break;
        }
        case U_WMR_F8:                    dbg_str << "<!-- F8 -->\n"; break;
        case U_WMR_CREATEPATTERNBRUSH:
        {
            dbg_str << "<!-- U_WMR_CREATEPATTERNBRUSH -->\n";
            insert_object(d, U_WMR_CREATEPATTERNBRUSH, contents);
            break;
          }
        case U_WMR_CREATEPENINDIRECT:
        {
            dbg_str << "<!-- U_WMR_EXTCREATEPEN -->\n";
            insert_object(d, U_WMR_CREATEPENINDIRECT, contents);
            break;
        }
        case U_WMR_CREATEFONTINDIRECT:
        {
            dbg_str << "<!-- U_WMR_CREATEFONTINDIRECT -->\n";
            insert_object(d, U_WMR_CREATEFONTINDIRECT, contents);
            break;
        }
        case U_WMR_CREATEBRUSHINDIRECT:
        {
            dbg_str << "<!-- U_WMR_CREATEBRUSHINDIRECT -->\n";
            insert_object(d, U_WMR_CREATEBRUSHINDIRECT, contents);
            break;
        }
        case U_WMR_CREATEBITMAPINDIRECT:
        {
            dbg_str << "<!-- U_WMR_CREATEBITMAPINDIRECT -->\n";
            insert_object(d, U_WMR_CREATEBITMAPINDIRECT, contents);
            break;
        }
        case U_WMR_CREATEBITMAP:
        {
            dbg_str << "<!-- U_WMR_CREATEBITMAP -->\n";
            insert_object(d, U_WMR_CREATEBITMAP, contents);
            break;
        }
        case U_WMR_CREATEREGION:
        {
            dbg_str << "<!-- U_WMR_CREATEREGION -->\n";
            insert_object(d, U_WMR_CREATEREGION, contents);
            break;
        }
        default:
            dbg_str << "<!-- U_WMR_??? -->\n";
            break;
    }  //end of switch
// When testing, uncomment the following to place a comment for each processed WMR record in the SVG
//    d->outsvg += dbg_str.str().c_str();
    d->path   += tmp_path.str().c_str();
    if(!nSize){ // There was some problem with the processing of this record, it is not safe to continue
        file_status = 0;
        break;
    } 

    }  //end of while on OK
// When testing, uncomment the following to show the final SVG derived from the WMF
// std::cout << d->outsvg << std::endl;
    (void) U_wmr_properties(U_WMR_INVALID);  // force the release of the lookup table memory, returned value is irrelevant

    return(file_status);
}

void Wmf::free_wmf_strings(WMF_STRINGS name){
    if(name.count){
        for(int i=0; i< name.count; i++){ free(name.strings[i]); }
        free(name.strings);
    }
    name.count = 0;
    name.size = 0;
}

SPDocument *
Wmf::open( Inkscape::Extension::Input * /*mod*/, const gchar *uri )
{

    if (uri == NULL) {
        return NULL;
    }

    WMF_CALLBACK_DATA d;
    
    d.n_obj = 0;     //these might not be set otherwise if the input file is corrupt
    d.wmf_obj=NULL; 

    // Default font, WMF spec says device can pick whatever it wants. 
    // WMF files that do not specify a  font are unlikely to look very good!
    d.dc[0].style.font_size.computed           = 16.0;
    d.dc[0].style.font_weight.value            = SP_CSS_FONT_WEIGHT_400;
    d.dc[0].style.font_style.value             = SP_CSS_FONT_STYLE_NORMAL;
    d.dc[0].style.text_decoration_line.underline    = 0;
    d.dc[0].style.text_decoration_line.line_through = 0;
    d.dc[0].style.baseline_shift.value         = 0;

    // Default pen, WMF files that do not specify a pen are unlikely to look very good!
    d.dc[0].style.stroke_dasharray.set         = 0;
    d.dc[0].style.stroke_linecap.computed      = 2; // U_PS_ENDCAP_SQUARE;
    d.dc[0].style.stroke_linejoin.computed     = 0; // U_PS_JOIN_MITER;
    d.dc[0].style.stroke_width.value           = 1.0; // will be reset to something reasonable once WMF drawing size is known
    d.dc[0].style.stroke.value.color.set( 0, 0, 0 );
    d.dc[0].stroke_set                         = true;

    // Default brush is none - no fill. WMF files that do not specify a brush are unlikely to look very good!
    d.dc[0].fill_set                           = false;

    d.dc[0].font_name = strdup("Arial"); // Default font, set only on lowest level, it copies up from there WMF spec says device can pick whatever it wants

    // set up the size default for patterns in defs.  This might not be referenced if there are no patterns defined in the drawing.

    d.defs += "\n";
    d.defs += "   <pattern id=\"WMFhbasepattern\"     \n";
    d.defs += "        patternUnits=\"userSpaceOnUse\"\n";
    d.defs += "        width=\"6\"                    \n";
    d.defs += "        height=\"6\"                   \n";
    d.defs += "        x=\"0\"                        \n";
    d.defs += "        y=\"0\">                       \n";
    d.defs += "   </pattern>                          \n";


    size_t length;
    char *contents;
    if(wmf_readdata(uri, &contents, &length))return(NULL);

    // set up the text reassembly system
    if(!(d.tri = trinfo_init(NULL)))return(NULL);
    (void) trinfo_load_ft_opts(d.tri, 1,
      FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING  | FT_LOAD_NO_BITMAP,
      FT_KERNING_UNSCALED);

    int good = myMetaFileProc(contents,length, &d);
    free(contents);

//    std::cout << "SVG Output: " << std::endl << d.outsvg << std::endl;

    SPDocument *doc = NULL;
    if (good) {
        doc = SPDocument::createNewDocFromMem(d.outsvg.c_str(), strlen(d.outsvg.c_str()), TRUE);
    }

    free_wmf_strings(d.hatches);
    free_wmf_strings(d.images);
    free_wmf_strings(d.clips);

    if (d.wmf_obj) {
        int i;
        for (i=0; i<d.n_obj; i++)
            delete_object(&d, i);
        delete[] d.wmf_obj;
    }

    d.dc[0].style.stroke_dasharray.values.clear();

    for(int i=0; i<=WMF_MAX_DC; i++){
      if(d.dc[i].font_name)free(d.dc[i].font_name);
    }

    d.tri = trinfo_release_except_FC(d.tri);

    // in earlier versions no viewbox was generated and a call to setViewBoxIfMissing() was needed here.

    return doc;
}


void
Wmf::init (void)
{
    /* WMF in */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("WMF Input") "</name>\n"
            "<id>org.inkscape.input.wmf</id>\n"
            "<input>\n"
                "<extension>.wmf</extension>\n"
                "<mimetype>image/x-wmf</mimetype>\n"
                "<filetypename>" N_("Windows Metafiles (*.wmf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Windows Metafiles") "</filetypetooltip>\n"
                "<output_extension>org.inkscape.output.wmf</output_extension>\n"
            "</input>\n"
        "</inkscape-extension>", new Wmf());

    /* WMF out */
    Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("WMF Output") "</name>\n"
            "<id>org.inkscape.output.wmf</id>\n"
            "<param name=\"textToPath\" gui-text=\"" N_("Convert texts to paths") "\" type=\"boolean\">true</param>\n"
            "<param name=\"TnrToSymbol\" gui-text=\"" N_("Map Unicode to Symbol font") "\" type=\"boolean\">true</param>\n"
            "<param name=\"TnrToWingdings\" gui-text=\"" N_("Map Unicode to Wingdings") "\" type=\"boolean\">true</param>\n"
            "<param name=\"TnrToZapfDingbats\" gui-text=\"" N_("Map Unicode to Zapf Dingbats") "\" type=\"boolean\">true</param>\n"
            "<param name=\"UsePUA\" gui-text=\"" N_("Use MS Unicode PUA (0xF020-0xF0FF) for converted characters") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTCharPos\" gui-text=\"" N_("Compensate for PPT font bug") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTDashLine\" gui-text=\"" N_("Convert dashed/dotted lines to single lines") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTGrad2Polys\" gui-text=\"" N_("Convert gradients to colored polygon series") "\" type=\"boolean\">false</param>\n"
            "<param name=\"FixPPTPatternAsHatch\" gui-text=\"" N_("Map all fill patterns to standard WMF hatches") "\" type=\"boolean\">false</param>\n"
            "<output>\n"
                "<extension>.wmf</extension>\n"
                "<mimetype>image/x-wmf</mimetype>\n"
                "<filetypename>" N_("Windows Metafile (*.wmf)") "</filetypename>\n"
                "<filetypetooltip>" N_("Windows Metafile") "</filetypetooltip>\n"
            "</output>\n"
        "</inkscape-extension>", new Wmf());

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
