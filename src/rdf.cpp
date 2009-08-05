/** @file
 * @brief  RDF manipulation functions
 *
 * @todo move these to xml/ instead of dialogs/
 */
/* Authors:
 *   Kees Cook <kees@outflux.net>
 *   Jon Phillips <jon@rejon.org>
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
   <license rdf:resource="http://creativecommons.org/licenses/by/2.0/" 
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
      "http://creativecommons.org/licenses/by/3.0/",
      rdf_license_cc_a,
    },

    { N_("CC Attribution-ShareAlike"), 
      "http://creativecommons.org/licenses/by-sa/3.0/",
      rdf_license_cc_a_sa,
    },

    { N_("CC Attribution-NoDerivs"), 
      "http://creativecommons.org/licenses/by-nd/3.0/",
      rdf_license_cc_a_nd,
    },

    { N_("CC Attribution-NonCommercial"), 
      "http://creativecommons.org/licenses/by-nc/3.0/",
      rdf_license_cc_a_nc,
    },

    { N_("CC Attribution-NonCommercial-ShareAlike"), 
      "http://creativecommons.org/licenses/by-nc-sa/3.0/",
      rdf_license_cc_a_nc_sa,
    },

    { N_("CC Attribution-NonCommercial-NoDerivs"), 
      "http://creativecommons.org/licenses/by-nc-nd/3.0/",
      rdf_license_cc_a_nc_nd,
    },

    { N_("Public Domain"),
      "http://creativecommons.org/licenses/publicdomain/",
      rdf_license_pd,
    },

    { N_("FreeArt"),
      "http://artlibre.org/licence.php/lalgb.html",
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

// Remember when using the "title" and "tip" elements to pass them through
// the localization functions when you use them!
struct rdf_work_entity_t rdf_work_entities [] = {
    { "title", N_("Title"), "dc:title", RDF_CONTENT,
      N_("Name by which this document is formally known."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "date", N_("Date"), "dc:date", RDF_CONTENT,
      N_("Date associated with the creation of this document (YYYY-MM-DD)."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "format", N_("Format"), "dc:format", RDF_CONTENT,
      N_("The physical or digital manifestation of this document (MIME type)."), RDF_FORMAT_LINE, RDF_EDIT_HARDCODED,
    },
    { "type", N_("Type"), "dc:type", RDF_RESOURCE,
      N_("Type of document (DCMI Type)."), RDF_FORMAT_LINE, RDF_EDIT_HARDCODED,
    },

    { "creator", N_("Creator"), "dc:creator", RDF_AGENT,
      N_("Name of entity primarily responsible for making the content of this document."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "rights", N_("Rights"), "dc:rights", RDF_AGENT,
      N_("Name of entity with rights to the Intellectual Property of this document."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "publisher", N_("Publisher"), "dc:publisher", RDF_AGENT,
      N_("Name of entity responsible for making this document available."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },

    { "identifier", N_("Identifier"), "dc:identifier", RDF_CONTENT,
      N_("Unique URI to reference this document."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "source", N_("Source"), "dc:source", RDF_CONTENT,
      N_("Unique URI to reference the source of this document."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "relation", N_("Relation"), "dc:relation", RDF_CONTENT,
      N_("Unique URI to a related document."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "language", N_("Language"), "dc:language", RDF_CONTENT,
      N_("Two-letter language tag with optional subtags for the language of this document.  (e.g. 'en-GB')"), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    { "subject", N_("Keywords"), "dc:subject", RDF_BAG,
      N_("The topic of this document as comma-separated key words, phrases, or classifications."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },
    // TRANSLATORS: "Coverage": the spatial or temporal characteristics of the content.
    // For info, see Appendix D of http://www.w3.org/TR/1998/WD-rdf-schema-19980409/
    { "coverage", N_("Coverage"), "dc:coverage", RDF_CONTENT,
      N_("Extent or scope of this document."), RDF_FORMAT_LINE, RDF_EDIT_GENERIC,
    },

    { "description", N_("Description"), "dc:description", RDF_CONTENT,
      N_("A short account of the content of this document."), RDF_FORMAT_MULTILINE, RDF_EDIT_GENERIC,
    },

    // FIXME: need to handle 1 agent per line of input
    { "contributor", N_("Contributors"), "dc:contributor", RDF_AGENT,
      N_("Names of entities responsible for making contributions to the content of this document."), RDF_FORMAT_MULTILINE, RDF_EDIT_GENERIC,
    },

    // TRANSLATORS: URL to a page that defines the license for the document
    { "license_uri", N_("URI"), "cc:license", RDF_RESOURCE,
      // TRANSLATORS: this is where you put a URL to a page that defines the license
      N_("URI to this document's license's namespace definition."), RDF_FORMAT_LINE, RDF_EDIT_SPECIAL,
    },

      // TRANSLATORS: fragment of XML representing the license of the document
    { "license_fragment", N_("Fragment"), "License", RDF_XML,
      N_("XML fragment for the RDF 'License' section."), RDF_FORMAT_MULTILINE, RDF_EDIT_SPECIAL,
    },
    
    { NULL, NULL, NULL, RDF_CONTENT,
      NULL, RDF_FORMAT_LINE, RDF_EDIT_HARDCODED,
    }
};

/**
 *  \brief   Retrieves a known RDF/Work entity by name
 *  \return  A pointer to an RDF/Work entity
 *  \param   name  The desired RDF/Work entity
 *  
 */
struct rdf_work_entity_t *
rdf_find_entity(gchar const * name)
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


/**
 *  \brief   Pull the text out of an RDF entity, depends on how it's stored
 *  \return  A pointer to the entity's static contents as a string
 *  \param   repr    The XML element to extract from
 *  \param   entity  The desired RDF/Work entity
 *  
 */
const gchar *
rdf_get_repr_text ( Inkscape::XML::Node * repr, struct rdf_work_entity_t * entity )
{
    g_return_val_if_fail (repr != NULL, NULL);
    g_return_val_if_fail (entity != NULL, NULL);
    static gchar * bag = NULL;
    gchar * holder = NULL;

    Inkscape::XML::Node * temp=NULL;
    switch (entity->datatype) {
        case RDF_CONTENT:
            temp = sp_repr_children(repr);
            if ( temp == NULL ) return NULL;
            
            return temp->content();

        case RDF_AGENT:
            temp = sp_repr_lookup_name ( repr, "cc:Agent", 1 );
            if ( temp == NULL ) return NULL;

            temp = sp_repr_lookup_name ( temp, "dc:title", 1 );
            if ( temp == NULL ) return NULL;

            temp = sp_repr_children(temp);
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
                temp = sp_repr_children(repr);
                if ( temp == NULL ) return NULL;
            
                return temp->content();
            }

            for ( temp = sp_repr_children(temp) ;
                  temp ;
                  temp = sp_repr_next(temp) ) {
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

unsigned int
rdf_set_repr_text ( Inkscape::XML::Node * repr,
                    struct rdf_work_entity_t * entity,
                    gchar const * text )
{
    g_return_val_if_fail ( repr != NULL, 0);
    g_return_val_if_fail ( entity != NULL, 0);
    g_return_val_if_fail ( text != NULL, 0);
    gchar * str = NULL;
    gchar** strlist = NULL;
    int i;

    Inkscape::XML::Node * temp=NULL;
    Inkscape::XML::Node * child=NULL;
    Inkscape::XML::Node * parent=repr;

    Inkscape::XML::Document * xmldoc = parent->document();
    g_return_val_if_fail (xmldoc != NULL, FALSE);

    // set document's title element to the RDF title
    if (!strcmp(entity->name, "title")) {
        Document *doc = SP_ACTIVE_DOCUMENT;
        if(doc && doc->root) doc->root->setTitle(text);
    }

    switch (entity->datatype) {
        case RDF_CONTENT:
            temp = sp_repr_children(parent);
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

            temp = sp_repr_children(parent);
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
                while ( (temp = sp_repr_children( parent )) ) {
                    parent->removeChild(temp);
                }

                temp = xmldoc->createElement ( "rdf:Bag" );
                g_return_val_if_fail (temp != NULL, FALSE);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);
            }
            parent = temp;

            /* toss all the old list items */
            while ( (temp = sp_repr_children( parent )) ) {
                parent->removeChild(temp);
            }

            /* chop our list up on commas */
            strlist = g_strsplit( text, ",", 0);

            for (i = 0; (str = strlist[i]); i++) {
                temp = xmldoc->createElement ( "rdf:li" );
                g_return_val_if_fail (temp != NULL, 0);

                parent->appendChild(temp);
                Inkscape::GC::release(temp);

                child = xmldoc->createTextNode( g_strstrip(str) );
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

Inkscape::XML::Node *
rdf_get_rdf_root_repr ( Document * doc, bool build )
{
    g_return_val_if_fail (doc        != NULL, NULL);
    g_return_val_if_fail (doc->rroot != NULL, NULL);

    Inkscape::XML::Document * xmldoc = sp_document_repr_doc(doc);
    g_return_val_if_fail (xmldoc != NULL, NULL);

    Inkscape::XML::Node * rdf = sp_repr_lookup_name ( doc->rroot, XML_TAG_NAME_RDF );

    if (rdf == NULL) {
        //printf("missing XML '%s'\n",XML_TAG_NAME_RDF);
        if (!build) return NULL;

        Inkscape::XML::Node * svg = sp_repr_lookup_name ( doc->rroot, XML_TAG_NAME_SVG );
        g_return_val_if_fail ( svg != NULL, NULL );

        Inkscape::XML::Node * parent = sp_repr_lookup_name ( svg, XML_TAG_NAME_METADATA );
        if ( parent == NULL ) {
            parent = xmldoc->createElement( XML_TAG_NAME_METADATA );
            g_return_val_if_fail ( parent != NULL, NULL);

            svg->appendChild(parent);
            Inkscape::GC::release(parent);
        }

        Inkscape::XML::Document * xmldoc = parent->document();
        g_return_val_if_fail (xmldoc != NULL, FALSE);

        rdf = xmldoc->createElement( XML_TAG_NAME_RDF );
        g_return_val_if_fail (rdf != NULL, NULL);

        parent->appendChild(rdf);
        Inkscape::GC::release(rdf);
    }

    /*
     * some implementations do not put RDF stuff inside <metadata>,
     * so we need to check for it and add it if we don't see it
     */
    Inkscape::XML::Node * want_metadata = sp_repr_parent ( rdf );
    g_return_val_if_fail (want_metadata != NULL, NULL);
    if (strcmp( want_metadata->name(), XML_TAG_NAME_METADATA )) {
            Inkscape::XML::Node * metadata = xmldoc->createElement( XML_TAG_NAME_METADATA );
            g_return_val_if_fail (metadata != NULL, NULL);

            /* attach the metadata node */
            want_metadata->appendChild(metadata);
            Inkscape::GC::release(metadata);

            /* move the RDF into it */
            Inkscape::GC::anchor(rdf);
            sp_repr_unparent ( rdf );
            metadata->appendChild(rdf);
            Inkscape::GC::release(rdf);
    }
    
    return rdf;
}

Inkscape::XML::Node *
rdf_get_xml_repr( Document * doc, gchar const * name, bool build )
{
    g_return_val_if_fail (name       != NULL, NULL);
    g_return_val_if_fail (doc        != NULL, NULL);
    g_return_val_if_fail (doc->rroot != NULL, NULL);

    Inkscape::XML::Node * rdf = rdf_get_rdf_root_repr ( doc, build );
    if (!rdf) return NULL;

    Inkscape::XML::Node * xml = sp_repr_lookup_name ( rdf, name );
    if (xml == NULL) {
        //printf("missing XML '%s'\n",name);
        if (!build) return NULL;

        Inkscape::XML::Document * xmldoc = sp_document_repr_doc(doc);
        g_return_val_if_fail (xmldoc != NULL, NULL);

        xml = xmldoc->createElement( name );
        g_return_val_if_fail (xml != NULL, NULL);

        xml->setAttribute("rdf:about", "" );

        rdf->appendChild(xml);
        Inkscape::GC::release(xml);
    }

    return xml;
}

Inkscape::XML::Node *
rdf_get_work_repr( Document * doc, gchar const * name, bool build )
{
    g_return_val_if_fail (name       != NULL, NULL);
    g_return_val_if_fail (doc        != NULL, NULL);
    g_return_val_if_fail (doc->rroot != NULL, NULL);

    Inkscape::XML::Node * work = rdf_get_xml_repr ( doc, XML_TAG_NAME_WORK, build );
    if (!work) return NULL;

    Inkscape::XML::Node * item = sp_repr_lookup_name ( work, name, 1 );
    if (item == NULL) {
        //printf("missing XML '%s'\n",name);
        if (!build) return NULL;

        Inkscape::XML::Document * xmldoc = sp_document_repr_doc(doc);
        g_return_val_if_fail (xmldoc != NULL, NULL);

        item = xmldoc->createElement( name );
        g_return_val_if_fail (item != NULL, NULL);

        work->appendChild(item);
        Inkscape::GC::release(item);
    }

    return item;
}



/**
 *  \brief   Retrieves a known RDF/Work entity's contents from the document XML by name
 *  \return  A pointer to the entity's static contents as a string, or NULL if no entity exists
 *  \param   entity  The desired RDF/Work entity
 *  
 */
const gchar *
rdf_get_work_entity(Document * doc, struct rdf_work_entity_t * entity)
{
    g_return_val_if_fail (doc    != NULL, NULL);
    if ( entity == NULL ) return NULL;
    //printf("want '%s'\n",entity->title);
    bool bIsTitle = !strcmp(entity->name, "title");

    Inkscape::XML::Node * item;
    if ( entity->datatype == RDF_XML ) {
        item = rdf_get_xml_repr ( doc, entity->tag, FALSE );
    }
    else {
        item = rdf_get_work_repr( doc, entity->tag, bIsTitle ); // build title if necessary
    }
    if ( item == NULL ) return NULL;
    const gchar * result = rdf_get_repr_text ( item, entity );
    if(!result && bIsTitle && doc->root) {         // if RDF title not set
        result = doc->root->title();               // get the document's <title>
        rdf_set_work_entity(doc, entity, result);  // and set the RDF
    }
    //printf("found '%s' == '%s'\n", entity->title, result );
    return result;
}

/**
 *  \brief   Stores a string into a named RDF/Work entity in the document XML
 *  \param   entity The desired RDF/Work entity to replace
 *  \param   string The string to replace the entity contents with
 *  
 */
unsigned int
rdf_set_work_entity(Document * doc, struct rdf_work_entity_t * entity,
                    const gchar * text)
{
    g_return_val_if_fail ( entity != NULL, 0 );
    if (text == NULL) {
        // FIXME: on a "NULL" text, delete the entity.  For now, blank it.
        text="";
    }

    /*
    printf("changing '%s' (%s) to '%s'\n",
        entity->title,
        entity->tag,
        text);
    */

    Inkscape::XML::Node * item = rdf_get_work_repr( doc, entity->tag, TRUE );
    g_return_val_if_fail ( item != NULL, 0 );

    return rdf_set_repr_text ( item, entity, text );
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

    for (Inkscape::XML::Node const *current = sp_repr_children(repr);
         current;
         current = sp_repr_next ( current ) ) {

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

/**
 *  \brief   Attempts to match and retrieve a known RDF/License from the document XML
 *  \return  A pointer to the static RDF license structure
 *  
 */
struct rdf_license_t *
rdf_get_license(Document * document)
{
    Inkscape::XML::Node const *repr = rdf_get_xml_repr ( document, XML_TAG_NAME_LICENSE, FALSE );
    if (repr) {
        for (struct rdf_license_t * license = rdf_licenses;
             license->name; license++ ) {
            if ( rdf_match_license ( repr, license ) ) return license;
        }
    }
#ifdef DEBUG_MATCH
    else {
        printf("no license XML\n");
    }
#endif
    return NULL;
}

/**
 *  \brief   Stores an RDF/License XML in the document XML
 *  \param   document  Which document to update
 *  \param   license   The desired RDF/License structure to store; NULL drops old license, so can be used for proprietary license. 
 *  
 */
void
rdf_set_license(Document * doc, struct rdf_license_t const * license)
{
    // drop old license section
    Inkscape::XML::Node * repr = rdf_get_xml_repr ( doc, XML_TAG_NAME_LICENSE, FALSE );
    if (repr) sp_repr_unparent(repr);

    if (!license) return;

    // build new license section
    repr = rdf_get_xml_repr ( doc, XML_TAG_NAME_LICENSE, TRUE );
    g_assert ( repr );

    repr->setAttribute("rdf:about", license->uri );

    Inkscape::XML::Document * xmldoc = sp_document_repr_doc(doc);
    g_return_if_fail (xmldoc != NULL);

    for (struct rdf_double_t const * detail = license->details;
         detail->name; detail++) {
        Inkscape::XML::Node * child = xmldoc->createElement( detail->name );
        g_assert ( child != NULL );

        child->setAttribute("rdf:resource", detail->resource );
        repr->appendChild(child);
        Inkscape::GC::release(child);
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

void
rdf_set_defaults ( Document * doc )
{
    g_assert ( doc != NULL );

    // Create metadata node if it doesn't already exist
    if (!sp_item_group_get_child_by_name ((SPGroup *) doc->root, NULL,
                                          XML_TAG_NAME_METADATA)) {
        // create repr
        Inkscape::XML::Document * xmldoc = sp_document_repr_doc(doc);
        g_return_if_fail (xmldoc != NULL);
        Inkscape::XML::Node * rnew = xmldoc->createElement (XML_TAG_NAME_METADATA);
        // insert into the document
        doc->rroot->addChild(rnew, NULL);
        // clean up
        Inkscape::GC::release(rnew);
    }

    /* install defaults */
    for ( struct rdf_entity_default_t * rdf_default = rdf_defaults;
          rdf_default->name;
          rdf_default++) {
        struct rdf_work_entity_t * entity = rdf_find_entity ( rdf_default->name );
        g_assert ( entity != NULL );

        if ( rdf_get_work_entity ( doc, entity ) == NULL ) {
            rdf_set_work_entity ( doc, entity, rdf_default->text );
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
