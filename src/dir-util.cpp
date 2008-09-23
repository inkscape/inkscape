/** @file
 * @brief Utility functions for filenames
 */

#define DIR_UTIL_C

#include <errno.h>
#include <string>
#include <cstring>
#include <glib/gutils.h>
#include <glib/gmem.h>
#include <glib/gerror.h>
#include <glib/gconvert.h>
#include <glib/gstrfuncs.h>

/** Returns a form of \a path relative to \a base if that is easy to construct (e.g. if \a path
    appears to be in the directory specified by \a base), otherwise returns \a path.

    N.B. The return value is a pointer into the \a path string.

    \a base is expected to be either NULL or the absolute path of a directory.

    \a path is expected to be an absolute path.

    \see inkscape_abs2rel for a more sophisticated version.
    \see prepend_current_dir_if_relative.
*/
char const *
sp_relative_path_from_path(char const *const path, char const *const base)
{
	if (base == NULL || path == NULL) {
		return path;
	}

	size_t base_len = strlen(base);
	while (base_len != 0
	       && (base[base_len - 1] == G_DIR_SEPARATOR))
	{
		--base_len;
	}

	if ((memcmp(path, base, base_len) == 0)
	    && (path[base_len] == G_DIR_SEPARATOR))
	{
		char const *ret = path + base_len + 1;
		while (*ret == G_DIR_SEPARATOR) {
			++ret;
		}
		if (*ret != '\0') {
			return ret;
		}
	}

	return path;
}

char const *
sp_extension_from_path(char const *const path)
{
	if (path == NULL) {
		return NULL;
	}

	char const *p = path;
	while (*p != '\0') p++;

	while ((p >= path) && (*p != G_DIR_SEPARATOR) && (*p != '.')) p--;
	if (* p != '.') return NULL;
	p++;

	return p;
}


/* current == "./", parent == "../" */
static char const dots[] = {'.', '.', G_DIR_SEPARATOR, '\0'};
static char const *const parent = dots;
static char const *const current = dots + 1;

/**
 * \brief   Convert a relative path name into absolute. If path is already absolute, does nothing except copying path to result.
 *
 *	\param path	relative path
 *	\param base	base directory (must be absolute path)
 *	\param result	result buffer
 *	\param size	size of result buffer
 *	\return		!= NULL: absolute path
 *			== NULL: error

\comment
 based on functions by Shigio Yamaguchi.
 FIXME:TODO: force it to also do path normalization of the entire resulting path,
 i.e. get rid of any .. and . in any place, even if 'path' is already absolute
 (now it returns it unchanged in this case)

 */
char *
inkscape_rel2abs (const char *path, const char *base, char *result, const size_t size)
{
  const char *pp, *bp;
  /* endp points the last position which is safe in the result buffer. */
  const char *endp = result + size - 1;
  char *rp;
  int length;
  if (*path == G_DIR_SEPARATOR)
    {
      if (strlen (path) >= size)
	goto erange;
	strcpy (result, path);
	goto finish;
    }
  else if (*base != G_DIR_SEPARATOR || !size)
    {
      errno = EINVAL;
      return (NULL);
    }
  else if (size == 1)
    goto erange;
  if (!strcmp (path, ".") || !strcmp (path, current))
    {
      if (strlen (base) >= size)
	goto erange;
      strcpy (result, base);
      /* rp points the last char. */
      rp = result + strlen (base) - 1;
      if (*rp == G_DIR_SEPARATOR)
	*rp = 0;
      else
	rp++;
      /* rp point NULL char */
      if (*++path == G_DIR_SEPARATOR)
	{
	  /* Append G_DIR_SEPARATOR to the tail of path name. */
	  *rp++ = G_DIR_SEPARATOR;
	  if (rp > endp)
	    goto erange;
	  *rp = 0;
	}
      goto finish;
    }
  bp = base + strlen (base);
  if (*(bp - 1) == G_DIR_SEPARATOR)
    --bp;
  /* up to root. */
  for (pp = path; *pp && *pp == '.';)
    {
      if (!strncmp (pp, parent, 3))
	{
	  pp += 3;
	  while (bp > base && *--bp != G_DIR_SEPARATOR)
	    ;
	}
      else if (!strncmp (pp, current, 2))
	{
	  pp += 2;
	}
      else if (!strncmp (pp, "..\0", 3))
	{
	  pp += 2;
	  while (bp > base && *--bp != G_DIR_SEPARATOR)
	    ;
	}
      else
	break;
    }
  /* down to leaf. */
  length = bp - base;
  if (length >= static_cast<int>(size))
    goto erange;
  strncpy (result, base, length);
  rp = result + length;
  if (*pp || *(pp - 1) == G_DIR_SEPARATOR || length == 0)
    *rp++ = G_DIR_SEPARATOR;
  if (rp + strlen (pp) > endp)
    goto erange;
  strcpy (rp, pp);
finish:
  return result;
erange:
  errno = ERANGE;
  return (NULL);
}

char *
inkscape_abs2rel (const char *path, const char *base, char *result, const size_t size)
{
  const char *pp, *bp, *branch;
  /* endp points the last position which is safe in the result buffer. */
  const char *endp = result + size - 1;
  char *rp;

  if (*path != G_DIR_SEPARATOR)
    {
      if (strlen (path) >= size)
	goto erange;
      strcpy (result, path);
      goto finish;
    }
  else if (*base != G_DIR_SEPARATOR || !size)
    {
      errno = EINVAL;
      return (NULL);
    }
  else if (size == 1)
    goto erange;
  /* seek to branched point. */
  branch = path;
  for (pp = path, bp = base; *pp && *bp && *pp == *bp; pp++, bp++)
    if (*pp == G_DIR_SEPARATOR)
      branch = pp;
  if ((*pp == 0 || *pp == G_DIR_SEPARATOR && *(pp + 1) == 0) &&
      (*bp == 0 || *bp == G_DIR_SEPARATOR && *(bp + 1) == 0))
    {
      rp = result;
      *rp++ = '.';
      if (*pp == G_DIR_SEPARATOR || *(pp - 1) == G_DIR_SEPARATOR)
	*rp++ = G_DIR_SEPARATOR;
      if (rp > endp)
	goto erange;
      *rp = 0;
      goto finish;
    }
  if (*pp == 0 && *bp == G_DIR_SEPARATOR || *pp == G_DIR_SEPARATOR && *bp == 0)
    branch = pp;
  /* up to root. */
  rp = result;
  for (bp = base + (branch - path); *bp; bp++)
    if (*bp == G_DIR_SEPARATOR && *(bp + 1) != 0)
      {
	if (rp + 3 > endp)
	  goto erange;
	*rp++ = '.';
	*rp++ = '.';
	*rp++ = G_DIR_SEPARATOR;
      }
  if (rp > endp)
    goto erange;
  *rp = 0;
  /* down to leaf. */
  if (*branch)
    {
      if (rp + strlen (branch + 1) > endp)
	goto erange;
      strcpy (rp, branch + 1);
    }
  else
    *--rp = 0;
finish:
  return result;
erange:
  errno = ERANGE;
  return (NULL);
}

void
prepend_current_dir_if_relative (char **result, const gchar *uri)
{
	if (!uri) {
		*(result) = NULL;
		return;
	}

	char *full_path = (char *) g_malloc (1001);
	char *cwd = g_get_current_dir();

	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* cwd_utf8 = g_filename_to_utf8 ( cwd,
                                                  -1,
                                                  &bytesRead,
                                                  &bytesWritten,
                                                  &error);

	inkscape_rel2abs (uri, cwd_utf8, full_path, 1000);
	*(result) = g_strdup (full_path);
	g_free (full_path);
	g_free (cwd);
}


