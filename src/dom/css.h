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
 *  
 * =======================================================================
 * NOTES
 * 
 * Views, Stylesheets and CSS are DOM Level 2 for the purposes of supporting
 * SVG.  Be prepared in the future when they make Level 3 and SVG is likewise
 * updated.  The API here and many of the comments come from this document:
 * http://www.w3.org/TR/DOM-Level-2-Style/css.html    
     
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
 * The CSSRule interface is the abstract base interface for any type of CSS 
 * statement. This includes both rule sets and at-rules. An implementation is 
 * expected to preserve all rules specified in a CSS style sheet, even if the 
 * rule is not recognized by the parser. Unrecognized rules are represented using 
 * the CSSUnknownRule interface.
 */
class CSSRule
{
public:

    /**
     * An integer indicating which type of rule this is.
     */	     
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
     * The type of the rule, as defined above. The expectation is that 
     * binding-specific casting methods can be used to cast down from an instance of 
     * the CSSRule interface to the specific derived interface implied by the type.
     */
    virtual unsigned short getType()
        {
        return type;
        }

    /**
     * The parsable textual representation of the rule. This reflects the current 
     * state of the rule and not its initial value.
     */
    virtual DOMString getCssText()
        {
        return cssText;
        }

    /**
     * The parsable textual representation of the rule. This reflects the current 
     * state of the rule and not its initial value.
     * Note that setting involves reparsing.     
     */
    virtual void setCssText(const DOMString &val) throw (dom::DOMException)
        {
        cssText = val;
        }

    /**
     * The style sheet that contains this rule.
     */
    virtual CSSStyleSheet *getParentStyleSheet()
        {
        return parentStyleSheet;
        }

    /**
     * If this rule is contained inside another rule (e.g. a style rule inside an 
     * @media block), this is the containing rule. If this rule is not nested inside 
     * any other rules, this returns null.
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
        assign(other);
        }

    /**
     *
     */
    CSSRule &operator=(const CSSRule &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSRule &other)
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
 * The CSSRuleList interface provides the abstraction of an ordered collection of 
 * CSS rules.
 * 
 * The items in the CSSRuleList are accessible via an integral index, starting 
 * from 0.
 */
class CSSRuleList
{
public:

    /**
     * The number of CSSRules in the list. The range of valid child rule indices is 0 
     * to length-1 inclusive.
     */
    virtual unsigned long getLength()
        {
        return rules.size();
        }

    /**
     * Used to retrieve a CSS rule by ordinal index. The order in this collection 
     * represents the order of the rules in the CSS style sheet. If index is greater 
     * than or equal to the number of rules in the list, this returns null.
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
    CSSRuleList &operator=(const CSSRuleList &other)
        {
        rules = other.rules;
        return *this;
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
 * The CSSStyleSheet interface is a concrete interface used to represent a CSS 
 * style sheet i.e., a style sheet whose content type is "text/css".
 */
class CSSStyleSheet : virtual public stylesheets::StyleSheet
{
public:

    /**
     * If this style sheet comes from an @import rule, the ownerRule attribute will 
     * contain the CSSImportRule. In that case, the ownerNode attribute in the 
     * StyleSheet interface will be null. If the style sheet comes from an element or 
     * a processing instruction, the ownerRule attribute will be null and the 
     * ownerNode attribute will contain the Node.
     */
    virtual CSSRule *getOwnerRule()
        {
        return ownerRule;
        }

    /**
     * The list of all CSS rules contained within the style sheet. This
     * 	 includes both rule sets and at-rules.
     */
    virtual CSSRuleList getCssRules()
        {
        return rules;
        }

    /**
     * Used to insert a new rule into the style sheet. The new rule now
     * 	 becomes part of the cascade. 
     */
    virtual unsigned long insertRule(const DOMString &/*ruleStr*/,
                                     unsigned long index)
                                     throw (dom::DOMException)
        {
        CSSRule rule;
        return rules.insertRule(rule, index);
        }

    /**
     * Used to delete a rule from the style sheet. 
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
        assign(other);
        }

    /**
     *
     */
    CSSStyleSheet &operator=(const CSSStyleSheet &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSStyleSheet &other)
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
 * The CSSValue interface represents a simple or a complex value. A CSSValue 
 * object only occurs in a context of a CSS property.
 */
class CSSValue
{
public:

    /**
     * An integer indicating which type of unit applies to the value.
     */
    typedef enum
        {
        CSS_INHERIT         = 0,
        CSS_PRIMITIVE_VALUE = 1,
        CSS_VALUE_LIST      = 2,
        CSS_CUSTOM          = 3
        } UnitTypes;

    /**
     * A code defining the type of the value as defined above.
     */
    virtual unsigned short getCssValueType()
        {
        return valueType;
        }

    /**
     * A string representation of the current value.
     */
    virtual DOMString getCssText()
        {
        return cssText;
        }

    /**
     * A string representation of the current value.
     * Note that setting implies parsing.     
     */
    virtual void setCssText(const DOMString &val)
                            throw (dom::DOMException)
        {
        cssText = val;
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
        assign(other);
        }

    /**
     *
     */
    CSSValue &operator=(const CSSValue &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSValue &other)
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

/**
 * The CSSStyleDeclaration interface represents a single CSS declaration block. 
 * This interface may be used to determine the style properties currently set in 
 * a block or to set style properties explicitly within the block.
 * 
 * While an implementation may not recognize all CSS properties within a CSS 
 * declaration block, it is expected to provide access to all specified 
 * properties in the style sheet through the CSSStyleDeclaration interface. 
 * Furthermore, implementations that support a specific level of CSS should 
 * correctly handle CSS shorthand properties for that level. For a further 
 * discussion of shorthand properties, see the CSS2Properties interface.
 * 
 * This interface is also used to provide a read-only access to the computed 
 * values of an element. See also the ViewCSS interface.
 * 
 * Note: The CSS Object Model doesn't provide an access to the specified or 
 * actual values of the CSS cascade.
 */
class CSSStyleDeclaration
{
private:

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
	

public:

    /**
     * The parsable textual representation of the declaration block (excluding the 
     * surrounding curly braces).
     */
    virtual DOMString getCssText()
        {
        return cssText;
        }

    /**
     * The parsable textual representation of the declaration block (excluding the 
     * surrounding curly braces). Setting this attribute will result in the parsing 
     * of the new value and resetting of all the properties in the declaration block 
     * including the removal or addition of properties.
     */
    virtual void setCssText(const DOMString &val)
                            throw (dom::DOMException)
        {
        cssText = val;
        }

    /**
     * Used to retrieve the value of a CSS property if it has been explicitly
     * 	 set within this declaration block. 
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
     * Used to retrieve the object representation of the value of a CSS property if 
     * it has been explicitly set within this declaration block. This method returns 
     * null if the property is a shorthand property. Shorthand property values can 
     * only be accessed and modified as strings, using the getPropertyValue and 
     * setProperty methods.
     */
    virtual CSSValue getPropertyCSSValue(const DOMString &/*propertyName*/)
        {
        CSSValue value;
        return value;
        }

    /**
     * Used to remove a CSS property if it has been explicitly set within
     * 	 this declaration block. 
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
     * Used to retrieve the priority of a CSS property (e.g. the "important" 
     * qualifier) if the property has been explicitly set in this declaration block.
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
     * Used to set a property value and priority within this declaration block. 
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
     * The number of properties that have been explicitly set in this declaration 
     * block. The range of valid indices is 0 to length-1 inclusive.
     */
    virtual unsigned long getLength()
        {
        return items.size();
        }

    /**
     * Used to retrieve the properties that have been explicitly set in this 
     * declaration block. The order of the properties retrieved using this method 
     * does not have to be the order in which they were set. This method can be used 
     * to iterate over all properties in this declaration block.
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
     * The CSS rule that contains this declaration block or null if this 
     * CSSStyleDeclaration is not attached to a CSSRule.
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
    CSSStyleDeclaration(const CSSStyleDeclaration &other)
        {
        assign(other);
        }

    /**
     *
     */
    CSSStyleDeclaration &operator=(const CSSStyleDeclaration &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSStyleDeclaration &other)
        {
        parentRule = other.parentRule;
        cssText    = other.cssText;
        items      = other.items;
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
 * The CSSStyleRule interface represents a single rule set in a CSS style sheet.
 */
class CSSStyleRule : virtual public CSSRule
{
public:

    /**
     * The textual representation of the selector for the rule set. The 
     * implementation may have stripped out insignificant whitespace while parsing 
     * the selector.
     */
    virtual DOMString getSelectorText()
        {
        return selectorText;
        }

    /**
     * The textual representation of the selector for the rule set. The 
     * implementation may have stripped out insignificant whitespace while parsing 
     * the selector.  Setting implies reparsing.
     */
    virtual void setSelectorText(const DOMString &val)
                throw (dom::DOMException)
        {
        selectorText = val;
        }


    /**
     * The declaration-block of this rule set.
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
        assign(other);
        }

    /**
     *
     */
    CSSStyleRule &operator=(const CSSStyleRule &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSStyleRule &other)
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
 * The CSSMediaRule interface represents a @media rule in a CSS style sheet. A 
 * @media rule can be used to delimit style rules for specific media types.
 */
class CSSMediaRule : virtual public CSSRule
{
public:

    /**
     * A list of media types for this rule.
     */
    virtual stylesheets::MediaList getMedia()
        {
        return mediaList;
        }

    /**
     * A list of all CSS rules contained within the media block.
     */
    virtual CSSRuleList getCssRules()
        {
        return cssRules;
        }

    /**
     * Used to insert a new rule into the media block. 
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
     * Used to delete a rule from the media block. 
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
        assign(other);
        }

    /**
     *
     */
    CSSMediaRule &operator=(const CSSMediaRule &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSMediaRule &other)
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
 * The CSSFontFaceRule interface represents a @font-face rule in a CSS style 
 * sheet. The @font-face rule is used to hold a set of font descriptions.
 */
class CSSFontFaceRule : virtual public CSSRule
{
public:

    /**
     * The declaration-block of this rule.
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
    CSSFontFaceRule &operator=(const CSSFontFaceRule &other)
        {
        style = other.style;
        return *this;
        }

    /**
     *
     */
    void assign(const CSSFontFaceRule &other)
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
 * The CSSPageRule interface represents a @page rule within a CSS style sheet. 
 * The @page rule is used to specify the dimensions, orientation, margins, etc. 
 * of a page box for paged media.
 */
class CSSPageRule : virtual public CSSRule
{
public:

    /**
     * The parsable textual representation of the page selector for the rule.
     */
    virtual DOMString getSelectorText()
        {
        return selectorText;
        }

    /**
     * The parsable textual representation of the page selector for the rule.
     * Setting implies parsing.     
     */
    virtual void setSelectorText(const DOMString &val)
                         throw(dom::DOMException)
        {
        selectorText = val;
        }


    /**
     * The declaration-block of this rule.
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
        assign(other);
        }

    /**
     *
     */
    CSSPageRule &operator=(const CSSPageRule &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSPageRule &other)
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
 * The CSSImportRule interface represents a @import rule within a CSS style 
 * sheet. The @import rule is used to import style rules from other style sheets.
 */
class CSSImportRule : virtual public CSSRule
{
public:

    /**
     * The location of the style sheet to be imported. The attribute will not contain 
     * the "url(...)" specifier around the URI.
     */
    virtual DOMString getHref()
        {
        return href;
        }

    /**
     * A list of media types for which this style sheet may be used.
     */
    virtual stylesheets::MediaList getMedia()
        {
        return mediaList;
        }

    /**
     * The style sheet referred to by this rule, if it has been loaded. The value of 
     * this attribute is null if the style sheet has not yet been loaded or if it 
     * will not be loaded (e.g. if the style sheet is for a media type not supported 
     * by the user agent).
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
        assign(other);
        }

    /**
     *
     */
    CSSImportRule &operator=(const CSSImportRule &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSSImportRule &other)
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
 * The CSSCharsetRule interface represents a @charset rule in a CSS style sheet. 
 * The value of the encoding attribute does not affect the encoding of text data 
 * in the DOM objects; this encoding is always UTF-16. After a stylesheet is 
 * loaded, the value of the encoding attribute is the value found in the @charset 
 * rule. If there was no @charset in the original document, then no 
 * CSSCharsetRule is created. The value of the encoding attribute may also be 
 * used as a hint for the encoding used on serialization of the style sheet.
 * 
 * The value of the @charset rule (and therefore of the CSSCharsetRule) may not 
 * correspond to the encoding the document actually came in; character encoding 
 * information e.g. in an HTTP header, has priority (see CSS document 
 * representation) but this is not reflected in the CSSCharsetRule.
 */
class CSSCharsetRule : virtual public CSSRule
{
public:

    /**
     * The encoding information used in this @charset rule.
     */
    virtual DOMString getEncoding()
        {
        return encoding;
        }

    /**
     * The encoding information used in this @charset rule.
     * Setting implies parsing.     
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
    CSSCharsetRule &operator=(const CSSCharsetRule &other)
        {
        encoding = other.encoding;
        return *this;
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
 * The CSSUnknownRule interface represents an at-rule not supported by
 *  this user agent.
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
    CSSUnknownRule &operator=(const CSSUnknownRule &/*other*/)
        {
        return *this;
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
 * The CSSValueList interface provides the abstraction of an ordered collection 
 * of CSS values.
 * 
 * Some properties allow an empty list into their syntax. In that case, these 
 * properties take the none identifier. So, an empty list means that the property 
 * has the value none.
 * 
 * The items in the CSSValueList are accessible via an integral index, starting 
 * from 0.
 */
class CSSValueList : virtual public CSSValue
{
public:

    /**
     * The number of CSSValues in the list. The range of valid values of the indices 
     * is 0 to length-1 inclusive.
     */
    virtual unsigned long getLength()
        {
        return items.size();
        }

    /**
     * Used to retrieve a CSSValue by ordinal index. The order in this collection 
     * represents the order of the values in the CSS style property. If index is 
     * greater than or equal to the number of values in the list, this returns null.
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
    CSSValueList &operator=(const CSSValueList &other)
        {
        items = other.items;
        return *this;
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
 * The CSSPrimitiveValue interface represents a single CSS value. This interface 
 * may be used to determine the value of a specific style property currently set 
 * in a block or to set a specific style property explicitly within the block. An 
 * instance of this interface might be obtained from the getPropertyCSSValue 
 * method of the CSSStyleDeclaration interface. A CSSPrimitiveValue object only 
 * occurs in a context of a CSS property.
 * 
 * Conversions are allowed between absolute values (from millimeters to 
 * centimeters, from degrees to radians, and so on) but not between relative 
 * values. (For example, a pixel value cannot be converted to a centimeter value.)
 * Percentage values can't be converted since they are relative to the parent 
 * value (or another property value). There is one exception for color percentage 
 * values: since a color percentage value is relative to the range 0-255, a color 
 * percentage value can be converted to a number; (see also the RGBColor 
 * interface).
 */
class CSSPrimitiveValue : virtual public CSSValue
{
public:

    /**
     *An integer indicating which type of unit applies to the value.
     */
    typedef enum
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
        } UnitTypes;


    /**
     * The type of the value as defined by the constants specified above.
     */
    virtual unsigned short getPrimitiveType()
        {
        return primitiveType;
        }

    /**
     * A method to set the float value with a specified unit. If the property 
     * attached with this value can not accept the specified unit or the float value, 
     * the value will be unchanged and a DOMException will be raised.
     */
    virtual void setFloatValue(unsigned short unitType,
                               double doubleValueArg)
                               throw (dom::DOMException)
        {
        primitiveType = unitType;
        doubleValue = doubleValueArg;
        }

    /**
     * This method is used to get a float value in a specified unit. If this CSS 
     * value doesn't contain a float value or can't be converted into the specified 
     * unit, a DOMException is raised.
     */
    virtual double getFloatValue(unsigned short /*unitType*/)
                                throw (dom::DOMException)
        {
        return doubleValue;
        }

    /**
     * A method to set the string value with the specified unit. If the property 
     * attached to this value can't accept the specified unit or the string value, 
     * the value will be unchanged and a DOMException will be raised.
     */
    virtual void setStringValue(unsigned short /*stringType*/,
                                const DOMString &stringValueArg)
                                throw (dom::DOMException)
        {
        stringValue = stringValueArg;
        }

    /**
     * This method is used to get the string value. If the CSS value doesn't contain 
     * a string value, a DOMException is raised.
     * 
     * Note: Some properties (like 'font-family' or 'voice-family') convert a 
     * whitespace separated list of idents to a string.
     */
    virtual DOMString getStringValue() throw (dom::DOMException)
        {
        return stringValue;
        }

    /**
     * This method is used to get the Counter value. If this CSS value doesn't 
     * contain a counter value, a DOMException is raised. Modification to the 
     * corresponding style property can be achieved using the Counter interface.
     */
    virtual Counter *getCounterValue() throw (dom::DOMException)
        {
        return NULL;
        }

    /**
     * This method is used to get the Rect value. If this CSS value doesn't contain a 
     * rect value, a DOMException is raised. Modification to the corresponding style 
     * property can be achieved using the Rect interface.
     */
    virtual Rect *getRectValue() throw (dom::DOMException)
        {
        return NULL;
        }

    /**
     * This method is used to get the RGB color. If this CSS value doesn't contain a 
     * RGB color value, a DOMException is raised. Modification to the corresponding 
     * style property can be achieved using the RGBColor interface.
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
    CSSPrimitiveValue &operator=(const CSSPrimitiveValue &/*other*/)
        {
        return *this;
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
 * The RGBColor interface is used to represent any RGB color value. This 
 * interface reflects the values in the underlying style property. Hence, 
 * modifications made to the CSSPrimitiveValue objects modify the style property.
 * 
 * A specified RGB color is not clipped (even if the number is outside the range 
 * 0-255 or 0%-100%). A computed RGB color is clipped depending on the device.
 * 
 * Even if a style sheet can only contain an integer for a color value, the 
 * internal storage of this integer is a float, and this can be used as a float 
 * in the specified or the computed style.
 * 
 * A color percentage value can always be converted to a number and vice versa.
 */
class RGBColor
{
public:

    /**
     * This attribute is used for the red value of the RGB color.
     */
    virtual CSSPrimitiveValue getRed()
        {
        return red;
        }

    /**
     * This attribute is used for the green value of the RGB color.
     */
    virtual CSSPrimitiveValue getGreen()
        {
        return green;
        }

    /**
     * This attribute is used for the blue value of the RGB color.
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
        assign(other);
        }

    /**
     *
     */
    RGBColor &operator=(const RGBColor &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const RGBColor &other)
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
 * The Rect interface is used to represent any rect value. This interface 
 * reflects the values in the underlying style property. Hence, modifications 
 * made to the CSSPrimitiveValue objects modify the style property.
 */
class Rect
{
public:

    /**
     * This attribute is used for the top of the rect.
     */
    virtual CSSPrimitiveValue getTop()
        {
        return top;
        }

    /**
     * This attribute is used for the right of the rect.
     */
    virtual CSSPrimitiveValue getRight()
        {
        return right;
        }

    /**
     * This attribute is used for the bottom of the rect.
     */
    virtual CSSPrimitiveValue getBottom()
        {
        return bottom;
        }

    /**
     * This attribute is used for the left of the rect.
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
        assign(other);
        }

    /**
     *
     */
    Rect &operator=(const Rect &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const Rect &other)
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
 * The Counter interface is used to represent any counter or counters function 
 * value. This interface reflects the values in the underlying style property.
 */
class Counter
{
public:

    /**
     * This attribute is used for the identifier of the counter.
     */
    virtual DOMString getIdentifier()
        {
        return identifier;
        }

    /**
     * This attribute is used for the style of the list.
     */
    virtual DOMString getListStyle()
        {
        return listStyle;
        }

    /**
     * This attribute is used for the separator of the nested counters.
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
        assign(other);
        }

    /**
     *
     */
    Counter &operator=(const Counter &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const Counter &other)
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
 * Inline style information attached to elements is exposed through the style 
 * attribute. This represents the contents of the STYLE attribute for HTML 
 * elements (or elements in other schemas or DTDs which use the STYLE attribute 
 * in the same way). The expectation is that an instance of the 
 * ElementCSSInlineStyle interface can be obtained by using binding-specific 
 * casting methods on an instance of the Element interface when the element 
 * supports inline CSS style informations.
 */
class ElementCSSInlineStyle
{
public:

    /**
     * The style attribute.
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
    ElementCSSInlineStyle &operator=(const ElementCSSInlineStyle &other)
        {
        style = other.style;
        return *this;
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
 * The CSS2Properties interface represents a convenience mechanism for retrieving 
 * and setting properties within a CSSStyleDeclaration. The attributes of this 
 * interface correspond to all the properties specified in CSS2. Getting an 
 * attribute of this interface is equivalent to calling the getPropertyValue 
 * method of the CSSStyleDeclaration interface. Setting an attribute of this 
 * interface is equivalent to calling the setProperty method of the 
 * CSSStyleDeclaration interface.
 * 
 * A conformant implementation of the CSS module is not required to implement the 
 * CSS2Properties interface. If an implementation does implement this interface, 
 * the expectation is that language-specific methods can be used to cast from an 
 * instance of the CSSStyleDeclaration interface to the CSS2Properties interface.
 * 
 * If an implementation does implement this interface, it is expected to 
 * understand the specific syntax of the shorthand properties, and apply their 
 * semantics; when the margin property is set, for example, the marginTop, 
 * marginRight, marginBottom and marginLeft properties are actually being set by 
 * the underlying implementation.
 * 
 * When dealing with CSS "shorthand" properties, the shorthand properties should 
 * be decomposed into their component longhand properties as appropriate, and 
 * when querying for their value, the form returned should be the shortest form 
 * exactly equivalent to the declarations made in the ruleset. However, if there 
 * is no shorthand declaration that could be added to the ruleset without 
 * changing in any way the rules already declared in the ruleset (i.e., by adding 
 * longhand rules that were previously not declared in the ruleset), then the 
 * empty string should be returned for the shorthand property.
 * 
 * For example, querying for the font property should not return "normal normal 
 * normal 14pt/normal Arial, sans-serif", when "14pt Arial, sans-serif" suffices. 
 * (The normals are initial values, and are implied by use of the longhand 
 * property.)
 * 
 * If the values for all the longhand properties that compose a particular string 
 * are the initial values, then a string consisting of all the initial values 
 * should be returned (e.g. a border-width value of "medium" should be returned 
 * as such, not as "").
 * 
 * For some shorthand properties that take missing values from other sides, such 
 * as the margin, padding, and border-[width|style|color] properties, the minimum 
 * number of sides possible should be used; i.e., "0px 10px" will be returned 
 * instead of "0px 10px 0px 10px".
 * 
 * If the value of a shorthand property can not be decomposed into its component 
 * longhand properties, as is the case for the font property with a value of 
 * "menu", querying for the values of the component longhand properties should 
 * return the empty string.
 *  */
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
        assign(other);
        }

    /**
     *
     */
    CSS2Properties &operator=(const CSS2Properties &other)
        {
        assign(other);
        return *this;
        }

    /**
     *
     */
    void assign(const CSS2Properties &other)
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
 * This interface represents a CSS view. The getComputedStyle method provides a 
 * read only access to the computed values of an element.
 * 
 * The expectation is that an instance of the ViewCSS interface can be obtained 
 * by using binding-specific casting methods on an instance of the AbstractView 
 * interface.
 * 
 * Since a computed style is related to an Element node, if this element is 
 * removed from the document, the associated CSSStyleDeclaration and CSSValue 
 * related to this declaration are no longer valid.
 */
class ViewCSS : virtual public views::AbstractView
{
public:

    /**
     * This method is used to get the computed style as it is defined in [CSS2]. 
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
    ViewCSS &operator=(const ViewCSS &/*other*/)
       {
       return *this;
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
 * This interface represents a document with a CSS view.
 * 
 * The getOverrideStyle method provides a mechanism through which a DOM author 
 * could effect immediate change to the style of an element without modifying the 
 * explicitly linked style sheets of a document or the inline style of elements 
 * in the style sheets. This style sheet comes after the author style sheet in 
 * the cascade algorithm and is called override style sheet. The override style 
 * sheet takes precedence over author style sheets. An "!important" declaration 
 * still takes precedence over a normal declaration. Override, author, and user 
 * style sheets all may contain "!important" declarations. User "!important" 
 * rules take precedence over both override and author "!important" rules, and 
 * override "!important" rules take precedence over author "!important" rules.
 * 
 * The expectation is that an instance of the DocumentCSS interface can be 
 * obtained by using binding-specific casting methods on an instance of the 
 * Document interface.
 */
class DocumentCSS : virtual public stylesheets::DocumentStyle
{
public:

    /**
     * This method is used to retrieve the override style declaration for a specified 
     * element and a specified pseudo-element.
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
    DocumentCSS &operator=(const DocumentCSS &/*other*/)
       {
       return *this;
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
 * This interface allows the DOM user to create a CSSStyleSheet outside the 
 * context of a document. There is no way to associate the new CSSStyleSheet with 
 * a document in DOM Level 2.
 */
class DOMImplementationCSS : virtual public DOMImplementation
{
public:

    /**
     * Creates a new CSSStyleSheet. 
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
    DOMImplementationCSS &operator=(const DOMImplementationCSS &/*other*/)
       {
       return *this;
       }

    /**
     *
     */
    virtual ~DOMImplementationCSS() {}
};








}  //namespace css
}  //namespace dom
}  //namespace w3c
}  //namespace org


#endif /* __CSS_H__ */

/*#########################################################################
## E N D    O F    F I L E
#########################################################################*/
