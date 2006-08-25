
#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "filedialog.h"

//#include "extension/internal/win32.h"

#include <windows.h>

#include <glib.h>

#include <extension/extension.h>
#include <extension/db.h>

#define UNSAFE_SCRATCH_BUFFER_SIZE 4096

namespace Inkscape
{
namespace UI
{
namespace Dialogs
{

/*#################################
# U T I L I T Y
#################################*/
static gboolean
win32_is_os_wide()
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

/*#################################
# F I L E    O P E N
#################################*/

struct FileOpenNativeData_def {
    char           *dir;
    FileDialogType fileTypes;
    char           *title;
};

FileOpenDialog::FileOpenDialog(
    const char *dir, FileDialogType fileTypes, const char *title) {

    nativeData = (FileOpenNativeData *)
            g_malloc(sizeof (FileOpenNativeData));
    if ( !nativeData ) {
        // do we want exceptions?
        return;
        }

    if ( !dir )
        dir = "";
    nativeData->dir       = g_strdup(dir);
    nativeData->fileTypes = fileTypes;
    nativeData->title     = g_strdup(title);

	extension = NULL;
	filename = NULL;
}



FileOpenDialog::~FileOpenDialog() {

    //do any cleanup here
    if ( nativeData ) {
        g_free(nativeData->dir);
        g_free(nativeData->title);
        g_free(nativeData);
    }

    if (filename) g_free(filename);
    extension = NULL;
}



bool
FileOpenDialog::show() {

    if ( !nativeData ) {
        //error
        return FALSE;
    }

    gint  retval  = FALSE;


    //Jon's UNICODE patch
    if ( win32_is_os_wide() ) {
        gunichar2 fnbufW[UNSAFE_SCRATCH_BUFFER_SIZE * sizeof(gunichar2)] = {0};
        gunichar2* dirW    =
            g_utf8_to_utf16( nativeData->dir,    -1, NULL, NULL, NULL );
        gunichar2 *filterW = (gunichar2 *) L"";
		if ( nativeData->fileTypes == SVG_TYPES )
		   filterW = (gunichar2 *) L"SVG files\0*.svg;*.svgz\0All files\0*\0";
		else if ( nativeData->fileTypes == IMPORT_TYPES )
		   filterW = (gunichar2 *) L"Image files\0*.svg;*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.xpm\0"
		            L"SVG files\0*.svg\0"
		            L"All files\0*\0";
        gunichar2* titleW  =
            g_utf8_to_utf16( nativeData->title,  -1, NULL, NULL, NULL );
        OPENFILENAMEW ofn = {
            sizeof (OPENFILENAMEW),
            NULL,                   // hwndOwner
            NULL,                   // hInstance
            (const WCHAR *)filterW, // lpstrFilter
            NULL,                   // lpstrCustomFilter
            0,                      // nMaxCustFilter
            1,                      // nFilterIndex
            (WCHAR *)fnbufW,        // lpstrFile
            sizeof (fnbufW) / sizeof(WCHAR), // nMaxFile
            NULL,                   // lpstrFileTitle
            0,                      // nMaxFileTitle
            (const WCHAR *)dirW,    // lpstrInitialDir
            (const WCHAR *)titleW,  // lpstrTitle
            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR, // Flags
            0,                      // nFileOffset
            0,                      // nFileExtension
            NULL,                   // lpstrDefExt
            0,                      // lCustData
            NULL,                   // lpfnHook
            NULL                    // lpTemplateName
        };

        retval = GetOpenFileNameW (&ofn);
        if (retval)
            filename = g_utf16_to_utf8( fnbufW, -1, NULL, NULL, NULL );

        g_free( dirW );
        g_free( titleW );

    } else {
        gchar *dir    = nativeData->dir;
        gchar *title  = nativeData->title;
        gchar fnbuf[UNSAFE_SCRATCH_BUFFER_SIZE] = {0};

        gchar *filter = "";
		if ( nativeData->fileTypes == SVG_TYPES )
		   filter = "SVG files\0*.svg;*.svgz\0All files\0*\0";
		else if ( nativeData->fileTypes == IMPORT_TYPES )
		   filter = "Image files\0*.svg;*.png;*.jpg;*.jpeg;*.bmp;*.gif;*.tiff;*.xpm\0"
		            "SVG files\0*.svg\0"
		            "All files\0*\0";

        OPENFILENAMEA ofn = {
            sizeof (OPENFILENAMEA),
            NULL,                  // hwndOwner
            NULL,                  // hInstance
            (const CHAR *)filter,  // lpstrFilter
            NULL,                  // lpstrCustomFilter
            0,                     // nMaxCustFilter
            1,                     // nFilterIndex
            fnbuf,                 // lpstrFile
            sizeof (fnbuf),        // nMaxFile
            NULL,                  // lpstrFileTitle
            0,                     // nMaxFileTitle
            (const CHAR *)dir,     // lpstrInitialDir
            (const CHAR *)title,   // lpstrTitle
            OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR, // Flags
            0,                     // nFileOffset
            0,                     // nFileExtension
            NULL,                  // lpstrDefExt
            0,                     // lCustData
            NULL,                  // lpfnHook
            NULL                   // lpTemplateName
        };

        retval = GetOpenFileNameA (&ofn);
        if ( retval ) {
            filename = g_strdup( fnbuf );
            /* ### We need to try something like this instead:
            GError *err = NULL;
            filename = g_filename_to_utf8(fnbuf, -1, NULL, NULL, &err);
            if ( !filename && err ) {
                g_warning("Charset conversion in show()[%d]%s\n",
                           err->code, err->message);
            }
            */
        }
    }

    if ( !retval ) {
        //int errcode = CommDlgExtendedError();
        return FALSE;
    }

    return TRUE;

}



/*#################################
# F I L E    S A V E
#################################*/

struct FileSaveNativeData_def {
    OPENFILENAME ofn;
    gchar filter[UNSAFE_SCRATCH_BUFFER_SIZE];
    gchar fnbuf[4096];
};



FileSaveDialog::FileSaveDialog(
   const char *dir, FileDialogType fileTypes, const char *title, const char * default_key) {

    nativeData = (FileSaveNativeData *)
            g_malloc(sizeof (FileSaveNativeData));
    if ( !nativeData ) {
        //do we want exceptions?
        return;
        }

    extension = NULL;
    filename = NULL;

    int default_item = 0;

    GSList* extension_list = Inkscape::Extension::db.get_output_list();
    g_assert (extension_list != NULL);

    /* Make up the filter string for the save dialogue using the list
    ** of available output types.
    */
    
    gchar *p = nativeData->filter;
    int N = UNSAFE_SCRATCH_BUFFER_SIZE;

    int n = 1;
    for (GSList* i = g_slist_next (extension_list); i != NULL; i = g_slist_next(i)) {

      Inkscape::Extension::DB::IOExtensionDescription* d =
	reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription*>(i->data);

	  if (!d->sensitive)
		  continue;

      int w = snprintf (p, N, "%s", d->name);
      N -= w + 1;
      p += w + 1;

      w = snprintf (p, N, "*");
      N -= w + 1;
      p += w + 1;

      g_assert (N >= 0);

      /* Look to see if this extension is the default */
      if (default_key &&
	  d->extension->get_id() &&
	  strcmp (default_key, d->extension->get_id()) == 0) {
	default_item = n;
	extension = d->extension;
      }
      
      n++;
    }

    *p = '\0';
    
    nativeData->fnbuf[0] = '\0';
    
    if (dir) {
      /* We must check that dir is not something like
      ** c:\foo\ (ie with a trailing \).  If it is,
      ** GetSaveFileName will give an error.
      */
      int n = strlen(dir);
      if (n > 0 && dir[n - 1] != '\\') {
	strncpy(nativeData->fnbuf, dir, sizeof(nativeData->fnbuf));
      }
    }

    OPENFILENAME ofn = {
        sizeof (OPENFILENAME),
        NULL,                       // hwndOwner
        NULL,                       // hInstance
        nativeData->filter,         // lpstrFilter
        NULL,                       // lpstrCustomFilter
        0,                          // nMaxCustFilter
        default_item,               // nFilterIndex
        nativeData->fnbuf,          // lpstrFile
        sizeof (nativeData->fnbuf), // nMaxFile
        NULL,                       // lpstrFileTitle
        0,                          // nMaxFileTitle
        (const CHAR *)dir,          // lpstrInitialDir
        (const CHAR *)title,        // lpstrTitle
        OFN_HIDEREADONLY | OFN_NOCHANGEDIR,           // Flags
        0,                          // nFileOffset
        0,                          // nFileExtension
        NULL,                       // lpstrDefExt
        0,                          // lCustData
        NULL,                       // lpfnHook
        NULL                        // lpTemplateName
        };
    
    nativeData->ofn = ofn;
}

FileSaveDialog::~FileSaveDialog() {

  //do any cleanup here
  g_free(nativeData);
  if (filename) g_free(filename);
  extension = NULL;
}

bool
FileSaveDialog::show() {

    if (!nativeData)
        return FALSE;
    int retval = GetSaveFileName (&(nativeData->ofn));
    if (!retval) {
        //int errcode = CommDlgExtendedError();
        return FALSE;
        }

    GSList* extension_list = Inkscape::Extension::db.get_output_list();
    g_assert (extension_list != NULL);

    /* Work out which extension corresponds to the user's choice of
    ** file type.
    */
    int n = nativeData->ofn.nFilterIndex - 1;
    GSList* i = g_slist_next (extension_list);

    while (n > 0 && i) {
      n--;
      i = g_slist_next(i);
    }

    Inkscape::Extension::DB::IOExtensionDescription* d =
      reinterpret_cast<Inkscape::Extension::DB::IOExtensionDescription*>(i->data);
    
    extension = d->extension;

    filename = g_strdup (nativeData->fnbuf);
	return TRUE;
}








} //namespace Dialogs
} //namespace UI
} //namespace Inkscape




