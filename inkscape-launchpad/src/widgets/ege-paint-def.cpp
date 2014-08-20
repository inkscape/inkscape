/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Eek Color Definition.
 *
 * The Initial Developer of the Original Code is
 * Jon A. Cruz.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include "config.h"

#ifdef HAVE_LIBINTL_H
#include <libintl.h>
#endif

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <glibmm/i18n.h>
#include <glibmm/stringutils.h>

#if !defined(_)
#define _(s) gettext(s)
#endif // !defined(_)

#include "ege-paint-def.h"

namespace ege
{

static std::string mimeTEXT("text/plain");
static std::string mimeX_COLOR("application/x-color");
static std::string mimeOSWB_COLOR("application/x-oswb-color");

PaintDef::PaintDef() :
    descr(_("none")),
    type(NONE),
    r(0),
    g(0),
    b(0),
    editable(false),
    _listeners()
{
}

PaintDef::PaintDef( ColorType type ) :
    descr(),
    type(type),
    r(0),
    g(0),
    b(0),
    editable(false),
    _listeners()
{
    switch (type) {
        case CLEAR:
            descr = _("remove");
            break;
        case NONE:
            descr = _("none");
            break;
        case RGB:
            descr = "";
            break;
    }
}

PaintDef::PaintDef( unsigned int r, unsigned int g, unsigned int b, const std::string& description ) :
    descr(description),
    type(RGB),
    r(r),
    g(g),
    b(b),
    editable(false),
    _listeners()
{
}

PaintDef::~PaintDef()
{
}

PaintDef::PaintDef( PaintDef const &other )
{
    if ( this != &other ) {
        *this = other;
    }
}

PaintDef& PaintDef::operator=( PaintDef const &other )
{
    if ( this != & other )
    {
        type = other.type;
        r = other.r;
        g = other.g;
        b = other.b;
        descr = other.descr;
        editable = other.editable;
        //TODO: _listeners should be assigned a value
    }
    return *this;
}

class PaintDef::HookData {
public:
    HookData( ColorCallback cb, void* data ) {_cb = cb; _data = data;}
    ColorCallback _cb;
    void* _data;
};


std::vector<std::string> PaintDef::getMIMETypes()
{
    std::vector<std::string> listing;
    listing.push_back(mimeOSWB_COLOR);
    listing.push_back(mimeX_COLOR);
    listing.push_back(mimeTEXT);
    return listing;
}

void PaintDef::getMIMEData(std::string const & type, char*& dest, int& len, int& format)
{
    if ( type == mimeTEXT ) {
        dest = new char[8];
        snprintf( dest, 8, "#%02x%02x%02x", getR(), getG(), getB() );
        dest[7] = 0;
        len = 8;
        format = 8;
    } else if ( type == mimeX_COLOR ) {
        uint16_t* tmp = new uint16_t[4];
        tmp[0] = (getR() << 8) | getR();
        tmp[1] = (getG() << 8) | getG();
        tmp[2] = (getB() << 8) | getB();
        tmp[3] = 0xffff;
        dest = reinterpret_cast<char*>(tmp);
        len = 8;
        format = 16;
    } else if ( type == mimeOSWB_COLOR ) {
        std::string tmp("<paint>");
        switch ( getType() ) {
            case ege::PaintDef::NONE:
            {
                tmp += "<nocolor/>";
            }
            break;
            case ege::PaintDef::CLEAR:
            {
                tmp += "<clear/>";
            }
            break;
            default:
            {
                tmp += std::string("<color name=\"") + descr + "\">";
                tmp += "<sRGB r=\"";
                tmp += Glib::Ascii::dtostr(getR()/255.0);
                tmp += "\" g=\"";
                tmp += Glib::Ascii::dtostr(getG()/255.0);
                tmp += "\" b=\"";
                tmp += Glib::Ascii::dtostr(getB()/255.0);
                tmp += "\"/>";
                tmp += "</color>";
            }
        }
        tmp += "</paint>";
        len = tmp.size();
        dest = new char[len];
        // Note that this is not null-terminated:
        memcpy(dest, tmp.c_str(), len);
        format = 8;
    } else {
        // nothing
        dest = 0;
        len = 0;
    }
}

bool PaintDef::fromMIMEData(std::string const & type, char const * data, int len, int /*format*/)
{
    bool worked = false;
    bool changed = false;
    if ( type == mimeTEXT ) {
    } else if ( type == mimeX_COLOR ) {
    } else if ( type == mimeOSWB_COLOR ) {
        std::string xml(data, len);
        if ( xml.find("<nocolor/>") != std::string::npos ) {
            if ( (this->type != ege::PaintDef::NONE)
                 || (this->r != 0)
                 || (this->g != 0)
                 || (this->b != 0) ) {
                this->type = ege::PaintDef::NONE;
                this->r = 0;
                this->g = 0;
                this->b = 0;
                changed = true;
            }
            worked = true;
        } else {
            size_t pos = xml.find("<sRGB");
            if ( pos != std::string::npos ) {
                size_t endPos = xml.find(">", pos);
                std::string srgb = xml.substr(pos, endPos);
                this->type = ege::PaintDef::RGB;
                size_t numPos = srgb.find("r=");
                if (numPos != std::string::npos) {
                    double dbl = Glib::Ascii::strtod(srgb.substr(numPos + 3));
                    this->r = static_cast<int>(255 * dbl);
                }
                numPos = srgb.find("g=");
                if (numPos != std::string::npos) {
                    double dbl = Glib::Ascii::strtod(srgb.substr(numPos + 3));
                    this->g = static_cast<int>(255 * dbl);
                }
                numPos = srgb.find("b=");
                if (numPos != std::string::npos) {
                    double dbl = Glib::Ascii::strtod(srgb.substr(numPos + 3));
                    this->b = static_cast<int>(255 * dbl);
                }

                size_t pos = xml.find("<color ");
                if ( pos != std::string::npos ) {
                    size_t endPos = xml.find(">", pos);
                    std::string colorTag = xml.substr(pos, endPos);

                    size_t namePos = colorTag.find("name=");
                    if (namePos != std::string::npos) {
                        char quote = colorTag[namePos + 5];
                        endPos = colorTag.find(quote, namePos + 6);
                        descr = colorTag.substr(namePos + 6, endPos - (namePos + 6));
                    }
                }
                changed = true;
                worked = true;
            }
        }
    }
    if ( changed ) {
        // beware of callbacks changing things
        for ( std::vector<HookData*>::iterator it = _listeners.begin(); it != _listeners.end(); ++it )
        {
            if ( (*it)->_cb )
            {
                (*it)->_cb( (*it)->_data );
            }
        }
    }
    return worked;
}

void PaintDef::setRGB( unsigned int r, unsigned int g, unsigned int b )
{
    if ( r != this->r || g != this->g || b != this->b ) {
        this->r = r;
        this->g = g;
        this->b = b;

        // beware of callbacks changing things
        for ( std::vector<HookData*>::iterator it = _listeners.begin(); it != _listeners.end(); ++it )
        {
            if ( (*it)->_cb )
            {
                (*it)->_cb( (*it)->_data );
            }
        }
    }
}

void PaintDef::addCallback( ColorCallback cb, void* data )
{
    _listeners.push_back( new HookData(cb, data) );
}

void PaintDef::removeCallback( ColorCallback /*cb*/, void* /*data*/ )
{
}


} // namespace ege

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
