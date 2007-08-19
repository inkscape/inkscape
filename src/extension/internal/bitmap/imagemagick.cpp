/*
 * Authors:
 *   Christopher Brown <audiere@gmail.com>
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2007 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <gtkmm/box.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/spinbutton.h>

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

void
ImageMagick::commitDocument(void) {
	_loaded = FALSE;
}

void
ImageMagick::cancelDocument(void) {	
	for (int i = 0; i < _imageCount; i++) {
		_nodes[i]->setAttribute("xlink:href", _originals[i], true);
	}
	
	_loaded = FALSE;
}

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
ImageMagick::effect (Inkscape::Extension::Effect *module, Inkscape::UI::View::View *document)
{
	refreshParameters(module);
	
	if (!_loaded)
	{		
		SPDesktop *desktop = (SPDesktop*)document;
		const GSList *selectedReprList = desktop->selection->reprList();
		int selectCount = g_slist_length((GSList *)selectedReprList);
		
		// Init the data-holders
		_nodes = new Inkscape::XML::Node*[selectCount];
		_originals = new const char*[selectCount];
		_images = new Magick::Image[selectCount];
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
				
				readImage(xlink, &_images[_imageCount]);
				
				_imageCount++;
			}			
		}
		
		_loaded = 1;
	}
	
	for (int i = 0; i < _imageCount; i++)
	{
		try
		{
			Magick::Image effectedImage = _images[i];
			applyEffect(&effectedImage);

			Magick::Blob blob;
			effectedImage.write(&blob);
				
				std::string raw_string = blob.base64();
				const char *raw = raw_string.c_str();

				/*
				const int raw_len = raw_string.length();
				const char *raw_i = raw;
                int formatted_len = (int)(raw_len / 76.0 * 78.0) + 100;
				char *formatted = new char[formatted_len];
				char *formatted_i = formatted;
				// data:image/png;base64,
				formatted_i = stpcpy(formatted_i, "data:image/");
				formatted_i = stpcpy(formatted_i, effectedImage.magick().c_str());
				formatted_i = stpcpy(formatted_i, ";base64, \n");
				while (strnlen(raw_i, 80) > 76)
				{
					formatted_i = stpncpy(formatted_i, raw_i, 76);
					formatted_i = stpcpy(formatted_i, "\n");					
					raw_i += 76;		
				}
				if (strlen(raw_i) > 0)
				{
					formatted_i = stpcpy(formatted_i, raw_i);
					formatted_i = stpcpy(formatted_i, "\n");
				}
				
				formatted_i = stpcpy(formatted_i, "\0");

				_nodes[i]->setAttribute("xlink:href", formatted, true);
				*/
				
				Glib::ustring buf = "data:image/";
				buf.append(effectedImage.magick());
				buf.append(";base64, \n");
				int col = 0;
				while (*raw)
                    {
                    buf.push_back(*raw++);
                    if (col>=76)
                        {
                        buf.push_back('\n');
                        col = 0;
                        }
                    }
                if (col)
                    buf.push_back('\n');

				_nodes[i]->setAttribute("xlink:href", buf.c_str(), true);
		}
		catch (Magick::Exception &error_) {
			printf("Caught exception: %s \n", error_.what());
		}
	}
}

/** \brief  A function to get the prefences for the grid
    \param  moudule  Module which holds the params
    \param  view     Unused today - may get style information in the future.

    Uses AutoGUI for creating the GUI.
*/
Gtk::Widget *
ImageMagick::prefs_effect(Inkscape::Extension::Effect *module, Inkscape::UI::View::View * view, sigc::signal<void> * changeSignal)
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
