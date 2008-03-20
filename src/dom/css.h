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

#ifndef __CSS_H__
#define __CSS_H__

#include "dom.h"
#include "stylesheets.h"
#include "views.h"

#include <vector>
#include <map>


namespace org {
namespace w3c {
namespace dom {
namespace css {




//Make local definitions
typedef dom::DOMString DOMString;
typedef dom::Element Element;
typedef dom::DOMImplementation DOMImplementation;

//forward declarations
class CSSRule;
class CSSStyleSheet;
class CSSStyleDeclaration;
class CSSValue;
class Counter;
class Rect;
class RGBColor;





/*#########################################################################
## CSSRule
#########################################################################*/

/**
 *
 */
class CSSRule
{
public:

    typedef enum
        {
        UNKNOWN_RULE    = 0,
        STYLE_RULE      = 1,
        CHARSET_RULE    = 2,
        IMPORT_RULE     = 3,
        MEDIA_RULE      = 4,
        FONT_FACE_RULE  = 5,
        PAGE_RULE       = 6
        } RuleType;


    /**
     *
     */
    virtual unsigned short getType()
        {
        return type;
        }

    /**
     *
     */
    virtual DOMString getCssText()
        {
        return cssText;
        }

    /**
     *
     */
    virtual void setCssText(const DOMString &val) throw (dom::DOMException)
        {
        cssText = val;
        }

    /**
     *
     */
    virtual CSSStyleSheet *getParentStyleSheet()
        {
        return parentStyleSheet;
        }

    /**
     *
     */
    virtual CSSRule *getParentRule()
        {
        return parentRule;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSRule()
        {
        type             = UNKNOWN_RULE;
        cssText          = "";
        parentStyleSheet = NULL;
        parentRule       = NULL;
        }

    /**
     *
     */
    CSSRule(const CSSRule &other)
        {
        type             = other.type;
        cssText          = other.cssText;
        parentStyleSheet = other.parentStyleSheet;
        parentRule       = other.parentRule;
        }

    /**
     *
     */
    virtual ~CSSRule() {}

protected:

    int type;

    DOMString cssText;

    CSSStyleSheet *parentStyleSheet;

    CSSRule *parentRule;
};



/*#########################################################################
## CSSRuleList
#########################################################################*/

/**
 *
 */
class CSSRuleList
{
public:

    /**
     *
     */
    virtual unsigned long getLength()
        {
        return rules.size();
        }

    /**
     *
     */
    virtual CSSRule item(unsigned long index)
        {
        if (index>=rules.size())
            {
            CSSRule rule;
            return rule;
            }
        return rules[index];
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSRuleList() {}


    /**
     *
     */
    CSSRuleList(const CSSRuleList &other)
        {
        rules = other.rules;
        }

    /**
     *
     */
    virtual ~CSSRuleList() {}

protected:

friend class CSSMediaRule;
friend class CSSStyleSheet;

    /**
     *
     */
    virtual void addRule(const CSSRule &rule)
        {
        rules.push_back(rule);
        }


    /**
     *
     */
    virtual void deleteRule(unsigned long index)
        {
        if (index>=rules.size())
            return;
        std::vector<CSSRule>::iterator iter = rules.begin() + index;
        rules.erase(iter);
        }


    /**
     *
     */
    virtual long insertRule(const CSSRule &rule, unsigned long index)
        {
        if (index>=rules.size())
            return -1;
        std::vector<CSSRule>::iterator iter = rules.begin() + index;
        rules.insert(iter, rule);
        return index;
        }

    std::vector<CSSRule>rules;
};


/*#########################################################################
## CSSStyleSheet
#########################################################################*/

/**
 *
 */
class CSSStyleSheet : virtual public stylesheets::StyleSheet
{
public:

    /**
     *
     */
    virtual CSSRule *getOwnerRule()
        {
        return ownerRule;
        }

    /**
     *
     */
    virtual CSSRuleList getCssRules()
        {
        return rules;
        }

    /**
     *
     */
    virtual unsigned long insertRule(const DOMString &/*ruleStr*/,
                                     unsigned long index)
                                     throw (dom::DOMException)
        {
        CSSRule rule;
        return rules.insertRule(rule, index);
        }

    /**
     *
     */
    virtual void deleteRule(unsigned long index)
                            throw (dom::DOMException)
        {
        rules.deleteRule(index);
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSStyleSheet() : stylesheets::StyleSheet()
        {
        }

    /**
     *
     */
    CSSStyleSheet(const CSSStyleSheet &other) :
                  stylesheets::StyleSheet(other)
        {
        ownerRule = other.ownerRule;
        rules     = other.rules;
        }

    /**
     *
     */
    virtual ~CSSStyleSheet() {}

protected:

    CSSRule *ownerRule;

    CSSRuleList rules;
};


/*#########################################################################
## CSSValue
#########################################################################*/

/**
 *
 */
class CSSValue
{
public:

    /**
     * UnitTypes
     */
    enum
        {
        CSS_INHERIT         = 0,
        CSS_PRIMITIVE_VALUE = 1,
        CSS_VALUE_LIST      = 2,
        CSS_CUSTOM          = 3
        };

    /**
     *
     */
    virtual DOMString getCssText()
        {
        return cssText;
        }

    /**
     *
     */
    virtual void setCssText(const DOMString &val)
                            throw (dom::DOMException)
        {
        cssText = val;
        }

    /**
     *
     */
    virtual unsigned short getCssValueType()
        {
        return valueType;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSValue()
        {
        valueType = CSS_INHERIT;
        }

    /**
     *
     */
    CSSValue(const CSSValue &other)
        {
        cssText   = other.cssText;
        valueType = other.valueType;
        }

    /**
     *
     */
    virtual ~CSSValue() {}

protected:

    DOMString cssText;
    int valueType;
};





/*#########################################################################
## CSSStyleDeclaration
#########################################################################*/

class CSSStyleDeclarationEntry
{
public:
    CSSStyleDeclarationEntry(const DOMString &nameArg,
                             const DOMString &valueArg,
                             const DOMString &prioArg)
    {
        name  = nameArg;
        value = valueArg;
        prio  = prioArg;
    }
    virtual ~CSSStyleDeclarationEntry(){}
    DOMString name;
    DOMString value;
    DOMString prio;
};



/**
 *
 */
class CSSStyleDeclaration
{
public:

    /**
     *
     */
    virtual DOMString getCssText()
        {
        return cssText;
        }

    /**
     *
     */
    virtual void setCssText(const DOMString &val)
                            throw (dom::DOMException)
        {
        cssText = val;
        }

    /**
     *
     */
    virtual DOMString getPropertyValue(const DOMString &propertyName)
        {
        std::vector<CSSStyleDeclarationEntry>::iterator iter;
        for (iter=items.begin() ; iter!=items.end() ; iter++)
            {
            if (iter->name == propertyName)
                return iter->value;
            }
        return "";
        }

    /**
     *
     */
    virtual CSSValue getPropertyCSSValue(const DOMString &/*propertyName*/)
        {
        CSSValue value;
        return value;
        }

    /**
     *
     */
    virtual DOMString removeProperty(const DOMString &propertyName)
                                     throw (dom::DOMException)
        {
        std::vector<CSSStyleDeclarationEntry>::iterator iter;
        for (iter=items.begin() ; iter!=items.end() ; iter++)
            {
            if (iter->name == propertyName)
                items.erase(iter);
            }
        return propertyName;
        }

    /**
     *
     */
    virtual DOMString getPropertyPriority(const DOMString &propertyName)
        {
        std::vector<CSSStyleDeclarationEntry>::iterator iter;
        for (iter=items.begin() ; iter!=items.end() ; iter++)
            {
            if (iter->name == propertyName)
                return iter->prio;
            }
        return "";
        }

    /**
     *
     */
    virtual void setProperty(const DOMString &propertyName,
                             const DOMString &value,
                             const DOMString &priority)
                             throw (dom::DOMException)
        {
        std::vector<CSSStyleDeclarationEntry>::iterator iter;
        for (iter=items.begin() ; iter!=items.end() ; iter++)
            {
            if (iter->name == propertyName)
                {
                iter->name  = propertyName;
                iter->value = value;
                iter->prio  = priority;
                return;
                }
            }
        CSSStyleDeclarationEntry entry(propertyName, value, priority);
        items.push_back(entry);
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
        if (index>=items.size())
            return "";
        DOMString ret = items[index].name;
        ret.append(":");
        ret.append(items[index].value);
        return ret;
        }

    /**
     *
     */
    virtual CSSRule *getParentRule()
        {
        return parentRule;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSStyleDeclaration()
        {
        parentRule = NULL;
        }

    /**
     *
     */
    CSSStyleDeclaration(const CSSStyleDeclaration &/*other*/)
        {
        }

    /**
     *
     */
    virtual ~CSSStyleDeclaration() {}

protected:

   DOMString cssText;

   CSSRule *parentRule;

   std::vector<CSSStyleDeclarationEntry> items;
};




/*#########################################################################
## CSSStyleRule
#########################################################################*/

/**
 *
 */
class CSSStyleRule : virtual public CSSRule
{
public:

    /**
     *
     */
    virtual DOMString getSelectorText()
        {
        return selectorText;
        }

    /**
     *
     */
    virtual void setSelectorText(const DOMString &val)
                throw (dom::DOMException)
        {
        selectorText = val;
        }


    /**
     *
     */
    virtual CSSStyleDeclaration &getStyle()
        {
        return style;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSStyleRule() : CSSRule()
        {
        type = STYLE_RULE;
        selectorText = "";
        }


    /**
     *
     */
    CSSStyleRule(const CSSStyleRule &other) : CSSRule(other)
        {
        selectorText = other.selectorText;
        style        = other.style;
        }

    /**
     *
     */
    virtual ~CSSStyleRule() {}

protected:

    DOMString selectorText;

    CSSStyleDeclaration style;

};

/*#########################################################################
## CSSMediaRule
#########################################################################*/

/**
 *
 */
class CSSMediaRule : virtual public CSSRule
{
public:

    /**
     *
     */
    virtual stylesheets::MediaList getMedia()
        {
        return mediaList;
        }

    /**
     *
     */
    virtual CSSRuleList getCssRules()
        {
        return cssRules;
        }

    /**
     *
     */
    virtual unsigned long insertRule(const DOMString &/*ruleStr*/,
                                     unsigned long index)
                                     throw (dom::DOMException)
        {
        if (index>cssRules.getLength())
            return 0;
        CSSRule rule;
        cssRules.insertRule(rule, index);
        return index;
        }

    /**
     *
     */
    virtual void deleteRule(unsigned long index)
                            throw(dom::DOMException)
        {
        cssRules.deleteRule(index);
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSMediaRule() : CSSRule()
        {
        type = MEDIA_RULE;
        }

    /**
     *
     */
    CSSMediaRule(const CSSMediaRule &other) : CSSRule(other)
        {
        mediaList = other.mediaList;
        cssRules  = other.cssRules;
        }

    /**
     *
     */
    virtual ~CSSMediaRule() {}

protected:

    stylesheets::MediaList mediaList;

    CSSRuleList cssRules;
};




/*#########################################################################
## CSSFontFaceRule
#########################################################################*/

/**
 *
 */
class CSSFontFaceRule : virtual public CSSRule
{
public:

    /**
     *
     */
    virtual CSSStyleDeclaration getStyle()
        {
        return style;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSFontFaceRule() : CSSRule()
        {
        type = FONT_FACE_RULE;
        }

    /**
     *
     */
    CSSFontFaceRule(const CSSFontFaceRule &other) : CSSRule(other)
        {
        style = other.style;
        }

    /**
     *
     */
    virtual ~CSSFontFaceRule() {}

protected:

    CSSStyleDeclaration style;
};




/*#########################################################################
## CSSPageRule
#########################################################################*/

/**
 *
 */
class CSSPageRule : virtual public CSSRule
{
public:

    /**
     *
     */
    virtual DOMString getSelectorText()
        {
        return selectorText;
        }

    /**
     *
     */
    virtual void setSelectorText(const DOMString &val)
                         throw(dom::DOMException)
        {
        selectorText = val;
        }


    /**
     *
     */
    virtual CSSStyleDeclaration getStyle()
        {
        return style;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSPageRule() : CSSRule()
        {
        type = PAGE_RULE;
        }

    /**
     *
     */
    CSSPageRule(const CSSPageRule &other) : CSSRule(other)
        {
        selectorText = other.selectorText;
        style        = other.style;
        }

    /**
     *
     */
    virtual ~CSSPageRule() {}

protected:

    DOMString selectorText;

    CSSStyleDeclaration style;
};





/*#########################################################################
## CSSImportRule
#########################################################################*/

/**
 *
 */
class CSSImportRule : virtual public CSSRule
{
public:

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
    virtual stylesheets::MediaList getMedia()
        {
        return mediaList;
        }

    /**
     *
     */
    virtual CSSStyleSheet getStyleSheet()
        {
        return styleSheet;
        }


    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSImportRule() : CSSRule()
        {
        type = IMPORT_RULE;
        }

    /**
     *
     */
    CSSImportRule(const CSSImportRule &other) : CSSRule(other)
        {
        mediaList  = other.mediaList;
        styleSheet = other.styleSheet;
        }

    /**
     *
     */
    virtual ~CSSImportRule() {}

protected:

    DOMString href;

    stylesheets::MediaList mediaList;

    CSSStyleSheet styleSheet;
};






/*#########################################################################
## CSSCharsetRule
#########################################################################*/

/**
 *
 */
class CSSCharsetRule : virtual public CSSRule
{
public:

    /**
     *
     */
    virtual DOMString getEncoding()
        {
        return encoding;
        }

    /**
     *
     */
    virtual void setEncoding(const DOMString &val) throw (dom::DOMException)
        {
        encoding = val;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSCharsetRule() : CSSRule()
        {
        type = CHARSET_RULE;
        }

    /**
     *
     */
    CSSCharsetRule(const CSSCharsetRule &other) : CSSRule(other)
        {
        encoding = other.encoding;
        }

    /**
     *
     */
    virtual ~CSSCharsetRule() {}

protected:

    DOMString encoding;

};





/*#########################################################################
## CSSUnknownRule
#########################################################################*/

/**
 *
 */
class CSSUnknownRule : virtual public CSSRule
{
public:

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSUnknownRule() : CSSRule()
        {
        type = UNKNOWN_RULE;
        }

    /**
     *
     */
    CSSUnknownRule(const CSSUnknownRule &other) : CSSRule(other)
        {
        }

    /**
     *
     */
    virtual ~CSSUnknownRule() {}
};







/*#########################################################################
## CSSValueList
#########################################################################*/

/**
 *
 */
class CSSValueList : virtual public CSSValue
{
public:

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
    virtual CSSValue item(unsigned long index)
        {
        if (index>=items.size())
            {
            CSSValue dummy;
            return dummy;
            }
        return items[index];
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSValueList()
        {
        }

    /**
     *
     */
    CSSValueList(const CSSValueList &other) : CSSValue(other)
        {
        items = other.items;
        }

    /**
     *
     */
    virtual ~CSSValueList() {}

protected:

    std::vector<CSSValue> items;
};




/*#########################################################################
## CSSPrimitiveValue
#########################################################################*/

/**
 *
 */
class CSSPrimitiveValue : virtual public CSSValue
{
public:

    /**
     * UnitTypes
     */
    enum
        {
        CSS_UNKNOWN    = 0,
        CSS_NUMBER     = 1,
        CSS_PERCENTAGE = 2,
        CSS_EMS        = 3,
        CSS_EXS        = 4,
        CSS_PX         = 5,
        CSS_CM         = 6,
        CSS_MM         = 7,
        CSS_IN         = 8,
        CSS_PT         = 9,
        CSS_PC         = 10,
        CSS_DEG        = 11,
        CSS_RAD        = 12,
        CSS_GRAD       = 13,
        CSS_MS         = 14,
        CSS_S          = 15,
        CSS_HZ         = 16,
        CSS_KHZ        = 17,
        CSS_DIMENSION  = 18,
        CSS_STRING     = 19,
        CSS_URI        = 20,
        CSS_IDENT      = 21,
        CSS_ATTR       = 22,
        CSS_COUNTER    = 23,
        CSS_RECT       = 24,
        CSS_RGBCOLOR   = 25
        };


    /**
     *
     */
    virtual unsigned short getPrimitiveType()
        {
        return primitiveType;
        }

    /**
     *
     */
    virtual void setFloatValue(unsigned short unitType,
                               double doubleValueArg)
                               throw (dom::DOMException)
        {
        primitiveType = unitType;
        doubleValue = doubleValueArg;
        }
    /**
     *
     */
    virtual double getFloatValue(unsigned short /*unitType*/)
                                throw (dom::DOMException)
        {
        return doubleValue;
        }

    /**
     *
     */
    virtual void setStringValue(unsigned short /*stringType*/,
                                const DOMString &stringValueArg)
                                throw (dom::DOMException)
        {
        stringValue = stringValueArg;
        }

    /**
     *
     */
    virtual DOMString getStringValue() throw (dom::DOMException)
        {
        return stringValue;
        }

    /**
     *
     */
    virtual Counter *getCounterValue() throw (dom::DOMException)
        {
        return NULL;
        }

    /**
     *
     */
    virtual Rect *getRectValue() throw (dom::DOMException)
        {
        return NULL;
        }

    /**
     *
     */
    virtual RGBColor *getRGBColorValue() throw (dom::DOMException)
        {
        return NULL;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSSPrimitiveValue() : CSSValue()
        {
        }

    /**
     *
     */
    CSSPrimitiveValue(const CSSPrimitiveValue &other) : CSSValue(other)
        {
        }

    /**
     *
     */
    virtual ~CSSPrimitiveValue() {}

protected:

    int primitiveType;

    double doubleValue;

    DOMString stringValue;


};



/*#########################################################################
## RGBColor
#########################################################################*/

/**
 *
 */
class RGBColor
{
public:

    /**
     *
     */
    virtual CSSPrimitiveValue getRed()
        {
        return red;
        }

    /**
     *
     */
    virtual CSSPrimitiveValue getGreen()
        {
        return green;
        }

    /**
     *
     */
    virtual CSSPrimitiveValue getBlue()
        {
        return blue;
        }

    /**
     * REPLACES: RGBColor CSSPrimitiveValue::getRGBColorValue() throw (dom::DOMException)
     */
    static RGBColor getRGBColorValue(const CSSPrimitiveValue &/*val*/)
        {
        RGBColor col;
        return col;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    RGBColor() {}

    /**
     *
     */
    RGBColor(const RGBColor &other)
        {
        red   = other.red;
        green = other.green;
        blue  = other.blue;
        }

    /**
     *
     */
    virtual ~RGBColor() {}

protected:

    CSSPrimitiveValue red;
    CSSPrimitiveValue green;
    CSSPrimitiveValue blue;
};




/*#########################################################################
## Rect
#########################################################################*/

/**
 *
 */
class Rect
{
public:

    /**
     *
     */
    virtual CSSPrimitiveValue getTop()
        {
        return top;
        }

    /**
     *
     */
    virtual CSSPrimitiveValue getRight()
        {
        return right;
        }

    /**
     *
     */
    virtual CSSPrimitiveValue getBottom()
        {
        return bottom;
        }

    /**
     *
     */
    virtual CSSPrimitiveValue getLeft()
        {
        return left;
        }

    /**
     * REPLACES: Rect CSSPrimitiveValue::getRectValue() throw (dom::DOMException)
     */
    static Rect getRectValue(const CSSPrimitiveValue &/*val*/)
        {
        Rect rect;
        return rect;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Rect() {}

    /**
     *
     */
    Rect(const Rect &other)
        {
        top    = other.top;
        right  = other.right;
        bottom = other.bottom;
        left   = other.left;
        }

    /**
     *
     */
    virtual ~Rect() {}

protected:

    CSSPrimitiveValue top;
    CSSPrimitiveValue right;
    CSSPrimitiveValue bottom;
    CSSPrimitiveValue left;
};






/*#########################################################################
## Counter
#########################################################################*/

/**
 *
 */
class Counter
{
public:

    /**
     *
     */
    virtual DOMString getIdentifier()
        {
        return identifier;
        }

    /**
     *
     */
    virtual DOMString getListStyle()
        {
        return listStyle;
        }

    /**
     *
     */
    virtual DOMString getSeparator()
        {
        return separator;
        }

    /**
     * REPLACES: Counter CSSPrimitiveValue::getCounterValue() throw (dom::DOMException)
     */
    static Counter getCounterValue(const CSSPrimitiveValue &/*val*/)
        {
        Counter counter;
        return counter;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    Counter() {}

    /**
     *
     */
    Counter(const Counter &other)
        {
        identifier = other.identifier;
        listStyle  = other.listStyle;
        separator  = other.separator;
        }

    /**
     *
     */
    virtual ~Counter() {}

protected:

    DOMString identifier;
    DOMString listStyle;
    DOMString separator;

};




/*#########################################################################
## ElementCSSInlineStyle
#########################################################################*/

/**
 *
 */
class ElementCSSInlineStyle
{
public:

    /**
     *
     */
    virtual CSSStyleDeclaration getStyle()
        {
        return style;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ElementCSSInlineStyle() {}

    /**
     *
     */
    ElementCSSInlineStyle(const ElementCSSInlineStyle &other)
        {
        style = other.style;
        }

    /**
     *
     */
    virtual ~ElementCSSInlineStyle() {}

protected:

    CSSStyleDeclaration style;
};






/*#########################################################################
## CSS2Properties
#########################################################################*/

/**
 *
 */
class CSS2Properties
{
public:


    /**
     *  return the 'azimuth' property
     */
    virtual DOMString getAzimuth()
        {
        return azimuth;
        }

    /**
     *  set the 'azimuth' property
     */
    virtual void setAzimuth(const DOMString &val)
                         throw (dom::DOMException)
        {
        azimuth = val;
        }

    /**
     *  return the 'background' property
     */
    virtual DOMString getBackground()
        {
        return background;
        }

    /**
     *  set the 'background' property
     */
    virtual void setBackground(const DOMString &val)
                         throw (dom::DOMException)
        {
        background = val;
        }

    /**
     *  return the 'backgroundAttachment' property
     */
    virtual DOMString getBackgroundAttachment()
        {
        return backgroundAttachment;
        }

    /**
     *  set the 'backgroundAttachment' property
     */
    virtual void setBackgroundAttachment(const DOMString &val)
                         throw (dom::DOMException)
        {
        backgroundAttachment = val;
        }

    /**
     *  return the 'backgroundColor' property
     */
    virtual DOMString getBackgroundColor()
        {
        return backgroundColor;
        }

    /**
     *  set the 'backgroundColor' property
     */
    virtual void setBackgroundColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        backgroundColor = val;
        }

    /**
     *  return the 'backgroundImage' property
     */
    virtual DOMString getBackgroundImage()
        {
        return backgroundImage;
        }

    /**
     *  set the 'backgroundImage' property
     */
    virtual void setBackgroundImage(const DOMString &val)
                         throw (dom::DOMException)
        {
        backgroundImage = val;
        }

    /**
     *  return the 'backgroundPosition' property
     */
    virtual DOMString getBackgroundPosition()
        {
        return backgroundPosition;
        }

    /**
     *  set the 'backgroundPosition' property
     */
    virtual void setBackgroundPosition(const DOMString &val)
                         throw (dom::DOMException)
        {
        backgroundPosition = val;
        }

    /**
     *  return the 'backgroundRepeat' property
     */
    virtual DOMString getBackgroundRepeat()
        {
        return backgroundRepeat;
        }

    /**
     *  set the 'backgroundRepeat' property
     */
    virtual void setBackgroundRepeat(const DOMString &val)
                         throw (dom::DOMException)
        {
        backgroundRepeat = val;
        }

    /**
     *  return the 'border' property
     */
    virtual DOMString getBorder()
        {
        return border;
        }

    /**
     *  set the 'border' property
     */
    virtual void setBorder(const DOMString &val)
                         throw (dom::DOMException)
        {
        border = val;
        }

    /**
     *  return the 'borderCollapse' property
     */
    virtual DOMString getBorderCollapse()
        {
        return borderCollapse;
        }

    /**
     *  set the 'borderCollapse' property
     */
    virtual void setBorderCollapse(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderCollapse = val;
        }

    /**
     *  return the 'borderColor' property
     */
    virtual DOMString getBorderColor()
        {
        return borderColor;
        }

    /**
     *  set the 'borderColor' property
     */
    virtual void setBorderColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderColor = val;
        }

    /**
     *  return the 'borderSpacing' property
     */
    virtual DOMString getBorderSpacing()
        {
        return borderSpacing;
        }

    /**
     *  set the 'borderSpacing' property
     */
    virtual void setBorderSpacing(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderSpacing = val;
        }

    /**
     *  return the 'borderStyle' property
     */
    virtual DOMString getBorderStyle()
        {
        return borderStyle;
        }

    /**
     *  set the 'borderStyle' property
     */
    virtual void setBorderStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderStyle = val;
        }

    /**
     *  return the 'borderTop' property
     */
    virtual DOMString getBorderTop()
        {
        return borderTop;
        }

    /**
     *  set the 'borderTop' property
     */
    virtual void setBorderTop(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderTop = val;
        }

    /**
     *  return the 'borderRight' property
     */
    virtual DOMString getBorderRight()
        {
        return borderRight;
        }

    /**
     *  set the 'borderRight' property
     */
    virtual void setBorderRight(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderRight = val;
        }

    /**
     *  return the 'borderBottom' property
     */
    virtual DOMString getBorderBottom()
        {
        return borderBottom;
        }

    /**
     *  set the 'borderBottom' property
     */
    virtual void setBorderBottom(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderBottom = val;
        }

    /**
     *  return the 'borderLeft' property
     */
    virtual DOMString getBorderLeft()
        {
        return borderLeft;
        }

    /**
     *  set the 'borderLeft' property
     */
    virtual void setBorderLeft(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderLeft = val;
        }

    /**
     *  return the 'borderTopColor' property
     */
    virtual DOMString getBorderTopColor()
        {
        return borderTopColor;
        }

    /**
     *  set the 'borderTopColor' property
     */
    virtual void setBorderTopColor(const DOMString &val)
                         throw (dom::DOMException)
        {
    borderTopColor = val;
        }

    /**
     *  return the 'borderRightColor' property
     */
    virtual DOMString getBorderRightColor()
        {
        return borderRightColor;
        }

    /**
     *  set the 'borderRightColor' property
     */
    virtual void setBorderRightColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderRightColor = val;
        }

    /**
     *  return the 'borderBottomColor' property
     */
    virtual DOMString getBorderBottomColor()
        {
        return borderBottomColor;
        }

    /**
     *  set the 'borderBottomColor' property
     */
    virtual void setBorderBottomColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderBottomColor = val;
        }

    /**
     *  return the 'borderLeftColor' property
     */
    virtual DOMString getBorderLeftColor()
        {
        return borderLeftColor;
        }

    /**
     *  set the 'borderLeftColor' property
     */
    virtual void setBorderLeftColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderLeftColor = val;
        }

    /**
     *  return the 'borderTopStyle' property
     */
    virtual DOMString getBorderTopStyle()
        {
        return borderTopStyle;
        }

    /**
     *  set the 'borderTopStyle' property
     */
    virtual void setBorderTopStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderTopStyle = val;
        }

    /**
     *  return the 'borderRightStyle' property
     */
    virtual DOMString getBorderRightStyle()
        {
        return borderRightStyle;
        }

    /**
     *  set the 'borderRightStyle' property
     */
    virtual void setBorderRightStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderRightStyle = val;
        }

    /**
     *  return the 'borderBottomStyle' property
     */
    virtual DOMString getBorderBottomStyle()
        {
        return borderBottomStyle;
        }

    /**
     *  set the 'borderBottomStyle' property
     */
    virtual void setBorderBottomStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderBottomStyle = val;
        }

    /**
     *  return the 'borderLeftStyle' property
     */
    virtual DOMString getBorderLeftStyle()
        {
        return borderLeftStyle;
        }

    /**
     *  set the 'borderLeftStyle' property
     */
    virtual void setBorderLeftStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderLeftStyle = val;
        }

    /**
     *  return the 'borderTopWidth' property
     */
    virtual DOMString getBorderTopWidth()
        {
        return borderTopWidth;
        }

    /**
     *  set the 'borderTopWidth' property
     */
    virtual void setBorderTopWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderTopWidth = val;
        }

    /**
     *  return the 'borderRightWidth' property
     */
    virtual DOMString getBorderRightWidth()
        {
        return borderRightWidth;
        }

    /**
     *  set the 'borderRightWidth' property
     */
    virtual void setBorderRightWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderRightWidth = val;
        }

    /**
     *  return the 'borderBottomWidth' property
     */
    virtual DOMString getBorderBottomWidth()
        {
        return borderBottomWidth;
        }

    /**
     *  set the 'borderBottomWidth' property
     */
    virtual void setBorderBottomWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderBottomWidth = val;
        }

    /**
     *  return the 'borderLeftWidth' property
     */
    virtual DOMString getBorderLeftWidth()
        {
        return borderLeftWidth;
        }

    /**
     *  set the 'borderLeftWidth' property
     */
    virtual void setBorderLeftWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderLeftWidth = val;
        }

    /**
     *  return the 'borderWidth' property
     */
    virtual DOMString getBorderWidth()
        {
        return borderWidth;
        }

    /**
     *  set the 'borderWidth' property
     */
    virtual void setBorderWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        borderWidth = val;
        }

    /**
     *  return the 'bottom' property
     */
    virtual DOMString getBottom()
        {
        return bottom;
        }

    /**
     *  set the 'bottom' property
     */
    virtual void setBottom(const DOMString &val)
                         throw (dom::DOMException)
        {
        bottom = val;
        }

    /**
     *  return the 'captionSide' property
     */
    virtual DOMString getCaptionSide()
        {
        return captionSide;
        }

    /**
     *  set the 'captionSide' property
     */
    virtual void setCaptionSide(const DOMString &val)
                         throw (dom::DOMException)
        {
        captionSide = val;
        }

    /**
     *  return the 'clear' property
     */
    virtual DOMString getClear()
        {
        return clear;
        }

    /**
     *  set the 'clear' property
     */
    virtual void setClear(const DOMString &val)
                         throw (dom::DOMException)
        {
        clear = val;
        }

    /**
     *  return the 'clip' property
     */
    virtual DOMString getClip()
        {
        return clip;
        }

    /**
     *  set the 'clip' property
     */
    virtual void setClip(const DOMString &val)
                         throw (dom::DOMException)
        {
        clip = val;
        }

    /**
     *  return the 'color' property
     */
    virtual DOMString getColor()
        {
        return color;
        }

    /**
     *  set the 'color' property
     */
    virtual void setColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        color = val;
        }

    /**
     *  return the 'content' property
     */
    virtual DOMString getContent()
        {
        return content;
        }

    /**
     *  set the 'content' property
     */
    virtual void setContent(const DOMString &val)
                         throw (dom::DOMException)
        {
        content = val;
        }

    /**
     *  return the 'counterIncrement' property
     */
    virtual DOMString getCounterIncrement()
        {
        return counterIncrement;
        }

    /**
     *  set the 'counterIncrement' property
     */
    virtual void setCounterIncrement(const DOMString &val)
                         throw (dom::DOMException)
        {
        counterIncrement = val;
        }

    /**
     *  return the 'counterReset' property
     */
    virtual DOMString getCounterReset()
        {
        return counterReset;
        }

    /**
     *  set the 'counterReset' property
     */
    virtual void setCounterReset(const DOMString &val)
                         throw (dom::DOMException)
        {
        counterReset = val;
        }

    /**
     *  return the 'cue' property
     */
    virtual DOMString getCue()
        {
        return cue;
        }

    /**
     *  set the 'cue' property
     */
    virtual void setCue(const DOMString &val)
                         throw (dom::DOMException)
        {
        cue = val;
        }

    /**
     *  return the 'cueAfter' property
     */
    virtual DOMString getCueAfter()
        {
        return cueAfter;
        }

    /**
     *  set the 'cueAfter' property
     */
    virtual void setCueAfter(const DOMString &val)
                         throw (dom::DOMException)
        {
        cueAfter = val;
        }

    /**
     *  return the 'cueBefore' property
     */
    virtual DOMString getCueBefore()
        {
        return cueBefore;
        }

    /**
     *  set the 'cueBefore' property
     */
    virtual void setCueBefore(const DOMString &val)
                         throw (dom::DOMException)
        {
        cueBefore = val;
        }

    /**
     *  return the 'cursor' property
     */
    virtual DOMString getCursor()
        {
        return cursor;
        }

    /**
     *  set the 'cursor' property
     */
    virtual void setCursor(const DOMString &val)
                         throw (dom::DOMException)
        {
        cursor = val;
        }

    /**
     *  return the 'direction' property
     */
    virtual DOMString getDirection()
        {
        return direction;
        }

    /**
     *  set the 'direction' property
     */
    virtual void setDirection(const DOMString &val)
                         throw (dom::DOMException)
        {
        direction = val;
        }

    /**
     *  return the 'display' property
     */
    virtual DOMString getDisplay()
        {
        return display;
        }

    /**
     *  set the 'display' property
     */
    virtual void setDisplay(const DOMString &val)
                         throw (dom::DOMException)
        {
        display = val;
        }

    /**
     *  return the 'elevation' property
     */
    virtual DOMString getElevation()
        {
        return elevation;
        }

    /**
     *  set the 'elevation' property
     */
    virtual void setElevation(const DOMString &val)
                         throw (dom::DOMException)
        {
        elevation = val;
        }

    /**
     *  return the 'emptyCells' property
     */
    virtual DOMString getEmptyCells()
        {
        return emptyCells;
        }

    /**
     *  set the 'emptyCells' property
     */
    virtual void setEmptyCells(const DOMString &val)
                         throw (dom::DOMException)
        {
        emptyCells = val;
        }

    /**
     *  return the 'cssFloat' property
     */
    virtual DOMString getCssFloat()
        {
        return cssFloat;
        }

    /**
     *  set the 'cssFloat' property
     */
    virtual void setCssFloat(const DOMString &val)
                         throw (dom::DOMException)
        {
        cssFloat = val;
        }

    /**
     *  return the 'font' property
     */
    virtual DOMString getFont()
        {
        return font;
        }

    /**
     *  set the 'font' property
     */
    virtual void setFont(const DOMString &val)
                         throw (dom::DOMException)
        {
        font = val;
        }

    /**
     *  return the 'fontFamily' property
     */
    virtual DOMString getFontFamily()
        {
        return fontFamily;
        }

    /**
     *  set the 'fontFamily' property
     */
    virtual void setFontFamily(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontFamily = val;
        }

    /**
     *  return the 'fontSize' property
     */
    virtual DOMString getFontSize()
        {
        return fontSize;
        }

    /**
     *  set the 'fontSize' property
     */
    virtual void setFontSize(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontSize = val;
        }

    /**
     *  return the 'fontSizeAdjust' property
     */
    virtual DOMString getFontSizeAdjust()
        {
        return fontSizeAdjust;
        }

    /**
     *  set the 'fontSizeAdjust' property
     */
    virtual void setFontSizeAdjust(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontSizeAdjust = val;
        }

    /**
     *  return the 'fontStretch' property
     */
    virtual DOMString getFontStretch()
        {
        return fontStretch;
        }

    /**
     *  set the 'fontStretch' property
     */
    virtual void setFontStretch(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontStretch = val;
        }

    /**
     *  return the 'fontStyle' property
     */
    virtual DOMString getFontStyle()
        {
        return fontStyle;
        }

    /**
     *  set the 'fontStyle' property
     */
    virtual void setFontStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontStyle = val;
        }

    /**
     *  return the 'fontVariant' property
     */
    virtual DOMString getFontVariant()
        {
        return fontVariant;
        }

    /**
     *  set the 'fontVariant' property
     */
    virtual void setFontVariant(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontVariant = val;
        }

    /**
     *  return the 'fontWeight' property
     */
    virtual DOMString getFontWeight()
        {
        return fontWeight;
        }

    /**
     *  set the 'fontWeight' property
     */
    virtual void setFontWeight(const DOMString &val)
                         throw (dom::DOMException)
        {
        fontWeight = val;
        }

    /**
     *  return the 'height' property
     */
    virtual DOMString getHeight()
        {
        return height;
        }

    /**
     *  set the 'height' property
     */
    virtual void setHeight(const DOMString &val)
                         throw (dom::DOMException)
        {
        height = val;
        }

    /**
     *  return the 'left' property
     */
    virtual DOMString getLeft()
        {
        return left;
        }

    /**
     *  set the 'left' property
     */
    virtual void setLeft(const DOMString &val)
                         throw (dom::DOMException)
        {
        left = val;
        }

    /**
     *  return the 'letterSpacing' property
     */
    virtual DOMString getLetterSpacing()
        {
        return letterSpacing;
        }

    /**
     *  set the 'letterSpacing' property
     */
    virtual void setLetterSpacing(const DOMString &val)
                         throw (dom::DOMException)
        {
        letterSpacing = val;
        }

    /**
     *  return the 'lineHeight' property
     */
    virtual DOMString getLineHeight()
        {
        return lineHeight;
        }

    /**
     *  set the 'lineHeight' property
     */
    virtual void setLineHeight(const DOMString &val)
                         throw (dom::DOMException)
        {
        lineHeight = val;
        }

    /**
     *  return the 'listStyle' property
     */
    virtual DOMString getListStyle()
        {
        return listStyle;
        }

    /**
     *  set the 'listStyle' property
     */
    virtual void setListStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        listStyle = val;
        }

    /**
     *  return the 'listStyleImage' property
     */
    virtual DOMString getListStyleImage()
        {
        return listStyleImage;
        }

    /**
     *  set the 'listStyleImage' property
     */
    virtual void setListStyleImage(const DOMString &val)
                         throw (dom::DOMException)
        {
        listStyleImage = val;
        }

    /**
     *  return the 'listStylePosition' property
     */
    virtual DOMString getListStylePosition()
        {
        return listStylePosition;
        }

    /**
     *  set the 'listStylePosition' property
     */
    virtual void setListStylePosition(const DOMString &val)
                         throw (dom::DOMException)
        {
        listStylePosition = val;
        }

    /**
     *  return the 'listStyleType' property
     */
    virtual DOMString getListStyleType()
        {
        return listStyleType;
        }

    /**
     *  set the 'listStyleType' property
     */
    virtual void setListStyleType(const DOMString &val)
                         throw (dom::DOMException)
        {
        listStyleType = val;
        }

    /**
     *  return the 'margin' property
     */
    virtual DOMString getMargin()
        {
        return margin;
        }

    /**
     *  set the 'margin' property
     */
    virtual void setMargin(const DOMString &val)
                         throw (dom::DOMException)
        {
        margin = val;
        }

    /**
     *  return the 'marginTop' property
     */
    virtual DOMString getMarginTop()
        {
        return marginTop;
        }

    /**
     *  set the 'marginTop' property
     */
    virtual void setMarginTop(const DOMString &val)
                         throw (dom::DOMException)
        {
        marginTop = val;
        }

    /**
     *  return the 'marginRight' property
     */
    virtual DOMString getMarginRight()
        {
        return marginRight;
        }

    /**
     *  set the 'marginRight' property
     */
    virtual void setMarginRight(const DOMString &val)
                         throw (dom::DOMException)
        {
        marginRight = val;
        }

    /**
     *  return the 'marginBottom' property
     */
    virtual DOMString getMarginBottom()
        {
        return marginBottom;
        }

    /**
     *  set the 'marginBottom' property
     */
    virtual void setMarginBottom(const DOMString &val)
                         throw (dom::DOMException)
        {
        marginBottom = val;
        }

    /**
     *  return the 'marginLeft' property
     */
    virtual DOMString getMarginLeft()
        {
        return marginLeft;
        }

    /**
     *  set the 'marginLeft' property
     */
    virtual void setMarginLeft(const DOMString &val)
                         throw (dom::DOMException)
        {
        marginLeft = val;
        }

    /**
     *  return the 'markerOffset' property
     */
    virtual DOMString getMarkerOffset()
        {
        return markerOffset;
        }

    /**
     *  set the 'markerOffset' property
     */
    virtual void setMarkerOffset(const DOMString &val)
                         throw (dom::DOMException)
        {
        markerOffset = val;
        }

    /**
     *  return the 'marks' property
     */
    virtual DOMString getMarks()
        {
        return marks;
        }

    /**
     *  set the 'marks' property
     */
    virtual void setMarks(const DOMString &val)
                         throw (dom::DOMException)
        {
        marks = val;
        }

    /**
     *  return the 'maxHeight' property
     */
    virtual DOMString getMaxHeight()
        {
        return maxHeight;
        }

    /**
     *  set the 'maxHeight' property
     */
    virtual void setMaxHeight(const DOMString &val)
                         throw (dom::DOMException)
        {
        maxHeight = val;
        }

    /**
     *  return the 'maxWidth' property
     */
    virtual DOMString getMaxWidth()
        {
        return maxWidth;
        }

    /**
     *  set the 'maxWidth' property
     */
    virtual void setMaxWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        maxWidth = val;
        }

    /**
     *  return the 'minHeight' property
     */
    virtual DOMString getMinHeight()
        {
        return minHeight;
        }

    /**
     *  set the 'minHeight' property
     */
    virtual void setMinHeight(const DOMString &val)
                         throw (dom::DOMException)
        {
        minHeight = val;
        }

    /**
     *  return the 'minWidth' property
     */
    virtual DOMString getMinWidth()
        {
        return minWidth;
        }

    /**
     *  set the 'minWidth' property
     */
    virtual void setMinWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        minWidth = val;
        }

    /**
     *  return the 'orphans' property
     */
    virtual DOMString getOrphans()
        {
        return orphans;
        }

    /**
     *  set the 'orphans' property
     */
    virtual void setOrphans(const DOMString &val)
                         throw (dom::DOMException)
        {
        orphans = val;
        }

    /**
     *  return the 'outline' property
     */
    virtual DOMString getOutline()
        {
        return outline;
        }

    /**
     *  set the 'outline' property
     */
    virtual void setOutline(const DOMString &val)
                         throw (dom::DOMException)
        {
        outline = val;
        }

    /**
     *  return the 'outlineColor' property
     */
    virtual DOMString getOutlineColor()
        {
        return outlineColor;
        }

    /**
     *  set the 'outlineColor' property
     */
    virtual void setOutlineColor(const DOMString &val)
                         throw (dom::DOMException)
        {
        outlineColor = val;
        }

    /**
     *  return the 'outlineStyle' property
     */
    virtual DOMString getOutlineStyle()
        {
        return outlineStyle;
        }

    /**
     *  set the 'outlineStyle' property
     */
    virtual void setOutlineStyle(const DOMString &val)
                         throw (dom::DOMException)
        {
        outlineStyle = val;
        }

    /**
     *  return the 'outlineWidth' property
     */
    virtual DOMString getOutlineWidth()
        {
        return outlineWidth;
        }

    /**
     *  set the 'outlineWidth' property
     */
    virtual void setOutlineWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        outlineWidth = val;
        }

    /**
     *  return the 'overflow' property
     */
    virtual DOMString getOverflow()
        {
        return overflow;
        }

    /**
     *  set the 'overflow' property
     */
    virtual void setOverflow(const DOMString &val)
                         throw (dom::DOMException)
        {
        overflow = val;
        }

    /**
     *  return the 'padding' property
     */
    virtual DOMString getPadding()
        {
        return padding;
        }

    /**
     *  set the 'padding' property
     */
    virtual void setPadding(const DOMString &val)
                         throw (dom::DOMException)
        {
        padding = val;
        }

    /**
     *  return the 'paddingTop' property
     */
    virtual DOMString getPaddingTop()
        {
        return paddingTop;
        }

    /**
     *  set the 'paddingTop' property
     */
    virtual void setPaddingTop(const DOMString &val)
                         throw (dom::DOMException)
        {
        paddingTop = val;
        }

    /**
     *  return the 'paddingRight' property
     */
    virtual DOMString getPaddingRight()
        {
        return paddingRight;
        }

    /**
     *  set the 'paddingRight' property
     */
    virtual void setPaddingRight(const DOMString &val)
                         throw (dom::DOMException)
        {
        paddingRight = val;
        }

    /**
     *  return the 'paddingBottom' property
     */
    virtual DOMString getPaddingBottom()
        {
        return paddingBottom;
        }

    /**
     *  set the 'paddingBottom' property
     */
    virtual void setPaddingBottom(const DOMString &val)
                         throw (dom::DOMException)
        {
        paddingBottom = val;
        }

    /**
     *  return the 'paddingLeft' property
     */
    virtual DOMString getPaddingLeft()
        {
        return paddingLeft;
        }

    /**
     *  set the 'paddingLeft' property
     */
    virtual void setPaddingLeft(const DOMString &val)
                         throw (dom::DOMException)
        {
        paddingLeft = val;
        }

    /**
     *  return the 'page' property
     */
    virtual DOMString getPage()
        {
        return page;
        }

    /**
     *  set the 'page' property
     */
    virtual void setPage(const DOMString &val)
                         throw (dom::DOMException)
        {
        page = val;
        }

    /**
     *  return the 'pageBreakAfter' property
     */
    virtual DOMString getPageBreakAfter()
        {
        return pageBreakAfter;
        }

    /**
     *  set the 'pageBreakAfter' property
     */
    virtual void setPageBreakAfter(const DOMString &val)
                         throw (dom::DOMException)
        {
        pageBreakAfter = val;
        }

    /**
     *  return the 'pageBreakBefore' property
     */
    virtual DOMString getPageBreakBefore()
        {
        return pageBreakBefore;
        }

    /**
     *  set the 'pageBreakBefore' property
     */
    virtual void setPageBreakBefore(const DOMString &val)
                         throw (dom::DOMException)
        {
        pageBreakBefore = val;
        }

    /**
     *  return the 'pageBreakInside' property
     */
    virtual DOMString getPageBreakInside()
        {
        return pageBreakInside;
        }

    /**
     *  set the 'pageBreakInside' property
     */
    virtual void setPageBreakInside(const DOMString &val)
                         throw (dom::DOMException)
        {
        pageBreakInside = val;
        }

    /**
     *  return the 'pause' property
     */
    virtual DOMString getPause()
        {
        return pause;
        }

    /**
     *  set the 'pause' property
     */
    virtual void setPause(const DOMString &val)
                         throw (dom::DOMException)
        {
        pause = val;
        }

    /**
     *  return the 'pauseAfter' property
     */
    virtual DOMString getPauseAfter()
        {
        return pauseAfter;
        }

    /**
     *  set the 'pauseAfter' property
     */
    virtual void setPauseAfter(const DOMString &val)
                         throw (dom::DOMException)
        {
        pauseAfter = val;
        }

    /**
     *  return the 'pauseBefore' property
     */
    virtual DOMString getPauseBefore()
        {
        return pauseBefore;
        }

    /**
     *  set the 'pauseBefore' property
     */
    virtual void setPauseBefore(const DOMString &val)
                         throw (dom::DOMException)
        {
        pauseBefore = val;
        }

    /**
     *  return the 'pitch' property
     */
    virtual DOMString getPitch()
        {
        return pitch;
        }

    /**
     *  set the 'pitch' property
     */
    virtual void setPitch(const DOMString &val)
                         throw (dom::DOMException)
        {
        pitch = val;
        }

    /**
     *  return the 'pitchRange' property
     */
    virtual DOMString getPitchRange()
        {
        return pitchRange;
        }

    /**
     *  set the 'pitchRange' property
     */
    virtual void setPitchRange(const DOMString &val)
                         throw (dom::DOMException)
        {
        pitchRange = val;
        }

    /**
     *  return the 'playDuring' property
     */
    virtual DOMString getPlayDuring()
        {
        return playDuring;
        }

    /**
     *  set the 'playDuring' property
     */
    virtual void setPlayDuring(const DOMString &val)
                         throw (dom::DOMException)
        {
        playDuring = val;
        }

    /**
     *  return the 'position' property
     */
    virtual DOMString getPosition()
        {
        return position;
        }

    /**
     *  set the 'position' property
     */
    virtual void setPosition(const DOMString &val)
                         throw (dom::DOMException)
        {
        position = val;
        }

    /**
     *  return the 'quotes' property
     */
    virtual DOMString getQuotes()
        {
        return quotes;
        }

    /**
     *  set the 'quotes' property
     */
    virtual void setQuotes(const DOMString &val)
                         throw (dom::DOMException)
        {
        quotes = val;
        }

    /**
     *  return the 'richness' property
     */
    virtual DOMString getRichness()
        {
        return richness;
        }

    /**
     *  set the 'richness' property
     */
    virtual void setRichness(const DOMString &val)
                         throw (dom::DOMException)
        {
        richness = val;
        }

    /**
     *  return the 'right' property
     */
    virtual DOMString getRight()
        {
        return right;
        }

    /**
     *  set the 'right' property
     */
    virtual void setRight(const DOMString &val)
                         throw (dom::DOMException)
        {
        right = val;
        }

    /**
     *  return the 'size' property
     */
    virtual DOMString getSize()
        {
        return size;
        }

    /**
     *  set the 'size' property
     */
    virtual void setSize(const DOMString &val)
                         throw (dom::DOMException)
        {
        size = val;
        }

    /**
     *  return the 'speak' property
     */
    virtual DOMString getSpeak()
        {
        return speak;
        }

    /**
     *  set the 'speak' property
     */
    virtual void setSpeak(const DOMString &val)
                         throw (dom::DOMException)
        {
        speak = val;
        }

    /**
     *  return the 'speakHeader' property
     */
    virtual DOMString getSpeakHeader()
        {
        return speakHeader;
        }

    /**
     *  set the 'speakHeader' property
     */
    virtual void setSpeakHeader(const DOMString &val)
                         throw (dom::DOMException)
        {
        speakHeader = val;
        }

    /**
     *  return the 'speakNumeral' property
     */
    virtual DOMString getSpeakNumeral()
        {
        return speakNumeral;
        }

    /**
     *  set the 'speakNumeral' property
     */
    virtual void setSpeakNumeral(const DOMString &val)
                         throw (dom::DOMException)
        {
        speakNumeral = val;
        }

    /**
     *  return the 'speakPunctuation' property
     */
    virtual DOMString getSpeakPunctuation()
        {
        return speakPunctuation;
        }

    /**
     *  set the 'speakPunctuation' property
     */
    virtual void setSpeakPunctuation(const DOMString &val)
                         throw (dom::DOMException)
        {
        speakPunctuation = val;
        }

    /**
     *  return the 'speechRate' property
     */
    virtual DOMString getSpeechRate()
        {
        return speechRate;
        }

    /**
     *  set the 'speechRate' property
     */
    virtual void setSpeechRate(const DOMString &val)
                         throw (dom::DOMException)
        {
        speechRate = val;
        }

    /**
     *  return the 'stress' property
     */
    virtual DOMString getStress()
        {
        return stress;
        }

    /**
     *  set the 'stress' property
     */
    virtual void setStress(const DOMString &val)
                         throw (dom::DOMException)
        {
        stress = val;
        }

    /**
     *  return the 'tableLayout' property
     */
    virtual DOMString getTableLayout()
        {
        return tableLayout;
        }

    /**
     *  set the 'tableLayout' property
     */
    virtual void setTableLayout(const DOMString &val)
                         throw (dom::DOMException)
        {
        tableLayout = val;
        }

    /**
     *  return the 'textAlign' property
     */
    virtual DOMString getTextAlign()
        {
        return textAlign;
        }

    /**
     *  set the 'textAlign' property
     */
    virtual void setTextAlign(const DOMString &val)
                         throw (dom::DOMException)
        {
        textAlign = val;
        }

    /**
     *  return the 'textDecoration' property
     */
    virtual DOMString getTextDecoration()
        {
        return textDecoration;
        }

    /**
     *  set the 'textDecoration' property
     */
    virtual void setTextDecoration(const DOMString &val)
                         throw (dom::DOMException)
        {
        textDecoration = val;
        }

    /**
     *  return the 'textIndent' property
     */
    virtual DOMString getTextIndent()
        {
        return textIndent;
        }

    /**
     *  set the 'textIndent' property
     */
    virtual void setTextIndent(const DOMString &val)
                         throw (dom::DOMException)
        {
        textIndent = val;
        }

    /**
     *  return the 'textShadow' property
     */
    virtual DOMString getTextShadow()
        {
        return textShadow;
        }

    /**
     *  set the 'textShadow' property
     */
    virtual void setTextShadow(const DOMString &val)
                         throw (dom::DOMException)
        {
        textShadow = val;
        }

    /**
     *  return the 'textTransform' property
     */
    virtual DOMString getTextTransform()
        {
        return textTransform;
        }

    /**
     *  set the 'textTransform' property
     */
    virtual void setTextTransform(const DOMString &val)
                         throw (dom::DOMException)
        {
        textTransform = val;
        }

    /**
     *  return the 'top' property
     */
    virtual DOMString getTop()
        {
        return top;
        }

    /**
     *  set the 'top' property
     */
    virtual void setTop(const DOMString &val)
                         throw (dom::DOMException)
        {
        top = val;
        }

    /**
     *  return the 'unicodeBidi' property
     */
    virtual DOMString getUnicodeBidi()
        {
        return unicodeBidi;
        }

    /**
     *  set the 'unicodeBidi' property
     */
    virtual void setUnicodeBidi(const DOMString &val)
                         throw (dom::DOMException)
        {
        unicodeBidi = val;
        }

    /**
     *  return the 'verticalAlign' property
     */
    virtual DOMString getVerticalAlign()
        {
        return verticalAlign;
        }

    /**
     *  set the 'verticalAlign' property
     */
    virtual void setVerticalAlign(const DOMString &val)
                         throw (dom::DOMException)
        {
        verticalAlign = val;
        }

    /**
     *  return the 'visibility' property
     */
    virtual DOMString getVisibility()
        {
        return visibility;
        }

    /**
     *  set the 'visibility' property
     */
    virtual void setVisibility(const DOMString &val)
                         throw (dom::DOMException)
        {
        visibility = val;
        }

    /**
     *  return the 'voiceFamily' property
     */
    virtual DOMString getVoiceFamily()
        {
        return voiceFamily;
        }

    /**
     *  set the 'voiceFamily' property
     */
    virtual void setVoiceFamily(const DOMString &val)
                         throw (dom::DOMException)
        {
        voiceFamily = val;
        }

    /**
     *  return the 'volume' property
     */
    virtual DOMString getVolume()
        {
        return volume;
        }

    /**
     *  set the 'volume' property
     */
    virtual void setVolume(const DOMString &val)
                         throw (dom::DOMException)
        {
        volume = val;
        }

    /**
     *  return the 'whiteSpace' property
     */
    virtual DOMString getWhiteSpace()
        {
        return whiteSpace;
        }

    /**
     *  set the 'whiteSpace' property
     */
    virtual void setWhiteSpace(const DOMString &val)
                         throw (dom::DOMException)
        {
        whiteSpace = val;
        }

    /**
     *  return the 'widows' property
     */
    virtual DOMString getWidows()
        {
        return widows;
        }

    /**
     *  set the 'widows' property
     */
    virtual void setWidows(const DOMString &val)
                         throw (dom::DOMException)
        {
        widows = val;
        }

    /**
     *  return the 'width' property
     */
    virtual DOMString getWidth()
        {
        return width;
        }

    /**
     *  set the 'width' property
     */
    virtual void setWidth(const DOMString &val)
                         throw (dom::DOMException)
        {
        width = val;
        }

    /**
     *  return the 'wordSpacing' property
     */
    virtual DOMString getWordSpacing()
        {
        return wordSpacing;
        }

    /**
     *  set the 'wordSpacing' property
     */
    virtual void setWordSpacing(const DOMString &val)
                         throw (dom::DOMException)
        {
        wordSpacing = val;
        }

    /**
     *  return the 'zIndex' property
     */
    virtual DOMString getZIndex()
        {
        return zIndex;
        }

    /**
     *  set the 'zIndex' property
     */
    virtual void setZIndex(const DOMString &val)
                         throw (dom::DOMException)
        {
        zIndex = val;
        }



    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    CSS2Properties()
        {
        }

    /**
     *
     */
    CSS2Properties(const CSS2Properties &other)
        {
        azimuth              = other.azimuth;
        background           = other.background;
        backgroundAttachment = other.backgroundAttachment;
        backgroundColor      = other.backgroundColor;
        backgroundImage      = other.backgroundImage;
        backgroundPosition   = other.backgroundPosition;
        backgroundRepeat     = other.backgroundRepeat;
        border               = other.border;
        borderCollapse       = other.borderCollapse;
        borderColor          = other.borderColor;
        borderSpacing        = other.borderSpacing;
        borderStyle          = other.borderStyle;
        borderTop            = other.borderTop;
        borderRight          = other.borderRight;
        borderBottom         = other.borderBottom;
        borderLeft           = other.borderLeft;
        borderTopColor       = other.borderTopColor;
        borderRightColor     = other.borderRightColor;
        borderBottomColor    = other.borderBottomColor;
        borderLeftColor      = other.borderLeftColor;
        borderTopStyle       = other.borderTopStyle;
        borderRightStyle     = other.borderRightStyle;
        borderBottomStyle    = other.borderBottomStyle;
        borderLeftStyle      = other.borderLeftStyle;
        borderTopWidth       = other.borderTopWidth;
        borderRightWidth     = other.borderRightWidth;
        borderBottomWidth    = other.borderBottomWidth;
        borderLeftWidth      = other.borderLeftWidth;
        borderWidth          = other.borderWidth;
        bottom               = other.bottom;
        captionSide          = other.captionSide;
        clear                = other.clear;
        clip                 = other.clip;
        color                = other.color;
        content              = other.content;
        counterIncrement     = other.counterIncrement;
        counterReset         = other.counterReset;
        cue                  = other.cue;
        cueAfter             = other.cueAfter;
        cueBefore            = other.cueBefore;
        cursor               = other.cursor;
        direction            = other.direction;
        display              = other.display;
        elevation            = other.elevation;
        emptyCells           = other.emptyCells;
        cssFloat             = other.cssFloat;
        font                 = other.font;
        fontFamily           = other.fontFamily;
        fontSize             = other.fontSize;
        fontSizeAdjust       = other.fontSizeAdjust;
        fontStretch          = other.fontStretch;
        fontStyle            = other.fontStyle;
        fontVariant          = other.fontVariant;
        fontWeight           = other.fontWeight;
        height               = other.height;
        left                 = other.left;
        letterSpacing        = other.letterSpacing;
        lineHeight           = other.lineHeight;
        listStyle            = other.listStyle;
        listStyleImage       = other.listStyleImage;
        listStylePosition    = other.listStylePosition;
        listStyleType        = other.listStyleType;
        margin               = other.margin;
        marginTop            = other.marginTop;
        marginRight          = other.marginRight;
        marginBottom         = other.marginBottom;
        marginLeft           = other.marginLeft;
        markerOffset         = other.markerOffset;
        marks                = other.marks;
        maxHeight            = other.maxHeight;
        maxWidth             = other.maxWidth;
        minHeight            = other.minHeight;
        minWidth             = other.minWidth;
        orphans              = other.orphans;
        outline              = other.outline;
        outlineColor         = other.outlineColor;
        outlineStyle         = other.outlineStyle;
        outlineWidth         = other.outlineWidth;
        overflow             = other.overflow;
        padding              = other.padding;
        paddingTop           = other.paddingTop;
        paddingRight         = other.paddingRight;
        paddingBottom        = other.paddingBottom;
        paddingLeft          = other.paddingLeft;
        page                 = other.page;
        pageBreakAfter       = other.pageBreakAfter;
        pageBreakBefore      = other.pageBreakBefore;
        pageBreakInside      = other.pageBreakInside;
        pause                = other.pause;
        pauseAfter           = other.pauseAfter;
        pauseBefore          = other.pauseBefore;
        pitch                = other.pitch;
        pitchRange           = other.pitchRange;
        playDuring           = other.playDuring;
        position             = other.position;
        quotes               = other.quotes;
        richness             = other.richness;
        right                = other.right;
        size                 = other.size;
        speak                = other.speak;
        speakHeader          = other.speakHeader;
        speakNumeral         = other.speakNumeral;
        speakPunctuation     = other.speakPunctuation;
        speechRate           = other.speechRate;
        stress               = other.stress;
        tableLayout          = other.tableLayout;
        textAlign            = other.textAlign;
        textDecoration       = other.textDecoration;
        textIndent           = other.textIndent;
        textShadow           = other.textShadow;
        textTransform        = other.textTransform;
        top                  = other.top;
        unicodeBidi          = other.unicodeBidi;
        verticalAlign        = other.verticalAlign;
        visibility           = other.visibility;
        voiceFamily          = other.voiceFamily;
        volume               = other.volume;
        whiteSpace           = other.whiteSpace;
        widows               = other.widows;
        width                = other.width;
        wordSpacing          = other.wordSpacing;
        zIndex               = other.zIndex;
        }

    /**
     *
     */
    virtual ~CSS2Properties() {}

protected:

    //######################
    //# P R O P E R T I E S
    //######################
    DOMString azimuth;
    DOMString background;
    DOMString backgroundAttachment;
    DOMString backgroundColor;
    DOMString backgroundImage;
    DOMString backgroundPosition;
    DOMString backgroundRepeat;
    DOMString border;
    DOMString borderCollapse;
    DOMString borderColor;
    DOMString borderSpacing;
    DOMString borderStyle;
    DOMString borderTop;
    DOMString borderRight;
    DOMString borderBottom;
    DOMString borderLeft;
    DOMString borderTopColor;
    DOMString borderRightColor;
    DOMString borderBottomColor;
    DOMString borderLeftColor;
    DOMString borderTopStyle;
    DOMString borderRightStyle;
    DOMString borderBottomStyle;
    DOMString borderLeftStyle;
    DOMString borderTopWidth;
    DOMString borderRightWidth;
    DOMString borderBottomWidth;
    DOMString borderLeftWidth;
    DOMString borderWidth;
    DOMString bottom;
    DOMString captionSide;
    DOMString clear;
    DOMString clip;
    DOMString color;
    DOMString content;
    DOMString counterIncrement;
    DOMString counterReset;
    DOMString cue;
    DOMString cueAfter;
    DOMString cueBefore;
    DOMString cursor;
    DOMString direction;
    DOMString display;
    DOMString elevation;
    DOMString emptyCells;
    DOMString cssFloat;
    DOMString font;
    DOMString fontFamily;
    DOMString fontSize;
    DOMString fontSizeAdjust;
    DOMString fontStretch;
    DOMString fontStyle;
    DOMString fontVariant;
    DOMString fontWeight;
    DOMString height;
    DOMString left;
    DOMString letterSpacing;
    DOMString lineHeight;
    DOMString listStyle;
    DOMString listStyleImage;
    DOMString listStylePosition;
    DOMString listStyleType;
    DOMString margin;
    DOMString marginTop;
    DOMString marginRight;
    DOMString marginBottom;
    DOMString marginLeft;
    DOMString markerOffset;
    DOMString marks;
    DOMString maxHeight;
    DOMString maxWidth;
    DOMString minHeight;
    DOMString minWidth;
    DOMString orphans;
    DOMString outline;
    DOMString outlineColor;
    DOMString outlineStyle;
    DOMString outlineWidth;
    DOMString overflow;
    DOMString padding;
    DOMString paddingTop;
    DOMString paddingRight;
    DOMString paddingBottom;
    DOMString paddingLeft;
    DOMString page;
    DOMString pageBreakAfter;
    DOMString pageBreakBefore;
    DOMString pageBreakInside;
    DOMString pause;
    DOMString pauseAfter;
    DOMString pauseBefore;
    DOMString pitch;
    DOMString pitchRange;
    DOMString playDuring;
    DOMString position;
    DOMString quotes;
    DOMString richness;
    DOMString right;
    DOMString size;
    DOMString speak;
    DOMString speakHeader;
    DOMString speakNumeral;
    DOMString speakPunctuation;
    DOMString speechRate;
    DOMString stress;
    DOMString tableLayout;
    DOMString textAlign;
    DOMString textDecoration;
    DOMString textIndent;
    DOMString textShadow;
    DOMString textTransform;
    DOMString top;
    DOMString unicodeBidi;
    DOMString verticalAlign;
    DOMString visibility;
    DOMString voiceFamily;
    DOMString volume;
    DOMString whiteSpace;
    DOMString widows;
    DOMString width;
    DOMString wordSpacing;
    DOMString zIndex;


};








/*#########################################################################
## ViewCSS
#########################################################################*/

/**
 *
 */
class ViewCSS : virtual public views::AbstractView
{
public:

    /**
     *
     */
    virtual CSSStyleDeclaration getComputedStyle(const Element &/*elt*/,
                                                 const DOMString &/*pseudoElt*/)
        {
        CSSStyleDeclaration style;
        return style;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    ViewCSS() : views::AbstractView()
       {
       }

    /**
     *
     */
    ViewCSS(const ViewCSS &other) : views::AbstractView(other)
       {
       }

    /**
     *
     */
    virtual ~ViewCSS() {}
};





/*#########################################################################
## DocumentCSS
#########################################################################*/

/**
 *
 */
class DocumentCSS : virtual public stylesheets::DocumentStyle
{
public:

    /**
     *
     */
    virtual CSSStyleDeclaration getOverrideStyle(const Element */*elt*/,
                                                 const DOMString &/*pseudoElt*/)
        {
        CSSStyleDeclaration style;
        return style;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DocumentCSS() : stylesheets::DocumentStyle()
        {
        }

    /**
     *
     */
    DocumentCSS(const DocumentCSS &other) : stylesheets::DocumentStyle(other)
       {
       }

    /**
     *
     */
    virtual ~DocumentCSS() {}
};






/*#########################################################################
## DOMImplementationCSS
#########################################################################*/

/**
 *
 */
class DOMImplementationCSS : virtual public DOMImplementation
{
public:

    /**
     *
     */
    virtual CSSStyleSheet createCSSStyleSheet(const DOMString &/*title*/,
                                              const DOMString &/*media*/)
                                               throw (dom::DOMException)
        {
        CSSStyleSheet sheet;
        return sheet;
        }

    //##################
    //# Non-API methods
    //##################

    /**
     *
     */
    DOMImplementationCSS() {}

    /**
     *
     */
    DOMImplementationCSS(const DOMImplementationCSS &other)
                         : DOMImplementation(other)
       {
       }

    /**
     *
     */
    virtual ~DOMImplementationCSS() {}
};








}  //namespace css
}  //namespace dom
}  //namespace org
}  //namespace w3c


#endif // __CSS_H__

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/
