/*
 * Inkscape::Debug::Logger - debug logging facility
 *
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 MenTaLguY
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <fstream>
#include <vector>
#include <glib/gmessages.h>
#include "debug/logger.h"
#include "debug/simple-event.h"
#include "gc-alloc.h"

namespace Inkscape {

namespace Debug {

bool Logger::_enabled=false;
bool Logger::_category_mask[Event::N_CATEGORIES];

namespace {

static void write_escaped_value(std::ostream &os, Util::ptr_shared<char> value) {
    for ( char const *current=value ; *current ; ++current ) {
        switch (*current) {
        case '&':
            os << "&amp;";
            break;
        case '"':
            os << "&quot;";
            break;
        case '\'':
            os << "&apos;";
            break;
        case '<':
            os << "&lt;";
            break;
        case '>':
            os << "&gt;";
            break;
        default:
            os.put(*current);
        }
    }
}

static void write_indent(std::ostream &os, unsigned depth) {
    for ( unsigned i = 0 ; i < depth ; i++ ) {
        os.write("  ", 2);
    }
}

static std::ofstream log_stream;
static bool empty_tag=false;
typedef std::vector<Util::ptr_shared<char>, GC::Alloc<Util::ptr_shared<char>, GC::MANUAL> > TagStack;
static TagStack &tag_stack() {
    static TagStack stack;
    return stack;
}

static void do_shutdown() {
    Debug::Logger::shutdown();
}

static bool equal_range(char const *c_string,
                        char const *start, char const *end)
{
    return !std::strncmp(start, c_string, end - start) &&
           !c_string[end - start];
}

static void set_category_mask(bool * const mask, char const *filter) {
    if (!filter) {
        for ( unsigned i = 0 ; i < Event::N_CATEGORIES ; i++ ) {
            mask[i] = true;
        }
        return;
    } else {
        for ( unsigned i = 0 ; i < Event::N_CATEGORIES ; i++ ) {
            mask[i] = false;
        }
        mask[Event::CORE] = true;
    }

    char const *start;
    char const *end;
    start = end = filter;
    while (*end) {
        while ( *end && *end != ',' ) { end++; }
        if ( start != end ) {
            if (equal_range("CORE", start, end)) {
                mask[Event::CORE] = true;
            } else if (equal_range("XML", start, end)) {
                mask[Event::XML] = true;
            } else if (equal_range("SPOBJECT", start, end)) {
                mask[Event::SPOBJECT] = true;
            } else if (equal_range("DOCUMENT", start, end)) {
                mask[Event::DOCUMENT] = true;
            } else if (equal_range("REFCOUNT", start, end)) {
                mask[Event::REFCOUNT] = true;
            } else if (equal_range("EXTENSION", start, end)) {
                mask[Event::EXTENSION] = true;
            } else {
                g_warning("Unknown debugging category %*s", end - start, start);
            }
        }
        if (*end) {
            start = end = end + 1;
        }
    }
}

}

void Logger::init() {
    if (!_enabled) {
        char const *log_filename=std::getenv("INKSCAPE_DEBUG_LOG");
        if (log_filename) {
            log_stream.open(log_filename);
            if (log_stream.is_open()) {
                char const *log_filter=std::getenv("INKSCAPE_DEBUG_FILTER");
                set_category_mask(_category_mask, log_filter);
                log_stream << "<?xml version=\"1.0\"?>\n";
                log_stream.flush();
                _enabled = true;
                start<SimpleEvent<Event::CORE> >(Util::share_static_string("session"));
                std::atexit(&do_shutdown);
            }
        }
    }
}

void Logger::_start(Event const &event) {
    Util::ptr_shared<char> name=event.name();

    if (empty_tag) {
        log_stream << ">\n";
    }

    write_indent(log_stream, tag_stack().size());

    log_stream << "<" << name.pointer();

    unsigned property_count=event.propertyCount();
    for ( unsigned i = 0 ; i < property_count ; i++ ) {
        Event::PropertyPair property=event.property(i);
        log_stream << " " << property.name.pointer() << "=\"";
        write_escaped_value(log_stream, property.value);
        log_stream << "\"";
    }

    log_stream.flush();

    tag_stack().push_back(name);
    empty_tag = true;
}

void Logger::_skip() {
    tag_stack().push_back(Util::ptr_shared<char>());
}

void Logger::_finish() {
    if (tag_stack().back()) {
        if (empty_tag) {
            log_stream << "/>\n";
        } else {
            write_indent(log_stream, tag_stack().size() - 1);
            log_stream << "</" << tag_stack().back().pointer() << ">\n";
        }
        log_stream.flush();

        empty_tag = false;
    }

    tag_stack().pop_back();
}

void Logger::shutdown() {
    if (_enabled) {
        while (!tag_stack().empty()) {
            finish();
        }
    }
}

}

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
