#ifndef __INKSCAPE_EXTENSION_INTERNAL_PRINT_GNOME_H__
#define __INKSCAPE_EXTENSION_INTERNAL_PRINT_GNOME_H__

/*
 * Gnome stuff
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Lauris: This code is in public domain
 * Ted: This code is under the GNU GPL
 */

#include <config.h>

#include <libgnomeprint/gnome-print.h>

#include "extension/implementation/implementation.h"
#include "extension/extension.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

class PrintGNOME : public Inkscape::Extension::Implementation::Implementation {
	GnomePrintContext * _gpc;

public:
	PrintGNOME (void);
	virtual ~PrintGNOME (void);

	/* Print functions */
	virtual unsigned int setup (Inkscape::Extension::Print * module);
	virtual unsigned int set_preview (Inkscape::Extension::Print * module);

	virtual unsigned int begin (Inkscape::Extension::Print * module, SPDocument *doc);
	virtual unsigned int finish (Inkscape::Extension::Print * module);

	/* Rendering methods */
	virtual unsigned int bind (Inkscape::Extension::Print * module, const NRMatrix *transform, float opacity);
	virtual unsigned int release (Inkscape::Extension::Print * module);
	virtual unsigned int fill (Inkscape::Extension::Print * module, const NRBPath *bpath, const NRMatrix *ctm, const SPStyle *style,
			       const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	virtual unsigned int stroke (Inkscape::Extension::Print * module, const NRBPath *bpath, const NRMatrix *transform, const SPStyle *style,
				 const NRRect *pbox, const NRRect *dbox, const NRRect *bbox);
	virtual unsigned int image (Inkscape::Extension::Print * module, unsigned char *px, unsigned int w, unsigned int h, unsigned int rs,
				const NRMatrix *transform, const SPStyle *style);
    virtual unsigned int comment(Inkscape::Extension::Print *module, const char * comment);

	static void init (void);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* __INKSCAPE_EXTENSION_INTERNAL_PRINT_GNOME_H__ */
