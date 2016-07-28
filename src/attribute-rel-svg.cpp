/*
 * attribute-rel-svg.cpp
 *
 *  Created on: Jul 25, 2011
 *      Author: abhishek
 */

/** \class SPAttributeRelSVG
 *
 * SPAttributeRelSVG class stores the mapping of element->attribute
 * relationship and provides a static function to access that
 * mapping indirectly(only reading).
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <fstream>
#include <sstream>
#include <string>

#include "attribute-rel-svg.h"

#include "path-prefix.h"
#include "preferences.h"

SPAttributeRelSVG * SPAttributeRelSVG::instance = NULL;
bool SPAttributeRelSVG::foundFile = false;

/*
 * This functions checks whether an element -> attribute pair is allowed or not
 */
bool SPAttributeRelSVG::findIfValid(Glib::ustring attribute, Glib::ustring element)
{
    if (SPAttributeRelSVG::instance == NULL) {
        SPAttributeRelSVG::instance = new SPAttributeRelSVG();
    }

    // Always valid if data file not found!
    if( !foundFile ) return true;

    // Strip of "svg:" from the element's name
    Glib::ustring temp = element;
    if ( temp.find("svg:") != std::string::npos ) {
        temp.erase( temp.find("svg:"), 4 );
    }
    
    // Check for attributes with -, role, aria etc. to allow for more accessbility
    if (attribute[0] == '-'
        || attribute.substr(0,4) == "role"
        || attribute.substr(0,4) == "aria"
        || attribute.substr(0,5) == "xmlns"
        || attribute.substr(0,9) == "inkscape:"
        || attribute.substr(0,9) == "sodipodi:"
        || attribute.substr(0,4) == "rdf:"
        || attribute.substr(0,3) == "cc:"
        || attribute.substr(0,4) == "ns1:"  // JessyInk
        || attribute.substr(0,4) == "osb:"  // Open Swatch Book
        || (SPAttributeRelSVG::instance->attributesOfElements[temp].find(attribute)
            != SPAttributeRelSVG::instance->attributesOfElements[temp].end()) ) {
        return true;
    } else {
        //g_warning( "Invalid attribute: %s used on <%s>", attribute.c_str(), element.c_str() );
        return false;
    }
}

/*
 * One timer singleton constructor, to load the element -> attributes data
 * into memory.
 */
SPAttributeRelSVG::SPAttributeRelSVG()
{
    std::fstream f;
    
    // Read data from standard path
    std::string filepath = INKSCAPE_ATTRRELDIR;
    filepath += "/svgprops";

    f.open(filepath.c_str(), std::ios::in);

    if (!f.is_open()) {
        // Display warning for file not open
        g_warning("Could not open the data file for XML attribute-element map construction: %s", filepath.c_str());
        f.close();
        return ;
    }

    foundFile = true;

    while (!f.eof()){
        std::stringstream ss;
        std::string s;

        std::getline(f,s,'"');
        std::getline(f,s,'"');
        if(s.size() > 0 && s[0] != '\n'){
            std::string prop = s;
            getline(f,s);
            ss << s;

            while(std::getline(ss,s,'"')){
                std::string element;
                std::getline(ss,s,'"');
                element = s;
                attributesOfElements[element].insert(prop);
            }
        }
    }
    
    f.close();
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
