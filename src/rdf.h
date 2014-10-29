/** @file
 * @brief headers for RDF types
 */
/* Authors:
 *  Kees Cook <kees@outflux.net>
 *  Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef SEEN_RDF_H
#define SEEN_RDF_H

#include <glibmm/i18n.h>
#include "document.h"

#define PREFS_METADATA      "/metadata/rdf/"

// yeah, it's not a triple yet...
/**
 * \brief Holds license name/resource doubles for rdf_license_t entries
 */
struct rdf_double_t {
    char const *name;
    char const *resource;
};

/**
 * \brief Holds license name and RDF information
 */
struct rdf_license_t {
    char const *name;        /* localized name of this license */
    char const *uri;         /* URL for the RDF/Work/license element */
    struct rdf_double_t *details; /* the license details */
//    char const *fragment;    /* XML contents for the RDF/License tag */
};

extern rdf_license_t rdf_licenses [];

/**
 * \brief Describes how a given RDF entity is stored in XML
 */
enum RDFType {
    RDF_CONTENT,  // direct between-XML-tags content
    RDF_AGENT,    // requires the "Agent" hierarchy before doing content
    RDF_RESOURCE, // stored in "rdf:resource" element
    RDF_XML,      // literal XML
    RDF_BAG       // rdf:Bag resources
};

/**
 * \brief Describes how a given RDF entity should be edited
 */
enum RDF_Format {
    RDF_FORMAT_LINE,          // uses single line data (GtkEntry)
    RDF_FORMAT_MULTILINE,     // uses multiline data (GtkTextView)
    RDF_FORMAT_SPECIAL        // uses some other edit methods
};

enum RDF_Editable {
    RDF_EDIT_GENERIC,       // editable via generic widgets
    RDF_EDIT_SPECIAL,       // special widgets are needed
    RDF_EDIT_HARDCODED      // isn't editable
};

/**
 * \brief Holds known RDF/Work tags
 */
struct rdf_work_entity_t {
    char const *name;       /* unique name of this entity for internal reference */
    char const *title;      /* localized title of this entity for data entry */
    char const *tag;        /* namespace tag for the RDF/Work element */
    RDFType datatype;   /* how to extract/inject the RDF information */
    char const *tip;        /* tool tip to explain the meaning of the entity */
    RDF_Format format;  /* in what format is this data edited? */
    RDF_Editable editable;/* in what way is the data editable? */
};

extern rdf_work_entity_t rdf_work_entities [];

/**
 * \brief Generic collection of RDF information for the RDF debug function
 */
struct rdf_t {
    char*                 work_title;
    char*                 work_date;
    char*                 work_creator;
    char*                 work_owner;
    char*                 work_publisher;
    char*                 work_type;
    char*                 work_source;
    char*                 work_subject;
    char*                 work_description;
    struct rdf_license_t* license;
};

struct rdf_work_entity_t * rdf_find_entity(char const * name);

/**
 *  \brief   Retrieves a known RDF/Work entity's contents from the document XML by name
 *  \return  A pointer to the entity's static contents as a string, or NULL if no entity exists
 *  \param   entity  The desired RDF/Work entity
 *  
 */
const gchar * rdf_get_work_entity(SPDocument const * doc,
                                  struct rdf_work_entity_t * entity);

/**
 *  \brief   Stores a string into a named RDF/Work entity in the document XML
 *  \param   entity The desired RDF/Work entity to replace
 *  \param   string The string to replace the entity contents with
 *  
 */
unsigned int  rdf_set_work_entity(SPDocument * doc,
                                  struct rdf_work_entity_t * entity,
                                  const char * text);

/**
 *  \brief   Attempts to match and retrieve a known RDF/License from the document XML
 *  \return  A pointer to the static RDF license structure
 *  
 */
struct rdf_license_t * rdf_get_license(SPDocument *doc);

/**
 *  \brief   Stores an RDF/License XML in the document XML
 *  \param   document  Which document to update
 *  \param   license   The desired RDF/License structure to store; NULL drops old license, so can be used for proprietary license. 
 *  
 */
void                   rdf_set_license(SPDocument * doc,
                                       struct rdf_license_t const * license);

void rdf_set_defaults ( SPDocument * doc );

void rdf_add_from_preferences ( SPDocument *doc );

#endif // SEEN_RDF_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
