#ifndef __STYLESHEETS_H__
#define __STYLESHEETS_H__

/**
 * Phoebe DOM Implementation.
 *
 * This is a C++ approximation of the W3C DOM model, which follows
 * fairly closely the specifications in the various .idl files, copies of
 * which are provided for reference.  Most important is this one:
 *
 * http://www.w3.org/TR/2004/REC-DOM-Level-3-Core-20040407/idl-definitions.html
 *
 * Authors:
 *   Bob Jamison
 *
 * Copyright (C) 2005 Bob Jamison
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *  
 * =========================================================================
 * NOTES    
 * 
 * Views, Stylesheets and CSS are DOM Level 2 for the purposes of supporting
 * SVG.  Be prepared in the future when they make Level 3 and SVG is likewise
 * updated.  The API here and many of the comments come from this document:
 * http://www.w3.org/TR/DOM-Level-2-Style/stylesheets.html     
 */


#include "dom.h"

#include <vector>


namespace org
{
namespace w3c
{
namespace dom
{
namespace stylesheets
{



//Make local definitions
typedef dom::DOMString DOMString;
typedef dom::Node Node;



/*#########################################################################
## MediaList
#########################################################################*/

/**
 * The MediaList interface provides the abstraction of an ordered collection of 
 * media, without defining or constraining how this collection is implemented. An 
 * empty list is the same as a list that contains the medium "all".
 * 
 * The items in the MediaList are accessible via an integral index, starting from 
 * 0.
 */
class MediaList
{
public:

    /**
     * The parsable textual representation of the media list. This is a
     * comma-separated list of media.
     */
    virtual DOMString getMediaText()
        {
        return mediaText;
        }

    /**
     * The parsable textual representation of the media list. This is a
     * comma-separated list of media.
     */
    virtual void setMediaText(const DOMString &val) throw (dom::DOMException)
        {
        mediaText = val;
        }

    /**
     * The number of media in the list. The range of valid media is 0 to
     * 	 length-1 inclusive.
     */
    virtual unsigned long getLength()
        {
        return items.size();
        }

    /**
     * Returns the indexth in the list. If index is greater than or equal to
     * 	 the number of media in the list, this returns null. 
     */
    virtual DOMString item(unsigned long index)
        {
        if (index >= items.size())
            return "";
        return items[index];
        }

    /**
     * Deletes the medium indicated by oldMedium from the list. 
     */
    virtual void deleteMedium(const DOMString& oldMedium)
                              throw (dom::DOMException)
        {
        std::vector<DOMString>::iterator iter;
        for (iter=items.begin() ; iter!=items.end() ; iter++)
            {
            if (*iter == oldMedium)
                items.erase(iter);
            }
        }

    /**
     * Adds the medium newMedium to the end of the list. If the newMedium
     * 	 is already used, it is first removed. 
     */
    virtual void appendMedium(const DOMString& newMedium)
                               throw (dom::DOMException)
        {
        items.push_back(newMedium);
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
   MediaList() {}


    /**
     *
     */
   MediaList(const MediaList &other)
       {
       assign(other);
       }

    /**
     *
     */
   MediaList &operator=(const MediaList &other)
       {
       assign(other);
       return *this;
       }

    /**
     *
     */
   void assign(const MediaList &other)
       {
       mediaText = other.mediaText;
       items     = other.items;
       }

    /**
     *
     */
    virtual ~MediaList() {}

protected:

    DOMString mediaText;

    std::vector<DOMString>items;
};



/*#########################################################################
## StyleSheet
#########################################################################*/

/**
 * The StyleSheet interface is the abstract base interface for any type of style 
 * sheet. It represents a single style sheet associated with a structured 
 * document. In HTML, the StyleSheet interface represents either an external 
 * style sheet, included via the HTML LINK element, or an inline STYLE element. 
 * In XML, this interface represents an external style sheet, included via a 
 * style sheet processing instruction.
 */
class StyleSheet
{
public:

    /**
     * This specifies the style sheet language for this style sheet. The style sheet 
     * language is specified as a content type (e.g. "text/css"). The content type is 
     * often specified in the ownerNode. Also see the type attribute definition for 
     * the LINK element in HTML 4.0, and the type pseudo-attribute for the XML style 
     * sheet processing instruction.
     */
    virtual DOMString getType()
        {
        return type;
        }

    /**
     * false if the style sheet is applied to the document. true if it is not. 
     * Modifying this attribute may cause a new resolution of style for the document. 
     * A stylesheet only applies if both an appropriate medium definition is present 
     * and the disabled attribute is false. So, if the media doesn't apply to the 
     * current user agent, the disabled attribute is ignored.
     */
    virtual bool getDisabled()
        {
        return disabled;
        }

    /**
     * Sets the value above.
     */
    virtual void setDisabled(bool val)
        {
        disabled = val;
        }

    /**
     * The node that associates this style sheet with the document. For HTML, this 
     * may be the corresponding LINK or STYLE element. For XML, it may be the linking 
     * processing instruction. For style sheets that are included by other style 
     * sheets, the value of this attribute is null.
     */
    virtual NodePtr getOwnerNode()
        {
        return ownerNode;
        }

    /**
     * For style sheet languages that support the concept of style sheet inclusion, 
     * this attribute represents the including style sheet, if one exists. If the 
     * style sheet is a top-level style sheet, or the style sheet language does not 
     * support inclusion, the value of this attribute is null.
     */
    virtual StyleSheet *getParentStyleSheet()
        {
        return parentStylesheet;
        }

    /**
     * If the style sheet is a linked style sheet, the value of its attribute is its 
     * location. For inline style sheets, the value of this attribute is null. See 
     * the href attribute definition for the LINK element in HTML 4.0, and the href 
     * pseudo-attribute for the XML style sheet processing instruction.
     */
    virtual DOMString getHref()
        {
        return href;
        }

    /**
     * The advisory title. The title is often specified in the ownerNode. See the 
     * title attribute definition for the LINK element in HTML 4.0, and the title 
     * pseudo-attribute for the XML style sheet processing instruction.
     */
    virtual DOMString getTitle()
        {
        return title;
        }

    /**
     * The intended destination media for style information. The media is often 
     * specified in the ownerNode. If no media has been specified, the MediaList will 
     * be empty. See the media attribute definition for the LINK element in HTML 4.0, 
     * and the media pseudo-attribute for the XML style sheet processing 
     * instruction . Modifying the media list may cause a change to the attribute 
     * disabled.
     */
    virtual MediaList &getMedia()
        {
        MediaList &mediaListRef = mediaList;
        return mediaListRef;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    StyleSheet()
        {
        type             = "";
        disabled         = false;
        ownerNode        = NULL;
        parentStylesheet = NULL;
        href             = "";
        title            = "";
        }


    /**
     *
     */
    StyleSheet(const StyleSheet &other)
        {
        assign(other);
        }

    /**
     *
     */
    StyleSheet &operator=(const StyleSheet &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const StyleSheet &other)
        {
        type             = other.type;
        disabled         = other.disabled;
        ownerNode        = other.ownerNode;
        parentStylesheet = other.parentStylesheet;
        href             = other.href;
        title            = other.title;
        mediaList        = other.mediaList;
        }

    /**
     *
     */
    virtual ~StyleSheet() {}

protected:

    DOMString type;

    bool disabled;

    NodePtr ownerNode;

    StyleSheet *parentStylesheet;

    DOMString href;

    DOMString title;

    MediaList mediaList;
};




/*#########################################################################
## StyleSheetList
#########################################################################*/

/**
 * The StyleSheetList interface provides the abstraction of an ordered collection 
 * of style sheets.
 * 
 * The items in the StyleSheetList are accessible via an integral index, starting 
 * from 0.
 */
class StyleSheetList
{
public:

    /**
     * The number of StyleSheets in the list. The range of valid child stylesheet 
     * indices is 0 to length-1 inclusive.
     */
    virtual unsigned long getLength()
        {
        return sheets.size();
        }

    /**
     * Used to retrieve a style sheet by ordinal index. If index is greater than or 
     * equal to the number of style sheets in the list, this returns null.
     */
    virtual StyleSheet *item(unsigned long index)
        {
        if (index >= sheets.size())
            return NULL;
        return sheets[index];
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    StyleSheetList() {}

    /**
     *
     */
    StyleSheetList(const StyleSheetList &other)
        {
        sheets = other.sheets;
        }

    /**
     *
     */
    StyleSheetList &operator=(const StyleSheetList &other)
        {
        sheets = other.sheets;
        return *this;
        }

    /**
     *
     */
    virtual ~StyleSheetList() {}

protected:

    std::vector<StyleSheet *>sheets;

};





/*#########################################################################
## LinkStyle
#########################################################################*/

/**
 * The LinkStyle interface provides a mechanism by which a style sheet can be 
 * retrieved from the node responsible for linking it into a document. An 
 * instance of the LinkStyle interface can be obtained using binding-specific 
 * casting methods on an instance of a linking node (HTMLLinkElement, 
 * HTMLStyleElement or ProcessingInstruction in DOM Level 2).
 */
class LinkStyle
{
public:

    /**
     * The style sheet.
     */
    virtual StyleSheet &getSheet()
        {
        StyleSheet &sheetRef = sheet;
        return sheetRef;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    LinkStyle()
        {
        }

    /**
     *
     */
    LinkStyle(const LinkStyle &other)
        {
        sheet = other.sheet;
        }

    /**
     *
     */
    LinkStyle &operator=(const LinkStyle &other)
        {
        sheet = other.sheet;
        return *this;
        }

    /**
     *
     */
    virtual ~LinkStyle() {}

protected:

    StyleSheet sheet;

};




/*#########################################################################
## DocumentStyle
#########################################################################*/

/**
 * The DocumentStyle interface provides a mechanism by which the style sheets 
 * embedded in a document can be retrieved. The expectation is that an instance 
 * of the DocumentStyle interface can be obtained by using binding-specific 
 * casting methods on an instance of the Document interface.
 */
class DocumentStyle
{
public:

    /**
     * A list containing all the style sheets explicitly linked into or embedded in a 
     * document. For HTML documents, this includes external style sheets, included 
     * via the HTML LINK element, and inline STYLE elements. In XML, this includes 
     * external style sheets, included via style sheet processing instructions (see 
     * [XML-StyleSheet]).
     */
    virtual StyleSheetList &getStyleSheets()
        {
        StyleSheetList &listRef = styleSheets;
        return listRef;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DocumentStyle() {}

    /**
     *
     */
    DocumentStyle(const DocumentStyle &other)
        {
        styleSheets = other.styleSheets;
        }

    /**
     *
     */
    DocumentStyle &operator=(const DocumentStyle &other)
        {
        styleSheets = other.styleSheets;
        return *this;
        }

    /**
     *
     */
    virtual ~DocumentStyle() {}

protected:

    StyleSheetList styleSheets;
};





}  //namespace stylesheets
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif // __STYLESHEETS_H__
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

