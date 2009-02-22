/* -*- Mode: C; indent-tabs-mode: ni; c-basic-offset: 8 -*- */

/*
 * This file is part of The Croco Library
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 * Author: Dodji Seketeli.
 * See the COPYRIGHTS file for copyright information.
 */

#include <string.h>
#include "cr-parsing-location.h"

/**
 *@file
 *Definition of the #CRparsingLocation class.
 */


/**
 *Instanciates a new parsing location.
 *@return the newly instanciated #CRParsingLocation.
 *Must be freed by cr_parsing_location_destroy()
 */
CRParsingLocation * 
cr_parsing_location_new (void)
{
	CRParsingLocation *result = 
	    (CRParsingLocation *)g_try_malloc (sizeof (CRParsingLocation)) ;
	if (!result) {
		cr_utils_trace_info ("Out of memory error") ;
		return NULL ;
	}
	cr_parsing_location_init (result) ;
	return result ;
}

/**
 *Initializes the an instance of #CRparsingLocation.
 *@param a_this the current instance of #CRParsingLocation.
 *@return CR_OK upon
 */
enum CRStatus 
cr_parsing_location_init (CRParsingLocation *a_this)
{
	g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR) ;

	memset (a_this, 0, sizeof (CRParsingLocation)) ;
	return CR_OK ;
}

/**
 *Copies an instance of CRParsingLocation into another one.
 *@param a_to the destination of the copy. 
 *Must be allocated by the caller.
 *@param a_from the source of the copy.
 *@return CR_OK upon succesful completion, an error code
 *otherwise.
 */
enum CRStatus 
cr_parsing_location_copy (CRParsingLocation *a_to,
			  CRParsingLocation *a_from)
{
	g_return_val_if_fail (a_to && a_from, CR_BAD_PARAM_ERROR) ;

	memcpy (a_to, a_from, sizeof (CRParsingLocation)) ;
	return CR_OK ;
}

/**
 *@param a_this the current instance of #CRParsingLocation.
 *@param a_mask a bitmap that defines which parts of the
 *parsing location are to be serialized (line, column or byte offset)
 *@return the serialized string or NULL in case of an error.
 */
gchar * 
cr_parsing_location_to_string (CRParsingLocation *a_this,
			       enum CRParsingLocationSerialisationMask a_mask)
{
	gchar *str = NULL ;

	g_return_val_if_fail (a_this, NULL) ;

	if (!a_mask) {
		a_mask = (enum CRParsingLocationSerialisationMask)
		    ((int)DUMP_LINE | (int)DUMP_COLUMN | (int)DUMP_BYTE_OFFSET) ;
	}
	GString *result = (GString *)g_string_new (NULL) ;
	if (!result)
		return NULL ;
	if (a_mask & DUMP_LINE) {
		g_string_append_printf (result, "line:%d ", 
					a_this->line) ;
	}
	if (a_mask & DUMP_COLUMN) {
		g_string_append_printf (result, "column:%d ", 
					a_this->column) ;
	}
	if (a_mask & DUMP_BYTE_OFFSET) {
		g_string_append_printf (result, "byte offset:%d ", 
					a_this->byte_offset) ;
	}
	if (result->len) {
		str = result->str ;
		g_string_free (result, FALSE) ;
	} else {
		g_string_free (result, TRUE) ;
	}
	return str ;
}

void
cr_parsing_location_dump (CRParsingLocation *a_this,
			  enum CRParsingLocationSerialisationMask a_mask,
			  FILE *a_fp)
{
	gchar *str = NULL ;

	g_return_if_fail (a_this && a_fp) ;
	str = cr_parsing_location_to_string (a_this, a_mask) ;
	if (str) {
		fprintf (a_fp, "%s", str) ;
		g_free (str) ;
		str = NULL ;
	}
}

/**
 *Destroys the current instance of #CRParsingLocation
 *@param a_this the current instance of #CRParsingLocation. Must
 *have been allocated with cr_parsing_location_new().
 */
void 
cr_parsing_location_destroy (CRParsingLocation *a_this)
{
	g_return_if_fail (a_this) ;
	g_free (a_this) ;
}

