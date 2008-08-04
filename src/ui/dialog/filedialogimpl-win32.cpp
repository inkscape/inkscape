/**
 * Implementation of the file dialog interfaces defined in filedialog.h for Win32
 *
 * Authors:
 *   Joel Holdsworth
 *   The Inkscape Organization
 *
 * Copyright (C) 2004-2008 The Inkscape Organization
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef WIN32

//General includes
#include <list>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <set>
#include <gdk/gdkwin32.h>
#include <glib/gstdio.h>
#include <glibmm/i18n.h>
#include <gtkmm/window.h>

//Inkscape includes
#include "inkscape.h"
#include "prefs-utils.h"
#include <dialogs/dialog-events.h>
#include <extension/input.h>
#include <extension/output.h>
#include <extension/db.h>

#include <libnr/nr-pixops.h>
#include <libnr/nr-translate-scale-ops.h>
#include <display/nr-arena-item.h>
#include <display/nr-arena.h>
#include "sp-item.h"
#include "display/canvas-arena.h"

#include "filedialog.h"
#include "filedialogimpl-win32.h"

#include <zlib.h>
#include <cairomm/win32_surface.h>
#include <cairomm/context.h>

using namespace std;
using namespace Glib;
using namespace Cairo;
using namespace Gdk::Cairo;

namespace Inkscape
{
namespace UI
{
namespace Dialog
{

const int PreviewWidening = 150;
const char PreviewWindowClassName[] = "PreviewWnd";
const unsigned long MaxPreviewFileSize = 1344; // kB

#define IDC_SHOW_PREVIEW    1000

// Windows 2000 version of OPENFILENAMEW
struct OPENFILENAMEEXW : public OPENFILENAMEW {
  void *        pvReserved;
  DWORD         dwReserved;
  DWORD         FlagsEx;
};

struct Filter
{
    gunichar2* name;
    glong name_length;
    gunichar2* filter;
    glong filter_length;
    Inkscape::Extension::Extension* mod;
};

ustring utf16_to_ustring(const wchar_t *utf16string, int utf16length = -1)
{
    gchar *utf8string = g_utf16_to_utf8((const gunichar2*)utf16string,
        utf16length, NULL, NULL, NULL);
    ustring result(utf8string);
    g_free(utf8string);

    return result;
}

/*#########################################################################
### F I L E     D I A L O G    B A S E    C L A S S
#########################################################################*/

FileDialogBaseWin32::FileDialogBaseWin32(Gtk::Window &parent,
        const Glib::ustring &dir, const gchar *title,
        FileDialogType type, gchar const* /*preferenceBase*/) :
        dialogType(type),
        parent(parent),
        _current_directory(dir)
{
    _main_loop = NULL;

	_filter_index = 1;
	_filter_count = 0;
	
    _title = (wchar_t*)g_utf8_to_utf16(title, -1, NULL, NULL, NULL);
	g_assert(_title != NULL);

    Glib::RefPtr<const Gdk::Window> parentWindow = parent.get_window();
    g_assert(parentWindow->gobj() != NULL);
    _ownerHwnd = (HWND)gdk_win32_drawable_get_handle((GdkDrawable*)parentWindow->gobj());
}

FileDialogBaseWin32::~FileDialogBaseWin32()
{
    g_free(_title);
}

Inkscape::Extension::Extension *FileDialogBaseWin32::getSelectionType()
{
    return _extension;
}

Glib::ustring FileDialogBaseWin32::getCurrentDirectory()
{
    return _current_directory;
}

/*#########################################################################
### F I L E    O P E N
#########################################################################*/

bool FileOpenDialogImplWin32::_show_preview = true;

/**
 * Constructor.  Not called directly.  Use the factory.
 */
FileOpenDialogImplWin32::FileOpenDialogImplWin32(Gtk::Window &parent,
                                       const Glib::ustring &dir,
                                       FileDialogType fileTypes,
                                       const gchar *title) :
    FileDialogBaseWin32(parent, dir, title, fileTypes, "dialogs.open")
{
    // Initalize to Autodetect
    _extension = NULL;

    // Set our dialog type (open, import, etc...)
    dialogType = fileTypes;

    _show_preview_button_bitmap = NULL;
    _preview_wnd = NULL;
    _file_dialog_wnd = NULL;
    _base_window_proc = NULL;

    _preview_file_size = 0;
    _preview_bitmap = NULL;
    _preview_file_icon = NULL;
    _preview_document_width = 0;
    _preview_document_height = 0;
    _preview_image_width = 0;
    _preview_image_height = 0;
    _preview_emf_image = false;
	
	_mutex = NULL;

    createFilterMenu();
}


/**
 * Destructor
 */
FileOpenDialogImplWin32::~FileOpenDialogImplWin32()
{
    if(_filter != NULL)
        delete[] _filter;
    if(_extension_map != NULL)
        delete[] _extension_map;
}

void FileOpenDialogImplWin32::createFilterMenu()
{
    list<Filter> filter_list;

    // Compose the filter string
    Inkscape::Extension::DB::InputList extension_list;
    Inkscape::Extension::db.get_input_list(extension_list);

    ustring all_inkscape_files_filter, all_image_files_filter;
    Filter all_files, all_inkscape_files, all_image_files;

    const gchar *all_files_filter_name = N_("All Files");
    const gchar *all_inkscape_files_filter_name = N_("All Inkscape Files");
    const gchar *all_image_files_filter_name = N_("All Image Files");

    // Calculate the amount of memory required
    int filter_count = 3;       // 3 - one for All Files, All Images and All Inkscape Files
    int filter_length = 1;

    for (Inkscape::Extension::DB::InputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); current_item++)
    {
        Filter filter;

        Inkscape::Extension::Input *imod = *current_item;
        if (imod->deactivated()) continue;

        // Type
        filter.name = g_utf8_to_utf16(_(imod->get_filetypename()),
            -1, NULL, &filter.name_length, NULL);

        // Extension
        const gchar *file_extension_name = imod->get_extension();
        filter.filter = g_utf8_to_utf16(file_extension_name,
            -1, NULL, &filter.filter_length, NULL);

        filter.mod = imod;
        filter_list.push_back(filter);

        filter_length += filter.name_length +
            filter.filter_length + 3;   // Add 3 for two \0s and a *

        // Add to the "All Inkscape Files" Entry
        if(all_inkscape_files_filter.length() > 0)
            all_inkscape_files_filter += ";*";
        all_inkscape_files_filter += file_extension_name;
        if( strncmp("image", imod->get_mimetype(), 5) == 0)
        {
            // Add to the "All Image Files" Entry
            if(all_image_files_filter.length() > 0)
                all_image_files_filter += ";*";
            all_image_files_filter += file_extension_name;
        }

        filter_count++;
    }

    int extension_index = 0;
    _extension_map = new Inkscape::Extension::Extension*[filter_count];

    // Filter Image Files
    all_image_files.name = g_utf8_to_utf16(all_image_files_filter_name,
        -1, NULL, &all_image_files.name_length, NULL);
    all_image_files.filter = g_utf8_to_utf16(all_image_files_filter.data(),
            -1, NULL, &all_image_files.filter_length, NULL);
	all_image_files.mod = NULL;
    filter_list.push_front(all_image_files);

    // Filter Inkscape Files
    all_inkscape_files.name = g_utf8_to_utf16(all_inkscape_files_filter_name,
        -1, NULL, &all_inkscape_files.name_length, NULL);
    all_inkscape_files.filter = g_utf8_to_utf16(all_inkscape_files_filter.data(),
            -1, NULL, &all_inkscape_files.filter_length, NULL);
	all_inkscape_files.mod = NULL;
    filter_list.push_front(all_inkscape_files);

    // Filter All Files
    all_files.name = g_utf8_to_utf16(all_files_filter_name,
        -1, NULL, &all_files.name_length, NULL);
    all_files.filter = NULL;
    all_files.filter_length = 0;
	all_files.mod = NULL;
    filter_list.push_front(all_files);

    filter_length += all_files.name_length + 3 +
                    all_inkscape_files.filter_length +
                    all_inkscape_files.name_length + 3 +
                    all_image_files.filter_length +
                    all_image_files.name_length + 3 + 1;
     // Add 3 for 2*2 \0s and a *, and 1 for a trailing \0
	 
	_filter = new wchar_t[filter_length];
    wchar_t *filterptr = _filter;
	
    for(list<Filter>::iterator filter_iterator = filter_list.begin();
        filter_iterator != filter_list.end(); filter_iterator++)
    {
        const Filter &filter = *filter_iterator;

        wcsncpy(filterptr, (wchar_t*)filter.name, filter.name_length);
        filterptr += filter.name_length;
        g_free(filter.name);

        *(filterptr++) = L'\0';
        *(filterptr++) = L'*';

        if(filter.filter != NULL)
        {
            wcsncpy(filterptr, (wchar_t*)filter.filter, filter.filter_length);
            filterptr += filter.filter_length;
            g_free(filter.filter);
        }

        *(filterptr++) = L'\0';

        // Associate this input extension with the file type name
        _extension_map[extension_index++] = filter.mod;
    }
    *(filterptr++) = L'\0';
	
	_filter_count = extension_index;
    _filter_index = 2;	// Select the 2nd filter in the list - 2 is NOT the 3rd
}

void FileOpenDialogImplWin32::GetOpenFileName_thread()
{
    OPENFILENAMEEXW ofn;

    g_assert(this != NULL);
	g_assert(_mutex != NULL);
    
    WCHAR* current_directory_string = (WCHAR*)g_utf8_to_utf16(
        _current_directory.data(), _current_directory.length(),
		NULL, NULL, NULL);

    memset(&ofn, 0, sizeof(ofn));

    // Copy the selected file name, converting from UTF-8 to UTF-16
    memset(_path_string, 0, sizeof(_path_string));
    gunichar2* utf16_path_string = g_utf8_to_utf16(
        myFilename.data(), -1, NULL, NULL, NULL);
    wcsncpy(_path_string, (wchar_t*)utf16_path_string, _MAX_PATH);
    g_free(utf16_path_string);

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = _ownerHwnd;
    ofn.lpstrFile = _path_string;
    ofn.nMaxFile = _MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = current_directory_string;
    ofn.lpstrTitle = _title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER | OFN_ENABLEHOOK | OFN_HIDEREADONLY | OFN_ENABLESIZING;
    ofn.lpstrFilter = _filter;
    ofn.nFilterIndex = _filter_index;
    ofn.lpfnHook = GetOpenFileName_hookproc;
    ofn.lCustData = (LPARAM)this;

    _result = GetOpenFileNameW(&ofn) != 0;

	g_assert(ofn.nFilterIndex >= 1 && ofn.nFilterIndex <= _filter_count);
    _filter_index = ofn.nFilterIndex;
    _extension = _extension_map[ofn.nFilterIndex - 1];

    // Copy the selected file name, converting from UTF-16 to UTF-8
    myFilename = utf16_to_ustring(_path_string, _MAX_PATH);

    // Tidy up
    g_free(current_directory_string);

    _mutex->lock();
    _finished = true;
    _mutex->unlock();
}

void FileOpenDialogImplWin32::register_preview_wnd_class()
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    const WNDCLASSA PreviewWndClass =
    {
        CS_HREDRAW | CS_VREDRAW,
        preview_wnd_proc,
        0,
        0,
        hInstance,
        NULL,
        LoadCursor(hInstance, IDC_ARROW),
        (HBRUSH)(COLOR_BTNFACE + 1),
        NULL,
        PreviewWindowClassName
    };

    RegisterClassA(&PreviewWndClass);
}

UINT_PTR CALLBACK FileOpenDialogImplWin32::GetOpenFileName_hookproc(
    HWND hdlg, UINT uiMsg, WPARAM, LPARAM lParam)
{
    FileOpenDialogImplWin32 *pImpl = (FileOpenDialogImplWin32*)
        GetWindowLongPtr(hdlg, GWLP_USERDATA);

    switch(uiMsg)
    {
    case WM_INITDIALOG:
        {
            HWND hParentWnd = GetParent(hdlg);
            HINSTANCE hInstance = GetModuleHandle(NULL);

            // Make the window a bit wider
            RECT rcRect;
            GetWindowRect(hParentWnd, &rcRect);
            MoveWindow(hParentWnd, rcRect.left, rcRect.top,
                rcRect.right - rcRect.left + PreviewWidening,
                rcRect.bottom - rcRect.top,
                FALSE);

            // Set the pointer to the object
            OPENFILENAMEW *ofn = (OPENFILENAMEW*)lParam;
            SetWindowLongPtr(hdlg, GWLP_USERDATA, ofn->lCustData);
            SetWindowLongPtr(hParentWnd, GWLP_USERDATA, ofn->lCustData);
            pImpl = (FileOpenDialogImplWin32*)ofn->lCustData;

            // Subclass the parent
            pImpl->_base_window_proc = (WNDPROC)GetWindowLongPtr(hParentWnd, GWL_WNDPROC);
            SetWindowLongPtr(hParentWnd, GWL_WNDPROC, (LONG_PTR)file_dialog_subclass_proc);

            // Add a button to the toolbar
            pImpl->_toolbar_wnd = FindWindowEx(hParentWnd, NULL, "ToolbarWindow32", NULL);

            pImpl->_show_preview_button_bitmap = LoadBitmap(
                hInstance, MAKEINTRESOURCE(IDC_SHOW_PREVIEW));
            TBADDBITMAP tbAddBitmap = {NULL, (UINT)pImpl->_show_preview_button_bitmap};
            const int iBitmapIndex = SendMessage(pImpl->_toolbar_wnd,
                TB_ADDBITMAP, 1, (LPARAM)&tbAddBitmap);

            TBBUTTON tbButton;
            memset(&tbButton, 0, sizeof(TBBUTTON));
            tbButton.iBitmap = iBitmapIndex;
            tbButton.idCommand = IDC_SHOW_PREVIEW;
            tbButton.fsState = (pImpl->_show_preview ? TBSTATE_CHECKED : 0)
                | TBSTATE_ENABLED;
            tbButton.fsStyle = TBSTYLE_CHECK;
            tbButton.iString = (INT_PTR)_("Show Preview");
            SendMessage(pImpl->_toolbar_wnd, TB_ADDBUTTONS, 1, (LPARAM)&tbButton);

            // Create preview pane
            register_preview_wnd_class();

            pImpl->_mutex->lock();

                pImpl->_file_dialog_wnd = hParentWnd;

                pImpl->_preview_wnd =
                    CreateWindowA(PreviewWindowClassName, "",
                        WS_CHILD | WS_VISIBLE,
                        0, 0, 100, 100, hParentWnd, NULL, hInstance, NULL);
                SetWindowLongPtr(pImpl->_preview_wnd, GWLP_USERDATA, ofn->lCustData);

            pImpl->_mutex->unlock();

            pImpl->layout_dialog();
        }
        break;

    case WM_NOTIFY:
        {

        OFNOTIFY *pOFNotify = reinterpret_cast<OFNOTIFY*>(lParam);
        switch(pOFNotify->hdr.code)
        {
        case CDN_SELCHANGE:
            {
                if(pImpl != NULL)
                {
                    // Get the file name
                    pImpl->_mutex->lock();

                    SendMessage(pOFNotify->hdr.hwndFrom, CDM_GETFILEPATH,
                        sizeof(pImpl->_path_string) / sizeof(wchar_t),
                        (LPARAM)pImpl->_path_string);

                    pImpl->_file_selected = true;

                    pImpl->_mutex->unlock();
                }
            }
            break;
        }
        }
        break;

    case WM_CLOSE:
        pImpl->_mutex->lock();
        pImpl->_preview_file_size = 0;

        pImpl->_file_dialog_wnd = NULL;
        DestroyWindow(pImpl->_preview_wnd);
        pImpl->_preview_wnd = NULL;
        DeleteObject(pImpl->_show_preview_button_bitmap);
        pImpl->_show_preview_button_bitmap = NULL;
        pImpl->_mutex->unlock();

        break;
    }

    // Use default dialog behaviour
    return 0;
}

LRESULT CALLBACK FileOpenDialogImplWin32::file_dialog_subclass_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    FileOpenDialogImplWin32 *pImpl = (FileOpenDialogImplWin32*)
        GetWindowLongPtr(hwnd, GWLP_USERDATA);

    LRESULT lResult = CallWindowProc(pImpl->_base_window_proc, hwnd, uMsg, wParam, lParam);

    switch(uMsg)
    {
    case WM_SHOWWINDOW:
        if(wParam != 0)
            pImpl->layout_dialog();
        break;

    case WM_SIZE:
        pImpl->layout_dialog();
        break;

    case WM_COMMAND:
        if(wParam == IDC_SHOW_PREVIEW)
        {
            const bool enable = SendMessage(pImpl->_toolbar_wnd,
                TB_ISBUTTONCHECKED, IDC_SHOW_PREVIEW, 0) != 0;
            pImpl->enable_preview(enable);
        }
        break;
    }

    return lResult;
}

LRESULT CALLBACK FileOpenDialogImplWin32::preview_wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    const int CaptionPadding = 4;
    const int IconSize = 32;

    FileOpenDialogImplWin32 *pImpl = (FileOpenDialogImplWin32*)
        GetWindowLongPtr(hwnd, GWLP_USERDATA);

    LRESULT lResult = 0;

    switch(uMsg)
    {
    case WM_ERASEBKGND:
        // Do nothing to erase the background
        //  - otherwise there'll be flicker
        lResult = 1;
        break;

    case WM_PAINT:
        {
            // Get the client rect
            RECT rcClient;
            GetClientRect(hwnd, &rcClient);

            // Prepare to paint
            PAINTSTRUCT paint_struct;
            HDC dc = BeginPaint(hwnd, &paint_struct);

            HFONT hCaptionFont = (HFONT)SendMessage(GetParent(hwnd),
                    WM_GETFONT, 0, 0);
            HFONT hOldFont = (HFONT)SelectObject(dc, hCaptionFont);
            SetBkMode(dc, TRANSPARENT);

            pImpl->_mutex->lock();

            if(pImpl->_path_string[0] == 0)
            {
                FillRect(dc, &rcClient, (HBRUSH)(COLOR_3DFACE + 1));
                DrawText(dc, _("No file selected"), -1, &rcClient,
                    DT_CENTER | DT_VCENTER | DT_NOPREFIX);
            }
            else if(pImpl->_preview_bitmap != NULL)
            {
                BITMAP bitmap;
                GetObject(pImpl->_preview_bitmap, sizeof(bitmap), &bitmap);
                const int destX = (rcClient.right - bitmap.bmWidth) / 2;

                // Render the image
                HDC hSrcDC = CreateCompatibleDC(dc);
                HBITMAP hOldBitmap = (HBITMAP)SelectObject(hSrcDC, pImpl->_preview_bitmap);

                BitBlt(dc, destX, 0, bitmap.bmWidth, bitmap.bmHeight,
                    hSrcDC, 0, 0, SRCCOPY);

                SelectObject(hSrcDC, hOldBitmap);
                DeleteDC(hSrcDC);

                // Fill in the background area
                HRGN hEraseRgn = CreateRectRgn(rcClient.left, rcClient.top,
                    rcClient.right, rcClient.bottom);
                HRGN hImageRgn = CreateRectRgn(destX, 0,
                    destX + bitmap.bmWidth, bitmap.bmHeight);
                CombineRgn(hEraseRgn, hEraseRgn, hImageRgn, RGN_DIFF);

                FillRgn(dc, hEraseRgn, GetSysColorBrush(COLOR_3DFACE));

                DeleteObject(hImageRgn);
                DeleteObject(hEraseRgn);

                // Draw the caption on
                RECT rcCaptionRect = {rcClient.left,
                    rcClient.top + bitmap.bmHeight + CaptionPadding,
                    rcClient.right, rcClient.bottom};

                WCHAR szCaption[_MAX_FNAME + 32];
                const int iLength = pImpl->format_caption(
                    szCaption, sizeof(szCaption) / sizeof(WCHAR));

                DrawTextW(dc, szCaption, iLength, &rcCaptionRect,
                    DT_CENTER | DT_TOP | DT_NOPREFIX | DT_PATH_ELLIPSIS);
            }
            else if(pImpl->_preview_file_icon != NULL)
            {
                FillRect(dc, &rcClient, (HBRUSH)(COLOR_3DFACE + 1));

                // Draw the files icon
                const int destX = (rcClient.right - IconSize) / 2;
                DrawIconEx(dc, destX, 0, pImpl->_preview_file_icon,
                    IconSize, IconSize, 0, NULL,
                    DI_NORMAL | DI_COMPAT);

                // Draw the caption on
                RECT rcCaptionRect = {rcClient.left,
                    rcClient.top + IconSize + CaptionPadding,
                    rcClient.right, rcClient.bottom};

                WCHAR szFileName[_MAX_FNAME], szCaption[_MAX_FNAME + 32];
                _wsplitpath(pImpl->_path_string, NULL, NULL, szFileName, NULL);

                const int iLength = snwprintf(szCaption,
                    sizeof(szCaption), L"%s\n%d kB",
                    szFileName, pImpl->_preview_file_size);

                DrawTextW(dc, szCaption, iLength, &rcCaptionRect,
                    DT_CENTER | DT_TOP | DT_NOPREFIX | DT_PATH_ELLIPSIS);
            }
            else
            {
                // Can't show anything!
                FillRect(dc, &rcClient, (HBRUSH)(COLOR_3DFACE + 1));
            }

            pImpl->_mutex->unlock();

            // Finish painting
            SelectObject(dc, hOldFont);
            EndPaint(hwnd, &paint_struct);
        }

        break;

    case WM_DESTROY:
        pImpl->free_preview();
        break;

    default:
        lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
        break;
    }

    return lResult;
}

void FileOpenDialogImplWin32::enable_preview(bool enable)
{
    _show_preview = enable;

    // Relayout the dialog
    ShowWindow(_preview_wnd, enable ? SW_SHOW : SW_HIDE);
    layout_dialog();

    // Load or unload the preview
    if(enable)
    {
        _mutex->lock();
        _file_selected = true;
        _mutex->unlock();
    }
    else free_preview();
}

void FileOpenDialogImplWin32::layout_dialog()
{
    union RECTPOINTS
    {
        RECT r;
        POINT p[2];
    };

    const float MaxExtentScale = 2.0f / 3.0f;

    RECT rcClient;
    GetClientRect(_file_dialog_wnd, &rcClient);

    // Re-layout the dialog
    HWND hFileListWnd = GetDlgItem(_file_dialog_wnd, lst2);
    HWND hFolderComboWnd = GetDlgItem(_file_dialog_wnd, cmb2);


    RECT rcFolderComboRect;
    RECTPOINTS rcFileList;
    GetWindowRect(hFileListWnd, &rcFileList.r);
    GetWindowRect(hFolderComboWnd, &rcFolderComboRect);
    const int iPadding = rcFileList.r.top - rcFolderComboRect.bottom;
    MapWindowPoints(NULL, _file_dialog_wnd, rcFileList.p, 2);

    RECT rcPreview;
    RECT rcBody = {rcFileList.r.left, rcFileList.r.top,
        rcClient.right - iPadding, rcFileList.r.bottom};
    rcFileList.r.right = rcBody.right;

    if(_show_preview)
    {
        rcPreview.top = rcBody.top;
        rcPreview.left = rcClient.right - (rcBody.bottom - rcBody.top);
        const int iMaxExtent = (int)(MaxExtentScale * (float)(rcBody.left + rcBody.right)) + iPadding / 2;
        if(rcPreview.left < iMaxExtent) rcPreview.left = iMaxExtent;
        rcPreview.bottom = rcBody.bottom;
        rcPreview.right = rcBody.right;

        // Re-layout the preview box
        _mutex->lock();

            _preview_width = rcPreview.right - rcPreview.left;
            _preview_height = rcPreview.bottom - rcPreview.top;

        _mutex->unlock();

        render_preview();

        MoveWindow(_preview_wnd, rcPreview.left, rcPreview.top,
            _preview_width, _preview_height, TRUE);

        rcFileList.r.right = rcPreview.left - iPadding;
    }

    // Re-layout the file list box
    MoveWindow(hFileListWnd, rcFileList.r.left, rcFileList.r.top,
        rcFileList.r.right - rcFileList.r.left,
        rcFileList.r.bottom - rcFileList.r.top, TRUE);

    // Re-layout the toolbar
    RECTPOINTS rcToolBar;
    GetWindowRect(_toolbar_wnd, &rcToolBar.r);
    MapWindowPoints(NULL, _file_dialog_wnd, rcToolBar.p, 2);
    MoveWindow(_toolbar_wnd, rcToolBar.r.left, rcToolBar.r.top,
        rcToolBar.r.right - rcToolBar.r.left, rcToolBar.r.bottom - rcToolBar.r.top, TRUE);
}

void FileOpenDialogImplWin32::file_selected()
{
    // Destroy any previous previews
    free_preview();


    // Determine if the file exists
    DWORD attributes = GetFileAttributesW(_path_string);
    if(attributes == 0xFFFFFFFF ||
        attributes == FILE_ATTRIBUTE_DIRECTORY)
    {
        InvalidateRect(_preview_wnd, NULL, FALSE);
        return;
    }

    // Check the file exists and get the file size
    HANDLE file_handle = CreateFileW(_path_string, GENERIC_READ,
        FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if(file_handle == INVALID_HANDLE_VALUE) return;
    const DWORD file_size = GetFileSize(file_handle, NULL);
    if (file_size == INVALID_FILE_SIZE) return;
    _preview_file_size = file_size / 1024;
    CloseHandle(file_handle);

    if(_show_preview) load_preview();
}

void FileOpenDialogImplWin32::load_preview()
{
    // Destroy any previous previews
    free_preview();

    // Try to get the file icon
    SHFILEINFOW fileInfo;
    if(SUCCEEDED(SHGetFileInfoW(_path_string, 0, &fileInfo,
        sizeof(fileInfo), SHGFI_ICON | SHGFI_LARGEICON)))
        _preview_file_icon = fileInfo.hIcon;

    // Will this file be too big?
    if(_preview_file_size > MaxPreviewFileSize)
    {
        InvalidateRect(_preview_wnd, NULL, FALSE);
        return;
    }

    // Prepare to render a preview
    const Glib::ustring svg = ".svg";
    const Glib::ustring svgz = ".svgz";
    const Glib::ustring emf = ".emf";
    const Glib::ustring wmf = ".wmf";
    const Glib::ustring path = utf16_to_ustring(_path_string);

    bool success = false;

    _preview_document_width = _preview_document_height = 0;

    if ((dialogType == SVG_TYPES || dialogType == IMPORT_TYPES) &&
            (hasSuffix(path, svg) || hasSuffix(path, svgz)))
        success = set_svg_preview();
    else if (hasSuffix(path, emf) || hasSuffix(path, wmf))
        success = set_emf_preview();
    else if (isValidImageFile(path))
        success = set_image_preview();
    else {
        // Show no preview
    }

    if(success) render_preview();

    InvalidateRect(_preview_wnd, NULL, FALSE);
}

void FileOpenDialogImplWin32::free_preview()
{
    _mutex->lock();
    if(_preview_bitmap != NULL)
        DeleteObject(_preview_bitmap);
    _preview_bitmap = NULL;

    if(_preview_file_icon != NULL)
        DestroyIcon(_preview_file_icon);
    _preview_file_icon = NULL;

    _preview_bitmap_image.clear();
    _preview_emf_image = false;
    _mutex->unlock();
}

bool FileOpenDialogImplWin32::set_svg_preview()
{
    const int PreviewSize = 512;

    gchar *utf8string = g_utf16_to_utf8((const gunichar2*)_path_string,
        _MAX_PATH, NULL, NULL, NULL);
    SPDocument *svgDoc = sp_document_new (utf8string, true);
    g_free(utf8string);

    // Check the document loaded properly
    if(svgDoc == NULL) return false;
    if(svgDoc->root == NULL)
    {
        sp_document_unref(svgDoc);
        return false;
    }

    // Get the size of the document
    const double svgWidth = sp_document_width(svgDoc);
    const double svgHeight = sp_document_height(svgDoc);

    // Find the minimum scale to fit the image inside the preview area
    const double scaleFactorX =    PreviewSize / svgWidth;
    const double scaleFactorY =    PreviewSize / svgHeight;
    const double scaleFactor = (scaleFactorX > scaleFactorY) ? scaleFactorY : scaleFactorX;

    // Now get the resized values
    const double scaledSvgWidth  = scaleFactor * svgWidth;
    const double scaledSvgHeight = scaleFactor * svgHeight;

    Geom::Rect area(Geom::Point(0, 0), Geom::Point(scaledSvgWidth, scaledSvgHeight));
    NRRectL areaL = {0, 0, scaledSvgWidth, scaledSvgHeight};
    NRRectL bbox = {0, 0, scaledSvgWidth, scaledSvgHeight};

    // write object bbox to area
    NR::Maybe<NR::Rect> maybeArea(from_2geom(area));
    sp_document_ensure_up_to_date (svgDoc);
    sp_item_invoke_bbox((SPItem *) svgDoc->root, &maybeArea,
        from_2geom(sp_item_i2r_affine((SPItem *)(svgDoc->root))), TRUE);

    NRArena *const arena = NRArena::create();

    unsigned const key = sp_item_display_key_new(1);

    NRArenaItem *root = sp_item_invoke_show((SPItem*)(svgDoc->root),
        arena, key, SP_ITEM_SHOW_DISPLAY);

    NRGC gc(NULL);
    gc.transform = from_2geom(Geom::Matrix(Geom::Scale(scaleFactor, scaleFactor)));

    nr_arena_item_invoke_update (root, NULL, &gc,
        NR_ARENA_ITEM_STATE_ALL, NR_ARENA_ITEM_STATE_NONE);

    // Prepare a GDI compatible NRPixBlock
    NRPixBlock pixBlock;
    pixBlock.size = NR_PIXBLOCK_SIZE_BIG;
    pixBlock.mode = NR_PIXBLOCK_MODE_R8G8B8;
    pixBlock.empty = 1;
    pixBlock.visible_area.x0 = pixBlock.area.x0 = 0;
    pixBlock.visible_area.y0 = pixBlock.area.y0 = 0;
    pixBlock.visible_area.x1 = pixBlock.area.x1 = scaledSvgWidth;
    pixBlock.visible_area.y1 = pixBlock.area.y1 = scaledSvgHeight;
    pixBlock.rs = 4 * ((3 * (int)scaledSvgWidth + 3) / 4);
    pixBlock.data.px = g_try_new (unsigned char, pixBlock.rs * scaledSvgHeight);

    // Fail if the pixblock failed to allocate
    if(pixBlock.data.px == NULL)
    {
        sp_document_unref(svgDoc);
        return false;
    }

    memset(pixBlock.data.px, 0xFF, pixBlock.rs * scaledSvgHeight);

    memcpy(&root->bbox, &areaL, sizeof(areaL));

    // Render the image
    nr_arena_item_invoke_render(NULL, root, &bbox, &pixBlock, /*0*/NR_ARENA_ITEM_RENDER_NO_CACHE);

    // Tidy up
    sp_document_unref(svgDoc);
    sp_item_invoke_hide((SPItem*)(svgDoc->root), key);
    nr_arena_item_unref(root);
    nr_object_unref((NRObject *) arena);

    // Create the GDK pixbuf
    _mutex->lock();

    _preview_bitmap_image = Gdk::Pixbuf::create_from_data(
        pixBlock.data.px, Gdk::COLORSPACE_RGB, false, 8,
        (int)scaledSvgWidth, (int)scaledSvgHeight, pixBlock.rs,
        sigc::ptr_fun(destroy_svg_rendering));

    _preview_document_width = scaledSvgWidth;
    _preview_document_height = scaledSvgHeight;
    _preview_image_width = svgWidth;
    _preview_image_height = svgHeight;

    _mutex->unlock();

    return true;
}

void FileOpenDialogImplWin32::destroy_svg_rendering(const guint8 *buffer)
{
    g_assert(buffer != NULL);
    g_free((void*)buffer);
}

bool FileOpenDialogImplWin32::set_image_preview()
{
    const Glib::ustring path = utf16_to_ustring(_path_string, _MAX_PATH);

    bool successful = false;

    _mutex->lock();

    try {
        _preview_bitmap_image = Gdk::Pixbuf::create_from_file(path);
        if (_preview_bitmap_image) {
            _preview_image_width = _preview_bitmap_image->get_width();
            _preview_document_width = _preview_image_width;
            _preview_image_height = _preview_bitmap_image->get_height();
            _preview_document_height = _preview_image_height;
            successful = true;
        }
    }
    catch (const Gdk::PixbufError&) {}
    catch (const Glib::FileError&) {}

    _mutex->unlock();

    return successful;
}

// Aldus Placeable Header ===================================================
// Since we are a 32bit app, we have to be sure this structure compiles to
// be identical to a 16 bit app's version. To do this, we use the #pragma
// to adjust packing, we use a WORD for the hmf handle, and a SMALL_RECT
// for the bbox rectangle.
#pragma pack( push )
#pragma pack( 2 )
typedef struct
{
    DWORD       dwKey;
    WORD        hmf;
    SMALL_RECT  bbox;
    WORD        wInch;
    DWORD       dwReserved;
    WORD        wCheckSum;
} APMHEADER, *PAPMHEADER;
#pragma pack( pop )


static HENHMETAFILE
MyGetEnhMetaFileW( const WCHAR *filename )
{
    // Try open as Enhanced Metafile
    HENHMETAFILE hemf = GetEnhMetaFileW(filename);

    if (!hemf) {
        // Try open as Windows Metafile
        HMETAFILE hmf = GetMetaFileW(filename);

        METAFILEPICT mp;
        HDC hDC;

        if (!hmf) {
            WCHAR szTemp[MAX_PATH];

            DWORD dw = GetShortPathNameW( filename, szTemp, MAX_PATH );
            if (dw) {
                hmf = GetMetaFileW( szTemp );
            }
        }

        if (hmf) {
            // Convert Windows Metafile to Enhanced Metafile
            DWORD nSize = GetMetaFileBitsEx( hmf, 0, NULL );
            
            if (nSize) {
                BYTE *lpvData = new BYTE[nSize];
                if (lpvData) {
                    DWORD dw = GetMetaFileBitsEx( hmf, nSize, lpvData );
                    if (dw) {
                        // Fill out a METAFILEPICT structure
                        mp.mm = MM_ANISOTROPIC;
                        mp.xExt = 1000;
                        mp.yExt = 1000;
                        mp.hMF = NULL;
                        // Get a reference DC
                        hDC = GetDC( NULL );
                        // Make an enhanced metafile from the windows metafile
                        hemf = SetWinMetaFileBits( nSize, lpvData, hDC, &mp );
                        // Clean up
                        ReleaseDC( NULL, hDC );
                        DeleteMetaFile( hmf );
                    }
                    delete[] lpvData;
                }
                else {
                    DeleteMetaFile( hmf );
                }
            }
            else {
                DeleteMetaFile( hmf );
            }
        }
        else {
            // Try open as Aldus Placeable Metafile
            HANDLE hFile;
            hFile = CreateFileW( filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

            if (hFile != INVALID_HANDLE_VALUE) {
                DWORD nSize = GetFileSize( hFile, NULL );
                if (nSize) {
                    BYTE *lpvData = new BYTE[nSize];
                    if (lpvData) {
                        DWORD dw = ReadFile( hFile, lpvData, nSize, &nSize, NULL );
                        if (dw) {
                            if ( ((PAPMHEADER)lpvData)->dwKey == 0x9ac6cdd7l ) {
                                // Fill out a METAFILEPICT structure
                                mp.mm = MM_ANISOTROPIC;
                                mp.xExt = ((PAPMHEADER)lpvData)->bbox.Right - ((PAPMHEADER)lpvData)->bbox.Left;
                                mp.xExt = ( mp.xExt * 2540l ) / (DWORD)(((PAPMHEADER)lpvData)->wInch);
                                mp.yExt = ((PAPMHEADER)lpvData)->bbox.Bottom - ((PAPMHEADER)lpvData)->bbox.Top;
                                mp.yExt = ( mp.yExt * 2540l ) / (DWORD)(((PAPMHEADER)lpvData)->wInch);
                                mp.hMF = NULL;
                                // Get a reference DC
                                hDC = GetDC( NULL );
                                // Create an enhanced metafile from the bits
                                hemf = SetWinMetaFileBits( nSize, lpvData+sizeof(APMHEADER), hDC, &mp );
                                // Clean up
                                ReleaseDC( NULL, hDC );
                            }
                        }
                        delete[] lpvData;
                    }
                }
                CloseHandle( hFile );
            }
        }
    }

    return hemf;
}


bool FileOpenDialogImplWin32::set_emf_preview()
{
    _mutex->lock();

    BOOL ok = FALSE;

    DWORD w = 0;
    DWORD h = 0;

    HENHMETAFILE hemf = MyGetEnhMetaFileW( _path_string );

    if (hemf)
    {
        ENHMETAHEADER emh;
        ZeroMemory(&emh, sizeof(emh));
        ok = GetEnhMetaFileHeader(hemf, sizeof(emh), &emh) != 0;

        w = (emh.rclFrame.right - emh.rclFrame.left);
        h = (emh.rclFrame.bottom - emh.rclFrame.top);

        DeleteEnhMetaFile(hemf);
    }

    if (ok)
    {
        const int PreviewSize = 512;

        // Get the size of the document
        const double emfWidth = w;
        const double emfHeight = h;

        // Find the minimum scale to fit the image inside the preview area
        const double scaleFactorX =    PreviewSize / emfWidth;
        const double scaleFactorY =    PreviewSize / emfHeight;
        const double scaleFactor = (scaleFactorX > scaleFactorY) ? scaleFactorY : scaleFactorX;

        // Now get the resized values
        const double scaledEmfWidth  = scaleFactor * emfWidth;
        const double scaledEmfHeight = scaleFactor * emfHeight;

        _preview_document_width = scaledEmfWidth;
        _preview_document_height = scaledEmfHeight;
        _preview_image_width = emfWidth;
        _preview_image_height = emfHeight;

        _preview_emf_image = true;
    }

    _mutex->unlock();

    return ok;
}

void FileOpenDialogImplWin32::render_preview()
{
    double x, y;
    const double blurRadius = 8;
    const double halfBlurRadius = blurRadius / 2;
    const int shaddowOffsetX = 0;
    const int shaddowOffsetY = 2;
    const int pagePadding = 5;
    const double shaddowAlpha = 0.75;

    // Is the preview showing?
    if(!_show_preview)
        return;

    // Do we have anything to render?
    _mutex->lock();

    if(!_preview_bitmap_image && !_preview_emf_image)
    {
        _mutex->unlock();
        return;
    }

    // Tidy up any previous bitmap renderings
    if(_preview_bitmap != NULL)
        DeleteObject(_preview_bitmap);
    _preview_bitmap = NULL;

    // Calculate the size of the caption
    int captionHeight = 0;

    if(_preview_wnd != NULL)
    {
        RECT rcCaptionRect;
        WCHAR szCaption[_MAX_FNAME + 32];
        const int iLength = format_caption(szCaption,
            sizeof(szCaption) / sizeof(WCHAR));

        HDC dc = GetDC(_preview_wnd);
        DrawTextW(dc, szCaption, iLength, &rcCaptionRect,
            DT_CENTER | DT_TOP | DT_NOPREFIX | DT_PATH_ELLIPSIS | DT_CALCRECT);
        ReleaseDC(_preview_wnd, dc);

        captionHeight = rcCaptionRect.bottom - rcCaptionRect.top;
    }

    // Find the minimum scale to fit the image inside the preview area
    const double scaleFactorX =
        ((double)_preview_width - pagePadding * 2 - blurRadius)  / _preview_document_width;
    const double scaleFactorY =
        ((double)_preview_height - pagePadding * 2
        - shaddowOffsetY - halfBlurRadius - captionHeight) / _preview_document_height;
    double scaleFactor = (scaleFactorX > scaleFactorY) ? scaleFactorY : scaleFactorX;
    scaleFactor = (scaleFactor > 1.0) ? 1.0 : scaleFactor;

    // Now get the resized values
    const double scaledSvgWidth  = scaleFactor * _preview_document_width;
    const double scaledSvgHeight = scaleFactor * _preview_document_height;

    const int svgX = pagePadding + halfBlurRadius;
    const int svgY = pagePadding;

    const int frameX = svgX - pagePadding;
    const int frameY = svgY - pagePadding;
    const int frameWidth = scaledSvgWidth + pagePadding * 2;
    const int frameHeight = scaledSvgHeight + pagePadding * 2;

    const int totalWidth = (int)ceil(frameWidth + blurRadius);
    const int totalHeight = (int)ceil(frameHeight + blurRadius);

    // Prepare the drawing surface
    HDC hDC = GetDC(_preview_wnd);
    HDC hMemDC = CreateCompatibleDC(hDC);
    _preview_bitmap = CreateCompatibleBitmap(hDC, totalWidth, totalHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, _preview_bitmap);
    Cairo::RefPtr<Win32Surface> surface = Win32Surface::create(hMemDC);
    Cairo::RefPtr<Context> context = Context::create(surface);

    // Paint the background to match the dialog colour
    const COLORREF background = GetSysColor(COLOR_3DFACE);
    context->set_source_rgb(
        GetRValue(background) / 255.0,
        GetGValue(background) / 255.0,
        GetBValue(background) / 255.0);
    context->paint();

    //----- Draw the drop shaddow -----//

    // Left Edge
    x = frameX + shaddowOffsetX - halfBlurRadius;
    Cairo::RefPtr<LinearGradient> leftEdgeFade = LinearGradient::create(
        x, 0.0, x + blurRadius, 0.0);
    leftEdgeFade->add_color_stop_rgba (0, 0, 0, 0, 0);
    leftEdgeFade->add_color_stop_rgba (1, 0, 0, 0, shaddowAlpha);
    context->set_source(leftEdgeFade);
    context->rectangle (x, frameY + shaddowOffsetY + halfBlurRadius,
        blurRadius, frameHeight - blurRadius);
    context->fill();

    // Right Edge
    x = frameX + frameWidth + shaddowOffsetX - halfBlurRadius;
    Cairo::RefPtr<LinearGradient> rightEdgeFade = LinearGradient::create(
        x, 0.0,    x + blurRadius, 0.0);
    rightEdgeFade->add_color_stop_rgba (0, 0, 0, 0, shaddowAlpha);
    rightEdgeFade->add_color_stop_rgba (1, 0, 0, 0, 0);
    context->set_source(rightEdgeFade);
    context->rectangle (frameX + frameWidth + shaddowOffsetX - halfBlurRadius,
        frameY + shaddowOffsetY + halfBlurRadius,
        blurRadius, frameHeight - blurRadius);
    context->fill();

    // Top Edge
    y = frameY + shaddowOffsetY - halfBlurRadius;
    Cairo::RefPtr<LinearGradient> topEdgeFade = LinearGradient::create(
        0.0, y, 0.0, y + blurRadius);
    topEdgeFade->add_color_stop_rgba (0, 0, 0, 0, 0);
    topEdgeFade->add_color_stop_rgba (1, 0, 0, 0, shaddowAlpha);
    context->set_source(topEdgeFade);
    context->rectangle (frameX + shaddowOffsetX + halfBlurRadius, y,
        frameWidth - blurRadius, blurRadius);
    context->fill();

    // Bottom Edge
    y = frameY + frameHeight + shaddowOffsetY - halfBlurRadius;
    Cairo::RefPtr<LinearGradient> bottomEdgeFade = LinearGradient::create(
        0.0, y,    0.0, y + blurRadius);
    bottomEdgeFade->add_color_stop_rgba (0, 0, 0, 0, shaddowAlpha);
    bottomEdgeFade->add_color_stop_rgba (1, 0, 0, 0, 0);
    context->set_source(bottomEdgeFade);
    context->rectangle (frameX + shaddowOffsetX + halfBlurRadius, y,
        frameWidth - blurRadius, blurRadius);
    context->fill();

    // Top Left Corner
    x = frameX + shaddowOffsetX - halfBlurRadius;
    y = frameY + shaddowOffsetY - halfBlurRadius;
    Cairo::RefPtr<RadialGradient> topLeftCornerFade = RadialGradient::create(
        x + blurRadius, y + blurRadius, 0, x + blurRadius, y + blurRadius, blurRadius);
    topLeftCornerFade->add_color_stop_rgba (0, 0, 0, 0, shaddowAlpha);
    topLeftCornerFade->add_color_stop_rgba (1, 0, 0, 0, 0);
    context->set_source(topLeftCornerFade);
    context->rectangle (x, y, blurRadius, blurRadius);
    context->fill();

    // Top Right Corner
    x = frameX + frameWidth + shaddowOffsetX - halfBlurRadius;
    y = frameY + shaddowOffsetY - halfBlurRadius;
    Cairo::RefPtr<RadialGradient> topRightCornerFade = RadialGradient::create(
        x, y + blurRadius, 0, x, y + blurRadius, blurRadius);
    topRightCornerFade->add_color_stop_rgba (0, 0, 0, 0, shaddowAlpha);
    topRightCornerFade->add_color_stop_rgba (1, 0, 0, 0, 0);
    context->set_source(topRightCornerFade);
    context->rectangle (x, y, blurRadius, blurRadius);
    context->fill();

    // Bottom Left Corner
    x = frameX + shaddowOffsetX - halfBlurRadius;
    y = frameY + frameHeight + shaddowOffsetY - halfBlurRadius;
    Cairo::RefPtr<RadialGradient> bottomLeftCornerFade = RadialGradient::create(
        x + blurRadius, y, 0, x + blurRadius, y, blurRadius);
    bottomLeftCornerFade->add_color_stop_rgba (0, 0, 0, 0, shaddowAlpha);
    bottomLeftCornerFade->add_color_stop_rgba (1, 0, 0, 0, 0);
    context->set_source(bottomLeftCornerFade);
    context->rectangle (x, y, blurRadius, blurRadius);
    context->fill();

    // Bottom Right Corner
    x = frameX + frameWidth + shaddowOffsetX - halfBlurRadius;
    y = frameY + frameHeight + shaddowOffsetY - halfBlurRadius;
    Cairo::RefPtr<RadialGradient> bottomRightCornerFade = RadialGradient::create(
        x, y, 0, x, y, blurRadius);
    bottomRightCornerFade->add_color_stop_rgba (0, 0, 0, 0, shaddowAlpha);
    bottomRightCornerFade->add_color_stop_rgba (1, 0, 0, 0, 0);
    context->set_source(bottomRightCornerFade);
    context->rectangle (frameX + frameWidth + shaddowOffsetX - halfBlurRadius,
        frameY + frameHeight + shaddowOffsetY - halfBlurRadius,
        blurRadius, blurRadius);
    context->fill();

    // Draw the frame
    context->set_line_width(1);
    context->rectangle (frameX, frameY,    frameWidth, frameHeight);

    context->set_source_rgb(1.0, 1.0, 1.0);
    context->fill_preserve();
    context->set_source_rgb(0.25, 0.25, 0.25);
    context->stroke_preserve();

    // Draw the image
    if(_preview_bitmap_image)    // Is the image a pixbuf?
    {
        // Set the transformation
        const Matrix matrix = {
            scaleFactor, 0,
            0, scaleFactor,
            svgX, svgY };
        context->set_matrix (matrix);

        // Render the image
        set_source_pixbuf (context, _preview_bitmap_image, 0, 0);
        context->paint();

        // Reset the transformation
        context->set_identity_matrix();
    }

    // Draw the inner frame
    context->set_source_rgb(0.75, 0.75, 0.75);
    context->rectangle (svgX, svgY,    scaledSvgWidth, scaledSvgHeight);
    context->stroke();

    _mutex->unlock();

    // Finish drawing
    surface->finish();

    if (_preview_emf_image) {
        HENHMETAFILE hemf = MyGetEnhMetaFileW(_path_string);
        if (hemf) {
            RECT rc;
            rc.top = svgY+2;
            rc.left = svgX+2;
            rc.bottom = scaledSvgHeight-2;
            rc.right = scaledSvgWidth-2;
            PlayEnhMetaFile(hMemDC, hemf, &rc);
            DeleteEnhMetaFile(hemf);
        }
    }

    SelectObject(hMemDC, hOldBitmap) ;
    DeleteDC(hMemDC);

    // Refresh the preview pane
    InvalidateRect(_preview_wnd, NULL, FALSE);
}

int FileOpenDialogImplWin32::format_caption(wchar_t *caption, int caption_size)
{
    wchar_t szFileName[_MAX_FNAME];
    _wsplitpath(_path_string, NULL, NULL, szFileName, NULL);

    return snwprintf(caption, caption_size,
        L"%s\n%d kB\n%d \xD7 %d", szFileName, _preview_file_size,
        (int)_preview_document_width, (int)_preview_document_height);
}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileOpenDialogImplWin32::show()
{
    // We can only run one worker thread at a time
    if(_mutex != NULL) return false;

    if(!Glib::thread_supported())
        Glib::thread_init();

    _result = false;
    _finished = false;
    _file_selected = false;
    _mutex = new Glib::Mutex();
    _main_loop = g_main_loop_new(g_main_context_default(), FALSE);

    if(Glib::Thread::create(sigc::mem_fun(*this, &FileOpenDialogImplWin32::GetOpenFileName_thread), true))
    {
        while(1)
        {
            g_main_context_iteration(g_main_context_default(), FALSE);

            if(_mutex->trylock())
            {
                // Read mutexed data
                const bool finished = _finished;
                const bool is_file_selected = _file_selected;
                _file_selected = false;
                _mutex->unlock();

                if(finished) break;
                if(is_file_selected) file_selected();
            }

            Sleep(10);
        }
    }

    // Tidy up
    delete _mutex;
    _mutex = NULL;

    return _result;
}

/**
 * To Get Multiple filenames selected at-once.
 */
std::vector<Glib::ustring>FileOpenDialogImplWin32::getFilenames()
{
    std::vector<Glib::ustring> result;
    result.push_back(getFilename());
    return result;
}


/*#########################################################################
### F I L E    S A V E
#########################################################################*/

/**
 * Constructor
 */
FileSaveDialogImplWin32::FileSaveDialogImplWin32(Gtk::Window &parent,
            const Glib::ustring &dir,
            FileDialogType fileTypes,
            const char *title,
            const Glib::ustring &/*default_key*/) :
    FileDialogBaseWin32(parent, dir, title, fileTypes, "dialogs.save_as")
{
    createFilterMenu();
}

FileSaveDialogImplWin32::~FileSaveDialogImplWin32()
{
}

void FileSaveDialogImplWin32::createFilterMenu()
{
    list<Filter> filter_list;

    knownExtensions.clear();

    // Compose the filter string
    Glib::ustring all_inkscape_files_filter, all_image_files_filter;
    Inkscape::Extension::DB::OutputList extension_list;
    Inkscape::Extension::db.get_output_list(extension_list);

    int filter_count = 0;
    int filter_length = 1;

    for (Inkscape::Extension::DB::OutputList::iterator current_item = extension_list.begin();
         current_item != extension_list.end(); current_item++)
    {
        Inkscape::Extension::Output *omod = *current_item;
        if (omod->deactivated()) continue;

        filter_count++;

        Filter filter;

        // Extension
        const gchar *filter_extension = omod->get_extension();
        filter.filter = g_utf8_to_utf16(
            filter_extension, -1, NULL, &filter.filter_length, NULL);
        knownExtensions.insert( Glib::ustring(filter_extension).casefold() );

        // Type
        filter.name = g_utf8_to_utf16(
            _(omod->get_filetypename()), -1, NULL, &filter.name_length, NULL);

        filter.mod = omod;

        filter_length += filter.name_length +
            filter.filter_length + 3;   // Add 3 for two \0s and a *

        filter_list.push_back(filter);
    }

    int extension_index = 0;
    _extension_map = new Inkscape::Extension::Extension*[filter_count];

    _filter = new wchar_t[filter_length];
    wchar_t *filterptr = _filter;

    for(list<Filter>::iterator filter_iterator = filter_list.begin();
        filter_iterator != filter_list.end(); filter_iterator++)
    {
        const Filter &filter = *filter_iterator;

        wcsncpy(filterptr, (wchar_t*)filter.name, filter.name_length);
        filterptr += filter.name_length;
        g_free(filter.name);

        *(filterptr++) = L'\0';
        *(filterptr++) = L'*';

        wcsncpy(filterptr, (wchar_t*)filter.filter, filter.filter_length);
        filterptr += filter.filter_length;
        g_free(filter.filter);

        *(filterptr++) = L'\0';

        // Associate this input extension with the file type name
        _extension_map[extension_index++] = filter.mod;
    }
    *(filterptr++) = 0;

	_filter_count = extension_index;
    _filter_index = 1;	// A value of 1 selects the 1st filter - NOT the 2nd
}

void FileSaveDialogImplWin32::GetSaveFileName_thread()
{
    OPENFILENAMEEXW ofn;

    g_assert(this != NULL);
    g_assert(_main_loop != NULL);

    WCHAR* current_directory_string = (WCHAR*)g_utf8_to_utf16(
        _current_directory.data(), _current_directory.length(),
		NULL, NULL, NULL);

    // Copy the selected file name, converting from UTF-8 to UTF-16
    memset(_path_string, 0, sizeof(_path_string));
    gunichar2* utf16_path_string = g_utf8_to_utf16(
        myFilename.data(), -1, NULL, NULL, NULL);
    wcsncpy(_path_string, (wchar_t*)utf16_path_string, _MAX_PATH);
    g_free(utf16_path_string);

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = _ownerHwnd;
    ofn.lpstrFile = _path_string;
    ofn.nMaxFile = _MAX_PATH;
    ofn.nFilterIndex = _filter_index;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = current_directory_string;
    ofn.lpstrTitle = _title;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrFilter = _filter;
    ofn.nFilterIndex = _filter_index;

    _result = GetSaveFileNameW(&ofn) != 0;

	g_assert(ofn.nFilterIndex >= 1 && ofn.nFilterIndex <= _filter_count);
    _filter_index = ofn.nFilterIndex;
    _extension = _extension_map[ofn.nFilterIndex - 1];

    // Copy the selected file name, converting from UTF-16 to UTF-8
    myFilename = utf16_to_ustring(_path_string, _MAX_PATH);

    // Tidy up
    g_free(current_directory_string);

    g_main_loop_quit(_main_loop);
}

/**
 * Show this dialog modally.  Return true if user hits [OK]
 */
bool
FileSaveDialogImplWin32::show()
{
    if(!Glib::thread_supported())
        Glib::thread_init();

    _result = false;
    _main_loop = g_main_loop_new(g_main_context_default(), FALSE);
	
	if(_main_loop != NULL)
	{
	    if(Glib::Thread::create(sigc::mem_fun(*this, &FileSaveDialogImplWin32::GetSaveFileName_thread), true))
	        g_main_loop_run(_main_loop);

	    if(_result)
	        appendExtension(myFilename, (Inkscape::Extension::Output*)_extension);
	}
	
    return _result;
}

void FileSaveDialogImplWin32::setSelectionType( Inkscape::Extension::Extension * /*key*/ )
{
    // If no pointer to extension is passed in, look up based on filename extension.

}

}
}
}

#endif

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
