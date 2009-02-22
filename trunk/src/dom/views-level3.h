#ifndef __VIEWS_LEVEL3_H__
#define __VIEWS_LEVEL3_H__

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
 * Copyright (C) 2005-2008 Bob Jamison
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

/**
 * Currently CSS is not at level 3, so we will probably not use this
 * Level 3 Views implementation.  Rather, we'll regress back to Level 2.
 * This should not affect using the rest of DOM Core 3
 */ 

#include "dom.h"



namespace org
{
namespace w3c
{
namespace dom
{
namespace views
{


//local aliases
typedef dom::Node Node;
typedef dom::DOMString DOMString;

//forward declarations
class Segment;
class VisualResource;
class VisualCharacter;
class VisualCharacterRun;
class VisualFrame;
class VisualImage;
class VisualFormButton;
class VisualFormField;




/*#########################################################################
## Match
#########################################################################*/

/**
 *
 */
class Match
{
public:

    typedef enum
        {
        IS_EQUAL                       = 0,
        IS_NOT_EQUAL                   = 1,
        INT_PRECEDES                   = 2,
        INT_PRECEDES_OR_EQUALS         = 3,
        INT_FOLLOWS                    = 4,
        INT_FOLLOWS_OR_EQUALS          = 5,
        STR_STARTS_WITH                = 6,
        STR_ENDS_WITH                  = 7,
        STR_CONTAINS                   = 8,
        SET_ANY                        = 9,
        SET_ALL                        = 10,
        SET_NOT_ANY                    = 11,
        SET_NOT_ALL                    = 12
        } MatchTestGroup;

    /**
     *
     */
    virtual unsigned short test()
        { return IS_NOT_EQUAL; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Match() {}

    /**
     *
     */
    Match(const Match &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~Match() {}
};



/*#########################################################################
## MatchString
#########################################################################*/

/**
 *
 */
class MatchString : virtual public Match
{
public:

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    /**
     *
     */
    virtual DOMString getValue()
        { return value; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MatchString() {}

    /**
     *
     */
    MatchString(const MatchString &other) : Match(other)
        {
        name  = other.name;
        value = other.value;
        }

    /**
     *
     */
    virtual ~MatchString() {}

protected:

    DOMString name;
    DOMString value;


};



/*#########################################################################
## MatchInteger
#########################################################################*/

/**
 *
 */
class MatchInteger : virtual public Match
{
public:

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    /**
     *
     */
    virtual long getValue()
        { return value; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MatchInteger() {}

    /**
     *
     */
    MatchInteger(const MatchInteger &other) : Match(other)
        {
        name  = other.name;
        value = other.value;
        }

    /**
     *
     */
    virtual ~MatchInteger() {}

protected:

    DOMString name;
    long value;
};



/*#########################################################################
## MatchBoolean
#########################################################################*/

/**
 *
 */
class MatchBoolean : virtual public Match
{
public:

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    /**
     *
     */
    virtual bool getValue()
        { return value; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MatchBoolean() {}

    /**
     *
     */
    MatchBoolean(const MatchBoolean &other) : Match(other)
        {
        name  = other.name;
        value = other.value;
        }

    /**
     *
     */
    virtual ~MatchBoolean() {}

protected:

    DOMString name;
    bool value;
};



/*#########################################################################
## MatchContent
#########################################################################*/

/**
 *
 */
class MatchContent : virtual public Match
{
public:

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    /**
     *
     */
    virtual NodePtr getNodeArg()
        { return nodeArg; }


    /**
     *
     */
    virtual unsigned long getOffset()
        { return offset; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MatchContent()
        {
        nodeArg = NULL;
        offset  = 0L;
        }

    /**
     *
     */
    MatchContent(const MatchContent &other) : Match(other)
        {
        name    = other.name;
        nodeArg = other.nodeArg;
        offset  = other.offset;
        }

    /**
     *
     */
    virtual ~MatchContent() {}

protected:

    DOMString     name;
    NodePtr       nodeArg;
    unsigned long offset;



};



/*#########################################################################
## MatchSet
#########################################################################*/

/**
 *
 */
class MatchSet : virtual public Match
{
public:

    /**
     *
     */
    virtual NodePtr getNodeArg()
        { return nodeArg; }

    /**
     *
     */
    virtual void addMatch(const Match &match)
        { matches.push_back(match); }

    /**
     *
     */
    virtual Match getMatch(unsigned long index)
        {
        if (index >= matches.size())
            {
            Match match;
            return match;
            }
        return matches[index];
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    MatchSet()
        {
        nodeArg = NULL;
        }

    /**
     *
     */
    MatchSet(const MatchSet &other) : Match(other)
        {
        nodeArg = other.nodeArg;
        matches = other.matches;
        }

    /**
     *
     */
    virtual ~MatchSet() {}

protected:

    NodePtr nodeArg;

    std::vector<Match> matches;

};



/*#########################################################################
## Item
#########################################################################*/

/**
 *
 */
class Item
{
public:

    /**
     *
     */
    virtual bool getExists()
        { return exists; }

    /**
     *
     */
    virtual DOMString getName()
        { return name; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Item() {}

    /**
     *
     */
    Item(const Item &other)
        {
        exists = other.exists;
        name   = other.name;
        }

    /**
     *
     */
    virtual ~Item() {}

protected:

    bool exists;

    DOMString name;


};



/*#########################################################################
## StringItem
#########################################################################*/

/**
 *
 */
class StringItem : virtual public Item
{
public:

    /**
     *
     */
    virtual DOMString getValue()
        { return value; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    StringItem() {}

    /**
     *
     */
    StringItem(const StringItem &other) : Item(other)
        {
        value = other.value;
        }

    /**
     *
     */
    virtual ~StringItem() {}

protected:

    DOMString value;


};



/*#########################################################################
## IntegerItem
#########################################################################*/

/**
 *
 */
class IntegerItem : virtual public Item
{
public:


    /**
     *
     */
    virtual long getValue()
        { return value; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    IntegerItem() {}

    /**
     *
     */
    IntegerItem(const IntegerItem &other) : Item(other)
        {
        value = other.value;
        }

    /**
     *
     */
    virtual ~IntegerItem() {}

protected:

    long value;

};


/*#########################################################################
## BooleanItem
#########################################################################*/

/**
 *
 */
class BooleanItem : virtual public Item
{
public:

    /**
     *
     */
    virtual bool getValue()
        { return value; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    BooleanItem() {}

    /**
     *
     */
    BooleanItem(const BooleanItem &other) : Item(other)
        {
        value = other.value;
        }

    /**
     *
     */
    virtual ~BooleanItem() {}

protected:

    bool value;

};


/*#########################################################################
## ContentItem
#########################################################################*/

/**
 *
 */
class ContentItem : virtual public Item
{
public:

    /**
     *
     */
    virtual NodePtr getNodeArg()
        { return nodeArg; }

    /**
     *
     */
    virtual unsigned long getOffset()
        { return offset; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ContentItem()
        {
        nodeArg = NULL;
        }

    /**
     *
     */
    ContentItem(const ContentItem &other) : Item(other)
        {
        nodeArg = other.nodeArg;
        offset  = other.offset;
        }

    /**
     *
     */
    virtual ~ContentItem() {}

protected:

    NodePtr nodeArg;
    long offset;


};







/*#########################################################################
## Segment
#########################################################################*/

/**
 *
 */
class Segment : virtual public Match
{
public:

    /**
     *
     */
    virtual Match getCriteria()
        { return criteria; }

    /**
     *
     */
    virtual void setCriteria(const Match &val)
        { criteria = val; }


    /**
     *
     */
    virtual DOMString getOrder()
        { return order; }

    /**
     *
     */
    virtual void setOrder(const DOMString &val)
        { order = val; }

    /**
     *
     */
    virtual MatchString createMatchString(unsigned short /*test*/,
                                          const DOMString &/*name*/,
                                          const DOMString &/*value*/)
        {
        MatchString ret;
        return ret;
        }

    /**
     *
     */
    virtual MatchInteger createMatchInteger(unsigned short /*test*/,
                                            const DOMString &/*name*/,
                                            long /*value*/)
        {
        MatchInteger ret;
        return ret;
        }

    /**
     *
     */
    virtual MatchBoolean createMatchBoolean(unsigned short /*test*/,
                                            const DOMString &/*name*/,
                                            bool /*value*/)
        {
        MatchBoolean ret;
        return ret;
        }

    /**
     *
     */
    virtual MatchContent createMatchContent(unsigned short /*test*/,
                                            const DOMString &/*name*/,
                                            unsigned long /*offset*/,
                                            const NodePtr /*nodeArg*/)
        {
        MatchContent ret;
        return ret;
        }

    /**
     *
     */
    virtual MatchSet createMatchSet(unsigned short /*test*/)
        {
        MatchSet ret;
        return ret;
        }

    /**
     *
     */
    virtual StringItem createStringItem(const DOMString &/*name*/)
        {
        StringItem ret;
        return ret;
        }

    /**
     *
     */
    virtual IntegerItem createIntegerItem(const DOMString &/*name*/)
        {
        IntegerItem ret;
        return ret;
        }

    /**
     *
     */
    virtual BooleanItem createBooleanItem(const DOMString &/*name*/)
        {
        BooleanItem ret;
        return ret;
        }

    /**
     *
     */
    virtual ContentItem createContentItem(const DOMString &/*name*/)
        {
        ContentItem ret;
        return ret;
        }

    /**
     *
     */
    virtual void addItem(const Item &item)
        {
        items.push_back(item);
        }

    /**
     *
     */
    virtual Item getItem(unsigned long index)
        {
        if (index >= items.size())
            {
            Item item;
            return item;
            }
        return items[index];
        }

    /**
     *
     */
    virtual bool getNext()
        {
        return false;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Segment() {}

    /**
     *
     */
    Segment(const Segment &other) : Match(other)
        {
        criteria = other.criteria;
        order    = other.order;
        items    = other.items;
        }

    /**
     *
     */
    virtual ~Segment() {}

protected:

    Match criteria;

    DOMString order;

    std::vector<Item> items;

};












/*#########################################################################
## View
#########################################################################*/

/**
 *
 */
class View
{
public:

    /**
     *
     */
    virtual void select(const NodePtr /*boundary*/,
                        unsigned long /*offset*/,
                        bool /*extend*/,
                        bool /*add*/)
        {
        }

    /**
     *
     */
    virtual Segment createSegment()
        {
        Segment ret;
        return ret;
        }

    /**
     *
     */
    virtual bool matchFirstSegment(Segment &/*todo*/) //inout parm, not const
                                        throw(dom::DOMException)
        {
        return false;
        }

    /**
     *
     */
    virtual long getIntegerProperty(const DOMString &/*name*/)
                                        throw(dom::DOMException)
        {
        long val=0;
        return val;
        }

    /**
     *
     */
    virtual DOMString getStringProperty(const DOMString &/*name*/)
                                        throw(dom::DOMException)
        {
        DOMString val;
        return val;
        }

    /**
     *
     */
    virtual bool getBooleanProperty(bool /*name*/)
                                        throw(dom::DOMException)
        {
        bool val=false;
        return val;
        }

    /**
     *
     */
    virtual NodePtr getContentPropertyNode(const DOMString &/*name*/)
                                        throw(dom::DOMException)
        {
        NodePtr val = NULL;
        return val;
        }

    /**
     *
     */
    virtual unsigned long getContentPropertyOffset(const DOMString &/*name*/)
                                        throw(dom::DOMException)
        {
        long val=0;
        return val;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    View() {}

    /**
     *
     */
    View(const View &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~View() {}
};


/*#########################################################################
## VisualResource
#########################################################################*/

/**
 *
 */
class VisualResource
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualResource() {}

    /**
     *
     */
    VisualResource(const VisualResource &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~VisualResource() {}
};


/*#########################################################################
## VisualFont
#########################################################################*/

/**
 *
 */
class VisualFont : virtual public VisualResource
{
public:

    /**
     *
     */
    virtual DOMString getMatchFontName()
        { return matchFontName; }

    /**
     *
     */
    virtual void setMatchFontName(const DOMString &val)
        { matchFontName = val; }

    /**
     *
     */
    virtual bool getExists()
        { return true; }

    /**
     *
     */
    virtual DOMString getFontName()
        { return fontName; }

    /**
     *
     */
    virtual bool getNext()
        { return next; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualFont() {}

    /**
     *
     */
    VisualFont(const VisualFont &other) : VisualResource(other)
        {
        matchFontName = other.matchFontName;
        fontName      = other.fontName;
        next          = other.next;
        }

    /**
     *
     */
    virtual ~VisualFont() {}

protected:

    DOMString matchFontName;
    DOMString fontName;
    bool next;


};


/*#########################################################################
## VisualSegment
#########################################################################*/

/**
 *
 */
class VisualSegment : virtual public VisualResource
{
public:


    /**
     *
     */
    virtual bool getMatchPosition()
        { return matchPosition; }

    /**
     *
     */
    virtual void setMatchPosition(bool val)
        { matchPosition = val; }

    /**
     *
     */
    virtual bool getMatchInside()
        { return matchInside; }

    /**
     *
     */
    virtual void setMatchInside(bool val)
        { matchInside = val; }

    /**
     *
     */
    virtual bool getMatchContaining()
        { return matchContaining; }

    /**
     *
     */
    virtual void setMatchContaining(bool val)
        { matchContaining = val; }

    /**
     *
     */
    virtual long getMatchX()
        { return matchX; }

    /**
     *
     */
    virtual void setMatchX(long val)
        { matchX = val; }

    /**
     *
     */
    virtual long getMatchY()
        { return matchY; }

    /**
     *
     */
    virtual void setMatchY(long val)
        { matchY = val; }

    /**
     *
     */
    virtual long getMatchXR()
        { return matchXR; }

    /**
     *
     */
    virtual void setMatchXR(long val)
        { matchXR = val; }

    /**
     *
     */
    virtual long getMatchYR()
        { return matchYR; }

    /**
     *
     */
    virtual void setMatchYR(long val)
        { matchYR = val; }

    /**
     *
     */
    virtual bool getMatchContent()
        { return matchContent; }

    /**
     *
     */
    virtual void setMatchContent(bool val)
        { matchContent = val; }

    /**
     *
     */
    virtual bool getMatchRange()
        { return matchRange; }

    /**
     *
     */
    virtual void setMatchRange(bool val)
        { matchRange = val; }

    /**
     *
     */
    virtual NodePtr getMatchNode()
        { return matchNode; }

    /**
     *
     */
    virtual void setMatchNode(const NodePtr val)
        { matchNode = (NodePtr )val; }

    /**
     *
     */
    virtual unsigned long getMatchOffset()
        { return matchOffset; }

    /**
     *
     */
    virtual void setMatchOffset(unsigned long val)
        { matchOffset = val; }

    /**
     *
     */
    virtual NodePtr getMatchNodeR()
        { return matchNodeR; }

    /**
     *
     */
    virtual void setMatchNodeR(const NodePtr val)
        { matchNodeR = (NodePtr )val; }

    /**
     *
     */
    virtual unsigned long getMatchOffsetR()
        { return matchOffsetR; }

    /**
     *
     */
    virtual void setMatchOffsetR(unsigned long val)
        { matchOffsetR = val; }

    /**
     *
     */
    virtual bool getMatchContainsSelected()
        { return matchContainsSelected; }

    /**
     *
     */
    virtual void setMatchContainsSelected(bool val)
        { matchContainsSelected = val; }

    /**
     *
     */
    virtual bool getMatchContainsVisible()
        { return matchContainsVisible; }

    /**
     *
     */
    virtual void setMatchContainsVisible(bool val)
        { matchContainsVisible = val; }


    /**
     *
     */
    virtual bool getExists()
        { return exists; }

    /**
     *
     */
    virtual NodePtr getStartNode()
        { return startNode; }

    /**
     *
     */
    virtual unsigned long getStartOffset()
        { return startOffset; }

    /**
     *
     */
    virtual NodePtr getEndNode()
        { return endNode; }

    /**
     *
     */
    virtual unsigned long getEndOffset()
        { return endOffset; }

    /**
     *
     */
    virtual long getTopOffset()
        { return topOffset; }

    /**
     *
     */
    virtual long getBottomOffset()
        { return bottomOffset; }

    /**
     *
     */
    virtual long getLeftOffset()
        { return leftOffset; }

    /**
     *
     */
    virtual long  getRightOffset()
        { return rightOffset; }

    /**
     *
     */
    virtual unsigned long getWidth()
        { return width; }

    /**
     *
     */
    virtual unsigned long getHeight()
        { return height; }

    /**
     *
     */
    virtual bool getSelected()
        { return selected; }

    /**
     *
     */
    virtual bool getVisible()
        { return visible; }

    /**
     *
     */
    virtual unsigned long getForegroundColor()
        { return foregroundColor; }

    /**
     *
     */
    virtual unsigned long getBackgroundColor()
        { return backgroundColor; }

    /**
     *
     */
    virtual DOMString getFontName()
        { return fontName; }

    /**
     *
     */
    virtual DOMString getFontHeight()
        { return fontHeight; }

    /**
     *
     */
    virtual bool getNext()
        { return next; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualSegment() {}

    /**
     *
     */
    VisualSegment(const VisualSegment &other) : VisualResource(other)
        {
        matchPosition         = other.matchPosition;
        matchInside           = other.matchInside;
        matchContaining       = other.matchContaining;
        matchX                = other.matchX;
        matchY                = other.matchY;
        matchXR               = other.matchXR;
        matchYR               = other.matchYR;
        matchContent          = other.matchContent;
        matchRange            = other.matchRange;
        matchNode             = other.matchNode;
        matchOffset           = other.matchOffset;
        matchNodeR            = other.matchNodeR;
        matchOffsetR          = other.matchOffsetR;
        matchContainsSelected = other.matchContainsSelected;
        matchContainsVisible  = other.matchContainsVisible;
        exists                = other.exists;
        startNode             = other.startNode;
        startOffset           = other.startOffset;
        endNode               = other.endNode;
        endOffset             = other.endOffset;
        topOffset             = other.topOffset;
        bottomOffset          = other.bottomOffset;
        leftOffset            = other.leftOffset;
        rightOffset           = other.rightOffset;
        width                 = other.width;
        height                = other.height;
        selected              = other.selected;
        visible               = other.visible;
        foregroundColor       = other.foregroundColor;
        backgroundColor       = other.backgroundColor;
        fontName              = other.fontName;
        fontHeight            = other.fontHeight;
        next                  = other.next;
        }

    /**
     *
     */
    virtual ~VisualSegment() {}


protected:

    bool            matchPosition;
    bool            matchInside;
    bool            matchContaining;
    long            matchX;
    long            matchY;
    long            matchXR;
    long            matchYR;
    bool            matchContent;
    bool            matchRange;
    NodePtr         matchNode;
    unsigned long   matchOffset;
    NodePtr         matchNodeR;
    unsigned long   matchOffsetR;
    bool            matchContainsSelected;
    bool            matchContainsVisible;
    bool            exists;
    NodePtr         startNode;
    unsigned long   startOffset;
    NodePtr         endNode;
    unsigned long   endOffset;
    long            topOffset;
    long            bottomOffset;
    long            leftOffset;
    long            rightOffset;
    unsigned long   width;
    unsigned long   height;
    bool            selected;
    bool            visible;
    unsigned long   foregroundColor;
    unsigned long   backgroundColor;
    DOMString       fontName;
    DOMString       fontHeight;
    bool            next;


};


/*#########################################################################
## VisualCharacter
#########################################################################*/

/**
 *
 */
class VisualCharacter : virtual public VisualSegment
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualCharacter()
        {}

    /**
     *
     */
    VisualCharacter(const VisualCharacter &other) : VisualResource(other),
                                                    VisualSegment(other)
        {
        }

    /**
     *
     */
    virtual ~VisualCharacter() {}
};



/*#########################################################################
## VisualCharacterRun
#########################################################################*/

/**
 *
 */
class VisualCharacterRun : virtual public VisualSegment
{
public:


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualCharacterRun() {}

    /**
     *
     */
    VisualCharacterRun(const VisualCharacterRun &other) : VisualResource(other),
                                                          VisualSegment(other)
        {
        }

    /**
     *
     */
    virtual ~VisualCharacterRun() {}

protected:


};



/*#########################################################################
## VisualFrame
#########################################################################*/

/**
 *
 */
class VisualFrame : virtual public VisualSegment
{
public:


    /**
     *
     */
    virtual VisualSegment getEmbedded()
        { return embedded; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualFrame() {}

    /**
     *
     */
    VisualFrame(const VisualFrame &other) : VisualResource(other),
                                            VisualSegment(other)
        {
        embedded = other.embedded;
        }

    /**
     *
     */
    virtual ~VisualFrame() {}

protected:

    VisualSegment embedded;
};



/*#########################################################################
## VisualImage
#########################################################################*/

/**
 *
 */
class VisualImage : virtual public VisualSegment
{
public:

    /**
     *
     */
    virtual DOMString getImageURL()
        { return imageURL; }

    /**
     *
     */
    virtual bool getIsLoaded()
        { return isLoaded; }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualImage() {}

    /**
     *
     */
    VisualImage(const VisualImage &other) : VisualResource(other),
                                            VisualSegment(other)
        {
        imageURL = other.imageURL;
        isLoaded = other.isLoaded;
        }

    /**
     *
     */
    virtual ~VisualImage() {}

protected:

    DOMString imageURL;
    bool isLoaded;

};



/*#########################################################################
## VisualFormButton
#########################################################################*/

/**
 *
 */
class VisualFormButton : virtual public VisualSegment
{
public:

    /**
     *
     */
    virtual bool getIsPressed()
        { return isPressed; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualFormButton()
        { isPressed = false; }

    /**
     *
     */
    VisualFormButton(const VisualFormButton &other) : VisualResource(other),
                                                      VisualSegment(other)
        {
        isPressed = other.isPressed;
        }

    /**
     *
     */
    virtual ~VisualFormButton() {}

protected:

    bool isPressed;

};



/*#########################################################################
## VisualFormField
#########################################################################*/

/**
 *
 */
class VisualFormField : virtual public VisualSegment
{
public:

    /**
     *
     */
    virtual DOMString getFormValue()
        { return formValue; }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualFormField() {}

    /**
     *
     */
    VisualFormField(const VisualFormField &other) : VisualResource(other),
                                                    VisualSegment(other)
        {
        formValue = other.formValue;
        }

    /**
     *
     */
    virtual ~VisualFormField() {}

protected:

    DOMString formValue;

};



/*#########################################################################
## VisualView
#########################################################################*/

/**
 *
 */
class VisualView
{
public:

    /**
     *
     */
    virtual bool getValue()
        { return value; }

    /**
     *
     */
    virtual DOMString getFontScheme()
        { return fontScheme; }

    /**
     *
     */
    virtual unsigned long getWidth()
        { return width; }

    /**
     *
     */
    virtual unsigned long getHeight()
        { return height; }

    /**
     *
     */
    virtual unsigned long getHorizontalDPI()
        { return horizontalDPI; }

    /**
     *
     */
    virtual unsigned long getVerticalDPI()
        { return verticalDPI; }

    /**
     *
     */
    virtual VisualCharacter createVisualCharacter()
        {
        VisualCharacter ret;
        return ret;
        }

    /**
     *
     */
    virtual VisualCharacterRun createVisualCharacterRun()
        {
        VisualCharacterRun ret;
        return ret;
        }
    /**
     *
     */
    virtual VisualFrame createVisualFrame()
        {
        VisualFrame ret;
        return ret;
        }


    /**
     *
     */
    virtual VisualImage createVisualImage()
        {
        VisualImage ret;
        return ret;
        }

    /**
     *
     */
    virtual VisualFormButton createVisualFormButton()
        {
        VisualFormButton ret;
        return ret;
        }

    /**
     *
     */
    virtual VisualFormField createVisualFormField()
        {
        VisualFormField ret;
        return ret;
        }

    /**
     *
     */
    virtual void select(const NodePtr /*boundary*/,
                        unsigned long /*offset*/,
                        bool /*extend*/,
                        bool /*add*/)
        {
        }

    /**
     *
     */
    virtual void matchSegment(const VisualResource */*segment*/)
        {
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    VisualView() {}

    /**
     *
     */
    VisualView(const VisualView &other)
        {
        value         = other.value;
        fontScheme    = other.fontScheme;
        width         = other.width;
        height        = other.height;
        horizontalDPI = other.horizontalDPI;
        verticalDPI   = other.verticalDPI;
        }

    /**
     *
     */
    virtual ~VisualView() {}

protected:

    bool value;

    DOMString fontScheme;

    unsigned long width;
    unsigned long height;
    unsigned long horizontalDPI;
    unsigned long verticalDPI;

};




}  //namespace views
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif  /* __VIEWS_LEVEL3_H__ */
/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/

