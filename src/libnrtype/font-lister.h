#ifndef FONT_LISTER_H
#define FONT_LISTER_H

/*
 * Font selection widgets
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * Copyright (C) 1999-2001 Ximian, Inc.
 * Copyright (C) 2002 Lauris Kaplinski
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <glibmm.h>
#include <gtkmm.h>
#include "nrtype-forward.h"
#include "nr-type-primitives.h"

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

                                FontListClass ()
                                {
                                    add (font);
                                    add (styles);
                                }
                        };

                        /* Case-insensitive < compare for standard strings */
                        class StringLessThan
                        {
                        public:
                            bool operator () (std::string str1, std::string str2) const
                            {
                                std::string s1=str1; // Can't transform the originals!
                                std::string s2=str2;
                                std::transform(s1.begin(), s1.end(), s1.begin(), (int(*)(int)) toupper);
                                std::transform(s2.begin(), s2.end(), s2.begin(), (int(*)(int)) toupper);
                                return s1<s2;
                            }
                        };

                        FontListClass FontList;
                        typedef std::map<Glib::ustring, Gtk::TreePath, StringLessThan> IterMapType; 

                        /** Returns the ListStore with the font names
                         *
                         * The return is const and the function is declared as const.
                         * The ListStore is ready to be used after class instantiation
                         * and should not (cannot) be modified.
                         */
                        const Glib::RefPtr<Gtk::ListStore>
                        get_font_list () const;

                        static Inkscape::FontLister*
                        get_instance ()
                        {
                            static Inkscape::FontLister* instance = new Inkscape::FontLister();
                            return instance;
                        }

                        Gtk::TreePath
                        get_row_for_font (Glib::ustring family)
                        {
                            IterMapType::iterator iter = font_list_store_iter_map.find (family);
                            if (iter == font_list_store_iter_map.end ()) throw FAMILY_NOT_FOUND; 
                            return (*iter).second;
                        }

                        const NRNameList
                        get_name_list () const
                        {
                            return families;
                        }
                        

                    private:

                        FontLister ();
       
                        NRNameList families;

                        Glib::RefPtr<Gtk::ListStore> font_list_store;
                        IterMapType font_list_store_iter_map;

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=99 :
