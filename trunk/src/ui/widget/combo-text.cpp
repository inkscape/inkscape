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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "combo-text.h"
#include <gtk/gtkcombobox.h>

ComboText::ComboText()
    : Gtk::ComboBox()
{
    m_model = Gtk::ListStore::create(m_text_columns);
    set_model(m_model);
    pack_start(m_text_columns.m_column);
}


ComboText::~ComboText()
{
  
}

void ComboText::append_text(const Glib::ustring& text)
{
    gtk_combo_box_append_text(gobj(), text.c_str());
}

void ComboText::insert_text(int position, const Glib::ustring& text)
{
    gtk_combo_box_insert_text(gobj(), position, text.c_str());
}

void ComboText::prepend_text(const Glib::ustring& text)
{
    gtk_combo_box_prepend_text(gobj(), text.c_str());
}

Glib::ustring ComboText::get_active_text() const
{
    Glib::ustring result;

    //Get the active row:
    Gtk::TreeModel::iterator active_row = get_active();
    if(active_row)
    {
	Gtk::TreeModel::Row row = *active_row;
	result = row[m_text_columns.m_column];
    }

    return result;
}

void ComboText::clear_text()
{
    m_model->clear();
}

void ComboText::set_active_text(const Glib::ustring& text)
{
    for(Gtk::TreeModel::iterator iter = m_model->children().begin(); iter != m_model->children().end(); ++iter)
    {
	Glib::ustring this_text = (*iter)[m_text_columns.m_column];

	if(this_text == text)
	{
	    set_active(iter);
	    return; //success
	}
    }

    //Not found, so mark it as blank:
    unset_active();
}
