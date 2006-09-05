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
 *
 */
class MediaList
{
public:

    /**
     *
     */
    virtual DOMString getMediaText()
        {
        return mediaText;
        }

    /**
     *
     */
    virtual void setMediaText(const DOMString &val) throw (dom::DOMException)
        {
        mediaText = val;
        }

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return items.size();
        }

    /**
     *
     */
    virtual DOMString item(unsigned long index)
        {
        if (index >= items.size())
            return "";
        return items[index];
        }

    /**
     *
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
     *
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
 *
 */
class StyleSheet
{
public:

    /**
     *
     */
    virtual DOMString getType()
        {
        return type;
        }

    /**
     *
     */
    virtual bool getDisabled()
        {
        return disabled;
        }

    /**
     *
     */
    virtual void setDisabled(bool val)
        {
        disabled = val;
        }

    /**
     *
     */
    virtual NodePtr getOwnerNode()
        {
        return ownerNode;
        }

    /**
     *
     */
    virtual StyleSheet *getParentStyleSheet()
        {
        return parentStylesheet;
        }

    /**
     *
     */
    virtual DOMString getHref()
        {
        return href;
        }

    /**
     *
     */
    virtual DOMString getTitle()
        {
        return title;
        }

    /**
     *
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
 *
 */
class StyleSheetList
{
public:

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return sheets.size();
        }

    /**
     *
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
    virtual ~StyleSheetList() {}

protected:

    std::vector<StyleSheet *>sheets;

};





/*#########################################################################
## LinkStyle
#########################################################################*/

/**
 *
 */
class LinkStyle
{
public:

    /**
     *
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
    virtual ~LinkStyle() {}

protected:

    StyleSheet sheet;

};




/*#########################################################################
## DocumentStyle
#########################################################################*/

/**
 *
 */
class DocumentStyle
{
public:

    /**
     *
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

