/** @file
 * @brief Windows Metafile Input/Output
 */
/* Authors:
 *   Ulf Erikson <ulferikson@users.sf.net>
 *
 * Copyright (C) 2006-2008 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_EXTENSION_INTERNAL_WMF_H
#define SEEN_EXTENSION_INTERNAL_WMF_H

#include <libuemf/uwmf.h>
#include "extension/internal/metafile-inout.h"  // picks up PNG
#include "extension/implementation/implementation.h"
#include "style.h"
#include "text_reassemble.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

#define DIRTY_NONE   0x00
#define DIRTY_TEXT   0x01
#define DIRTY_FILL   0x02
#define DIRTY_STROKE 0x04 // not used currently

typedef struct {
    int type;
    int level;
    char *record;
} WMF_OBJECT, *PWMF_OBJECT;

typedef struct {
    int size;         // number of slots allocated in strings
    int count;        // number of slots used in strings
    char **strings;   // place to store strings
} WMF_STRINGS, *PWMF_STRINGS;

typedef struct wmf_device_context {
    struct SPStyle  style;
    char           *font_name;
    bool            stroke_set;
    int             stroke_mode;  // enumeration from drawmode, not used if fill_set is not True
    int             stroke_idx;   // used with DRAW_PATTERN and DRAW_IMAGE to return the appropriate fill
    int             stroke_recidx;// record used to regenerate hatch when it needs to be redone due to bkmode, textmode, etc. change
    bool            fill_set;
    int             fill_mode;    // enumeration from drawmode, not used if fill_set is not True
    int             fill_idx;     // used with DRAW_PATTERN and DRAW_IMAGE to return the appropriate fill
    int             fill_recidx;  // record used to regenerate hatch when it needs to be redone due to bkmode, textmode, etc. change
    int             dirty;        // holds the dirty bits for text, stroke, fill
    int             active_pen;   // used when the active object is deleted to set the default values, -1 is none active
    int             active_brush; // ditto
    int             active_font;  // ditto. also used to hold object number in case font needs to be remade due to textcolor change.
    U_POINT16       sizeWnd;
    U_POINT16       sizeView;
    U_POINT16       winorg;
    U_POINT16       vieworg;
    double          ScaleInX, ScaleInY;
    double          ScaleOutX, ScaleOutY;
    uint16_t        bkMode;
    U_COLORREF      bkColor;
    U_COLORREF      textColor;
    uint16_t        textAlign;
    U_POINT16       cur;
} WMF_DEVICE_CONTEXT, *PWMF_DEVICE_CONTEXT;

#define WMF_MAX_DC 128


// like this causes a mysterious crash on the return from Wmf::open
//typedef struct emf_callback_data {
// this fixes it, so some confusion between this struct and the one in emf-inout???
//typedef struct wmf_callback_data {
// as does this
typedef struct {
    Glib::ustring *outsvg;
    Glib::ustring *path;
    Glib::ustring *outdef;
    Glib::ustring *defs;

    WMF_DEVICE_CONTEXT dc[WMF_MAX_DC+1]; // FIXME: This should be dynamic..
    int level;
    
    double E2IdirY;                     // WMF Y direction relative to Inkscape Y direction.  Will be negative for MM_LOMETRIC etc.
    double D2PscaleX,D2PscaleY;         // WMF device to Inkscape Page scale.
    float  PixelsInX, PixelsInY;        // size of the drawing, in WMF device pixels
    float  PixelsOutX, PixelsOutY;      // size of the drawing, in Inkscape pixels
    double ulCornerInX,ulCornerInY;     // Upper left corner, from header rclBounds, in logical units
    double ulCornerOutX,ulCornerOutY;   // Upper left corner, in Inkscape pixels
    uint32_t mask;                      // Draw properties
    int arcdir;                         // U_AD_COUNTERCLOCKWISE 1 or U_AD_CLOCKWISE 2
    
    uint32_t dwRop2;                    // Binary raster operation, 0 if none (use brush/pen unmolested)
    uint32_t dwRop3;                    // Ternary raster operation, 0 if none (use brush/pen unmolested)

    unsigned int id;
    unsigned int drawtype;    // one of 0 or U_WMR_FILLPATH, U_WMR_STROKEPATH, U_WMR_STROKEANDFILLPATH
                              // both of these end up in <defs> under the names shown here.  These structures allow duplicates to be avoided.
    WMF_STRINGS hatches;      // hold pattern names, all like WMFhatch#_$$$$$$ where # is the WMF hatch code and $$$$$$ is the color
    WMF_STRINGS images;       // hold images, all like Image#, where # is the slot the image lives.
    TR_INFO    *tri;          // Text Reassembly data structure


    int n_obj;
    int low_water;            // first object slot which _might_ be unoccupied.  Everything below is filled.
    PWMF_OBJECT wmf_obj;
} WMF_CALLBACK_DATA, *PWMF_CALLBACK_DATA;

class Wmf : public Metafile
{

public:
    Wmf(); // Empty constructor

    virtual ~Wmf();//Destructor

    bool check(Inkscape::Extension::Extension *module); //Can this module load (always yes for now)

    void save(Inkscape::Extension::Output *mod, // Save the given document to the given filename
              SPDocument *doc,
              gchar const *filename);

    virtual SPDocument *open( Inkscape::Extension::Input *mod,
                                const gchar *uri );

    static void init(void);//Initialize the class

private:
protected:
   static void        print_document_to_file(SPDocument *doc, const gchar *filename);
   static double      current_scale(PWMF_CALLBACK_DATA d);
   static std::string current_matrix(PWMF_CALLBACK_DATA d, double x, double y, int useoffset);
   static double      current_rotation(PWMF_CALLBACK_DATA d);
   static void        enlarge_hatches(PWMF_CALLBACK_DATA d);
   static int         in_hatches(PWMF_CALLBACK_DATA d, char *test);
   static uint32_t    add_hatch(PWMF_CALLBACK_DATA d, uint32_t hatchType, U_COLORREF hatchColor);
   static void        enlarge_images(PWMF_CALLBACK_DATA d);
   static int         in_images(PWMF_CALLBACK_DATA d, char *test);
   static uint32_t    add_dib_image(PWMF_CALLBACK_DATA d, const char *dib, uint32_t iUsage);
   static uint32_t    add_bm16_image(PWMF_CALLBACK_DATA d, U_BITMAP16 Bm16, const char *px);
   static void        output_style(PWMF_CALLBACK_DATA d);
   static double      _pix_x_to_point(PWMF_CALLBACK_DATA d, double px);
   static double      _pix_y_to_point(PWMF_CALLBACK_DATA d, double py);
   static double      pix_to_x_point(PWMF_CALLBACK_DATA d, double px, double py);
   static double      pix_to_y_point(PWMF_CALLBACK_DATA d, double px, double py);
   static double      pix_to_abs_size(PWMF_CALLBACK_DATA d, double px);
   static std::string pix_to_xy(PWMF_CALLBACK_DATA d, double x, double y);
   static void        select_brush(PWMF_CALLBACK_DATA d, int index);
   static void        select_font(PWMF_CALLBACK_DATA d, int index);
   static void        select_pen(PWMF_CALLBACK_DATA d, int index);
   static int         insertable_object(PWMF_CALLBACK_DATA d);
   static void        delete_object(PWMF_CALLBACK_DATA d, int index);
   static int         insert_object(PWMF_CALLBACK_DATA d, int type, const char *record);
   static uint32_t   *unknown_chars(size_t count);
   static void        common_dib_to_image(PWMF_CALLBACK_DATA d, const char *dib,
                         double dx, double dy, double dw, double dh, int sx, int sy, int sw, int sh, uint32_t iUsage);
   static void        common_bm16_to_image(PWMF_CALLBACK_DATA d, U_BITMAP16 Bm16, const char *px,
                         double dx, double dy, double dw, double dh, int sx, int sy, int sw, int sh);
   static int         myMetaFileProc(const char *contents, unsigned int length, PWMF_CALLBACK_DATA d);
   static void        free_wmf_strings(WMF_STRINGS name);

};

} } }  /* namespace Inkscape, Extension, Implementation */


#endif /* EXTENSION_INTERNAL_WMF_H */

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
