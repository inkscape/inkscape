
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

#define COLUMNS_FOR_SMALL 16
#define COLUMNS_FOR_LARGE 8
//#define COLUMNS_FOR_SMALL 48
//#define COLUMNS_FOR_LARGE 32


namespace Inkscape {
namespace UI {


PreviewHolder::PreviewHolder() :
    VBox(),
    PreviewFillable(),
    _scroller(0),
    _anchor(Gtk::ANCHOR_CENTER),
    _baseSize(Gtk::ICON_SIZE_MENU),
    _view(VIEW_TYPE_LIST)
{
    _scroller = manage(new Gtk::ScrolledWindow());
    _insides = manage(new Gtk::Table( 1, 2 ));
    _insides->set_col_spacings( 8 );

    // Add a container with the scroller and a spacer
    Gtk::Table* spaceHolder = manage( new Gtk::Table(1, 2) );
    _scroller->add( *_insides );
    spaceHolder->attach( *_scroller, 0, 1, 0, 1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );

    pack_start(*spaceHolder, Gtk::PACK_EXPAND_WIDGET);
}

PreviewHolder::~PreviewHolder()
{
}



void PreviewHolder::clear()
{
    items.clear();
    _prefCols = 0;
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
        int width = _baseSize == Gtk::ICON_SIZE_MENU ? COLUMNS_FOR_SMALL : COLUMNS_FOR_LARGE;
        if ( _prefCols > 0 ) {
            width = _prefCols;
        }
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
                dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->set_policy( Gtk::POLICY_ALWAYS, Gtk::POLICY_AUTOMATIC );
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
        rebuildUI();
    }
}

void PreviewHolder::setColumnPref( int cols )
{
    _prefCols = cols;
}

void PreviewHolder::on_size_allocate( Gtk::Allocation& allocation )
{
//     g_message( "on_size_allocate(%d, %d) (%d, %d)", allocation.get_x(), allocation.get_y(), allocation.get_width(), allocation.get_height() );
//     g_message("            anchor:%d", _anchor);
    Gtk::VBox::on_size_allocate( allocation );
}

void PreviewHolder::on_size_request( Gtk::Requisition* requisition )
{
//     g_message( "on_size_request(%d, %d)", requisition->width, requisition->height );
    Gtk::VBox::on_size_request( requisition );
//     g_message( "   super       (%d, %d)", requisition->width, requisition->height );
//     g_message("            anchor:%d", _anchor);
//     g_message("             items:%d", (int)items.size());
}

void PreviewHolder::rebuildUI()
{
    _scroller->remove();
    _insides = 0; // remove() call should have deleted the Gtk::Table.

    if ( _view == VIEW_TYPE_LIST ) {
        _insides = manage(new Gtk::Table( 1, 2 ));
        _insides->set_col_spacings( 8 );

        for ( unsigned int i = 0; i < items.size(); i++ ) {
            Gtk::Widget* label = manage(items[i]->getPreview(PREVIEW_STYLE_BLURB, _view, _baseSize));
            //label->set_alignment(Gtk::ALIGN_LEFT, Gtk::ALIGN_CENTER);

            Gtk::Widget* thing = manage(items[i]->getPreview(PREVIEW_STYLE_PREVIEW, _view, _baseSize));

            _insides->attach( *thing, 0, 1, i, i+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
            _insides->attach( *label, 1, 2, i, i+1, Gtk::FILL|Gtk::EXPAND, Gtk::SHRINK );
        }
        _scroller->add( *_insides );
    } else {
        int col = 0;
        int row = 0;
        int width = items.size();
        int height = 1;

        for ( unsigned int i = 0; i < items.size(); i++ ) {
            Gtk::Widget* thing = manage(items[i]->getPreview(PREVIEW_STYLE_PREVIEW, _view, _baseSize));

            if ( !_insides ) {
                if ( _anchor == Gtk::ANCHOR_SOUTH || _anchor == Gtk::ANCHOR_NORTH ) {
                    // pad on out
                    Gtk::Requisition req = _scroller->size_request();

                    //Gtk::VScrollbar* vs = dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->get_vscrollbar();
                    Gtk::HScrollbar* hs = dynamic_cast<Gtk::ScrolledWindow*>(_scroller)->get_hscrollbar();
                    if ( hs ) {
                        Gtk::Requisition scrollReq = hs->size_request();

                        // the +8 is a temporary hack
                        req.height -= scrollReq.height + 8;
                    }

                    Gtk::Requisition req2 = thing->size_request();

                    int h2 = req.height / req2.height;
                    int w2 = req.width / req2.width;
                    if ( (h2 > 1) && (w2 < (int)items.size()) ) {
                        width = (items.size() + (h2 - 1)) / h2;
                    }

                } else {
                    width = _baseSize == Gtk::ICON_SIZE_MENU ? COLUMNS_FOR_SMALL : COLUMNS_FOR_LARGE;
                    if ( _prefCols > 0 ) {
                        width = _prefCols;
                    }
                    height = (items.size() + (width - 1)) / width;
                    if ( height < 1 ) {
                        height = 1;
                    }
                }
                _insides = manage(new Gtk::Table( height, width ));
            }

            _insides->attach( *thing, col, col+1, row, row+1, Gtk::FILL|Gtk::EXPAND, Gtk::FILL|Gtk::EXPAND );
            if ( ++col >= width ) {
                col = 0;
                row++;
            }
        }
        if ( !_insides ) {
            _insides = manage(new Gtk::Table( 1, 2 ));
        }

        _scroller->add( *_insides );
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
