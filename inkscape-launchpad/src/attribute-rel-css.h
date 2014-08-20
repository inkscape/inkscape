#ifndef __SP_ATTRIBUTE_REL_CSS_H__
#define __SP_ATTRIBUTE_REL_CSS_H__

/*
 * attribute-rel-css.h
 *
 *  Created on: Jul 25, 2011
 *      Author: abhishek
 */

#include <string>
#include <map>
#include <set>
#include <glibmm/ustring.h>

// This data structure stores the valid (element -> set of CSS properties) pair
typedef std::map<Glib::ustring, std::set<Glib::ustring> > hashList;

/*
 * Utility class that helps check whether a given element -> CSS property is 
 * valid or not and whether the value assumed by a CSS property has a default
 * value.
 */
class SPAttributeRelCSS {
public:
    static bool findIfValid(Glib::ustring property, Glib::ustring element);
    static bool findIfDefault(Glib::ustring property, Glib::ustring value);
    static bool findIfInherit(Glib::ustring property);
    static bool findIfProperty(Glib::ustring property);

private:
    SPAttributeRelCSS();
    SPAttributeRelCSS(const SPAttributeRelCSS&);
    SPAttributeRelCSS& operator= (const SPAttributeRelCSS&);

private:
    /* 
     * Allows checking whether data loading is to be done for element -> CSS properties
     * or CSS property -> default value.
     */
    enum storageType {
        prop_element_pair,
        prop_defValue_pair
    };
    static SPAttributeRelCSS *instance;
    static bool foundFileProp;
    static bool foundFileDefault;
    hashList propertiesOfElements;

    // Data structure to store CSS property and default value pair
    std::map<Glib::ustring, Glib::ustring> defaultValuesOfProps;
    std::map<Glib::ustring, gboolean> inheritProps;
    bool readDataFromFileIn(Glib::ustring fileName, storageType type);
};


#endif /* __SP_ATTRIBUTE_REL_CSS_H__ */

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
