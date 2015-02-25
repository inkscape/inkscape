/*
 * SVG <text> and <tspan> implementation
 *
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Jon A. Cruz <jon@joncruz.org>
 *   Abhishek Sharma
 *
 * Copyright (C) 1999-2002 Lauris Kaplinski
 * Copyright (C) 2000-2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

/*
 * fixme:
 *
 * These subcomponents should not be items, or alternately
 * we have to invent set of flags to mark, whether standard
 * attributes are applicable to given item (I even like this
 * idea somewhat - Lauris)
 *
 */

#include "sp-string.h"
#include "style.h"

#include "xml/repr.h"

#include <iostream>

/*#####################################################
#  SPSTRING
#####################################################*/

SPString::SPString() : SPObject() {
}

SPString::~SPString() {
}

void SPString::build(SPDocument *doc, Inkscape::XML::Node *repr) {
    SPString* object = this;
    object->read_content();

    SPObject::build(doc, repr);
}

void SPString::release() {
    SPObject::release();
}


void SPString::read_content() {

    SPString* object = this;
    SPString *string = SP_STRING(object);

    string->string.clear();

    //XML Tree being used directly here while it shouldn't be.
    gchar const *xml_string = string->getRepr()->content();

    // std::cout << ">" << (xml_string?xml_string:"Null") << "<" << std::endl;

    // SVG2/CSS Text Level 3 'white-space' has five values.
    // See: http://dev.w3.org/csswg/css-text/#white-space
    //            |  New Lines |  Spaces/Tabs | Text Wrapping
    //   ---------|------------|--------------|--------------
    //   normal   |  Collapes  |   Collapse   |     Wrap
    //   pre      |  Preserve  |   Preserve   |   No Wrap
    //   nowrap   |  Collapse  |   Collapse   |   No Wrap
    //   pre-wrap |  Preserve  |   Preserve   |     Wrap
    //   pre-line |  Preserve  |   Collapse   |     Wrap

    // 'xml:space' has two values:
    //   'default' which corresponds to 'normal' (without wrapping).
    //   'preserve' which corresponds to 'pre' except new lines are converted to spaces.
    //  See algorithms described in svg 1.1 section 10.15

    bool collapse_space = true;
    bool collapse_line  = true;
    bool is_css         = false;

    // Strings don't have style, check parent for style
    if( object->parent && object->parent->style ) {
        if( object->parent->style->white_space.computed == SP_CSS_WHITE_SPACE_PRE     ||
            object->parent->style->white_space.computed == SP_CSS_WHITE_SPACE_PREWRAP ||
            object->parent->style->white_space.computed == SP_CSS_WHITE_SPACE_PRELINE  ) {
            collapse_line = false;
        }
        if( object->parent->style->white_space.computed == SP_CSS_WHITE_SPACE_PRE     ||
            object->parent->style->white_space.computed == SP_CSS_WHITE_SPACE_PREWRAP ) {
            collapse_space = false;
        }
        if( object->parent->style->white_space.computed != SP_CSS_WHITE_SPACE_NORMAL ) {
            is_css = true; // If white-space not normal, we assume white-space is set.
        }
    }
    if( !is_css ) {
        // SVG 2: Use 'xml:space' only if 'white-space' not 'normal'.
        if (object->xml_space.value == SP_XML_SPACE_PRESERVE) {
            collapse_space = false;
        }
    }

    bool white_space = false;
    for ( ; *xml_string ; xml_string = g_utf8_next_char(xml_string) ) {

        gunichar c = g_utf8_get_char(xml_string);
        switch (c) {
            case 0xd: // Carriage return
                // XML Parsers convert 0xa, 0xd, 0xD 0xA to 0xA. CSS also follows this rule so we
                // should never see 0xd.
                std::cerr << "SPString: Carriage Return found! Argh!" << std::endl;
                continue;
                break;
            case 0xa: // Line feed
                if( collapse_line ) {
                    if( !is_css && collapse_space ) continue; // xml:space == 'default' strips LFs.
                    white_space = true;        // Convert to space and collapse
                } else {
                    string->string += c;       // Preserve line feed
                    continue;
                }
                break;
            case '\t': // Tab
                if( collapse_space ) {
                    white_space = true;        // Convert to space and collapse
                } else {
                    string->string += c;       // Preserve tab
                    continue;
                }
                break;
            case ' ': // Space
                if( collapse_space ) {
                    white_space = true;        // Collapse white space
                } else {
                    string->string += c;       // Preserve space
                    continue;
                }
                break;
            default:
                if( white_space && (!string->string.empty() || (object->getPrev() != NULL))) {
                    string->string += ' ';
                }
                string->string += c;
                white_space = false;

        } // End switch
    } // End loop

    // Insert white space at end if more text follows
    if (white_space && object->getRepr()->next() != NULL) { // can't use SPObject::getNext() when the SPObject tree is still being built
        string->string += ' ';
    }

    // std::cout << ">" << string->string << "<" << std::endl;
    object->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
}

void SPString::update(SPCtx * /*ctx*/, unsigned /*flags*/) {
//    SPObject::onUpdate(ctx, flags);

    // if (flags & (SP_OBJECT_STYLE_MODIFIED_FLAG | SP_OBJECT_MODIFIED_FLAG)) {
    //     /* Parent style or we ourselves changed, so recalculate */
    //     flags &= ~SP_OBJECT_USER_MODIFIED_FLAG_B; // won't be "just a transformation" anymore, we're going to recompute "x" and "y" attributes
    // }
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
