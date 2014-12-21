/*
 * Frontends
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 2001-2002 Lauris Kaplinski
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "display/sp-canvas.h"
#include "display/sp-canvas-item.h"
#include "desktop.h"
#include "desktop-handles.h"

SPDocument *
sp_desktop_document (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->doc();
}

SPCanvas *
sp_desktop_canvas (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return (SP_CANVAS_ITEM(desktop->main))->canvas;
}

SPCanvasItem *
sp_desktop_acetate (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->acetate;
}

SPCanvasGroup *
sp_desktop_main (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->main;
}

SPCanvasGroup *
sp_desktop_gridgroup (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->gridgroup;
}

SPCanvasGroup *
sp_desktop_guides (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->guides;
}

SPCanvasItem *
sp_desktop_drawing (SPDesktop const *desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->drawing;
}

SPCanvasGroup *
sp_desktop_sketch (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->sketch;
}

SPCanvasGroup *
sp_desktop_controls (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->controls;
}

SPCanvasGroup *
sp_desktop_tempgroup (SPDesktop const * desktop)
{
    g_return_val_if_fail (desktop != NULL, NULL);

    return desktop->tempgroup;
}

Inkscape::MessageStack *
sp_desktop_message_stack (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->messageStack();
}
