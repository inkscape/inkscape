/** @file
 * @brief headers for RDF types
 */
/* Authors:
 *  Kees Cook <kees@outflux.net>
 *
 * Copyright (C) 2004 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef _RDF_H_
#define _RDF_H_

#include <glib.h>
#include <glibmm/i18n.h>
#include "document.h"

// yeah, it's not a triple yet...
/**
 * \brief Holds license name/resource doubles for rdf_license_t entries
 */
struct rdf_double_t {
    gchar const *name;
    gchar const *resource;
};

/**
 * \brief Holds license name and RDF information
 */
struct rdf_license_t {
    gchar const *name;        /* localized name of this license */
    gchar const *uri;         /* URL for the RDF/Work/license element */
    struct rdf_double_t *details; /* the license details */
//    gchar const *fragment;    /* XML contents for the RDF/License tag */
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
    gchar const *title;      /* localized title of this entity for data entry */
    gchar const *tag;        /* namespace tag for the RDF/Work element */
    RDFType datatype;   /* how to extract/inject the RDF information */
    gchar const *tip;        /* tool tip to explain the meaning of the entity */
    RDF_Format format;  /* in what format is this data edited? */
    RDF_Editable editable;/* in what way is the data editable? */
};

extern rdf_work_entity_t rdf_work_entities [];

/**
 * \brief Generic collection of RDF information for the RDF debug function
 */
struct rdf_t {
    gchar*                work_title;
    gchar*                work_date;
    gchar*                work_creator;
    gchar*                work_owner;
    gchar*                work_publisher;
    gchar*                work_type;
    gchar*                work_source;
    gchar*                work_subject;
    gchar*                work_description;
    struct rdf_license_t* license;
};

struct rdf_work_entity_t * rdf_find_entity(gchar const * name);

const gchar * rdf_get_work_entity(SPDocument * doc,
                                  struct rdf_work_entity_t * entity);
unsigned int  rdf_set_work_entity(SPDocument * doc,
                                  struct rdf_work_entity_t * entity,
                                  const gchar * text);

struct rdf_license_t * rdf_get_license(SPDocument * doc);
void                   rdf_set_license(SPDocument * doc,
                                       struct rdf_license_t const * license);

void rdf_set_defaults ( SPDocument * doc );

#endif // _RDF_H_

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
