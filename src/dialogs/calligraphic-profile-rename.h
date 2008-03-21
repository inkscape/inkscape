/**
 *
 * \brief  Dialog for naming calligraphic profiles
 *
 * Author:
 *   Aubanel MONNIER 
 *
 * Copyright (C) 2007 Aubanel MONNIER
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information
 */

#ifndef INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H
#define INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H

#include <gtkmm/dialog.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/table.h>

namespace Inkscape {
  namespace UI {
    namespace Dialogs {
      
      class  CalligraphicProfileDialog: public Gtk::Dialog {  
      public:
	CalligraphicProfileDialog();
	virtual ~CalligraphicProfileDialog(){} ;
	static void show(SPDesktop *desktop);
	static bool applied(){return instance()._applied;}
	static Glib::ustring getProfileName() { return instance()._profile_name;}

	Glib::ustring getName() const { return "CalligraphicProfileDialog"; }

	
      protected:
	void _close();
	void _apply();

	Gtk::Label        _profile_name_label;
	Gtk::Entry        _profile_name_entry;
	Gtk::Table        _layout_table;
	Gtk::Button       _close_button;
	Gtk::Button       _apply_button;
	Glib::ustring _profile_name;
	bool _applied;
      private:
	static CalligraphicProfileDialog &instance(){static CalligraphicProfileDialog instance; return instance;}
	CalligraphicProfileDialog(CalligraphicProfileDialog const &); // no copy
	CalligraphicProfileDialog &operator=(CalligraphicProfileDialog  const &); // no assign
      };      
    }
  }
}

#endif INKSCAPE_DIALOG_CALLIGRAPHIC_PROFILE_H
