#ifndef __SP_ATTRIBUTE_REL_SVG_H__
#define __SP_ATTRIBUTE_REL_SVG_H__

/*
 * attribute-rel-svg.h
 *
 *  Created on: Jul 25, 2011
 *      Author: abhishek
 */

#include <string>
#include <map>
#include <set>
#include <glibmm/ustring.h>

// This data structure stores the valid (element -> set of attributes) pair
typedef std::map<Glib::ustring, std::set<Glib::ustring> > hashList;

/* 
 * Utility class to check whether a combination of element and attribute
 * is valid or not.
 */
class SPAttributeRelSVG {
public:
    static bool findIfValid(Glib::ustring attribute, Glib::ustring element);

private:
    SPAttributeRelSVG();
    SPAttributeRelSVG(const SPAttributeRelSVG&);
    SPAttributeRelSVG& operator= (const SPAttributeRelSVG&);

private:
    static SPAttributeRelSVG *instance;
    static bool foundFile;
    hashList attributesOfElements;
};


#endif /* __SP_ATTRIBUTE_REL_SVG_H__ */

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
