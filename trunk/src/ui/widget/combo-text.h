/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_MODE_DESIGN_COMBO_TEXTGLADE_HH
#define GLOM_MODE_DESIGN_COMBO_TEXTGLADE_HH

#include <gtkmm/combobox.h>

#include <gtkmm/liststore.h>

/** This class should just derive from Gtk::ComboBoxText and provide a
 * constuctor suitable for libglade's get_widget_derived() template.
 * However, I have reimplemented Gtk::ComboBoxText here temporarily,
 * until the fixes in gtkmm 2.4.3 are widely available.
 */
class ComboText : public Gtk::ComboBox
{
 public:
    ComboText();
    virtual ~ComboText();

    void append_text(const Glib::ustring& text);
    void insert_text(int position, const Glib::ustring& text);
    void prepend_text(const Glib::ustring& text);
    Glib::ustring get_active_text() const;

    //This is not part of ComboBoxText:
    void clear_text();
    void set_active_text(const Glib::ustring& text);
  
 protected:

    //Tree model columns:
    //These columns are used by the model that is created by the default constructor
    class TextModelColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:
	    TextModelColumns()
		{ add(m_column); }

	    Gtk::TreeModelColumn<Glib::ustring> m_column;
	};

    TextModelColumns m_text_columns;
    Glib::RefPtr<Gtk::ListStore> m_model;
  
};

#endif //GLOM_MODE_DESIGN_COMBO_TEXTGLADE_HH
