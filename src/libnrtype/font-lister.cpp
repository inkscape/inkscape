#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <libnrtype/font-instance.h>
#include <libnrtype/TextWrapper.h>
#include <libnrtype/one-glyph.h>

#include <glibmm.h>
#include <gtkmm.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>

#include "font-lister.h"
#include "FontFactory.h"

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
                
                font_list_store_iter_map.insert(std::make_pair(familyName, Gtk::TreePath(treeModelIter)));
            }
        }
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




