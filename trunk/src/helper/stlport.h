#ifndef __STL_PORT_H__
#define __STK_PORT_H__


#include <list>
#include <glib/glist.h>
#include <glib/gslist.h>

template <typename T>
class StlConv {
public :
    static void slist(std::list<T> &stlList, const GSList *slist) {
        for (const GSList *l = slist; l != NULL; l = l->next) {
            T item = reinterpret_cast<T>(l->data);
            stlList.push_back(item);
        }
    }
    static void list(std::list<T> &stlList, const GList *list) {
        for (const GList *l = list; l != NULL; l = l->next) {
            T item = reinterpret_cast<T>(l->data);
            stlList.push_back(item);
        }
    }
};

#endif
