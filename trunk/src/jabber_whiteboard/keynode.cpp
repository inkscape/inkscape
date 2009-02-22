/**
 * Inkscape::Whiteboard::KeyNodeTable - structure for lookup of values from keys
 * and vice versa
 *
 * Authors:
 * Bob Jamison
 *
 * Copyright (c) 2005 Authors
 */
#include "keynode.h"
#include "util/ucompose.hpp"

namespace Inkscape
{
namespace Whiteboard
{



void KeyNodeTable::clear()
{
    items.clear();
}

void KeyNodeTable::append(const KeyNodeTable &other)
{
    for (unsigned int i = 0; i<other.size() ; i++)
        {
        KeyNodePair pair = other.item(i);
        put(pair);
        }
}

void KeyNodeTable::put(const KeyNodePair &pair)
{
    put(pair.key, pair.node);
}

void KeyNodeTable::put(const Glib::ustring &key, const XML::Node *node)
{
    //delete existing
    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; )
        {
        if (key == iter->key || node == iter->node)
            iter = items.erase(iter);
        else
            iter++;
        }

    //add new
    KeyNodePair pair(key, node);
    items.push_back(pair);
}

XML::Node * KeyNodeTable::get(const Glib::ustring &key) const
{
    std::vector<KeyNodePair>::const_iterator iter;
    for (iter = items.begin() ; iter != items.end() ; iter++)
        {
        if (key == iter->key)
            return iter->node;
        }
    return NULL;
}


void KeyNodeTable::remove(const Glib::ustring &key)
{
    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; )
        {
        if (key == iter->key)
            iter = items.erase(iter);
        else
            iter++;
        }
}


Glib::ustring KeyNodeTable::get(XML::Node *node) const
{
    std::vector<KeyNodePair>::const_iterator iter;
    for (iter = items.begin() ; iter != items.end() ; iter++)
        {
        if (node == iter->node)
            return iter->key;
        }
    return "";
}

unsigned int KeyNodeTable::incrementVersion(XML::Node *node)
{
    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; iter++)
    {
        if (node == iter->node)
            break;
    }
    return ++iter->version;
}

unsigned int KeyNodeTable::getVersion(XML::Node *node)
{
    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; iter++)
    {
        if (node == iter->node)
            break;
    }
    return iter->version;
}

void KeyNodeTable::addHistory(XML::Node *node, Glib::ustring attribute, Glib::ustring value)
{
    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; iter++)
    {
        if (node == iter->node)
        {
            Configure pair(attribute, value);
            iter->history.push_back(pair);
        }
    }
}

Glib::ustring KeyNodeTable::getLastHistory(XML::Node *node, Glib::ustring att)
{
    std::list<Configure> hist;

    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; iter++)
    {
        if (node == iter->node)
            hist = iter->history;
    }

    std::list<Configure>::iterator it;
    for(it = hist.end() ; it != hist.begin() ; it--)
    {
        if(it->first == att)
        {
            //g_warning("hist %s %s",it->first,it->second);
            return it->second;
        }
    }
    return "";
}

void KeyNodeTable::remove(XML::Node *node)
{
    std::vector<KeyNodePair>::iterator iter;
    for (iter = items.begin() ; iter != items.end() ; )
        {
        if (node == iter->node)
            iter = items.erase(iter);
        else
            iter++;
        }
}

unsigned int KeyNodeTable::size() const
{
    return items.size();
}


KeyNodePair KeyNodeTable::item(unsigned int index) const
{
    if (index>=items.size())
        {
        KeyNodePair pair("", NULL);
        return pair;
        }
    return items[index];
}

Glib::ustring 
KeyNodeTable::generateKey(Glib::ustring jid)
{
    return String::ucompose("%1/%2",this->counter++,jid);
}


} // namespace Whiteboard

} // namespace Inkscape
//#########################################################################
//# E N D    O F    F I L E
//#########################################################################
