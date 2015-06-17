#ifndef SEEN_DIR_UTIL_H
#define SEEN_DIR_UTIL_H

/*
 * path-util.h
 *
 * here are functions sp_relative_path & cousins
 * maybe they are already implemented in standard libs
 *
 */

#include <cstdlib>
#include <string>

/**
 * Returns a form of \a path relative to \a base if that is easy to construct (eg if \a path
 * appears to be in the directory specified by \a base), otherwise returns \a path.
 *
 * @param path is expected to be an absolute path.
 * @param base is expected to be either empty or the absolute path of a directory.
 *
 * @return a relative version of the path, if reasonable.
 *
 * @see inkscape_abs2rel for a more sophisticated version.
 * @see prepend_current_dir_if_relative.
*/
std::string sp_relative_path_from_path(std::string const &path, std::string const &base);

char const *sp_extension_from_path(char const *path);

/**
 * Convert a relative path name into absolute. If path is already absolute, does nothing except copying path to result.
 *
 * @param path relative path.
 * @param base base directory (must be absolute path).
 * @param result result buffer.
 * @param size size of result buffer.
 *
 * @return != NULL: absolute path
 *         == NULL: error
 *
 * based on functions by Shigio Yamaguchi.
 * FIXME:TODO: force it to also do path normalization of the entire resulting path,
 * i.e. get rid of any .. and . in any place, even if 'path' is already absolute
 * (now it returns it unchanged in this case)
 *
 */
char *inkscape_rel2abs(char const *path, char const *base, char *result, size_t const size);

char *inkscape_abs2rel(char const *path, char const *base, char *result, size_t const size);

char *prepend_current_dir_if_relative(char const *filename);


#endif // !SEEN_DIR_UTIL_H

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
