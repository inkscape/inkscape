/** @file
 * @brief Windows-specific stuff
 */
/* Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Lauris: This code is in public domain
 * Ted: This code is released under the GNU GPL
 */

#ifndef __INKSCAPE_EXTENSION_INTERNAL_PRINT_WIN32_H__
#define __INKSCAPE_EXTENSION_INTERNAL_PRINT_WIN32_H__
#ifdef WIN32

#ifdef HAVE_CONFIG_H
 #include <config.h>
#endif

#ifdef DATADIR
#undef DATADIR
#endif
#include <windows.h>

#include "extension/extension.h"
#include "extension/implementation/implementation.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

/* Initialization */

class PrintWin32 : public Inkscape::Extension::Implementation::Implementation {
    /* Document dimensions */
    float _PageWidth;
    float _PageHeight;

    HDC _hDC;

    unsigned int _landscape;

    void main_init (int argc, char **argv, const char *name);
    void finish (void);

    /* File dialogs */
    char *get_open_filename (unsigned char *dir, unsigned char *filter, unsigned char *title);
    char *get_write_filename (unsigned char *dir, unsigned char *filter, unsigned char *title);
    char *get_save_filename (unsigned char *dir, unsigned int *spns);

    VOID CALLBACK timer (HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);


public:
    PrintWin32 (void);
    virtual ~PrintWin32 (void);

    /* Tell modules about me */
    static void init (void);

    /* Platform detection */
    static gboolean is_os_wide();

    /* Print functions */
    virtual unsigned int setup (Inkscape::Extension::Print * module);
    //virtual unsigned int set_preview (Inkscape::Extension::Print * module);

    virtual unsigned int begin (Inkscape::Extension::Print * module, Document *doc);
    virtual unsigned int finish (Inkscape::Extension::Print * module);

    /* Rendering methods */
    /*
    virtual unsigned int bind (Inkscape::Extension::Print * module, const Geom::Matrix *transform, float opacity);
    virtual unsigned int release (Inkscape::Extension::Print * module);
    virtual unsigned int comment (Inkscape::Extension::Print * module, const char * comment);
    virtual unsigned int image (Inkscape::Extension::Print * module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
			    const Geom::Matrix *transform, const SPStyle *style);
    */
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif // ifdef WIN32
#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_WIN32_H__ */
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
