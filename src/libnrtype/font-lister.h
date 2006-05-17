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

namespace Inkscape
{
                /**
                 *  This class enumerates fonts using libnrtype into reusable data stores and
                 *  allows for random access to the font list
                 */
                class FontLister
                {
                    public:
                        ~FontLister ();

                        /** GtkTreeModelColumnRecord for the font list Gtk::ListStore
                         */
                        class FontListClass
                            : public Gtk::TreeModelColumnRecord
                        {
                            public:
                                /** Column containing the family name
                                 */
                                Gtk::TreeModelColumn<std::string> font; 

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

                        FontListClass FontList;
                        typedef std::map<std::string, Gtk::TreePath> IterMapType; 

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
                        get_row_for_font (std::string family)
                        {
                            IterMapType::iterator iter = font_list_store_iter_map.find (family);
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
