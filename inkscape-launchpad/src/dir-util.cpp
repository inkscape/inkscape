/**
 * @file
 * Utility functions for filenames.
 */

#include <errno.h>
#include <string>
#include <cstring>
#include <glib.h>
#include "dir-util.h"

std::string sp_relative_path_from_path( std::string const &path, std::string const &base)
{
    std::string result;
    if ( !base.empty() && !path.empty() ) {
        size_t base_len = base.length();
        while (base_len != 0
               && (base[base_len - 1] == G_DIR_SEPARATOR))
        {
            --base_len;
        }

        if ( (path.substr(0, base_len) == base.substr(0, base_len))
             && (path[base_len] == G_DIR_SEPARATOR))
        {
            size_t retPos = base_len + 1;
            while ( (retPos < path.length()) && (path[retPos] == G_DIR_SEPARATOR) ) {
                retPos++;
            }
            if ( (retPos + 1) < path.length() ) {
                result = path.substr(retPos);
            }
        }

    }
    if ( result.empty() ) {
        result = path;
    }
    return result;
}

char const *sp_extension_from_path(char const *const path)
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

char *inkscape_rel2abs(const char *path, const char *base, char *result, const size_t size)
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

char *inkscape_abs2rel(const char *path, const char *base, char *result, const size_t size)
{
    const char *pp, *bp, *branch;
    // endp points the last position which is safe in the result buffer.
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
    if (((*pp == 0) || ((*pp == G_DIR_SEPARATOR) && (*(pp + 1) == 0))) &&
        ((*bp == 0) || ((*bp == G_DIR_SEPARATOR) && (*(bp + 1) == 0))))
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
    if (((*pp == 0) && (*bp == G_DIR_SEPARATOR)) || ((*pp == G_DIR_SEPARATOR) && (*bp == 0)))
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

char *prepend_current_dir_if_relative(gchar const *uri)
{
	if (!uri) {
		return NULL;
	}

	gchar *full_path = (gchar *) g_malloc (1001);
	gchar *cwd = g_get_current_dir();

	gsize bytesRead = 0;
	gsize bytesWritten = 0;
	GError* error = NULL;
	gchar* cwd_utf8 = g_filename_to_utf8 ( cwd,
                                                  -1,
                                                  &bytesRead,
                                                  &bytesWritten,
                                                  &error);

	inkscape_rel2abs (uri, cwd_utf8, full_path, 1000);
	gchar *ret = g_strdup (full_path);
	g_free (full_path);
	g_free (cwd);
	return ret;
}

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vi: set autoindent shiftwidth=4 tabstop=8 filetype=cpp expandtab softtabstop=4 encoding=utf-8 textwidth=99 :
