#ifndef FONT_LISTER_H
#define FONT_LISTER_H

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Tavmjong Bah <tavmjong@free.fr>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 * Copyright (C) 2013 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <map>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/treepath.h>
#include <glibmm/ustring.h>
#include "nr-type-primitives.h"

class SPObject;
class SPDocument;

namespace Inkscape
{
                /**
                 *  This class enumerates fonts using libnrtype into reusable data stores and
                 *  allows for random access to the font list
                 */
                class FontLister
                {
                    public:

                        enum Exceptions
                        {
                            FAMILY_NOT_FOUND
                        };


                        virtual ~FontLister ();

                        /** GtkTreeModelColumnRecord for the font list Gtk::ListStore
                         */
                        class FontListClass
                            : public Gtk::TreeModelColumnRecord
                        {
                            public:
                                /** Column containing the family name
                                 */
                                Gtk::TreeModelColumn<Glib::ustring> font; 

                                /** Column containing an std::vector<std::string> with style names
                                 * for the corresponding family 
                                 */
                                Gtk::TreeModelColumn<GList*> styles;

                                /** Column containing flag if font is on system
                                */
                                Gtk::TreeModelColumn<gboolean> onSystem;

                                FontListClass ()
                                {
                                    add (font);
                                    add (styles);
                                    add (onSystem);
                                }
                        };

                        FontListClass FontList;

                        /** Returns the ListStore with the font names
                         *
                         * The return is const and the function is declared as const.
                         * The ListStore is ready to be used after class instantiation
                         * and should not (cannot) be modified.
                         */
                        const Glib::RefPtr<Gtk::ListStore>
                        get_font_list () const;

                        /** Updates font list to include fonts in document
                         *
                         */
                        void
                        update_font_list ( SPDocument* document);

                    private:
                        void
                        update_font_list_recursive( SPObject *r, std::list<Glib::ustring> *l );

                    public:
                        static Inkscape::FontLister*
                        get_instance ()
                        {
                            static Inkscape::FontLister* instance = new Inkscape::FontLister();
                            return instance;
                        }

                        Gtk::TreePath
                        get_row_for_font (Glib::ustring family);

                        const NRNameList
                        get_name_list () const
                        {
                            return families;
                        }
                        
                    private:

                        FontLister ();
       
                        NRNameList families;

                        Glib::RefPtr<Gtk::ListStore> font_list_store;

                };
}

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
