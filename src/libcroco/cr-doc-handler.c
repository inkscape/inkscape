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
 * See COPRYRIGHTS file for copyright information.
 */

#include <string.h>
#include "cr-doc-handler.h"
#include "cr-parser.h"

/**
 *@file
 *The definition of the CRDocHandler class.
 *Contains methods to instantiate, destroy,
 *and initialyze instances of #CRDocHandler
 *to custom values.
 */

#define PRIVATE(obj) (obj)->priv

struct _CRDocHandlerPriv {
	/**
	 *This pointer is to hold an application parsing context.
	 *For example, it used by the Object Model parser to 
	 *store it parsing context. #CRParser does not touch it, but
	 *#CROMParser does. #CROMParser allocates this pointer at
	 *the beginning of the css document, and frees it at the end
	 *of the document.
	 */
        gpointer context;

	/**
	 *The place where #CROMParser puts the result of its parsing, if
	 *any.
	 */
        gpointer result;
	/**
	 *a pointer to the parser used to parse
	 *the current document.
	 */
	CRParser *parser ;
};

/**
 *Constructor of #CRDocHandler.
 *@return the newly built instance of
 *#CRDocHandler
 */
CRDocHandler *
cr_doc_handler_new (void)
{
        CRDocHandler *result = ( CRDocHandler *)g_try_malloc (sizeof (CRDocHandler));

        g_return_val_if_fail (result, NULL);

        memset (result, 0, sizeof (CRDocHandler));

        result->priv =  (CRDocHandlerPriv *)g_try_malloc (sizeof (CRDocHandlerPriv));
        if (!result->priv) {
                cr_utils_trace_info ("Out of memory exception");
                g_free (result);
                return NULL;
        }

        cr_doc_handler_set_default_sac_handler (result);

        return result;
}

/**
 *Returns the private parsing context.
 *The private parsing context is used by libcroco only.
 *@param a_this the current instance of #CRDocHandler.
 *@param a_ctxt out parameter. The new parsing context.
 *@return CR_OK upon successfull completion, an error code otherwise.
 *@return the parsing context, or NULL if an error occured.
 */
enum CRStatus
cr_doc_handler_get_ctxt (CRDocHandler * a_this, gpointer * a_ctxt)
{
        g_return_val_if_fail (a_this && a_this->priv, CR_BAD_PARAM_ERROR);

        *a_ctxt = a_this->priv->context;

        return CR_OK;
}

/**
 *Sets the private parsing context.
 *This is used by libcroco only.
 *@param a_this the current instance of #CRDocHandler
 *@param a_ctxt a pointer to the parsing context.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_doc_handler_set_ctxt (CRDocHandler * a_this, gpointer a_ctxt)
{
        g_return_val_if_fail (a_this && a_this->priv, CR_BAD_PARAM_ERROR);
        a_this->priv->context = a_ctxt;
        return CR_OK;
}

/**
 *Returns the private parsing result.
 *The private parsing result is used by libcroco only.
 *@param a_this the current instance of #CRDocHandler
 *@param a_result out parameter. The returned result.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_doc_handler_get_result (CRDocHandler * a_this, gpointer * a_result)
{
        g_return_val_if_fail (a_this && a_this->priv, CR_BAD_PARAM_ERROR);

        *a_result = a_this->priv->result;

        return CR_OK;
}

/**
 *Sets the private parsing context.
 *This is used by libcroco only.
 *@param a_this the current instance of #CRDocHandler
 *@param a_result the new result.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_doc_handler_set_result (CRDocHandler * a_this, gpointer a_result)
{
        g_return_val_if_fail (a_this && a_this->priv, CR_BAD_PARAM_ERROR);
        a_this->priv->result = a_result;
        return CR_OK;
}

/**
 *Sets the sac handlers contained in the current
 *instance of DocHandler to the default handlers.
 *For the time being the default handlers are
 *test handlers. This is expected to change in a
 *near future, when the libcroco gets a bit debugged.
 *
 *@param a_this a pointer to the current instance of #CRDocHandler.
 *@return CR_OK upon successfull completion, an error code otherwise.
 */
enum CRStatus
cr_doc_handler_set_default_sac_handler (CRDocHandler * a_this)
{
        g_return_val_if_fail (a_this, CR_BAD_PARAM_ERROR);

        a_this->start_document = NULL;
        a_this->end_document = NULL;
        a_this->import_style = NULL;
        a_this->namespace_declaration = NULL;
        a_this->comment = NULL;
        a_this->start_selector = NULL;
        a_this->end_selector = NULL;
        a_this->property = NULL;
        a_this->start_font_face = NULL;
        a_this->end_font_face = NULL;
        a_this->start_media = NULL;
        a_this->end_media = NULL;
        a_this->start_page = NULL;
        a_this->end_page = NULL;
        a_this->ignorable_at_rule = NULL;
        a_this->error = NULL;
        a_this->unrecoverable_error = NULL;
        return CR_OK;
}

/**
 *Increases the reference count of the doc handler
 *@param a_this the current instance of #CRDocHandler.
 */
void
cr_doc_handler_ref (CRDocHandler * a_this)
{
        g_return_if_fail (a_this);

        a_this->ref_count++;
}

/**
 *Decreases the ref count of the current instance of #CRDocHandler.
 *If the ref count reaches '0' then, destroys the instance.
 *@param a_this the currrent instance of #CRDocHandler.
 *@return TRUE if the instance as been destroyed, FALSE otherwise.
 */
gboolean
cr_doc_handler_unref (CRDocHandler * a_this)
{
        g_return_val_if_fail (a_this, FALSE);

        if (a_this->ref_count > 0) {
                a_this->ref_count--;
        }

        if (a_this->ref_count == 0) {
                cr_doc_handler_destroy (a_this);
                return TRUE;
        }
        return FALSE ;
}

/**
 *The destructor of the #CRDocHandler class.
 *@param a_this the instance of #CRDocHandler to
 *destroy.
 */
void
cr_doc_handler_destroy (CRDocHandler * a_this)
{
        g_return_if_fail (a_this);

        if (a_this->priv) {
                g_free (a_this->priv);
                a_this->priv = NULL;
        }
        g_free (a_this);
}

/**
 *Associates a parser to the current document handler
 *@param a_this the current instance of document handler.
 *@param a_parser the parser to associate.
 */
void
cr_doc_handler_associate_a_parser (CRDocHandler *a_this,
				   gpointer a_parser)
{
	g_return_if_fail (a_this && PRIVATE (a_this) 
			  && a_parser) ;

	PRIVATE (a_this)->parser = (CRParser *)a_parser ;
}
