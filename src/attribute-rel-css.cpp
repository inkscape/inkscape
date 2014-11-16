/*
 * attribute-rel-css.cpp
 *
 *  Created on: Jul 25, 2011
 *      Author: abhishek
 */

/** \class SPAttributeRelCSS
 *
 * SPAttributeRelCSS class stores the mapping of element->style_properties
 * relationship and provides a static function to access that
 * mapping indirectly(only reading).
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#include "attribute-rel-css.h"

#include "path-prefix.h"
#include "preferences.h"

SPAttributeRelCSS * SPAttributeRelCSS::instance = NULL;
bool SPAttributeRelCSS::foundFileProp = false;
bool SPAttributeRelCSS::foundFileDefault = false;

/*
 * This function checks whether an element -> CSS property pair 
 * is allowed or not
 */
bool SPAttributeRelCSS::findIfValid(Glib::ustring property, Glib::ustring element)
{
    if (SPAttributeRelCSS::instance == NULL) {
        SPAttributeRelCSS::instance = new SPAttributeRelCSS();
    }
    
    // Always valid if data file not found!
    if( !foundFileProp ) return true;

    // Strip of "svg:" from the element's name
    Glib::ustring temp = element;
    if ( temp.find("svg:") != std::string::npos ) {
        temp.erase( temp.find("svg:"), 4 );
    }

    // Don't check for properties with -, role, aria etc. to allow for more accessbility
    // FixMe: Name space list should be created when file read in.
    if (property[0] == '-'
        || property.substr(0,4) == "role"
        || property.substr(0,4) == "aria"
        || property.substr(0,5) == "xmlns"
        || property.substr(0,8) == "inkscape:"
        || property.substr(0,9) == "sodipodi:"
        || property.substr(0,4) == "rdf:"
        || property.substr(0,3) == "cc:"
        || property.substr(0,4) == "ns1:"  // JessyInk
        || property.substr(0,4) == "osb:"  // Open Swatch Book
        || (SPAttributeRelCSS::instance->propertiesOfElements[temp].find(property)
            != SPAttributeRelCSS::instance->propertiesOfElements[temp].end()) ) {
        return true;
    } else {
        //g_warning( "Invalid attribute: %s used on <%s>", property.c_str(), element.c_str() );
        return false;
    }
}

/*
 * This function checks whether an CSS property -> default value 
 * pair is allowed or not
 */
bool SPAttributeRelCSS::findIfDefault(Glib::ustring property, Glib::ustring value)
{
    if (SPAttributeRelCSS::instance == NULL) {
        SPAttributeRelCSS::instance = new SPAttributeRelCSS();
    }

    // Always false if data file not found!
    if( !foundFileDefault ) return false;

    if( instance->defaultValuesOfProps[property] == value) {
        return true;
    } else {
        return false;
    }
}

/*
 * Check if property can be inherited.
 */
bool SPAttributeRelCSS::findIfInherit(Glib::ustring property)
{
    if (SPAttributeRelCSS::instance == NULL) {
        SPAttributeRelCSS::instance = new SPAttributeRelCSS();
    }

    // Always false if data file not found!
    if( !foundFileDefault ) return false;

    return instance->inheritProps[property];
}

/*
 * Check if attribute is a property.
 */
bool SPAttributeRelCSS::findIfProperty(Glib::ustring property)
{
    if (SPAttributeRelCSS::instance == NULL) {
        SPAttributeRelCSS::instance = new SPAttributeRelCSS();
    }

    // Always true if data file not found!
    if( !foundFileProp ) return true;

    return ( instance->defaultValuesOfProps.find( property )
             != instance->defaultValuesOfProps.end() );
}

SPAttributeRelCSS::SPAttributeRelCSS()
{
    // Read data from standard path
    std::string filepath = INKSCAPE_ATTRRELDIR;
    filepath += "/cssprops";

    // Try and load data from filepath
    if (readDataFromFileIn(filepath, SPAttributeRelCSS::prop_element_pair)) {
        foundFileProp = true;
    }
    
    // Read data from standard path
    filepath = INKSCAPE_ATTRRELDIR;
    filepath += "/css_defaults";
    
    // Try and load data from filepath
    if (readDataFromFileIn(filepath, SPAttributeRelCSS::prop_defValue_pair)) {
        foundFileDefault = true;
    }

}

bool SPAttributeRelCSS::readDataFromFileIn(Glib::ustring fileName, storageType type)
{
    std::fstream file;
    file.open(fileName.c_str(), std::ios::in);
    
    if (!file.is_open()) {
        // Display warning for file not open
        g_warning("Could not open the data file for CSS attribute-element map construction: %s", fileName.c_str());
        file.close();
        return false;
    }

    while (!file.eof()) {
        std::stringstream ss;
        std::string s;

        std::getline(file,s,'"');
        std::getline(file,s,'"');
        if (s.size() > 0 && s[0] != '\n') {
            std::string prop = s;
            getline(file,s);
            ss << s;
            
            // Load data to structure that holds element -> set of CSS props
            if (type == SPAttributeRelCSS::prop_element_pair) {
                while (std::getline(ss,s,'"')) {
                    std::string element;
                    std::getline(ss,s,'"');
                    element = s;                               
                    propertiesOfElements[element].insert(prop);
                }
            // Load data to structure that holds CSS prop -> default value    
            } else if (type == SPAttributeRelCSS::prop_defValue_pair) {
                std::string value;
                std::getline(ss,s,'"');
                std::getline(ss,s,'"');
                value = s;
                defaultValuesOfProps[prop] = value;
                std::getline(ss,s,'"');
                std::getline(ss,s,'"');
                gboolean inherit = false;
                if ( s.find( "yes" ) != std::string::npos ) {
                    inherit = true;
                }
                inheritProps[prop] = inherit;
            }
        }
    }
    
    file.close();
    return true;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
