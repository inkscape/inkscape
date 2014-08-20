/*
 * This is file is kind of the junk file.  Basically everything that
 * didn't fit in one of the other well defined areas, well, it's now
 * here.  Which is good in someways, but this file really needs some
 * definition.  Hopefully that will come ASAP.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_EXTENSION_SYSTEM_H__
#define INKSCAPE_EXTENSION_SYSTEM_H__

#include <glibmm/ustring.h>

class SPDocument;

namespace Inkscape {

namespace Extension {
class Extension;
class Print;

namespace Implementation {
class Implementation;
}

/**
 * Used to distinguish between the various invocations of the save dialogs (and thus to determine
 * the file type and save path offered in the dialog)
 */
enum FileSaveMethod {
    FILE_SAVE_METHOD_SAVE_AS,
    FILE_SAVE_METHOD_SAVE_COPY,
    FILE_SAVE_METHOD_EXPORT,
    // Fallback for special cases (e.g., when saving a document for the first time or after saving
    // it in a lossy format)
    FILE_SAVE_METHOD_INKSCAPE_SVG,
    // For saving temporary files; we return the same data as for FILE_SAVE_METHOD_SAVE_AS
    FILE_SAVE_METHOD_TEMPORARY,
};

SPDocument *open(Extension *key, gchar const *filename);
void save(Extension *key, SPDocument *doc, gchar const *filename,
          bool setextension, bool check_overwrite, bool official,
          Inkscape::Extension::FileSaveMethod save_method);
Print *get_print(gchar const *key);
Extension *build_from_file(gchar const *filename);
Extension *build_from_mem(gchar const *buffer, Implementation::Implementation *in_imp);

/**
 * Determine the desired default file extension depending on the given file save method.
 * The returned string is guaranteed to be non-empty.
 *
 * @param method the file save method of the dialog
 * @return the corresponding default file extension
 */
Glib::ustring get_file_save_extension (FileSaveMethod method);

/**
 * Determine the desired default save path depending on the given FileSaveMethod.
 * The returned string is guaranteed to be non-empty.
 *
 * @param method the file save method of the dialog
 * @param doc the file's document
 * @return the corresponding default save path
 */
Glib::ustring get_file_save_path (SPDocument *doc, FileSaveMethod method);

/**
 * Write the given file extension back to prefs so that it can be used later on.
 *
 * @param extension the file extension which should be written to prefs
 * @param method the file save mathod of the dialog
 */
void store_file_extension_in_prefs (Glib::ustring extension, FileSaveMethod method);

/**
 * Write the given path back to prefs so that it can be used later on.
 *
 * @param path the path which should be written to prefs
 * @param method the file save mathod of the dialog
 */
void store_save_path_in_prefs (Glib::ustring path, FileSaveMethod method);

} } /* namespace Inkscape::Extension */

#endif /* INKSCAPE_EXTENSION_SYSTEM_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
