/**
 * \brief Find dialog
 *
 * Authors:
 *   Bryce W. Harrington <bryce@bryceharrington.org>
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2004-2006 Authors
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "find.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

Find::Find() 
    : Dialog ("dialogs.find", SP_VERB_DIALOG_FIND),
      _entry_text(_("_Text: "), _("Find objects by their text content (exact or partial match)")),
      _entry_id(_("_ID: "), _("Find objects by the value of the id attribute (exact or partial match)")),
      _entry_style(_("_Style: "), _("Find objects by the value of the style attribute (exact or partial match)")),
      _entry_attribute(_("_Attribute: "), _("Find objects by the name of an attribute (exact or partial match)")),
      _check_search_selection(_("Search in s_election"), _("Limit search to the current selection")),
      _check_search_layer(_("Search in current _layer"), _("Limit search to the current layer")),
      _check_include_hidden(_("Include _hidden"), _("Include hidden objects in search")),
      _check_include_locked(_("Include l_ocked"), _("Include locked objects in search")),

      _check_all(_("All types"), _("Search in all object types")),
      _check_all_shapes(_("All shapes"), _("Search all shapes")),
      _check_rects(_("Rectangles"), _("Search rectangles")),
      _check_ellipses(_("Ellipses"), _("Search ellipses, arcs, circles")),
      _check_stars(_("Stars"), _("Search stars and polygons")),
      _check_spirals(_("Spirals"), _("Search spirals")),
      _check_paths(_("Paths"), _("Search paths, lines, polylines")),
      _check_texts(_("Texts"), _("Search text objects")),
      _check_groups(_("Groups"), _("Search groups")),
      _check_clones(_("Clones"), _("Search clones")),
      _check_images(_("Images"), _("Search images")),
      _check_offsets(_("Offsets"), _("Search offset objects")),
    
      _button_clear(_("_Clear"), _("Clear values")),
      _button_find(_("_Find"), _("Select objects matching all of the fields you filled in"))
{
    // Top level vbox
    Gtk::VBox *vbox = get_vbox();
    vbox->set_spacing(4);
    
    vbox->pack_start(_entry_text, true, true);
    vbox->pack_start(_entry_id, true, true);
    vbox->pack_start(_entry_style, true, true);
    vbox->pack_start(_entry_attribute, true, true);

    vbox->pack_start(_check_all, true, true);
    vbox->pack_start(_check_all_shapes, true, true);
    vbox->pack_start(_check_rects, true, true);
    vbox->pack_start(_check_ellipses, true, true);
    vbox->pack_start(_check_stars, true, true);
    vbox->pack_start(_check_spirals, true, true);
    vbox->pack_start(_check_paths, true, true);
    vbox->pack_start(_check_texts, true, true);
    vbox->pack_start(_check_groups, true, true);
    vbox->pack_start(_check_clones, true, true);
    vbox->pack_start(_check_images, true, true);
    vbox->pack_start(_check_offsets, true, true);

    vbox->pack_start(_check_search_selection, true, true);
    vbox->pack_start(_check_search_layer, true, true);
    vbox->pack_start(_check_include_hidden, true, true);
    vbox->pack_start(_check_include_locked, true, true);

    vbox->pack_start(_button_clear, true, true);
    vbox->pack_start(_button_find, true, true);


    show_all_children();
}

Find::~Find() 
{
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

