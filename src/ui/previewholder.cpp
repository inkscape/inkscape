
/*
 * A simple interface for previewing representations.
 *
 * Authors:
 *   Jon A. Cruz
 *
 * Copyright (C) 2005 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */


#include "previewholder.h"

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/scrollbar.h>


namespace Inkscape {
namespace UI {


PreviewHolder::PreviewHolder() :
    VBox(),
    PreviewFillable(),
    _scroller(0),
    _zee0(0),
    _zee1(0),
    _zee2(0),
    _anchor(Gtk::ANCHOR_CENTER),
    _baseSize(Gtk::ICON_SIZE_MENU),
    _view(VIEW_TYPE_LIST)
{
    _scroller = manage(new Gtk::ScrolledWindow());
    Gtk::Table* stuff = manage(new Gtk::Table( 1, 2 ));
    stuff->set_col_spacings( 8 );
    _insides = stuff;

    // Add a container with the scroller and a spacer
    Gtk::Table* spaceHolder = manage( new Gtk::Table(1, 2) );
    _zee0 = manage( new Gtk::VBox() );
    _scroller->add(*stuff);
    spaceHolder->attach( *_scroller, 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
    spaceHolder->attach( *_zee0, 1, 2, 0, 1, Gtk::SHRINK, Gtk::FILL|Gtk::EXPAND );

    pack_start(*spaceHolder, Gtk::PACK_EXPAND_WIDGET);
}

PreviewHolder::~PreviewHolder()
{
}



void PreviewHolder::clear()
{
    items.clear();
    rebuildUI();
}

void PreviewHolder::addPreview( Previewable* preview )
{
    items.push_back(preview);

    int i = items.size() - 1;
    if ( _view == VIEW_TYPE_LIST ) {
        Gtk::Widget* label = manage(preview->getPreview(PREVIEW_STYLE_BLURB, VIEW_TYPE_LIST, _baseSize));
        Gtk::Widget* thing = manage(preview->getPreview(PREVIEW_STYLE_PREVIEW, VIEW_TYPE_LIST, _baseSize));

        _insides->attach( *thing, 0, 1, i, i+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
        _insides->attach( *label, 1, 2, i, i+1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK );
    } else {
        Gtk::Widget* thing = manage(items[i]->getPreview(PREVIEW_STYLE_PREVIEW, VIEW_TYPE_GRID, _baseSize));
        int width = _baseSize == Gtk::ICON_SIZE_MENU ? 16 : 8;
        int col = i % width;
        int row = i / width;
        if ( col == 0 ) {
            // we just started a new row
            _insides->resize( row + 1, width );
        }
        _insides->attach( *thing, col, col+1, row, row+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
    }

    _scroller->show_all_children();
    _scroller->queue_draw();
}

void PreviewHolder::setStyle(Gtk::BuiltinIconSize size, ViewType view)
{
    if ( size != _baseSize || view != _view ) {
        _baseSize = size;
        _view = view;
        rebuildUI();
    }
}

void PreviewHolder::setOrientation( Gtk::AnchorType how )
{
    if ( _anchor != how )
    {
        _anchor = how;
        switch ( _anchor )
        {
            case Gtk::ANCHOR_NORTH:
            case Gtk::ANCHOR_SOUTH:
            {
                //dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_NEVER );
                dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );
                if ( !_zee1 )
                {
                    _zee1 = manage( new Gtk::VBox() );
                    _zee2 = manage( new Gtk::VBox() );

                    // Trick to get the scrolled window to a minimum height larger than the scrollbar

                    Gtk::VScrollbar* vs = dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->get_vscrollbar(); 
                    // TODO fix leakage
                    Glib::RefPtr<Gtk::SizeGroup> sizer = Gtk::SizeGroup::create(Gtk::SIZE_GROUP_VERTICAL);
                    sizer->add_widget( *_zee1 );
                    sizer->add_widget( *_zee2 );
                    sizer->add_widget( *vs );

                    _zee0->pack_start( *_zee1 );
                    _zee0->pack_start( *_zee2 );
                }
            }
            break;

            case Gtk::ANCHOR_EAST:
            case Gtk::ANCHOR_WEST:
            {
                dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->set_policy( Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC );
            }
            break;

            default:
            {
                dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->set_policy( Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC );
            }
        }
    }
}

void PreviewHolder::rebuildUI()
{
    _scroller->remove();

    if ( _view == VIEW_TYPE_LIST ) {
        Gtk::Table* stuff = manage(new Gtk::Table( 1, 2 ));
        _insides = stuff;
        stuff->set_col_spacings( 8 );

        for ( unsigned int i = 0; i < items.size(); i++ ) {
            Gtk::Widget* label = manage(items[i]->getPreview(PREVIEW_STYLE_BLURB, _view, _baseSize));
            //label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

            Gtk::Widget* thing = manage(items[i]->getPreview(PREVIEW_STYLE_PREVIEW, _view, _baseSize));

            stuff->attach( *thing, 0, 1, i, i+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
            stuff->attach( *label, 1, 2, i, i+1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK );
        }
        _scroller->add(*stuff);
    } else {
        int width = _baseSize == Gtk::ICON_SIZE_MENU ? 16 : 8;
        int height = (items.size() + (width - 1)) / width;
        if ( height < 1 ) {
            height = 1;
        }

        Gtk::Table* stuff = manage(new Gtk::Table( height, width ));
        _insides = stuff;
        int col = 0;
        int row = 0;

        for ( unsigned int i = 0; i < items.size(); i++ ) {
            Gtk::Widget* thing = manage(items[i]->getPreview(PREVIEW_STYLE_PREVIEW, _view, _baseSize));

            stuff->attach( *thing, col, col+1, row, row+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
            col++;
            if ( col >= width ) {
                col = 0;
                row++;
            }
        }
        _scroller->add(*stuff);
    }

    _scroller->show_all_children();
    _scroller->queue_draw();
}





} //namespace UI
} //namespace Inkscape


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
