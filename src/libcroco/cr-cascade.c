/* -*- Mode: C; indent-tabs-mode:nil; c-basic-offset: 8-*- */

/*
 * This file is part of The Croco Library
 *
 * Copyright (C) 2002-2003 Dodji Seketeli <dodji@seketeli.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the 
 * GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the 
 * GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

/*
 *$Id: cr-cascade.c,v 1.6 2004/03/07 13:22:47 dodji Exp $
 */

#include <string.h>
#include "cr-cascade.h"

#define PRIVATE(a_this) ((a_this)->priv)

struct _CRCascadePriv {
 /**
	 *the 3 style sheets of the cascade:
	 *author, user, and useragent sheet.
	 *Intended to be addressed by
	 *sheets[ORIGIN_AUTHOR] or sheets[ORIGIN_USER]
	 *of sheets[ORIGIN_UA] ;
	 */
        CRStyleSheet *sheets[3];
        guint ref_count;
};

/**
 *Constructor of the #CRCascade class.
 *Note that all three parameters of this
 *method are ref counted and their refcount is increased.
 *Their refcount will be decreased at the destruction of
 *the instance of #CRCascade.
 *So the caller should not call their destructor. The caller
 *should call their ref/unref method instead if it wants
 *@param a_author_sheet the autor origin style sheet
 *@param a_user_sheet the user origin style sheet.
 *@param a_ua_sheet the user agent origin style sheet.
 *@return the newly built instance of CRCascade or NULL if
 *an error arose during constrution.
 */
CRCascade *
cr_cascade_new (CRStyleSheet * a_author_sheet,
                CRStyleSheet * a_user_sheet, CRStyleSheet * a_ua_sheet)
{
        CRCascade *result = (CRCascade *)g_try_malloc (sizeof (CRCascade));
        if (!result) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (result, 0, sizeof (CRCascade));

        PRIVATE (result) = (CRCascadePriv *)g_try_malloc (sizeof (CRCascadePriv));
        if (!PRIVATE (result)) {
                cr_utils_trace_info ("Out of memory");
                return NULL;
        }
        memset (PRIVATE (result), 0, sizeof (CRCascadePriv));

        if (a_author_sheet) {
                cr_cascade_set_sheet (result, a_author_sheet, ORIGIN_AUTHOR);
        }
        if (a_user_sheet) {
                cr_cascade_set_sheet (result, a_user_sheet, ORIGIN_USER);
        }
        if (a_ua_sheet) {
                cr_cascade_set_sheet (result, a_ua_sheet, ORIGIN_UA);
        }

        return result;
}

/**
 *Gets a given origin sheet.
 *Note that the returned stylesheet
 *is refcounted so if the caller wants
 *to manage its lifecycle, it must use
 *cr_stylesheet_ref()/cr_stylesheet_unref() instead
 *of the cr_stylesheet_destroy() method.
 *@param a_this the current instance of #CRCascade.
 *@param a_origin the origin of the style sheet as
 *defined in the css2 spec in chapter 6.4.
 *@return the style sheet, or NULL if it does not
 *exist.
 */
CRStyleSheet *
cr_cascade_get_sheet (CRCascade * a_this, enum CRStyleOrigin a_origin)
{
        g_return_val_if_fail (a_this
                              && (unsigned)a_origin < NB_ORIGINS, NULL);

        return PRIVATE (a_this)->sheets[a_origin];
}

/**
 *Sets a stylesheet in the cascade
 *@param a_this the current instance of #CRCascade.
 *@param a_sheet the stylesheet to set.
 *@param a_origin the origin of the stylesheet.
 *@return CR_OK upon successfull completion, an error
 *code otherwise.
 */
enum CRStatus
cr_cascade_set_sheet (CRCascade * a_this,
                      CRStyleSheet * a_sheet, enum CRStyleOrigin a_origin)
{
        g_return_val_if_fail (a_this
                              && a_sheet
                              && (unsigned)a_origin < NB_ORIGINS, CR_BAD_PARAM_ERROR);

        if (PRIVATE (a_this)->sheets[a_origin])
                cr_stylesheet_unref (PRIVATE (a_this)->sheets[a_origin]);
        PRIVATE (a_this)->sheets[a_origin] = a_sheet;
        cr_stylesheet_ref (a_sheet);
        a_sheet->origin = a_origin;
        return CR_OK;
}

/**
 *Increases the reference counter of the current instance
 *of #CRCascade.
 *@param a_this the current instance of #CRCascade
 *
 */
void
cr_cascade_ref (CRCascade * a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this));

        PRIVATE (a_this)->ref_count++;
}

/**
 *Decrements the reference counter associated
 *to this instance of #CRCascade. If the reference
 *counter reaches zero, the instance is destroyed 
 *using cr_cascade_destroy()
 *@param a_this the current instance of 
 *#CRCascade.
 */
void
cr_cascade_unref (CRCascade * a_this)
{
        g_return_if_fail (a_this && PRIVATE (a_this));

        if (PRIVATE (a_this)->ref_count)
                PRIVATE (a_this)->ref_count--;
        if (!PRIVATE (a_this)->ref_count) {
                cr_cascade_destroy (a_this);
        }
}

/**
 *Destructor of #CRCascade.
 */
void
cr_cascade_destroy (CRCascade * a_this)
{
        g_return_if_fail (a_this);

        if (PRIVATE (a_this)) {
                gulong i = 0;

                for (i = 0; PRIVATE (a_this)->sheets && i < NB_ORIGINS; i++) {
                        if (PRIVATE (a_this)->sheets[i]) {
                                if (cr_stylesheet_unref
                                    (PRIVATE (a_this)->sheets[i])
                                    == TRUE) {
                                        PRIVATE (a_this)->sheets[i] = NULL;
                                }
                        }
                }
                g_free (PRIVATE (a_this));
                PRIVATE (a_this) = NULL;
        }
        g_free (a_this);
}
