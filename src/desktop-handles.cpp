#define __SP_DESKTOP_HANDLES_C__

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
#include "desktop.h"

SPEventContext *
sp_desktop_event_context (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->event_context;
}

Inkscape::Selection *
sp_desktop_selection (SPDesktop const * desktop)
{
	g_assert(desktop != NULL);

	return desktop->selection;
}

Document *
sp_desktop_document (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->doc();
}

SPCanvas *
sp_desktop_canvas (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return ((SPCanvasItem *) desktop->main)->canvas;
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

SPNamedView *
sp_desktop_namedview (SPDesktop const * desktop)
{
	g_return_val_if_fail (desktop != NULL, NULL);

	return desktop->namedview;
}


