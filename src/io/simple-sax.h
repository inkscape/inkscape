#ifndef SEEN_SIMPLE_SAX_H
#define SEEN_SIMPLE_SAX_H

/*
 * SimpleSAX
 *
 * Authors:
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2004 AUTHORS
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <libxml/parser.h>
#include <glibmm/ustring.h>

namespace Inkscape {
namespace IO
{

class SaxHandler
{
public:
    SaxHandler();
    virtual ~SaxHandler();

    int parseMemory( const char* buffer, int size );
    int parseFile( const char* filename );

    static const char* errToStr( int errVal );

protected:
    virtual void _startDocument() {}
    virtual void _endDocument() {}
    virtual void _startElement(const xmlChar */*name*/, const xmlChar **/*attrs*/) {}
    virtual void _endElement(const xmlChar */*name*/) {}
    virtual void _characters(const xmlChar */*ch*/, int /*len*/) {}

private:
    static void startDocument(void *user_data);
    static void endDocument(void *user_data);
    static void startElement(void *user_data,
                             const xmlChar *name,
                             const xmlChar **attrs);
    static void endElement(void *user_data,
                           const xmlChar *name);
    static void characters(void *  user_data,
                           const xmlChar *ch,
                           int len);

    // Disable:
    SaxHandler(SaxHandler const &);
    SaxHandler &operator=(SaxHandler const &);

    xmlSAXHandler sax;
};



class FlatSaxHandler : public SaxHandler
{
public:
    FlatSaxHandler();
    virtual ~FlatSaxHandler();

protected:
    virtual void _startElement(const xmlChar *name, const xmlChar **attrs);
    virtual void _endElement(const xmlChar *name);
    virtual void _characters(const xmlChar *ch, int len);

    Glib::ustring data;

private:
    // Disable:
    FlatSaxHandler(FlatSaxHandler const &);
    FlatSaxHandler &operator=(FlatSaxHandler const &);
};



} // namespace IO
} // namespace Inkscape


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

#endif // SEEN_SIMPLE_SAX_H
