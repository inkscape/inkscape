/** @file
 * RDF manipulation functions.
 *
 * @todo move these to xml/ instead of dialogs/
 */
/* Authors:
 *   Kees Cook <kees@outflux.net>
 *   Jon Phillips <jon@rejon.org>
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 Kees Cook <kees@outflux.net>
 * Copyright (C) 2006 Jon Phillips <jon@rejon.org>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "xml/repr.h"
#include "rdf.h"
#include "sp-item-group.h"
#include "inkscape.h"
#include "sp-root.h"
#include "preferences.h"

/*
   Example RDF XML from various places...
 
<rdf:RDF xmlns="http://creativecommons.org/ns#"
    xmlns:dc="http://purl.org/dc/elements/1.1/"
    xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
<Work rdf:about="">
   <dc:title>title of work</dc:title>
   <dc:date>year</dc:date>
   <dc:description>description of work</dc:description>
   <dc:creator><Agent>
      <dc:title>creator</dc:title>
   </Agent></dc:creator>
   <dc:rights><Agent>
      <dc:title>holder</dc:title>
   </Agent></dc:rights>
   <dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
   <dc:source rdf:resource="source"/>
   <license rdf:resource="http://creativecommons.org/licenses/by/4.0/" 
/>
</Work>


  <rdf:RDF xmlns="http://creativecommons.org/ns#"
      xmlns:dc="http://purl.org/dc/elements/1.1/"
      xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#">
  <Work rdf:about="">
     <dc:title>SVG Road Signs</dc:title>
     <dc:rights><Agent>
        <dc:title>John Cliff</dc:title>
     </Agent></dc:rights>
     <dc:type rdf:resource="http://purl.org/dc/dcmitype/StillImage" />
     <license rdf:resource="http://creativecommons.org/ns#PublicDomain" />
  </Work>
  
  <License rdf:about="http://creativecommons.org/ns#PublicDomain">
     <permits rdf:resource="http://creativecommons.org/ns#Reproduction" />
     <permits rdf:resource="http://creativecommons.org/ns#Distribution" />
     <permits rdf:resource="http://creativecommons.org/ns#DerivativeWorks" />
  </License>
  
</rdf:RDF>


Bag example:

<dc:subject>
<rdf:Bag>
<rdf:li>open clip art logo</rdf:li>
<rdf:li>images</rdf:li>
<rdf:li>logo</rdf:li>
<rdf:li>clip art</rdf:li>
<rdf:li>ocal</rdf:li>
<rdf:li>logotype</rdf:li>
<rdf:li>filetype</rdf:li>
</rdf:Bag>
</dc:subject>
*/

struct rdf_double_t rdf_license_empty [] = {
    { NULL, NULL }
};

struct rdf_double_t rdf_license_cc_a [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { "cc:permits", "http://creativecommons.org/ns#DerivativeWorks", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_cc_a_sa [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { "cc:permits", "http://creativecommons.org/ns#DerivativeWorks", },
    { "cc:requires", "http://creativecommons.org/ns#ShareAlike", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_cc_a_nd [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_cc_a_nc [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { "cc:prohibits", "http://creativecommons.org/ns#CommercialUse", },
    { "cc:permits", "http://creativecommons.org/ns#DerivativeWorks", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_cc_a_nc_sa [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { "cc:prohibits", "http://creativecommons.org/ns#CommercialUse", },
    { "cc:permits", "http://creativecommons.org/ns#DerivativeWorks", },
    { "cc:requires", "http://creativecommons.org/ns#ShareAlike", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_cc_a_nc_nd [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { "cc:prohibits", "http://creativecommons.org/ns#CommercialUse", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_pd [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:permits", "http://creativecommons.org/ns#DerivativeWorks", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_freeart [] = {
    { "cc:permits", "http://creativecommons.org/ns#Reproduction", },
    { "cc:permits", "http://creativecommons.org/ns#Distribution", },
    { "cc:permits", "http://creativecommons.org/ns#DerivativeWorks", },
    { "cc:requires", "http://creativecommons.org/ns#ShareAlike", },
    { "cc:requires", "http://creativecommons.org/ns#Notice", },
    { "cc:requires", "http://creativecommons.org/ns#Attribution", },
    { NULL, NULL }
};

struct rdf_double_t rdf_license_ofl [] = {
    { "cc:permits", "http://scripts.sil.org/pub/OFL/Reproduction", },
    { "cc:permits", "http://scripts.sil.org/pub/OFL/Distribution", },
    { "cc:permits", "http://scripts.sil.org/pub/OFL/Embedding", },
    { "cc:permits", "http://scripts.sil.org/pub/OFL/DerivativeWorks", },
    { "cc:requires", "http://scripts.sil.org/pub/OFL/Notice", },
    { "cc:requires", "http://scripts.sil.org/pub/OFL/Attribution", },
    { "cc:requires", "http://scripts.sil.org/pub/OFL/ShareAlike", },
    { "cc:requires", "http://scripts.sil.org/pub/OFL/DerivativeRenaming", },
    { "cc:requires", "http://scripts.sil.org/pub/OFL/BundlingWhenSelling", },
    { NULL, NULL }
};

struct rdf_license_t rdf_licenses [] = {
    { N_("CC Attribution"), 
      "http://creativecommons.org/licenses/by/4.0/",
      rdf_license_cc_a,
    },

    { N_("CC Attribution-ShareAlike"), 
      "http://creativecommons.org/licenses/by-sa/4.0/",
      rdf_license_cc_a_sa,
    },

    { N_("CC Attribution-NoDerivs"), 
      "http://creativecommons.org/licenses/by-nd/4.0/",
      rdf_license_cc_a_nd,
    },

    { N_("CC Attribution-NonCommercial"), 
      "http://creativecommons.org/licenses/by-nc/4.0/",
      rdf_license_cc_a_nc,
    },

    { N_("CC Attribution-NonCommercial-ShareAlike"), 
      "http://creativecommons.org/licenses/by-nc-sa/4.0/",
      rdf_license_cc_a_nc_sa,
    },

    { N_("CC Attribution-NonCommercial-NoDerivs"), 
      "http://creativecommons.org/licenses/by-nc-nd/4.0/",
      rdf_license_cc_a_nc_nd,
    },

    { N_("CC0 Public Domain Dedication"),
      "http://creativecommons.org/publicdomain/zero/1.0/",
      rdf_license_pd,
    },

    { N_("FreeArt"),
      "http://artlibre.org/licence/lal",
      rdf_license_freeart,
    },

    { N_("Open Font License"),
      "http://scripts.sil.org/OFL",
      rdf_license_ofl,
    },

    { NULL, NULL, rdf_license_empty, }
};

#define XML_TAG_NAME_SVG      "svg:svg"
#define XML_TAG_NAME_METADATA "svg:metadata"
#define XML_TAG_NAME_RDF      "rdf:RDF"
#define XML_TAG_NAME_WORK     "cc:Work"
#define XML_TAG_NAME_LICENSE  "cc:License"
// Note the lowercase L!
#define XML_TAG_NAME_LICENSE_PROP  "cc:license"


// Remember when using the "title" and "tip" elements to pass them through
// the localization functions when you use them!
struct rdf_work_entity_t rdf_work_entities [] = {
    { "title", N_("Title:"), "dc:title", RDF_CONTENT,
      N_("A name given to the resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "date", N_("Date:"), "dc:date", RDF_CONTENT,
      N_("A point or period of time associated with an event in the lifecycle of the resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "format", N_("Format:"), "dc:format", RDF_CONTENT,
      N_("The file format, physical medium, or dimensions of the resource"), RDF_FORMAT_LINE, RDF_EDIT_HARDCODED,
    },
    { "type", N_("Type:"), "dc:type", RDF_RESOURCE,
      N_("The nature or genre of the resource"), RDF_FORMAT_LINE, RDF_EDIT_HARDCODED,
    },

    { "creator", N_("Creator:"), "dc:creator", RDF_AGENT,
      N_("An entity primarily responsible for making the resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "rights", N_("Rights:"), "dc:rights", RDF_AGENT,
      N_("Information about rights held in and over the resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "publisher", N_("Publisher:"), "dc:publisher", RDF_AGENT,
      N_("An entity responsible for making the resource available"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },

    { "identifier", N_("Identifier:"), "dc:identifier", RDF_CONTENT,
      N_("An unambiguous reference to the resource within a given context"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "source", N_("Source:"), "dc:source", RDF_CONTENT,
      N_("A related resource from which the described resource is derived"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "relation", N_("Relation:"), "dc:relation", RDF_CONTENT,
      N_("A related resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "language", N_("Language:"), "dc:language", RDF_CONTENT,
      N_("A language of the resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "subject", N_("Keywords:"), "dc:subject", RDF_BAG,
      N_("The topic of the resource"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    // TRANSLATORS: "Coverage": the spatial or temporal characteristics of the content.
    // For info, see Appendix D of http://www.w3.org/TR/1998/WD-rdf-schema-19980409/
    { "coverage", N_("Coverage:"), "dc:coverage", RDF_CONTENT,
      N_("The spatial or temporal topic of the resource, the spatial applicability of the resource, or the jurisdiction under which the resource is relevant"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },

    { "description", N_("Description:"), "dc:description", RDF_CONTENT,
      N_("An account of the resource"), RDF_FORMAT_MULTILINE, RDF_EDIT_GENERIC,
    },

    // FIXME: need to handle 1 agent per line of input
    { "contributor", N_("Contributors:"), "dc:contributor", RDF_AGENT,
      N_("An entity responsible for making contributions to the resource"), RDF_FORMAT_MULTILINE, RDF_EDIT_GENERIC,
    },

    // TRANSLATORS: URL to a page that defines the license for the document
    { "license_uri", N_("URI:"), "cc:license", RDF_RESOURCE,
      // TRANSLATORS: this is where you put a URL to a page that defines the license
      N_("URI to this document's license's namespace definition"), RDF_FORMAT_LINE, RDF_EDIT_SPECIAL,
    },

      // TRANSLATORS: fragment of XML representing the license of the document
    { "license_fragment", N_("Fragment:"), "License", RDF_XML,
      N_("XML fragment for the RDF 'License' section"), RDF_FORMAT_MULTILINE, RDF_EDIT_SPECIAL,
    },

    { NULL, NULL, NULL, RDF_CONTENT,
      NULL, RDF_FORMAT_LINE, RDF_EDIT_HARDCODED,
    }
};


// Simple start of C++-ification:
class RDFImpl
{
public:
    /**
     * Some implementations do not put RDF stuff inside <metadata>,
     * so we need to check for it and add it if we don't see it.
     */
    static void ensureParentIsMetadata( SPDocument *doc, Inkscape::XML::Node *node );

    static Inkscape::XML::Node const *getRdfRootRepr( SPDocument const * doc );
    static Inkscape::XML::Node *ensureRdfRootRepr( SPDocument * doc );

    static Inkscape::XML::Node const *getXmlRepr( SPDocument const * doc, gchar const * name );
    static Inkscape::XML::Node *getXmlRepr( SPDocument * doc, gchar const * name );
    static Inkscape::XML::Node *ensureXmlRepr( SPDocument * doc, gchar const * name );

    static Inkscape::XML::Node const *getWorkRepr( SPDocument const * doc, gchar const * name );
    static Inkscape::XML::Node *ensureWorkRepr( SPDocument * doc, gchar const * name );

    static const gchar *getWorkEntity(SPDocument const * doc, struct rdf_work_entity_t & entity);
    static unsigned int setWorkEntity(SPDocument * doc, struct rdf_work_entity_t & entity, gchar const * text);

    static void setDefaults( SPDocument * doc );

    /**
     *  Pull the text out of an RDF entity, depends on how it's stored.
     *
     *  @return  A pointer to the entity's static contents as a string
     *  @param   repr    The XML element to extract from
     *  @param   entity  The desired RDF/Work entity
     *  
     */
    static const gchar *getReprText( Inkscape::XML::Node const * repr, struct rdf_work_entity_t const & entity );

    static unsigned int setReprText( Inkscape::XML::Node * repr,
                                     struct rdf_work_entity_t const & entity,
                                     gchar const * text );

    static struct rdf_license_t *getLicense(SPDocument *document);

    static void setLicense(SPDocument * doc, struct rdf_license_t const * license);
};

/**
 *  Retrieves a known RDF/Work entity by name.
 *
 *  @return  A pointer to an RDF/Work entity
 *  @param   name  The desired RDF/Work entity
 *  
 */
struct rdf_work_entity_t *rdf_find_entity(gchar const * name)
{
    struct rdf_work_entity_t *entity;
    for (entity=rdf_work_entities; entity->name; entity++) {
        if (strcmp(entity->name,name)==0) break;
    }
    if (entity->name) return entity;
    return NULL;
}

/*
 * Takes the inkscape rdf struct and spits out a static RDF, which is only
 * useful for testing.  We must merge the rdf struct into the XML DOM for
 * changes to be saved.
 */
/*

   Since g_markup_printf_escaped doesn't exist for most people's glib
   right now, this function will remain commented out since it's only
   for generic debug anyway.  --Kees

gchar *
rdf_string(struct rdf_t * rdf)
{
    gulong overall=0;
    gchar *string=NULL;

    gchar *rdf_head="\
<rdf:RDF xmlns=\"http://creativecommons.org/ns#\"\
    xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\
    xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\
";
    gchar *work_head="\
<Work rdf:about=\"\">\
   <dc:type rdf:resource=\"http://purl.org/dc/dcmitype/StillImage\" />\
";
    gchar *work_title=NULL;
    gchar *work_date=NULL;
    gchar *work_description=NULL;
    gchar *work_creator=NULL;
    gchar *work_owner=NULL;
    gchar *work_source=NULL;
    gchar *work_license=NULL;
    gchar *license_head=NULL;
    gchar *license=NULL;
    gchar *license_end="</License>\n";
    gchar *work_end="</Work>\n";
    gchar *rdf_end="</rdf:RDF>\n";

    if (rdf && rdf->work_title && rdf->work_title[0]) {
        work_title=g_markup_printf_escaped("   <dc:title>%s</dc:title>\n",
            rdf->work_title);
    overall+=strlen(work_title);
    }
    if (rdf && rdf->work_date && rdf->work_date[0]) {
        work_date=g_markup_printf_escaped("   <dc:date>%s</dc:date>\n",
            rdf->work_date);
    overall+=strlen(work_date);
    }
    if (rdf && rdf->work_description && rdf->work_description[0]) {
        work_description=g_markup_printf_escaped("   <dc:description>%s</dc:description>\n",
            rdf->work_description);
    overall+=strlen(work_description);
    }
    if (rdf && rdf->work_creator && rdf->work_creator[0]) {
        work_creator=g_markup_printf_escaped("   <dc:creator><Agent>\
      <dc:title>%s</dc:title>\
   </Agent></dc:creator>\n",
            rdf->work_creator);
    overall+=strlen(work_creator);
    }
    if (rdf && rdf->work_owner && rdf->work_owner[0]) {
        work_owner=g_markup_printf_escaped("   <dc:rights><Agent>\
      <dc:title>%s</dc:title>\
   </Agent></dc:rights>\n",
            rdf->work_owner);
    overall+=strlen(work_owner);
    }
    if (rdf && rdf->work_source && rdf->work_source[0]) {
        work_source=g_markup_printf_escaped("   <dc:source rdf:resource=\"%s\" />\n",
            rdf->work_source);
    overall+=strlen(work_source);
    }
    if (rdf && rdf->license && rdf->license->work_rdf && rdf->license->work_rdf[0]) {
        work_license=g_markup_printf_escaped("   <license rdf:resource=\"%s\" />\n",
            rdf->license->work_rdf);
    overall+=strlen(work_license);

    license_head=g_markup_printf_escaped("<License rdf:about=\"%s\">\n",
            rdf->license->work_rdf);
    overall+=strlen(license_head);
    overall+=strlen(rdf->license->license_rdf);
    overall+=strlen(license_end);
    }

    overall+=strlen(rdf_head)+strlen(rdf_end);
    overall+=strlen(work_head)+strlen(work_end);

    overall++; // NULL term

    if (!(string=(gchar*)g_malloc(overall))) {
        return NULL;
    }

    string[0]='\0';
    strcat(string,rdf_head);
    strcat(string,work_head);

    if (work_title)       strcat(string,work_title);
    if (work_date)        strcat(string,work_date);
    if (work_description) strcat(string,work_description);
    if (work_creator)     strcat(string,work_creator);
    if (work_owner)       strcat(string,work_owner);
    if (work_source)      strcat(string,work_source);
    if (work_license)     strcat(string,work_license);

    strcat(string,work_end);
    if (license_head) {
        strcat(string,license_head);
    strcat(string,rdf->license->license_rdf);
    strcat(string,license_end);
    }
    strcat(string,rdf_end);

    return string;
}
*/


const gchar *RDFImpl::getReprText( Inkscape::XML::Node const * repr, struct rdf_work_entity_t const & entity )
{
    g_return_val_if_fail (repr != NULL, NULL);
    static gchar * bag = NULL;
    gchar * holder = NULL;

    Inkscape::XML::Node const * temp = NULL;
    switch (entity.datatype) {
        case RDF_CONTENT:
            temp = repr->firstChild();
            if ( temp == NULL ) return NULL;
            
            return temp->content();

        case RDF_AGENT:
            temp = sp_repr_lookup_name ( repr, "cc:Agent", 1 );
            if ( temp == NULL ) return NULL;

            temp = sp_repr_lookup_name ( temp, "dc:title", 1 );
            if ( temp == NULL ) return NULL;

            temp = temp->firstChild();
            if ( temp == NULL ) return NULL;

            return temp->content();

        case RDF_RESOURCE:
            return repr->attribute("rdf:resource");

        case RDF_XML:
            return "xml goes here";

        case RDF_BAG:
            /* clear the static string.  yucky. */
            if (bag) g_free(bag);
            bag = NULL;

            temp = sp_repr_lookup_name ( repr, "rdf:Bag", 1 );
            if ( temp == NULL ) {
                /* backwards compatible: read contents */
                temp = repr->firstChild();
                if ( temp == NULL ) return NULL;
            
                return temp->content();
            }

            for ( temp = temp->firstChild() ;
                  temp ;
                  temp = temp->next() ) {
                if (!strcmp(temp->name(),"rdf:li") &&
                    temp->firstChild()) {
                    const gchar * str = temp->firstChild()->content();
                    if (bag) {
                        holder = bag;
                        bag = g_strconcat(holder, ", ", str, NULL);
                        g_free(holder);
                    }
                    else {
                        bag = g_strdup(str);
                    }
                }
            }
            return bag;

        default:
            break;
    }
    return NULL;
}

unsigned int RDFImpl::setReprText( Inkscape::XML::Node * repr,
                                   struct rdf_work_entity_t const & entity,
                                   gchar const * text )
{
    g_return_val_if_fail ( repr != NULL, 0);
    g_return_val_if_fail ( text != NULL, 0);
    gchar * str = NULL;
    gchar** strlist = NULL;
    int i;

    Inkscape::XML::Node * temp=NULL;
    Inkscape::XML::Node * parent=repr;

    Inkscape::XML::Document * xmldoc = parent->document();
    g_return_val_if_fail (xmldoc != NULL, FALSE);

    // set document's title element to the RDF title
    if (!strcmp(entity.name, "title")) {
        SPDocument *doc = SP_ACTIVE_DOCUMENT;
        if (doc && doc->getRoot()) {
            doc->getRoot()->setTitle(text);
        }
    }

    switch (entity.datatype) {
        case RDF_CONTENT:
            temp = parent->firstChild();
            if ( temp == NULL ) {
                temp = xmldoc->createTextNode( text );
                g_return_val_if_fail (temp != NULL, FALSE);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);

                return TRUE;
            }
            else {
                temp->setContent(text);
                return TRUE;
            }

        case RDF_AGENT:
            temp = sp_repr_lookup_name ( parent, "cc:Agent", 1 );
            if ( temp == NULL ) {
                temp = xmldoc->createElement ( "cc:Agent" );
                g_return_val_if_fail (temp != NULL, FALSE);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);
            }
            parent = temp;

            temp = sp_repr_lookup_name ( parent, "dc:title", 1 );
            if ( temp == NULL ) {
                temp = xmldoc->createElement ( "dc:title" );
                g_return_val_if_fail (temp != NULL, FALSE);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);
            }
            parent = temp;

            temp = parent->firstChild();
            if ( temp == NULL ) {
                temp = xmldoc->createTextNode( text );
                g_return_val_if_fail (temp != NULL, FALSE);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);

                return TRUE;
            }
            else {
                temp->setContent(text);
				return TRUE;
            }

        case RDF_RESOURCE:
            parent->setAttribute("rdf:resource", text );
            return true;

        case RDF_XML:
            return 1;

        case RDF_BAG:
            /* find/create the rdf:Bag item */
            temp = sp_repr_lookup_name ( parent, "rdf:Bag", 1 );
            if ( temp == NULL ) {
                /* backward compatibility: drop the dc:subject contents */
                while ( (temp = parent->firstChild()) ) {
                    parent->removeChild(temp);
                }

                temp = xmldoc->createElement ( "rdf:Bag" );
                g_return_val_if_fail (temp != NULL, FALSE);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);
            }
            parent = temp;

            /* toss all the old list items */
            while ( (temp = parent->firstChild()) ) {
                parent->removeChild(temp);
            }

            /* chop our list up on commas */
            strlist = g_strsplit( text, ",", 0);

            for (i = 0; (str = strlist[i]); i++) {
                temp = xmldoc->createElement ( "rdf:li" );
                g_return_val_if_fail (temp != NULL, 0);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);

                Inkscape::XML::Node * child = xmldoc->createTextNode( g_strstrip(str) );
                g_return_val_if_fail (child != NULL, 0);

                temp->appendChild(child);
                Inkscape::GC::release(child);
            }
            g_strfreev( strlist );

            return 1;

        default:
            break;
    }
    return 0;
}

void RDFImpl::ensureParentIsMetadata( SPDocument *doc, Inkscape::XML::Node *node )
{
    if ( !node ) {
        g_critical("Null node passed to ensureParentIsMetadata().");
    } else if ( !node->parent() ) {
        g_critical( "No parent node when verifying <metadata> placement." );
    } else {
        Inkscape::XML::Node * currentParent = node->parent();
        if ( strcmp( currentParent->name(), XML_TAG_NAME_METADATA ) != 0 ) {
            Inkscape::XML::Node * metadata = doc->getReprDoc()->createElement( XML_TAG_NAME_METADATA );
            if ( !metadata ) {
                g_critical("Unable to create metadata element.");
            } else {
                // attach the metadata node
                currentParent->appendChild( metadata );
                Inkscape::GC::release( metadata );

                // move the node into it
                Inkscape::GC::anchor( node );
                sp_repr_unparent( node );
                metadata->appendChild( node );
                Inkscape::GC::release( node );
            }
        }
    }
}

Inkscape::XML::Node const *RDFImpl::getRdfRootRepr( SPDocument const * doc )
{
    Inkscape::XML::Node const *rdf = 0;
    if ( !doc ) {
        g_critical("Null doc passed to getRdfRootRepr()");
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else {
        rdf = sp_repr_lookup_name( doc->getReprDoc(), XML_TAG_NAME_RDF );
    }

    return rdf;
}

Inkscape::XML::Node *RDFImpl::ensureRdfRootRepr( SPDocument * doc )
{
    Inkscape::XML::Node *rdf = 0;
    if ( !doc ) {
        g_critical("Null doc passed to ensureRdfRootRepr()");
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else {
        rdf = sp_repr_lookup_name( doc->getReprDoc(), XML_TAG_NAME_RDF );
        if ( !rdf ) {
            Inkscape::XML::Node * svg = sp_repr_lookup_name( doc->getReprRoot(), XML_TAG_NAME_SVG );
            if ( !svg ) {
                g_critical("Unable to locate svg element.");
            } else {
                Inkscape::XML::Node * parent = sp_repr_lookup_name( svg, XML_TAG_NAME_METADATA );
                if ( parent == NULL ) {
                    parent = doc->getReprDoc()->createElement( XML_TAG_NAME_METADATA );
                    if ( !parent ) {
                        g_critical("Unable to create metadata element");
                    } else {
                        svg->appendChild(parent);
                        Inkscape::GC::release(parent);
                    }
                }

                if ( parent && !parent->document() ) {
                    g_critical("Parent has no document");
                } else if ( parent ) {
                    rdf = parent->document()->createElement( XML_TAG_NAME_RDF );
                    if ( !rdf ) {
                        g_critical("Unable to create root RDF element.");
                    } else {
                        parent->appendChild(rdf);
                        Inkscape::GC::release(rdf);
                    }
                }
            }
        }
    }

    if ( rdf ) {
        ensureParentIsMetadata( doc, rdf );
    }

    return rdf;
}

Inkscape::XML::Node const *RDFImpl::getXmlRepr( SPDocument const * doc, gchar const * name )
{
    Inkscape::XML::Node const * xml = 0;
    if ( !doc ) {
        g_critical("Null doc passed to getXmlRepr()");
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else if (!name) {
        g_critical("Null name passed to getXmlRepr()");        
    } else {
        Inkscape::XML::Node const * rdf = getRdfRootRepr( doc );
        if ( rdf ) {
            xml = sp_repr_lookup_name( rdf, name );
        }
    }
    return xml;
}

Inkscape::XML::Node *RDFImpl::getXmlRepr( SPDocument * doc, gchar const * name )
{
    Inkscape::XML::Node const *xml = getXmlRepr( const_cast<SPDocument const *>(doc), name );

    return const_cast<Inkscape::XML::Node *>(xml);
}

Inkscape::XML::Node *RDFImpl::ensureXmlRepr( SPDocument * doc, gchar const * name )
{
    Inkscape::XML::Node * xml = 0;
    if ( !doc ) {
        g_critical("Null doc passed to ensureXmlRepr()");
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else if (!name) {
        g_critical("Null name passed to ensureXmlRepr()");        
    } else {
        Inkscape::XML::Node * rdf = ensureRdfRootRepr( doc );
        if ( rdf ) {
            xml = sp_repr_lookup_name( rdf, name );
            if ( !xml ) {
                xml = doc->getReprDoc()->createElement( name );
                if ( !xml ) {
                    g_critical("Unable to create xml element <%s>.", name);
                } else {
                    xml->setAttribute("rdf:about", "" );

                    rdf->appendChild(xml);
                    Inkscape::GC::release(xml);
                }
            }
        }
    }
    return xml;
}

Inkscape::XML::Node const *RDFImpl::getWorkRepr( SPDocument const * doc, gchar const * name )
{
    Inkscape::XML::Node const * item = 0;
    if ( !doc ) {
        g_critical("Null doc passed to getWorkRepr()");
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else if (!name) {
        g_critical("Null name passed to getWorkRepr()");        
    } else {
        Inkscape::XML::Node const* work = getXmlRepr( doc, XML_TAG_NAME_WORK );
        if ( work ) {
            item = sp_repr_lookup_name( work, name, 1 );
        }
    }
    return item;
}

Inkscape::XML::Node *RDFImpl::ensureWorkRepr( SPDocument * doc, gchar const * name )
{
    Inkscape::XML::Node * item = 0;
    if ( !doc ) {
        g_critical("Null doc passed to ensureWorkRepr()");
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else if (!name) {
        g_critical("Null name passed to ensureWorkRepr()");        
    } else {
        Inkscape::XML::Node * work = ensureXmlRepr( doc, XML_TAG_NAME_WORK );
        if ( work ) {
            item = sp_repr_lookup_name( work, name, 1 );
            if ( !item ) {
                //printf("missing XML '%s'\n",name);
                item = doc->getReprDoc()->createElement( name );
                if ( !item ) {
                    g_critical("Unable to create xml element <%s>", name);
                } else {
                    work->appendChild(item);
                    Inkscape::GC::release(item);
                }
            }
        }
    }
    return item;
}


// Public API:
const gchar *rdf_get_work_entity(SPDocument const * doc, struct rdf_work_entity_t * entity)
{
    const gchar *result = 0;
    if ( !doc ) {
        g_critical("Null doc passed to rdf_get_work_entity()");
    } else if ( entity ) {
        //g_message("want '%s'\n",entity->title);

        result = RDFImpl::getWorkEntity( doc, *entity );

        //g_message("found '%s' == '%s'\n", entity->title, result );
    }
    return result;
}

const gchar *RDFImpl::getWorkEntity(SPDocument const * doc, struct rdf_work_entity_t & entity)
{
    gchar const *result = 0;

    Inkscape::XML::Node const * item = getWorkRepr( doc, entity.tag );
    if ( item ) {
        result = getReprText( item, entity );
        // TODO note that this is the location that used to set the title if needed. Ensure code it not required.
    }

    return result;
}

// Public API:
unsigned int rdf_set_work_entity(SPDocument * doc, struct rdf_work_entity_t * entity,
                                 const gchar * text)
{
    unsigned int result = 0;
    if ( !doc ) {
        g_critical("Null doc passed to rdf_set_work_entity()");
    } else if ( entity ) {
        result = RDFImpl::setWorkEntity( doc, *entity, text );
    }

    return result;
}

unsigned int RDFImpl::setWorkEntity(SPDocument * doc, struct rdf_work_entity_t & entity, const gchar * text)
{
    int result = 0;
    if ( !text ) {
        // FIXME: on a "NULL" text, delete the entity.  For now, blank it.
        text = "";
    }

    /*
      printf("changing '%s' (%s) to '%s'\n",
      entity->title,
      entity->tag,
      text);
    */

    Inkscape::XML::Node * item = ensureWorkRepr( doc, entity.tag );
    if ( !item ) {
        g_critical("Unable to get work element");
    } else {
        result = setReprText( item, entity, text );
    }
    return result;
}


#undef DEBUG_MATCH

static bool
rdf_match_license(Inkscape::XML::Node const *repr, struct rdf_license_t const *license)
{
    g_assert ( repr != NULL );
    g_assert ( license != NULL );

    bool result=TRUE;
#ifdef DEBUG_MATCH
    printf("checking against '%s'\n",license->name);
#endif

    int count = 0;
    for (struct rdf_double_t const *details = license->details;
         details->name; details++ ) {
        count++;
    }
    bool * matched = (bool*)calloc(count,sizeof(bool));

    for (Inkscape::XML::Node const *current = repr->firstChild();
         current;
         current = current->next() ) {

        gchar const * attr = current->attribute("rdf:resource");
        if ( attr == NULL ) continue;

#ifdef DEBUG_MATCH
        printf("\texamining '%s' => '%s'\n", current->name(), attr);
#endif

        bool found_match=FALSE;
        for (int i=0; i<count; i++) {
            // skip already matched items
            if (matched[i]) continue;

#ifdef DEBUG_MATCH
            printf("\t\t'%s' vs '%s'\n", current->name(), license->details[i].name);
            printf("\t\t'%s' vs '%s'\n", attr, license->details[i].resource);
#endif

            if (!strcmp( current->name(),
                         license->details[i].name ) &&
                !strcmp( attr,
                         license->details[i].resource )) {
                matched[i]=TRUE;
                found_match=TRUE;
#ifdef DEBUG_MATCH
                printf("\t\tgood!\n");
#endif
                break;
            }
        }
        if (!found_match) {
            // if we checked each known item of the license
            // and didn't find it, we must abort
            result=FALSE;
#ifdef DEBUG_MATCH
            printf("\t\tno '%s' element matched XML (bong)!\n",license->name);
#endif
            break;
        }
    }
#ifdef DEBUG_MATCH
    if (result) printf("\t\tall XML found matching elements!\n");
#endif
    for (int i=0; result && i<count; i++) {
        // scan looking for an unmatched item
        if (matched[i]==0) {
            result=FALSE;
#ifdef DEBUG_MATCH
            printf("\t\tnot all '%s' elements used to match (bong)!\n", license->name);
#endif
        }
    }

#ifdef DEBUG_MATCH
    printf("\t\tall '%s' elements used to match!\n",license->name);
#endif

    free(matched);

#ifdef DEBUG_MATCH
    if (result) printf("matched '%s'\n",license->name);
#endif
    return result;
}

// Public API:
struct rdf_license_t *rdf_get_license(SPDocument *document)
{
    return RDFImpl::getLicense(document);
}

struct rdf_license_t *RDFImpl::getLicense(SPDocument *document)
{
    // Base license lookup on the URI of cc:license rather than the license
    // properties, per instructions from the ccREL gurus.
    // (Fixes https://bugs.launchpad.net/inkscape/+bug/372427)

    struct rdf_work_entity_t *entity = rdf_find_entity("license_uri");
    if (entity == NULL) {
        g_critical("Can't find internal entity structure for 'license_uri'");
        return NULL;
    }

    const gchar *uri = getWorkEntity(document, *entity);
    struct rdf_license_t * license_by_uri = NULL;

    if (uri != NULL) {
        for (struct rdf_license_t * license = rdf_licenses; license->name; license++) {
            if (g_strcmp0(uri, license->uri) == 0) {
                license_by_uri = license;
                break;
            }
        }
    }

    // To improve backward compatibility, the old license matching code is
    // kept as fallback and to warn about and fix discrepancies.

    // TODO: would it be better to do this code on document load?  Is
    // sp_metadata_build() then the right place to put the call to sort out
    // any RDF mess?
    
    struct rdf_license_t * license_by_properties = NULL;
    
    Inkscape::XML::Node const *repr = getXmlRepr( document, XML_TAG_NAME_LICENSE );
    if (repr) {
        for ( struct rdf_license_t * license = rdf_licenses; license->name; license++ ) {
            if ( rdf_match_license( repr, license ) ) {
                license_by_properties = license;
                break;
            }
        }
    }

    if (license_by_uri != NULL && license_by_properties != NULL) {
        // Both property and structure, use property
        if (license_by_uri != license_by_properties) {
            // TODO: this should be a user-visible warning, but how?
            g_warning("Mismatch between %s and %s metadata:\n"
                      "%s value URI:   %s (using this one!)\n"
                      "%s derived URI: %s",
                      XML_TAG_NAME_LICENSE_PROP,
                      XML_TAG_NAME_LICENSE,
                      XML_TAG_NAME_LICENSE_PROP,
                      license_by_uri->uri,
                      XML_TAG_NAME_LICENSE,
                      license_by_properties->uri);
        }

        // Reset license structure to match so the document is consistent
        // (and this will also silence the warning above on repeated calls).
        setLicense(document, license_by_uri);

        return license_by_uri;
    }
    else if (license_by_uri != NULL) {
        // Only cc:license property, set structure for backward compatiblity
        setLicense(document, license_by_uri);

        return license_by_uri;
    }
    else if (license_by_properties != NULL) {
        // Only cc:License structure
        // TODO: this could be a user-visible warning too
        g_warning("No %s metadata found, derived license URI from %s: %s",
                  XML_TAG_NAME_LICENSE_PROP, XML_TAG_NAME_LICENSE,
                  license_by_properties->uri);
        
        // Set license property to match
        setWorkEntity(document, *entity, license_by_properties->uri);

        return license_by_properties;
    }

    // No license info at all
    return NULL;
}

// Public API:
void rdf_set_license(SPDocument * doc, struct rdf_license_t const * license)
{
    RDFImpl::setLicense( doc, license );
}

void RDFImpl::setLicense(SPDocument * doc, struct rdf_license_t const * license)
{
    // When basing license check on only the license URI (see fix for
    // https://bugs.launchpad.net/inkscape/+bug/372427 above) we should
    // really drop this license section, but keep writing it for a while for
    // compatibility with older versions.

    // drop old license section
    Inkscape::XML::Node * repr = getXmlRepr( doc, XML_TAG_NAME_LICENSE );
    if (repr) {
        sp_repr_unparent(repr);
    }

    if ( !license ) {
        // All done
    } else if ( !doc->getReprDoc() ) {
        g_critical("XML doc is null.");
    } else {
        // build new license section
        repr = ensureXmlRepr( doc, XML_TAG_NAME_LICENSE );
        g_assert( repr );

        repr->setAttribute("rdf:about", license->uri );

        for (struct rdf_double_t const * detail = license->details; detail->name; detail++) {
            Inkscape::XML::Node * child = doc->getReprDoc()->createElement( detail->name );
            g_assert ( child != NULL );

            child->setAttribute("rdf:resource", detail->resource );
            repr->appendChild(child);
            Inkscape::GC::release(child);
        }
    }
}

struct rdf_entity_default_t {
    gchar const * name;
    gchar const * text;
};
struct rdf_entity_default_t rdf_defaults[] = {
    { "format",      "image/svg+xml", },
    { "type",        "http://purl.org/dc/dcmitype/StillImage", },
    { NULL,          NULL, }
};

// Public API:
void rdf_set_defaults( SPDocument * doc )
{
    RDFImpl::setDefaults( doc );

}

void RDFImpl::setDefaults( SPDocument * doc )
{
    g_assert( doc != NULL );

    // Create metadata node if it doesn't already exist
    if (!sp_item_group_get_child_by_name( doc->getRoot(), NULL,
                                          XML_TAG_NAME_METADATA)) {
        if ( !doc->getReprDoc()) {
            g_critical("XML doc is null.");
        } else {
            // create repr
            Inkscape::XML::Node * rnew = doc->getReprDoc()->createElement(XML_TAG_NAME_METADATA);

            // insert into the document
            doc->getReprRoot()->addChild(rnew, NULL);

            // clean up
            Inkscape::GC::release(rnew);
        }
    }

    // install defaults
    for ( struct rdf_entity_default_t * rdf_default = rdf_defaults; rdf_default->name; rdf_default++) {
        struct rdf_work_entity_t * entity = rdf_find_entity( rdf_default->name );
        g_assert( entity != NULL );

        if ( getWorkEntity( doc, *entity ) == NULL ) {
            setWorkEntity( doc, *entity, rdf_default->text );
        }
    }
}

/*
 * Add the metadata stored in the users preferences to the document if
 *   a) the preference 'Input/Output->Add default metatdata to new documents' is true, and
 *   b) there is no metadata already in the file (such as from a template)
 */
void rdf_add_from_preferences(SPDocument *doc)
{
    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    if (!prefs->getBool("/metadata/addToNewFile")) {
        return;
    }

    // If there is already some metadata in the doc (from a template) dont add default metadata
    for (struct rdf_work_entity_t *entity = rdf_work_entities; entity && entity->name; entity++) {
        if ( entity->editable == RDF_EDIT_GENERIC &&
                rdf_get_work_entity (doc, entity)) {
            return;
        }
    }

    // Put the metadata from user preferences into the doc
    for (struct rdf_work_entity_t *entity = rdf_work_entities; entity && entity->name; entity++) {
        if ( entity->editable == RDF_EDIT_GENERIC ) {
            Glib::ustring text = prefs->getString(PREFS_METADATA + Glib::ustring(entity->name));
            if (text.length() > 0) {
                rdf_set_work_entity (doc, entity, text.c_str());
            }
        }
    }
}

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
