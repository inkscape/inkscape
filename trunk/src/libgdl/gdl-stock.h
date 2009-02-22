/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- 
 * gdl-stock.h
 * 
 * Copyright (C) 2003 Jeroen Zwartepoorte
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __GDL_STOCK_H__
#define __GDL_STOCK_H__

#include <glib/gmacros.h>   // G_BEGIN_DECLS

G_BEGIN_DECLS

#define GDL_STOCK_CLOSE			"gdl-close"
#define GDL_STOCK_MENU_LEFT		"gdl-menu-left"
#define GDL_STOCK_MENU_RIGHT		"gdl-menu-right"

void gdl_stock_init (void);

G_END_DECLS

#endif /* __GDL_STOCK_H__ */
