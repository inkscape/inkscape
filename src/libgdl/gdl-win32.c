/*
 * Windows stuff
 *
 * Author:
 *   Albin Sunnanbo
 *   Based on code by Lauris Kaplinski <lauris@kaplinski.com> (/src/extension/internal/win32.cpp)
 *
 * This code is in public domain
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "gdl-win32.h"

/* Platform detection */
gboolean
is_os_vista()
{
	static gboolean initialized = FALSE;
	static gboolean is_vista = FALSE;
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
			if (osver.dwMajorVersion == WIN32_MAJORVERSION_VISTA)
				is_vista = TRUE;
		}
	}

	return is_vista;
}
