/* 
 *  This file came from libwpg as a source, their utility wpg2svg
 *  specifically.  It has been modified to work as an Inkscape extension.
 *  The Inkscape extension code is covered by this copyright, but the
 *  rest is covered by the one bellow.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2006 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 *
 */

/* libwpg
 * Copyright (C) 2006 Ariya Hidayat (ariya@kde.org)
 * Copyright (C) 2005 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 * For further information visit http://libwpg.sourceforge.net
 */

/* "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include <stdio.h>
#include "config.h"

#ifdef WITH_LIBWPG

#include "wpg-input.h"
#include "extension/system.h"
#include "extension/input.h"
#include "document.h"

#include "libwpg/libwpg.h"
#include "libwpg/WPGStreamImplementation.h"


using namespace libwpg;

namespace Inkscape {
namespace Extension {
namespace Internal {


SPDocument *
WpgInput::open(Inkscape::Extension::Input * mod, const gchar * uri) {
    WPXInputStream* input = new libwpg::WPGFileStream(uri);
    if (input->isOLEStream()) {
        WPXInputStream* olestream = input->getDocumentOLEStream();
        if (olestream) {
            delete input;
            input = olestream;
        }
    }

    if (!WPGraphics::isSupported(input)) {
        //! \todo Dialog here
        // fprintf(stderr, "ERROR: Unsupported file format (unsupported version) or file is encrypted!\n");
        // printf("I'm giving up not supported\n");
        delete input;
        return NULL;
    }

    libwpg::WPGString output;
    if (!libwpg::WPGraphics::generateSVG(input, output)) {
        delete input;
        return NULL;
    }

    //printf("I've got a doc: \n%s", painter.document.c_str());

    SPDocument * doc = sp_document_new_from_mem(output.cstr(), strlen(output.cstr()), TRUE);
    delete input;
    return doc;
}

#include "clear-n_.h"

void
WpgInput::init(void) {
    Inkscape::Extension::Extension * ext;

    ext = Inkscape::Extension::build_from_mem(
        "<inkscape-extension xmlns=\"" INKSCAPE_EXTENSION_URI "\">\n"
            "<name>" N_("WPG Input") "</name>\n"
            "<id>org.inkscape.input.wpg</id>\n"
            "<input>\n"
                "<extension>.wpg</extension>\n"
                "<mimetype>image/x-wpg</mimetype>\n"
                "<filetypename>" N_("WordPerfect Graphics (*.wpg)") "</filetypename>\n"
                "<filetypetooltip>" N_("Vector graphics format used by Corel WordPerfect") "</filetypetooltip>\n"
            "</input>\n"
        "</inkscape-extension>", new WpgInput());
} // init

} } }  /* namespace Inkscape, Extension, Implementation */
#endif /* WITH_LIBWPG */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
