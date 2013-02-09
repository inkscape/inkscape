#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <gtkmm.h>
#include <libnrtype/font-instance.h>
#include <libnrtype/TextWrapper.h>
#include <libnrtype/one-glyph.h>

#include <glibmm.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>

#include "font-lister.h"
#include "FontFactory.h"

#include "sp-object.h"
#include "sp-root.h"
#include "document.h"
#include "xml/repr.h"

namespace Inkscape
{
    FontLister::FontLister ()
    {
        font_list_store = Gtk::ListStore::create (FontList);
        
        FamilyToStylesMap familyStyleMap;
        font_factory::Default()->GetUIFamiliesAndStyles(&familyStyleMap);
       
        // Grab the family names into a list and then sort them
        std::list<Glib::ustring> familyList;
        for (FamilyToStylesMap::iterator iter = familyStyleMap.begin();
                 iter != familyStyleMap.end();
                 iter++) {
            familyList.push_back((*iter).first);
        }
        familyList.sort();
        
        // Traverse through the family names and set up the list store (note that
        // the styles list that are the map's values are already sorted)
        while (!familyList.empty()) {
            Glib::ustring familyName = familyList.front();
            familyList.pop_front();
            
            if (!familyName.empty()) {
                Gtk::TreeModel::iterator treeModelIter = font_list_store->append();
                (*treeModelIter)[FontList.font] = reinterpret_cast<const char*>(g_strdup(familyName.c_str()));
                
                // Now go through the styles
                GList *styles = NULL;
                std::list<Glib::ustring> &styleStrings = familyStyleMap[familyName];
                for (std::list<Glib::ustring>::iterator it=styleStrings.begin();
                        it != styleStrings.end();
                        it++) {
                    styles = g_list_append(styles, g_strdup((*it).c_str()));
                }
                
                (*treeModelIter)[FontList.styles] = styles;
                (*treeModelIter)[FontList.onSystem] = true;
            }
        }
    }

    // Example of how to use "foreach_iter"
    // bool
    // FontLister::print_document_font( const Gtk::TreeModel::iterator &iter ) {
    //   Gtk::TreeModel::Row row = *iter;
    //   if( !row[FontList.onSystem] ) {
    // 	   std::cout << " Not on system: " << row[FontList.font] << std::endl;
    // 	   return false;
    //   }
    //   return true;
    // }
    // font_list_store->foreach_iter( sigc::mem_fun(*this, &FontLister::print_document_font ));

    void
    FontLister::update_font_list( SPDocument* document ) {

      SPObject *r = document->getRoot();
      if( !r ) {
	return;
      }

      /* Clear all old document font-family entries */
      Gtk::TreeModel::iterator iter = font_list_store->get_iter( "0" );
      while( iter != font_list_store->children().end() ) {
	Gtk::TreeModel::Row row = *iter;
	if( !row[FontList.onSystem] ) {
	  // std::cout << " Not on system: " << row[FontList.font] << std::endl;
	  iter = font_list_store->erase( iter );
	} else {
	  // std::cout << " First on system: " << row[FontList.font] << std::endl;
	  break;
	}
      }

      /* Create default styles for use when font-family is unknown on system. */
      static GList *default_styles = NULL;
      if( default_styles == NULL ) {
        default_styles = g_list_append( default_styles, g_strdup("Normal") );
        default_styles = g_list_append( default_styles, g_strdup("Italic") );
        default_styles = g_list_append( default_styles, g_strdup("Bold") );
        default_styles = g_list_append( default_styles, g_strdup("Bold Italic") );
        default_styles = g_list_append( default_styles, g_strdup("Loopy") );
      }

      /* Get "font-family"s used in document. */
      std::list<Glib::ustring> fontfamilies;
      update_font_list_recursive( r, &fontfamilies );

      fontfamilies.sort();
      fontfamilies.unique();
      fontfamilies.reverse();

      /* Insert separator */
      if( !fontfamilies.empty() ) {
	Gtk::TreeModel::iterator treeModelIter = font_list_store->prepend();
	(*treeModelIter)[FontList.font] = "#";
	(*treeModelIter)[FontList.onSystem] = false;
      }

      /* Insert font-family's in document. */
      std::list<Glib::ustring>::iterator i;
      for( i = fontfamilies.begin(); i != fontfamilies.end(); ++i) {

	GList *styles = default_styles;

        /* See if font-family (or first in fallback list) is on system. If so, get styles. */
        std::vector<Glib::ustring> tokens = Glib::Regex::split_simple(",", *i );
        if( !tokens[0].empty() ) {

	  Gtk::TreeModel::iterator iter2 = font_list_store->get_iter( "0" );
	  while( iter2 != font_list_store->children().end() ) {
	    Gtk::TreeModel::Row row = *iter2;
	    if( row[FontList.onSystem] && tokens[0].compare( row[FontList.font] ) == 0 ) {
	      styles = row[FontList.styles];
	      break;
	    }
	    ++iter2;
	  }
	}

	Gtk::TreeModel::iterator treeModelIter = font_list_store->prepend();
	(*treeModelIter)[FontList.font] = reinterpret_cast<const char*>(g_strdup((*i).c_str()));
	(*treeModelIter)[FontList.styles] = styles;
	(*treeModelIter)[FontList.onSystem] = false;
      }
    }

    void
    FontLister::update_font_list_recursive( SPObject *r, std::list<Glib::ustring> *l ) {

      const gchar *style = r->getRepr()->attribute("style");
      if( style != NULL ) {

        std::vector<Glib::ustring> tokens = Glib::Regex::split_simple(";", style );
        for( size_t i=0; i < tokens.size(); ++i ) {

	  Glib::ustring token = tokens[i];
	  size_t found = token.find("font-family:");

	  if( found != Glib::ustring::npos ) {

	    // Remove "font-family:"
	    token.erase(found,12);

	    // Remove any leading single or double quote
	    if( token[0] == '\'' || token[0] == '"' ) {
	      token.erase(0,1);
	    }

	    // Remove any trailing single or double quote
	    if( token[token.length()-1] == '\'' || token[token.length()-1] == '"' ) {
	      token.erase(token.length()-1);
	    }

	    l->push_back( token );
	  }
        }
      }

      for (SPObject *child = r->firstChild(); child; child = child->getNext()) {
        update_font_list_recursive( child, l );
      }
    }

    Gtk::TreePath
    FontLister::get_row_for_font (Glib::ustring family)
    {
      Gtk::TreePath path;

      Gtk::TreeModel::iterator iter = font_list_store->get_iter( "0" );
      while( iter != font_list_store->children().end() ) {

	Gtk::TreeModel::Row row = *iter;

	if( family.compare( row[FontList.font] ) == 0 ) {
	  return font_list_store->get_path( iter );
	}

	++iter;
      }

      throw FAMILY_NOT_FOUND;
    }

    FontLister::~FontLister ()
    {
    };

    const Glib::RefPtr<Gtk::ListStore>
    FontLister::get_font_list () const
    {
        return font_list_store;
    }
}




