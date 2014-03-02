#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <map>
#include <glib-object.h>
#include <stddef.h>
#include <string>
#include <sigc++/sigc++.h>

namespace Inkscape
{
class ConnectionPool
{
  public:

    enum Exception
    {
      NAME_EXISTS,
      NAME_DOES_NOT_EXIST
    };

    typedef std::map<std::string, sigc::connection*> ConnectionMap;

    virtual ~ConnectionPool ()
    {
        for (ConnectionMap::iterator iter = map.begin (), end = map.end (); iter != end; ++iter) {
            sigc::connection* connection = (*iter).second;
            connection->disconnect ();
            delete connection;
        }
    }

    void
    add_connection (std::string name, sigc::connection* connection)
    {
        if (map.find (name) != map.end ()) throw NAME_EXISTS;
        map.insert (std::make_pair (name, connection)); 
    }

    void
    del_connection (std::string name)
    {
      ConnectionMap::iterator iter = map.find (name); 
      if (iter == map.end ()) throw NAME_DOES_NOT_EXIST;
      sigc::connection* connection = (*iter).second;
      connection->disconnect ();
      delete connection;
    }


    static Inkscape::ConnectionPool*
    new_connection_pool (std::string name)
    {
       return new ConnectionPool (name); 
    }

    static void
    del_connection_pool (Inkscape::ConnectionPool* pool)
    {
      delete pool;
    }

    static void
    connect_destroy (GObject *obj, Inkscape::ConnectionPool *pool)
    {
      g_object_connect (obj, "swapped-signal::destroy", G_CALLBACK (del_connection_pool), pool, NULL);
    }

    operator std::string ()
    {
      return name;
    }

  private:

    ConnectionPool (std::string name) : name (name)
    {}

    ConnectionMap map;
    std::string   name;
};
}

#endif
