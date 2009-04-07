#ifndef SEEN_DIR_UTIL_H
#define SEEN_DIR_UTIL_H

/*
 * path-util.h
 *
 * here are functions sp_relative_path & cousins
 * maybe they are already implemented in standard libs
 *
 */

#include <stdlib.h>
#include <glib/gtypes.h>

char const *sp_relative_path_from_path(char const *path, char const *base);
char const *sp_extension_from_path(char const *path);
char *inkscape_rel2abs(char const *path, char const *base, char *result, size_t const size);
char *inkscape_abs2rel(char const *path, char const *base, char *result, size_t const size);
gchar *prepend_current_dir_if_relative(gchar const *filename);


#endif /* !SEEN_DIR_UTIL_H */

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
