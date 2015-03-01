/**
 * @file
 * Memory statistics dialog.
 */
/* Authors:
 *   MenTaLguY <mental@rydia.net>
 *
 * Copyright (C) 2005 
 *
 * Released under GNU GPL.  Read the file 'COPYING' for more information.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "ui/dialog/memory.h"
#include <glibmm/main.h>
#include <glibmm/i18n.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>

#include "inkgc/gc-core.h"
#include "debug/heap.h"
#include "verbs.h"

namespace Inkscape {
namespace UI {
namespace Dialog {

namespace {

Glib::ustring format_size(std::size_t value) {
    if (!value) {
        return Glib::ustring("0");
    }

    typedef std::vector<char> Digits;
    typedef std::vector<Digits *> Groups;

    Groups groups;

    Digits *digits;

    while (value) {
        unsigned places=3;
        digits = new Digits();
        digits->reserve(places);

        while ( value && places ) {
            digits->push_back('0' + (char)( value % 10 ));
            value /= 10;
            --places;
        }

        groups.push_back(digits);
    }

    Glib::ustring temp;

    while (true) {
        digits = groups.back();
        while (!digits->empty()) {
            temp.append(1, digits->back());
            digits->pop_back();
        }
        delete digits;

        groups.pop_back();
        if (groups.empty()) {
            break;
        }

        temp.append(",");
    }

    return temp;
}

}

struct Memory::Private {
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<Glib::ustring> name;
        Gtk::TreeModelColumn<Glib::ustring> used;
        Gtk::TreeModelColumn<Glib::ustring> slack;
        Gtk::TreeModelColumn<Glib::ustring> total;

        ModelColumns() { add(name); add(used); add(slack); add(total); }
    };

    Private() {
        model = Gtk::ListStore::create(columns);
        view.set_model(model);
        view.append_column(_("Heap"), columns.name);
        view.append_column(_("In Use"), columns.used);
        // TRANSLATORS: "Slack" refers to memory which is in the heap but currently unused.
        //  More typical usage is to call this memory "free" rather than "slack".
        view.append_column(_("Slack"), columns.slack);
        view.append_column(_("Total"), columns.total);
    }

    void update();

    void start_update_task();
    void stop_update_task();

    ModelColumns columns;
    Glib::RefPtr<Gtk::ListStore> model;
    Gtk::TreeView view;

    sigc::connection update_task;
};

void Memory::Private::update() {
    Debug::Heap::Stats total = { 0, 0 };

    int aggregate_features = Debug::Heap::SIZE_AVAILABLE | Debug::Heap::USED_AVAILABLE;
    Gtk::ListStore::iterator row;

    row = model->children().begin();

    for ( unsigned i = 0 ; i < Debug::heap_count() ; i++ ) {
        Debug::Heap *heap=Debug::get_heap(i);
        if (heap) {
            Debug::Heap::Stats stats=heap->stats();
            int features=heap->features();

            aggregate_features &= features;

            if ( row == model->children().end() ) {
                row = model->append();
            }

            row->set_value(columns.name, Glib::ustring(heap->name()));
            if ( features & Debug::Heap::SIZE_AVAILABLE ) {
                row->set_value(columns.total, format_size(stats.size));
                total.size += stats.size;
            } else {
                row->set_value(columns.total, Glib::ustring(_("Unknown")));
            }
            if ( features & Debug::Heap::USED_AVAILABLE ) {
                row->set_value(columns.used, format_size(stats.bytes_used));
                total.bytes_used += stats.bytes_used;
            } else {
                row->set_value(columns.used, Glib::ustring(_("Unknown")));
            }
            if ( features & Debug::Heap::SIZE_AVAILABLE &&
                 features & Debug::Heap::USED_AVAILABLE )
            {
                row->set_value(columns.slack, format_size(stats.size - stats.bytes_used));
            } else {
                row->set_value(columns.slack, Glib::ustring(_("Unknown")));
            }

            ++row;
        }
    }

    if ( row == model->children().end() ) {
        row = model->append();
    }

    Glib::ustring value;

    row->set_value(columns.name, Glib::ustring(_("Combined")));

    if ( aggregate_features & Debug::Heap::SIZE_AVAILABLE ) {
        row->set_value(columns.total, format_size(total.size));
    } else {
        row->set_value(columns.total, Glib::ustring("> ") + format_size(total.size));
    }

    if ( aggregate_features & Debug::Heap::USED_AVAILABLE ) {
        row->set_value(columns.used, format_size(total.bytes_used));
    } else {
        row->set_value(columns.used, Glib::ustring("> ") + format_size(total.bytes_used));
    }

    if ( aggregate_features & Debug::Heap::SIZE_AVAILABLE &&
         aggregate_features & Debug::Heap::USED_AVAILABLE )
    {
        row->set_value(columns.slack, format_size(total.size - total.bytes_used));
    } else {
        row->set_value(columns.slack, Glib::ustring(_("Unknown")));
    }

    ++row;

    while ( row != model->children().end() ) {
        row = model->erase(row);
    }
}

void Memory::Private::start_update_task() {
    update_task.disconnect();
    update_task = Glib::signal_timeout().connect(
        sigc::bind_return(sigc::mem_fun(*this, &Private::update), true),
        500
    );
}

void Memory::Private::stop_update_task() {
    update_task.disconnect();
}

Memory::Memory() 
    : UI::Widget::Panel ("", "/dialogs/memory", SP_VERB_HELP_MEMORY, _("Recalculate")),
      _private(*(new Memory::Private())) 
{
    _getContents()->add(_private.view);

    _private.update();

    show_all_children();

    signal_show().connect(sigc::mem_fun(_private, &Private::start_update_task));
    signal_hide().connect(sigc::mem_fun(_private, &Private::stop_update_task));

    _private.start_update_task();
}

Memory::~Memory() {
    delete &_private;
}

void Memory::_apply() {
    GC::Core::gcollect();
    _private.update();
}

} // namespace Dialog
} // namespace UI
} // namespace Inkscape

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
