/*
 * Implementation of the Pedro mini-DOM parser and tree
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



#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "pedrodom.h"

namespace Pedro
{



//########################################################################
//# E L E M E N T
//########################################################################

Element *Element::clone()
{
    Element *elem = new Element(name, value);
    elem->parent     = parent;
    elem->attributes = attributes;
    elem->namespaces = namespaces;

    ElementList::iterator iter;
    for (iter = children.begin(); iter != children.end() ; iter++)
        {
        elem->addChild((*iter)->clone());
        }
    return elem;
}


void Element::findElementsRecursive(std::vector<Element *>&res, const DOMString &name)
{
    if (getName() == name)
        {
        res.push_back(this);
        }
    for (unsigned int i=0; i<children.size() ; i++)
        children[i]->findElementsRecursive(res, name);
}

std::vector<Element *> Element::findElements(const DOMString &name)
{
    std::vector<Element *> res;
    findElementsRecursive(res, name);
    return res;
}

DOMString Element::getAttribute(const DOMString &name)
{
    for (unsigned int i=0 ; i<attributes.size() ; i++)
        if (attributes[i].getName() ==name)
            return attributes[i].getValue();
    return "";
}

DOMString Element::getTagAttribute(const DOMString &tagName, const DOMString &attrName)
{
    ElementList elems = findElements(tagName);
    if (elems.size() <1)
        return "";
    DOMString res = elems[0]->getAttribute(attrName);
    return res;
}

DOMString Element::getTagValue(const DOMString &tagName)
{
    ElementList elems = findElements(tagName);
    if (elems.size() <1)
        return "";
    DOMString res = elems[0]->getValue();
    return res;
}

void Element::addChild(Element *child)
{
    if (!child)
        return;
    child->parent = this;
    children.push_back(child);
}


void Element::addAttribute(const DOMString &name, const DOMString &value)
{
    Attribute attr(name, value);
    attributes.push_back(attr);
}

void Element::addNamespace(const DOMString &prefix, const DOMString &namespaceURI)
{
    Namespace ns(prefix, namespaceURI);
    namespaces.push_back(ns);
}

void Element::writeIndentedRecursive(FILE *f, int indent)
{
    int i;
    if (!f)
        return;
    //Opening tag, and attributes
    for (i=0;i<indent;i++)
        fputc(' ',f);
    fprintf(f,"<%s",name.c_str());
    for (unsigned int i=0 ; i<attributes.size() ; i++)
        {
        fprintf(f," %s=\"%s\"",
              attributes[i].getName().c_str(),
              attributes[i].getValue().c_str());
        }
    for (unsigned int i=0 ; i<namespaces.size() ; i++)
        {
        fprintf(f," xmlns:%s=\"%s\"",
              namespaces[i].getPrefix().c_str(),
              namespaces[i].getNamespaceURI().c_str());
        }
    fprintf(f,">\n");

    //Between the tags
    if (value.size() > 0)
        {
        for (int i=0;i<indent;i++)
            fputc(' ', f);
        fprintf(f," %s\n", value.c_str());
        }

    for (unsigned int i=0 ; i<children.size() ; i++)
        children[i]->writeIndentedRecursive(f, indent+2);

    //Closing tag
    for (int i=0; i<indent; i++)
        fputc(' ',f);
    fprintf(f,"</%s>\n", name.c_str());
}

void Element::writeIndented(FILE *f)
{
    writeIndentedRecursive(f, 0);
}

void Element::print()
{
    writeIndented(stdout);
}


//########################################################################
//# P A R S E R
//########################################################################



typedef struct
    {
    char *escaped;
    char value;
    } EntityEntry;

static EntityEntry entities[] =
{
    { "&amp;" , '&'  },
    { "&lt;"  , '<'  },
    { "&gt;"  , '>'  },
    { "&apos;", '\'' },
    { "&quot;", '"'  },
    { NULL    , '\0' }
};



/**
 *  Removes whitespace from beginning and end of a string
 */
DOMString Parser::trim(const DOMString &s)
{
    if (s.size() < 1)
        return s;
    
    //Find first non-ws char
    unsigned int begin = 0;
    for ( ; begin < s.size() ; begin++)
        {
        if (!isspace(s[begin]))
            break;
        }

    //Find first non-ws char, going in reverse
    unsigned int end = s.size() - 1;
    for ( ; end > begin ; end--)
        {
        if (!isspace(s[end]))
            break;
        }
    //trace("begin:%d  end:%d", begin, end);

    DOMString res = s.substr(begin, end-begin+1);
    return res;
}

void Parser::getLineAndColumn(long pos, long *lineNr, long *colNr)
{
    long line = 1;
    long col  = 1;
    for (long i=0 ; i<pos ; i++)
        {
        XMLCh ch = parsebuf[i];
        if (ch == '\n' || ch == '\r')
            {
            col = 0;
            line ++;
            }
        else
            col++;
        }
    *lineNr = line;
    *colNr  = col;

}


void Parser::error(char *fmt, ...)
{
    long lineNr;
    long colNr;
    getLineAndColumn(currentPosition, &lineNr, &colNr);
    va_list args;
    fprintf(stderr, "xml error at line %ld, column %ld:", lineNr, colNr);
    va_start(args,fmt);
    vfprintf(stderr,fmt,args);
    va_end(args) ;
    fprintf(stderr, "\n");
}



int Parser::peek(long pos)
{
    if (pos >= parselen)
        return -1;
    currentPosition = pos;
    int ch = parsebuf[pos];
    //printf("ch:%c\n", ch);
    return ch;
}



DOMString Parser::encode(const DOMString &str)
{
    DOMString ret;
    for (unsigned int i=0 ; i<str.size() ; i++)
        {
        XMLCh ch = (XMLCh)str[i];
        if (ch == '&')
            ret.append("&amp;");
        else if (ch == '<')
            ret.append("&lt;");
        else if (ch == '>')
            ret.append("&gt;");
        else if (ch == '\'')
            ret.append("&apos;");
        else if (ch == '"')
            ret.append("&quot;");
        else
            ret.push_back(ch);

        }
    return ret;
}


int Parser::match(long p0, const char *text)
{
    int p = p0;
    while (*text)
        {
        if (peek(p) != *text)
            return p0;
        p++; text++;
        }
    return p;
}



int Parser::skipwhite(long p)
{

    while (p<parselen)
        {
        int p2 = match(p, "<!--");
        if (p2 > p)
            {
            p = p2;
            while (p<parselen)
              {
              p2 = match(p, "-->");
              if (p2 > p)
                  {
                  p = p2;
                  break;
                  }
              p++;
              }
          }
      XMLCh b = peek(p);
      if (!isspace(b))
          break;
      p++;
      }
  return p;
}

/* modify this to allow all chars for an element or attribute name*/
int Parser::getWord(int p0, DOMString &buf)
{
    int p = p0;
    while (p<parselen)
        {
        XMLCh b = peek(p);
        if (b<=' ' || b=='/' || b=='>' || b=='=')
            break;
        buf.push_back(b);
        p++;
        }
    return p;
}

int Parser::getQuoted(int p0, DOMString &buf, int do_i_parse)
{

    int p = p0;
    if (peek(p) != '"' && peek(p) != '\'')
        return p0;
    p++;

    while ( p<parselen )
        {
        XMLCh b = peek(p);
        if (b=='"' || b=='\'')
            break;
        if (b=='&' && do_i_parse)
            {
            bool found = false;
            for (EntityEntry *ee = entities ; ee->value ; ee++)
                {
                int p2 = match(p, ee->escaped);
                if (p2>p)
                    {
                    buf.push_back(ee->value);
                    p = p2;
                    found = true;
                    break;
                    }
                }
            if (!found)
                {
                error("unterminated entity");
                return false;
                }
            }
        else
            {
            buf.push_back(b);
            p++;
            }
        }
    return p;
}

int Parser::parseVersion(int p0)
{
    //printf("### parseVersion: %d\n", p0);

    int p = p0;

    p = skipwhite(p0);

    if (peek(p) != '<')
        return p0;

    p++;
    if (p>=parselen || peek(p)!='?')
        return p0;

    p++;

    DOMString buf;

    while (p<parselen)
        {
        XMLCh ch = peek(p);
        if (ch=='?')
            {
            p++;
            break;
            }
        buf.push_back(ch);
        p++;
        }

    if (peek(p) != '>')
        return p0;
    p++;

    //printf("Got version:%s\n",buf.c_str());
    return p;
}

int Parser::parseDoctype(int p0)
{
    //printf("### parseDoctype: %d\n", p0);

    int p = p0;
    p = skipwhite(p);

    if (p>=parselen || peek(p)!='<')
        return p0;

    p++;

    if (peek(p)!='!' || peek(p+1)=='-')
        return p0;
    p++;

    DOMString buf;
    while (p<parselen)
        {
        XMLCh ch = peek(p);
        if (ch=='>')
            {
            p++;
            break;
            }
        buf.push_back(ch);
        p++;
        }

    //printf("Got doctype:%s\n",buf.c_str());
    return p;
}

int Parser::parseElement(int p0, Element *par,int depth)
{

    int p = p0;

    int p2 = p;

    p = skipwhite(p);

    //## Get open tag
    XMLCh ch = peek(p);
    if (ch!='<')
        return p0;

    p++;

    DOMString openTagName;
    p = skipwhite(p);
    p = getWord(p, openTagName);
    //printf("####tag :%s\n", openTagName.c_str());
    p = skipwhite(p);

    //Add element to tree
    Element *n = new Element(openTagName);
    n->parent = par;
    par->addChild(n);

    // Get attributes
    if (peek(p) != '>')
        {
        while (p<parselen)
            {
            p = skipwhite(p);
            ch = peek(p);
            //printf("ch:%c\n",ch);
            if (ch=='>')
                break;
            else if (ch=='/' && p<parselen+1)
                {
                p++;
                p = skipwhite(p);
                ch = peek(p);
                if (ch=='>')
                    {
                    p++;
                    //printf("quick close\n");
                    return p;
                    }
                }
            DOMString attrName;
            p2 = getWord(p, attrName);
            if (p2==p)
                break;
            //printf("name:%s",buf);
            p=p2;
            p = skipwhite(p);
            ch = peek(p);
            //printf("ch:%c\n",ch);
            if (ch!='=')
                break;
            p++;
            p = skipwhite(p);
            // ch = parsebuf[p];
            // printf("ch:%c\n",ch);
            DOMString attrVal;
            p2 = getQuoted(p, attrVal, true);
            p=p2+1;
            //printf("name:'%s'   value:'%s'\n",attrName.c_str(),attrVal.c_str());
            char *namestr = (char *)attrName.c_str();
            if (strncmp(namestr, "xmlns:", 6)==0)
                n->addNamespace(attrName, attrVal);
            else
                n->addAttribute(attrName, attrVal);
            }
        }

    bool cdata = false;

    p++;
    // ### Get intervening data ### */
    DOMString data;
    while (p<parselen)
        {
        //# COMMENT
        p2 = match(p, "<!--");
        if (!cdata && p2>p)
            {
            p = p2;
            while (p<parselen)
                {
                p2 = match(p, "-->");
                if (p2 > p)
                    {
                    p = p2;
                    break;
                    }
                p++;
                }
            }

        ch = peek(p);
        //# END TAG
        if (ch=='<' && !cdata && peek(p+1)=='/')
            {
            break;
            }
        //# CDATA
        p2 = match(p, "<![CDATA[");
        if (p2 > p)
            {
            cdata = true;
            p = p2;
            continue;
            }

        //# CHILD ELEMENT
        if (ch == '<')
            {
            p2 = parseElement(p, n, depth+1);
            if (p2 == p)
                {
                /*
                printf("problem on element:%s.  p2:%d p:%d\n",
                      openTagName.c_str(), p2, p);
                */
                return p0;
                }
            p = p2;
            continue;
            }
        //# ENTITY
        if (ch=='&' && !cdata)
            {
            bool found = false;
            for (EntityEntry *ee = entities ; ee->value ; ee++)
                {
                int p2 = match(p, ee->escaped);
                if (p2>p)
                    {
                    data.push_back(ee->value);
                    p = p2;
                    found = true;
                    break;
                    }
                }
            if (!found)
                {
                error("unterminated entity");
                return -1;
                }
            continue;
            }

        //# NONE OF THE ABOVE
        data.push_back(ch);
        p++;
        }/*while*/


    n->value = data;
    //printf("%d : data:%s\n",p,data.c_str());

    //## Get close tag
    p = skipwhite(p);
    ch = peek(p);
    if (ch != '<')
        {
        error("no < for end tag\n");
        return p0;
        }
    p++;
    ch = peek(p);
    if (ch != '/')
        {
        error("no / on end tag");
        return p0;
        }
    p++;
    ch = peek(p);
    p = skipwhite(p);
    DOMString closeTagName;
    p = getWord(p, closeTagName);
    if (openTagName != closeTagName)
        {
        error("Mismatched closing tag.  Expected </%s>. Got '%s'.",
                openTagName.c_str(), closeTagName.c_str());
        return p0;
        }
    p = skipwhite(p);
    if (peek(p) != '>')
        {
        error("no > on end tag for '%s'", closeTagName.c_str());
        return p0;
        }
    p++;
    // printf("close element:%s\n",closeTagName.c_str());
    p = skipwhite(p);
    return p;
}




Element *Parser::parse(XMLCh *buf,int pos,int len)
{
    parselen = len;
    parsebuf = buf;
    Element *rootNode = new Element("root");
    pos = parseVersion(pos);
    pos = parseDoctype(pos);
    pos = parseElement(pos, rootNode, 0);
    return rootNode;
}


Element *Parser::parse(const char *buf, int pos, int len)
{
    XMLCh *charbuf = new XMLCh[len + 1];
    long i = 0;
    for ( ; i < len ; i++)
        charbuf[i] = (XMLCh)buf[i];
    charbuf[i] = '\0';

    Element *n = parse(charbuf, pos, len);
    delete[] charbuf;
    return n;
}

Element *Parser::parse(const DOMString &buf)
{
    long len = (long)buf.size();
    XMLCh *charbuf = new XMLCh[len + 1];
    long i = 0;
    for ( ; i < len ; i++)
        charbuf[i] = (XMLCh)buf[i];
    charbuf[i] = '\0';

    Element *n = parse(charbuf, 0, len);
    delete[] charbuf;
    return n;
}

Element *Parser::parseFile(const DOMString &fileName)
{

    //##### LOAD INTO A CHAR BUF, THEN CONVERT TO XMLCh
    FILE *f = fopen(fileName.c_str(), "rb");
    if (!f)
        return NULL;

    struct stat   statBuf;
    if (fstat(fileno(f),&statBuf)<0)
        {
        fclose(f);
        return NULL;
        }
    long filelen = statBuf.st_size;

    //printf("length:%d\n",filelen);
    XMLCh *charbuf = new XMLCh[filelen + 1];
    for (XMLCh *p=charbuf ; !feof(f) ; p++)
        {
        *p = (XMLCh)fgetc(f);
        }
    fclose(f);
    charbuf[filelen] = '\0';


    /*
    printf("nrbytes:%d\n",wc_count);
    printf("buf:%ls\n======\n",charbuf);
    */
    Element *n = parse(charbuf, 0, filelen);
    delete [] charbuf;
    return n;
}







}//namespace Pedro

#if 0
//########################################################################
//#  T E S T
//########################################################################

bool doTest(char *fileName)
{
    Pedro::Parser parser;

    Pedro::Element *elem = parser.parseFile(fileName);

    if (!elem)
        {
        printf("Parsing failed\n");
        return false;
        }

    elem->print();

    delete elem;

    return true;
}



int main(int argc, char **argv)
{
    if (argc != 2)
        {
        printf("usage: %s <xmlfile>\n", argv[0]);
        return 1;
        }

    if (!doTest(argv[1]))
        return 1;

    return 0;
}

#endif

//########################################################################
//#  E N D    O F    F I L E
//########################################################################


