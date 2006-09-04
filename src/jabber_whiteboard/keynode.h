/**
 * Inkscape::Whiteboard::KeyNodeTable - structure for lookup of values from keys
 * and vice versa
 *
 * Authors:
 * Bob Jamison
 *
 * Copyright (c) 2005 Authors
 */
#ifndef __KEY_NODE_H__
#define __KEY_NODE_H__

#include <glibmm.h>

#include <vector>

#include "xml/node.h"
#include "jabber_whiteboard/defines.h"

namespace Inkscape
{
namespace Whiteboard
{

class KeyNodePair
{
public:

    KeyNodePair(const Glib::ustring &keyArg, const XML::Node *nodeArg)
    {
        this->key  = keyArg; 
        this->node = (XML::Node *)nodeArg;
        this->version = 0;
        this->index = 0;
        this->history.push_back(Configure("",""));
    }

    KeyNodePair(const Glib::ustring &keyArg, const XML::Node *nodeArg,
        unsigned int version, signed int index)
    {
        this->key  = keyArg; 
        this->node = (XML::Node *)nodeArg;
        this->version = version;
        this->index = index;
        this->history.push_back(Configure("",""));
    }

    KeyNodePair(const KeyNodePair &other)
    {
        this->key  = other.key; 
        this->node = other.node;
        this->version = other.version;
        this->index = other.index;
        this->history = other.history;
    }

    virtual ~KeyNodePair() {}

    Glib::ustring key;
    XML::Node *node;
    unsigned int version;
    signed int index;
    std::list< Configure > history;
};

class KeyNodeTable
{
public:

    KeyNodeTable()
        { this->counter = 0; }

    KeyNodeTable(const KeyNodeTable &other)
        {
        items = other.items;
        this->counter = 0;
        }

    virtual ~KeyNodeTable()
        {}

    virtual void clear();

    virtual void append(const KeyNodeTable &other);

    virtual void put(const KeyNodePair &pair);

    virtual void put(const Glib::ustring &key, const XML::Node *node);

    virtual XML::Node * get(const Glib::ustring &key) const;

    virtual void remove(const Glib::ustring &key);

    virtual Glib::ustring get(XML::Node *node) const;

    virtual void remove(XML::Node *node);

    virtual unsigned int size() const;

    virtual KeyNodePair item(unsigned int index) const;

    virtual Glib::ustring generateKey(Glib::ustring);

    virtual unsigned int getVersion(XML::Node *node);

    virtual unsigned int incrementVersion(XML::Node *node);

    virtual void addHistory(XML::Node *node, Glib::ustring attribute, Glib::ustring value);

    virtual Glib::ustring getLastHistory(XML::Node *node, Glib::ustring attribute);

private:

    std::vector<KeyNodePair> items;

    unsigned int counter;

};



} // namespace Whiteboard

} // namespace Inkscape


#endif /* __KEY_NODE_H__ */


