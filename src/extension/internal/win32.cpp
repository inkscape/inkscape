#define __SP_MODULE_WIN32_C__

/*
 * Windows stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gmem.h>
#include <libnr/nr-macros.h>
#include <libnr/nr-matrix.h>

#include "display/nr-arena-item.h"
#include "display/nr-arena.h"
#include "document.h"

#include "win32.h"
#include "extension/system.h"
#include "extension/print.h"
#include <gtk/gtk.h>

/* Initialization */

namespace Inkscape {
namespace Extension {
namespace Internal {

static unsigned int SPWin32Modal = FALSE;

/**
 * Callback function..  not a method
 */
static void
my_gdk_event_handler (GdkEvent *event)
{
    if (SPWin32Modal) {
        /* Win32 widget is modal, filter events */
        switch (event->type) {
        case GDK_NOTHING:
        case GDK_DELETE:
        case GDK_SCROLL:
        case GDK_BUTTON_PRESS:
        case GDK_2BUTTON_PRESS:
        case GDK_3BUTTON_PRESS:
        case GDK_BUTTON_RELEASE:
        case GDK_KEY_PRESS:
        case GDK_KEY_RELEASE:
        case GDK_DRAG_STATUS:
        case GDK_DRAG_ENTER:
        case GDK_DRAG_LEAVE:
        case GDK_DRAG_MOTION:
        case GDK_DROP_START:
        case GDK_DROP_FINISHED:
            return;
            break;
        default:
            break;
        }
    }
    gtk_main_do_event (event);
}

void
PrintWin32::main_init (int argc, char **argv, const char *name)
{
    gdk_event_handler_set ((GdkEventFunc) my_gdk_event_handler, NULL, NULL);
}

void
PrintWin32::finish (void)
{
}

#define SP_FOREIGN_MAX_ITER 10


/**
 * Callback function..  not a method
 */
static VOID CALLBACK
my_timer (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    int cdown = 0;
    while ((cdown++ < SP_FOREIGN_MAX_ITER) && gdk_events_pending ()) {
        gtk_main_iteration_do (FALSE);
    }
    gtk_main_iteration_do (FALSE);
}


/* Platform detection */

gboolean
PrintWin32::is_os_wide()
{
    static gboolean initialized = FALSE;
    static gboolean is_wide = FALSE;
    static OSVERSIONINFOA osver;

    if ( !initialized )
    {
        BOOL result;

        initialized = TRUE;

        memset (&osver, 0, sizeof(OSVERSIONINFOA));
        osver.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
        result = GetVersionExA (&osver);
        if (result)
        {
            if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
                is_wide = TRUE;
        }
        // If we can't even call to get the version, fall back to ANSI API
    }

    return is_wide;
}


/* Printing */

PrintWin32::PrintWin32 (void)
{
    /* Nothing here */
}


PrintWin32::~PrintWin32 (void)
{
    DeleteDC (_hDC);
}


/**
 * Callback function..  not a method
 */
static UINT_PTR CALLBACK
print_hook (HWND hdlg, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
#if 0
    int cdown = 0;
    while ((cdown++ < SP_FOREIGN_MAX_ITER) && gdk_events_pending ()) {
        gtk_main_iteration_do (FALSE);
    }
    gtk_main_iteration_do (FALSE);
#endif
    return 0;
}

unsigned int
PrintWin32::setup (Inkscape::Extension::Print *mod)
{
    HRESULT res;
    PRINTDLG pd = {
        sizeof (PRINTDLG),
        NULL, /* hwndOwner */
        NULL, /* hDevMode */
        NULL, /* hDevNames */
        NULL, /* hDC */
        PD_NOPAGENUMS | PD_NOSELECTION | PD_RETURNDC | PD_USEDEVMODECOPIESANDCOLLATE, /* Flags */
        1, 1, 1, 1, /* nFromPage, nToPage, nMinPage, nMaxPage */
        1, /* nCoies */
        NULL, /* hInstance */
        0, /* lCustData */
        NULL, NULL, NULL, NULL, NULL, NULL
    };
    UINT_PTR timer;

    SPWin32Modal = TRUE;
    pd.Flags |= PD_ENABLEPRINTHOOK;
    pd.lpfnPrintHook = print_hook;
    timer = SetTimer (NULL, 0, 40, my_timer);

    res = PrintDlg (&pd);

    KillTimer (NULL, timer);
    SPWin32Modal = FALSE;

    if (!res) return FALSE;

    _hDC = pd.hDC;

#if 0
    caps = GetDeviceCaps (_hDC, RASTERCAPS);
    if (caps & RC_BANDING) {
        printf ("needs banding\n");
    }
    if (caps & RC_BITBLT) {
        printf ("does bitblt\n");
    }
    if (caps & RC_DIBTODEV) {
        printf ("does dibtodev\n");
    }
    if (caps & RC_STRETCHDIB) {
        printf ("does stretchdib\n");
    }
#endif
    if (pd.hDevMode) {
        DEVMODE *devmodep;
        devmodep = (DEVMODE *)pd.hDevMode;
        if (devmodep->dmFields & DM_ORIENTATION) {
            _landscape = (devmodep->dmOrientation == DMORIENT_LANDSCAPE);
        }
    }

    return TRUE;
}

unsigned int
PrintWin32::begin (Inkscape::Extension::Print *mod, SPDocument *doc)
{
    DOCINFO di = {
        sizeof (DOCINFO),
        NULL, /* lpszDocName */
        NULL, /* lpszOutput */
        NULL, /* lpszDatatype */
        0 /* DI_APPBANDING */ /* fwType */
    };
    int res;

    _PageWidth = sp_document_width (doc);
    _PageHeight = sp_document_height (doc);

    di.lpszDocName = SP_DOCUMENT_NAME (doc);

    SPWin32Modal = TRUE;

    res = StartDoc (_hDC, &di);
    res = StartPage (_hDC);

    SPWin32Modal = FALSE;

    return 0;
}

unsigned int
PrintWin32::finish (Inkscape::Extension::Print *mod)
{
    int dpiX, dpiY;
    int pPhysicalWidth, pPhysicalHeight;
    int pPhysicalOffsetX, pPhysicalOffsetY;
    int pPrintableWidth, pPrintableHeight;
    float scalex, scaley;
    int x0, y0, x1, y1;
    int width, height;
    NR::Matrix affine;
    unsigned char *px;
    int sheight, row;
    BITMAPINFO bmInfo = {
        {
            sizeof (BITMAPINFOHEADER), // bV4Size
            64,      // biWidth
            64,      // biHeight
            1,       // biPlanes
            32,      // biBitCount
            BI_RGB,  // biCompression
            0,       // biSizeImage
            2835,    // biXPelsPerMeter
            2835,    // biYPelsPerMeter
            0,       // biClrUsed
            0        // biClrImportant
        },
        { { 0, 0, 0, 0 } } // bmiColors
    };
    //RECT wrect;
    int res;

    SPWin32Modal = TRUE;

    // Number of pixels per logical inch
    dpiX = (int) GetDeviceCaps (_hDC, LOGPIXELSX);
    dpiY = (int) GetDeviceCaps (_hDC, LOGPIXELSY);
    // Size in pixels of the printable area
    pPhysicalWidth = GetDeviceCaps (_hDC, PHYSICALWIDTH);
    pPhysicalHeight = GetDeviceCaps (_hDC, PHYSICALHEIGHT);
    // Top left corner of prontable area
    pPhysicalOffsetX = GetDeviceCaps (_hDC, PHYSICALOFFSETX);
    pPhysicalOffsetY = GetDeviceCaps (_hDC, PHYSICALOFFSETY);
    // Size in pixels of the printable area
    pPrintableWidth = GetDeviceCaps (_hDC, HORZRES);
    pPrintableHeight = GetDeviceCaps (_hDC, VERTRES);

    // Scaling from document to device
    scalex = dpiX / 72.0;
    scaley = dpiY / 72.0;

    // We simply map document 0,0 to physical page 0,0
    affine[0] = scalex / 1.25;
    affine[1] = 0.0;
    affine[2] = 0.0;
    affine[3] = scaley / 1.25;
    affine[4] = 0.0;
    affine[5] = 0.0;

    nr_arena_item_set_transform (mod->root, &affine);

    // Calculate printable area in device coordinates
    x0 = pPhysicalOffsetX;
    y0 = pPhysicalOffsetY;
    x1 = x0 + pPrintableWidth;
    y1 = y0 + pPrintableHeight;
    x1 = MIN (x1, (int) (_PageWidth * scalex));
    y1 = MIN (y1, (int) (_PageHeight * scaley));

    width = x1 - x0;
    height = y1 - y0;

    px = g_new (unsigned char, 4 * 64 * width);
    sheight = 64;

    /* Printing goes here */
    for (row = 0; row < height; row += 64) {
        NRPixBlock pb;
        NRRectL bbox;
        NRGC gc(NULL);
        int num_rows;
        int i;

        num_rows = sheight;
        if ((row + num_rows) > height) num_rows = height - row;

        /* Set area of interest */
        bbox.x0 = x0;
        bbox.y0 = y0 + row;
        bbox.x1 = bbox.x0 + width;
        bbox.y1 = bbox.y0 + num_rows;
        /* Update to renderable state */
        gc.transform.set_identity();
        nr_arena_item_invoke_update (mod->root, &bbox, &gc, NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

        nr_pixblock_setup_extern (&pb, NR_PIXBLOCK_MODE_R8G8B8A8N, bbox.x0, bbox.y0, bbox.x1, bbox.y1, px, 4 * (bbox.x1 - bbox.x0), FALSE, FALSE);

        /* Blitter goes here */
        bmInfo.bmiHeader.biWidth = bbox.x1 - bbox.x0;
        bmInfo.bmiHeader.biHeight = -(bbox.y1 - bbox.y0);

        memset (px, 0xff, 4 * num_rows * width);
        /* Render */
        nr_arena_item_invoke_render (NULL, mod->root, &bbox, &pb, 0);

        /* Swap red and blue channels; we use RGBA, whereas
         * the Win32 GDI uses BGRx.
         */
        for ( i = 0 ; i < num_rows * width ; i++ ) {
            unsigned char temp=px[i*4];
            px[i*4] = px[i*4+2];
            px[i*4+2] = temp;
        }

        SetStretchBltMode(_hDC, COLORONCOLOR);
        res = StretchDIBits (_hDC,
                        bbox.x0 - x0, bbox.y0 - y0, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0,
                        0, 0, bbox.x1 - bbox.x0, bbox.y1 - bbox.y0,
                        px,
                        &bmInfo,
                        DIB_RGB_COLORS,
                        SRCCOPY);

        /* Blitter ends here */

        nr_pixblock_release (&pb);
    }

    g_free (px);

    res = EndPage (_hDC);
    res = EndDoc (_hDC);

    SPWin32Modal = FALSE;

    return 0;
}

/* File dialogs */

char *
PrintWin32::get_open_filename (unsigned char *dir, unsigned char *filter, unsigned char *title)
{
    char fnbuf[4096] = {0};
    OPENFILENAME ofn = {
        sizeof (OPENFILENAME),
        NULL, /* hwndOwner */
        NULL, /* hInstance */
        (const CHAR *)filter, /* lpstrFilter */
        NULL, /* lpstrCustomFilter */
        0, /* nMaxCustFilter  */
        1, /* nFilterIndex */
        fnbuf, /* lpstrFile */
        sizeof (fnbuf), /* nMaxFile */
        NULL, /* lpstrFileTitle */
        0, /* nMaxFileTitle */
        (const CHAR *)dir, /* lpstrInitialDir */
        (const CHAR *)title, /* lpstrTitle */
        OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY, /* Flags */
        0, /* nFileOffset */
        0, /* nFileExtension */
        NULL, /* lpstrDefExt */
        0, /* lCustData */
        NULL, /* lpfnHook */
        NULL /* lpTemplateName */
    };
    int retval;
    UINT_PTR timer;

    SPWin32Modal = TRUE;
    timer = SetTimer (NULL, 0, 40, my_timer);

    retval = GetOpenFileName (&ofn);

    KillTimer (NULL, timer);
    SPWin32Modal = FALSE;

    if (!retval) {
        int errcode;
        errcode = CommDlgExtendedError();
        return NULL;
    }
    return g_strdup (fnbuf);
}

char *
PrintWin32::get_write_filename (unsigned char *dir, unsigned char *filter, unsigned char *title)
{
    return NULL;
}

char *
PrintWin32::get_save_filename (unsigned char *dir, unsigned int *spns)
{
    char fnbuf[4096] = {0};
    OPENFILENAME ofn = {
        sizeof (OPENFILENAME),
        NULL, /* hwndOwner */
        NULL, /* hInstance */
        "Inkscape SVG (*.svg)\0*\0Plain SVG (*.svg)\0*\0", /* lpstrFilter */
        NULL, /* lpstrCustomFilter */
        0, /* nMaxCustFilter  */
        1, /* nFilterIndex */
        fnbuf, /* lpstrFile */
        sizeof (fnbuf), /* nMaxFile */
        NULL, /* lpstrFileTitle */
        0, /* nMaxFileTitle */
        (const CHAR *)dir, /* lpstrInitialDir */
        "Save document to file", /* lpstrTitle */
        OFN_HIDEREADONLY, /* Flags */
        0, /* nFileOffset */
        0, /* nFileExtension */
        NULL, /* lpstrDefExt */
        0, /* lCustData */
        NULL, /* lpfnHook */
        NULL /* lpTemplateName */
    };
    int retval;
    UINT_PTR timer;

    SPWin32Modal = TRUE;
    timer = SetTimer (NULL, 0, 40, my_timer);

    retval = GetSaveFileName (&ofn);

    KillTimer (NULL, timer);
    SPWin32Modal = FALSE;

    if (!retval) {
        int errcode;
        errcode = CommDlgExtendedError();
        return NULL;
    }
    *spns = (ofn.nFilterIndex != 2);
    return g_strdup (fnbuf);
}

#include "clear-n_.h"

void
PrintWin32::init (void)
{
    Inkscape::Extension::Extension * ext;

    /* SVG in */
    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("Windows 32-bit Print") "</name>\n"
            "<id>" SP_MODULE_KEY_PRINT_WIN32 "</id>\n"
            "<param name=\"textToPath\" type=\"boolean\">true</param>\n"
            "<print/>\n"
        "</inkscape-extension>", new PrintWin32());

    return;
}

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */
