/** @file
 * Generic Factory
 *//*
 * Authors:
 *   Markus Engel
 *
 * Copyright (C) 2013 Authors
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef FACTORY_H_SEEN
#define FACTORY_H_SEEN

#include <exception>
#include <map>
#include <string>
#include "xml/node.h"

/**
 * A simple singleton implementation.
 */
template <class T>
struct Singleton {
    static T &instance() {
        static T inst;
        return inst;
    }
};

namespace FactoryExceptions {
class TypeNotRegistered : public std::exception {
public:
    TypeNotRegistered(std::string const &type)
        : std::exception()
        , _type_string(type) {
    }

    virtual ~TypeNotRegistered() throw() {
    }

    char const *what() const throw() {
        return _type_string.c_str();
    }

private:
    std::string const _type_string;
};
} // namespace FactoryExceptions

/**
 * A Factory for creating objects which can be identified by strings.
 */
template <class BaseObject>
class Factory {
public:
    typedef BaseObject *CreateFunction();

    bool registerObject(std::string const &id, CreateFunction *creator) {
        return this->_object_map.insert(std::make_pair(id, creator)).second;
    }

    BaseObject *createObject(std::string const &id) const {
        typename std::map<std::string const, CreateFunction *>::const_iterator it = this->_object_map.find(id);

        if (it == this->_object_map.end()) {
            //throw FactoryExceptions::TypeNotRegistered(id);
            if (!(id.empty() // comments, usually
                  || id == "rdf:RDF" // no SP node yet
                  || id == "inkscape:clipboard" // SP node not necessary
                  || id == "inkscape:_templateinfo")) {
                g_warning("unknown type: %s", id.c_str());
            }
            return NULL;
        }

        return it->second();
    }

private:
    std::map<std::string const, CreateFunction *> _object_map;
};


struct NodeTraits {
    static std::string get_type_string(Inkscape::XML::Node const &node) {
        std::string name;

        switch (node.type()) {
        case Inkscape::XML::TEXT_NODE:
            name = "string";
            break;

        case Inkscape::XML::ELEMENT_NODE: {
            gchar const *const sptype = node.attribute("sodipodi:type");

            if (sptype) {
                name = sptype;
            } else {
                name = node.name();
            }
            break;
        }
        default:
            name = "";
            break;
        }

        return name;
    }
};

#endif

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
