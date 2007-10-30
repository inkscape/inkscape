/*
 * Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libintl.h>

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm.h>

#include <glib/gstdio.h>

#include "desktop.h"
#include "desktop-handles.h"
#include "selection.h"
#include "sp-object.h"
#include "util/glib-list-iterators.h"

#include "extension/effect.h"
#include "extension/system.h"

#include "imagemagick.h"

namespace Inkscape {
namespace Extension {
namespace Internal {
namespace Bitmap {

bool
ImageMagick::load(Inkscape::Extension::Extension *module)
{
	_loaded = FALSE;
	
	return TRUE;
}

/*
void
ImageMagick::commitDocument(void) {
	_loaded = FALSE;
}

void
ImageMagick::cancelDocument(void) {	
	for (int i = 0; i < _imageCount; i++) {
		_nodes[i]->setAttribute("xlink:href", _originals[i], true);
		
		if (strlen(_originals[i]) < 256)
			_nodes[i]->setAttribute("sodipodi:absref", _originals[i], true);
	}
	
	_loaded = FALSE;
}*/

void
ImageMagick::readImage(const char *xlink, Magick::Image *image)
{
	// Find if the xlink:href is base64 data, i.e. if the image is embedded 
	char *search = (char *) g_strndup(xlink, 30);
	if (strstr(search, "base64") != (char*)NULL) {
		// 7 = strlen("base64") + strlen(",")
		char* pureBase64 = strstr(xlink, "base64") + 7;		
		Magick::Blob blob;
		blob.base64(pureBase64);
		image->read(blob);
	}
	else {
		image->read(xlink);
	}
}

void
ImageMagick::effect (Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache)
{
	refreshParameters(module);
	_loaded = FALSE;
	
	if (!_loaded)
	{
		SPDesktop *desktop = (SPDesktop*)document;
		const GSList *selectedReprList = desktop->selection->reprList();
		int selectCount = g_slist_length((GSList *)selectedReprList);

		// Init the data-holders
		_nodes = new Inkscape::XML::Node*[selectCount];
		_originals = new const char*[selectCount];
		_caches = new char*[selectCount];
		_cacheLengths = new unsigned[selectCount];
		_images = new Magick::Image*[selectCount];
		_imageCount = 0;

		// Loop through selected nodes
		for (; selectedReprList != NULL; selectedReprList = g_slist_next(selectedReprList))
		{
			Inkscape::XML::Node *node = reinterpret_cast<Inkscape::XML::Node *>(selectedReprList->data);
			if (!strcmp(node->name(), "image") || !strcmp(node->name(), "svg:image"))
			{
				_nodes[_imageCount] = node;	
				char const *xlink = node->attribute("xlink:href");

				_originals[_imageCount] = xlink;
				_caches[_imageCount] = "";
				_cacheLengths[_imageCount] = 0;
				_images[_imageCount] = new Magick::Image();
				readImage(xlink, _images[_imageCount]);			

				_imageCount++;
			}			
		}

		_loaded = 1;
	}

	for (int i = 0; i < _imageCount; i++)
	{
		try
		{
			Magick::Image effectedImage = *_images[i]; // make a copy
			applyEffect(&effectedImage);

			Magick::Blob *blob = new Magick::Blob();
			effectedImage.write(blob);

			std::string raw_string = blob->base64();
			const int raw_len = raw_string.length();
			const char *raw_i = raw_string.c_str();

			unsigned new_len = (int)(raw_len * (77.0 / 76.0) + 100);
			if (new_len > _cacheLengths[i]) {
				_cacheLengths[i] = (int)(new_len * 1.2);
				_caches[i] = new char[_cacheLengths[i]];
			}
			char *formatted_i = _caches[i];
			const char *src;

			for (src = "data:image/"; *src; )
				*formatted_i++ = *src++;
			for (src = effectedImage.magick().c_str(); *src ; )
				*formatted_i++ = *src++;
			for (src = ";base64, \n" ; *src; )
				*formatted_i++ = *src++;

			int col = 0;
			while (*raw_i) {
			   *formatted_i++ = *raw_i++;
			   if (col++ > 76) {
				   *formatted_i++ = '\n';
				   col = 0;
			   }
			}			
			if (col) {
			   *formatted_i++ = '\n';
			}
			*formatted_i = '\0';

			_nodes[i]->setAttribute("xlink:href", _caches[i], true);			
			_nodes[i]->setAttribute("sodipodi:absref", NULL, true);
		}
		catch (Magick::Exception &error_) {
			printf("Caught exception: %s \n", error_.what());
		}

		while(Gtk::Main::events_pending())
			Gtk::Main::iteration();
	}
}

/** \brief  A function to get the prefences for the grid
    \param  moudule  Module which holds the params
    \param  view     Unused today - may get style information in the future.

    Uses AutoGUI for creating the GUI.
*/
Gtk::Widget *
ImageMagick::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal, Inkscape::Extension::Implementation::ImplementationDocumentCache * docCache)
{
    SPDocument * current_document = view->doc();

    using Inkscape::Util::GSListConstIterator;
    GSListConstIterator<SPItem *> selected = sp_desktop_selection((SPDesktop *)view)->itemList();
    Inkscape::XML::Node * first_select = NULL;
    if (selected != NULL) 
        first_select = SP_OBJECT_REPR(*selected);

    return module->autogui(current_document, first_select, changeSignal);
}

}; /* namespace Bitmap */
}; /* namespace Internal */
}; /* namespace Extension */
}; /* namespace Inkscape */
