/**
 * @file
 * Symbols dialog.
 */
/* Authors:
 * Copyright (C) 2012 Tavmjong Bah
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <iostream>
#include <algorithm>
#include <locale>
#include <functional>
#include <sstream>

#include <gtkmm/buttonbox.h>
#include <gtkmm/label.h>

#if WITH_GTKMM_3_0
# include <gtkmm/togglebutton.h>
# include <gtkmm/grid.h>
#else
# include <gtkmm/table.h>
#endif

#include <gtkmm/scrolledwindow.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/iconview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treemodelcolumn.h>
#include <gtkmm/clipboard.h>
#include <glibmm/stringutils.h>
#include <glibmm/markup.h>
#include <glibmm/i18n.h>
#include "path-prefix.h"
#include "io/sys.h"

#include "ui/cache/svg_preview_cache.h"
#include "ui/clipboard.h"
#include "ui/icon-names.h"

#include "symbols.h"

#include "selection.h"
#include "desktop.h"

#include "document.h"
#include "inkscape.h"
#include "sp-root.h"
#include "sp-use.h"
#include "sp-defs.h"
#include "sp-symbol.h"
#include "widgets/icon.h"

#ifdef WITH_LIBVISIO
  #include <libvisio/libvisio.h>

  // TODO: Drop this check when librevenge is widespread.
  #if WITH_LIBVISIO01
    #include <librevenge-stream/librevenge-stream.h>

    using librevenge::RVNGFileStream;
    using librevenge::RVNGStringVector;
  #else
    #include <libwpd-stream/libwpd-stream.h>

    typedef WPXFileStream             RVNGFileStream;
    typedef libvisio::VSDStringVector RVNGStringVector;
  #endif
#endif

#include "verbs.h"
#include "helper/action.h"
#include "helper/action-context.h"
#include "xml/repr.h"

namespace Inkscape {
namespace UI {

namespace Dialog {

// See: http://developer.gnome.org/gtkmm/stable/classGtk_1_1TreeModelColumnRecord.html
class SymbolColumns : public Gtk::TreeModel::ColumnRecord
{
public:

  Gtk::TreeModelColumn<Glib::ustring>                symbol_id;
  Gtk::TreeModelColumn<Glib::ustring>                symbol_title;
  Gtk::TreeModelColumn< Glib::RefPtr<Gdk::Pixbuf> >  symbol_image;

  SymbolColumns() {
    add(symbol_id);
    add(symbol_title);
    add(symbol_image);
  }
};

SymbolColumns* SymbolsDialog::getColumns()
{
  SymbolColumns* columns = new SymbolColumns();
  return columns;
}

/**
 * Constructor
 */
SymbolsDialog::SymbolsDialog( gchar const* prefsPath ) :
  UI::Widget::Panel("", prefsPath, SP_VERB_DIALOG_SYMBOLS),
  store(Gtk::ListStore::create(*getColumns())),
  iconView(0),
  currentDesktop(0),
  deskTrack(),
  currentDocument(0),
  previewDocument(0),
  instanceConns()
{

  /********************    Table    *************************/
#if WITH_GTKMM_3_0
  Gtk::Grid *table = new Gtk::Grid();
#else
  Gtk::Table *table = new Gtk::Table(2, 4, false);
#endif

  // panel is a cloked Gtk::VBox
  _getContents()->pack_start(*Gtk::manage(table), Gtk::PACK_EXPAND_WIDGET);
  guint row = 0;

  /******************** Symbol Sets *************************/
  Gtk::Label* labelSet = new Gtk::Label(_("Symbol set: "));

#if WITH_GTKMM_3_0
  table->attach(*Gtk::manage(labelSet),0,row,1,1);
#else
  table->attach(*Gtk::manage(labelSet),0,1,row,row+1,Gtk::SHRINK,Gtk::SHRINK);
#endif

  symbolSet = new Gtk::ComboBoxText();  // Fill in later
  symbolSet->append(_("Current Document"));
  symbolSet->set_active_text(_("Current Document"));

#if WITH_GTKMM_3_0
  symbolSet->set_hexpand();
  table->attach(*Gtk::manage(symbolSet),1,row,1,1);
#else
  table->attach(*Gtk::manage(symbolSet),1,2,row,row+1,Gtk::FILL|Gtk::EXPAND,Gtk::SHRINK);
#endif

  sigc::connection connSet = symbolSet->signal_changed().connect(
          sigc::mem_fun(*this, &SymbolsDialog::rebuild));
  instanceConns.push_back(connSet);
  
  ++row;

  /********************* Icon View **************************/
  SymbolColumns* columns = getColumns();

  iconView = new Gtk::IconView(static_cast<Glib::RefPtr<Gtk::TreeModel> >(store));
  //iconView->set_text_column(  columns->symbol_id  );
  iconView->set_tooltip_column( 1 );
  iconView->set_pixbuf_column( columns->symbol_image );
  // Giving the iconview a small minimum size will help users understand
  // What the dialog does.
  iconView->set_size_request( 100, 200 );

  std::vector< Gtk::TargetEntry > targets;
  targets.push_back(Gtk::TargetEntry( "application/x-inkscape-paste"));

  iconView->enable_model_drag_source (targets, Gdk::BUTTON1_MASK, Gdk::ACTION_COPY);
  iconView->signal_drag_data_get().connect(
          sigc::mem_fun(*this, &SymbolsDialog::iconDragDataGet));

  sigc::connection connIconChanged;
  connIconChanged = iconView->signal_selection_changed().connect(
          sigc::mem_fun(*this, &SymbolsDialog::iconChanged));
  instanceConns.push_back(connIconChanged);

  Gtk::ScrolledWindow *scroller = new Gtk::ScrolledWindow();
  scroller->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_ALWAYS);
  scroller->add(*Gtk::manage(iconView));

#if WITH_GTKMM_3_0
  scroller->set_hexpand();
  scroller->set_vexpand();
  table->attach(*Gtk::manage(scroller),0,row,2,1);
#else
  table->attach(*Gtk::manage(scroller),0,2,row,row+1,Gtk::EXPAND|Gtk::FILL,Gtk::EXPAND|Gtk::FILL);
#endif

  ++row;

  /******************** Tools *******************************/
  Gtk::Button* button;
  Gtk::HBox* tools = new Gtk::HBox();

  //tools->set_layout( Gtk::BUTTONBOX_END );
#if WITH_GTKMM_3_0
  scroller->set_hexpand();
  table->attach(*Gtk::manage(tools),0,row,2,1);
#else
  table->attach(*Gtk::manage(tools),0,2,row,row+1,Gtk::EXPAND|Gtk::FILL,Gtk::FILL);
#endif

  addSymbol = Gtk::manage(new Gtk::Button());
  addSymbol->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("symbol-add")))) );
  addSymbol->set_tooltip_text(_("Add Symbol from the current document."));
  addSymbol->set_relief( Gtk::RELIEF_NONE );
  addSymbol->set_focus_on_click( false );
  addSymbol->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::insertSymbol));
  tools->pack_start(* addSymbol, Gtk::PACK_SHRINK);

  removeSymbol = Gtk::manage(new Gtk::Button());
  removeSymbol->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("symbol-remove")))) );
  removeSymbol->set_tooltip_text(_("Remove Symbol from the current document."));
  removeSymbol->set_relief( Gtk::RELIEF_NONE );
  removeSymbol->set_focus_on_click( false );
  removeSymbol->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::revertSymbol));
  tools->pack_start(* removeSymbol, Gtk::PACK_SHRINK);

  Gtk::Label* spacer = Gtk::manage(new Gtk::Label(""));
  tools->pack_start(* Gtk::manage(spacer));

  // Pack size (controls display area)
  pack_size = 2; // Default 32px
  button = Gtk::manage(new Gtk::Button());
  button->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("pack-more")))) );
  button->set_tooltip_text(_("Display more icons in row."));
  button->set_relief( Gtk::RELIEF_NONE );
  button->set_focus_on_click( false );
  button->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::packmore));
  tools->pack_start(* button, Gtk::PACK_SHRINK);

  button = Gtk::manage(new Gtk::Button());
  button->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("pack-less")))) );
  button->set_tooltip_text(_("Display fewer icons in row."));
  button->set_relief( Gtk::RELIEF_NONE );
  button->set_focus_on_click( false );
  button->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::packless));
  tools->pack_start(* button, Gtk::PACK_SHRINK);

  // Toggle scale to fit on/off
  fitSymbol = Gtk::manage(new Gtk::ToggleButton());
  fitSymbol->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("symbol-fit")))) );
  fitSymbol->set_tooltip_text(_("Toggle 'fit' symbols in icon space."));
  fitSymbol->set_relief( Gtk::RELIEF_NONE );
  fitSymbol->set_focus_on_click( false );
  fitSymbol->set_active( true );
  fitSymbol->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::rebuild));
  tools->pack_start(* fitSymbol, Gtk::PACK_SHRINK);

  // Render size (scales symbols within display area)
  scale_factor = 0; // Default 1:1 * pack_size/pack_size default
  zoomOut = Gtk::manage(new Gtk::Button());
  zoomOut->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("symbol-smaller")))) );
  zoomOut->set_tooltip_text(_("Make symbols smaller by zooming out."));
  zoomOut->set_relief( Gtk::RELIEF_NONE );
  zoomOut->set_focus_on_click( false );
  zoomOut->set_sensitive( false );
  zoomOut->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::zoomout));
  tools->pack_start(* zoomOut, Gtk::PACK_SHRINK);

  zoomIn = Gtk::manage(new Gtk::Button());
  zoomIn->add(*Gtk::manage(Glib::wrap(
      sp_icon_new (Inkscape::ICON_SIZE_SMALL_TOOLBAR, INKSCAPE_ICON("symbol-bigger")))) );
  zoomIn->set_tooltip_text(_("Make symbols bigger by zooming in."));
  zoomIn->set_relief( Gtk::RELIEF_NONE );
  zoomIn->set_focus_on_click( false );
  zoomIn->set_sensitive( false );
  zoomIn->signal_clicked().connect(sigc::mem_fun(*this, &SymbolsDialog::zoomin));
  tools->pack_start(* zoomIn, Gtk::PACK_SHRINK);

  ++row;

  /**********************************************************/
  currentDesktop  = SP_ACTIVE_DESKTOP;
  currentDocument = currentDesktop->getDocument();

  previewDocument = symbols_preview_doc(); /* Template to render symbols in */
  previewDocument->ensureUpToDate(); /* Necessary? */

  key = SPItem::display_key_new(1);
  renderDrawing.setRoot(previewDocument->getRoot()->invoke_show(renderDrawing, key, SP_ITEM_SHOW_DISPLAY ));

  // This might need to be a global variable so setTargetDesktop can modify it
  SPDefs *defs = currentDocument->getDefs();
  sigc::connection defsModifiedConn = defs->connectModified(sigc::mem_fun(*this, &SymbolsDialog::defsModified));
  instanceConns.push_back(defsModifiedConn);

  sigc::connection selectionChangedConn = currentDesktop->selection->connectChanged(
          sigc::mem_fun(*this, &SymbolsDialog::selectionChanged));
  instanceConns.push_back(selectionChangedConn);

  sigc::connection documentReplacedConn = currentDesktop->connectDocumentReplaced(
          sigc::mem_fun(*this, &SymbolsDialog::documentReplaced));
  instanceConns.push_back(documentReplacedConn);

  get_symbols();
  add_symbols( currentDocument ); /* Defaults to current document */

  sigc::connection desktopChangeConn =
    deskTrack.connectDesktopChanged( sigc::mem_fun(*this, &SymbolsDialog::setTargetDesktop) );
  instanceConns.push_back( desktopChangeConn );

  deskTrack.connect(GTK_WIDGET(gobj()));
}

SymbolsDialog::~SymbolsDialog()
{
  for (std::vector<sigc::connection>::iterator it =  instanceConns.begin(); it != instanceConns.end(); ++it) {
      it->disconnect();
  }
  instanceConns.clear();
  deskTrack.disconnect();
}

SymbolsDialog& SymbolsDialog::getInstance()
{
  return *new SymbolsDialog();
}

void SymbolsDialog::packless() {
  if(pack_size < 4) {
      pack_size++;
      rebuild();
  }
}

void SymbolsDialog::packmore() {
  if(pack_size > 0) {
      pack_size--;
      rebuild();
  }
}

void SymbolsDialog::zoomin() {
  if(scale_factor < 4) {
      scale_factor++;
      rebuild();
  }
}

void SymbolsDialog::zoomout() {
  if(scale_factor > -8) {
      scale_factor--;
      rebuild();
  }
}

void SymbolsDialog::rebuild() {

  if( fitSymbol->get_active() ) {
    zoomIn->set_sensitive( false );
    zoomOut->set_sensitive( false );
  } else {
    zoomIn->set_sensitive( true);
    zoomOut->set_sensitive( true );
  }

  store->clear();
  Glib::ustring symbolSetString = symbolSet->get_active_text();

  SPDocument* symbolDocument = symbolSets[symbolSetString];
  if( !symbolDocument ) {
    // Symbol must be from Current Document (this method of
    // checking should be language independent).
    symbolDocument = currentDocument;
    addSymbol->set_sensitive( true );
    removeSymbol->set_sensitive( true );
  } else {
    addSymbol->set_sensitive( false );                                              
    removeSymbol->set_sensitive( false );
  }
  add_symbols( symbolDocument );
}

void SymbolsDialog::insertSymbol() {
    Inkscape::Verb *verb = Inkscape::Verb::get( SP_VERB_EDIT_SYMBOL );
    SPAction *action = verb->get_action(Inkscape::ActionContext( (Inkscape::UI::View::View *) this->currentDesktop) );
    sp_action_perform (action, NULL);
}

void SymbolsDialog::revertSymbol() {
    Inkscape::Verb *verb = Inkscape::Verb::get( SP_VERB_EDIT_UNSYMBOL );
    SPAction *action = verb->get_action(Inkscape::ActionContext( (Inkscape::UI::View::View *) this->currentDesktop ) );
    sp_action_perform (action, NULL);
}

void SymbolsDialog::iconDragDataGet(const Glib::RefPtr<Gdk::DragContext>& /*context*/, Gtk::SelectionData& data, guint /*info*/, guint /*time*/)
{
#if WITH_GTKMM_3_0
  std::vector<Gtk::TreePath> iconArray = iconView->get_selected_items();
#else
  Gtk::IconView::ArrayHandle_TreePaths iconArray = iconView->get_selected_items();
#endif

  if( iconArray.empty() ) {
    //std::cout << "  iconArray empty: huh? " << std::endl;
  } else {
    Gtk::TreeModel::Path const & path = *iconArray.begin();
    Gtk::ListStore::iterator row = store->get_iter(path);
    Glib::ustring symbol_id = (*row)[getColumns()->symbol_id];

    GdkAtom dataAtom = gdk_atom_intern( "application/x-inkscape-paste", FALSE );
    gtk_selection_data_set( data.gobj(), dataAtom, 9, (guchar*)symbol_id.c_str(), symbol_id.length() );
  }

}

void SymbolsDialog::defsModified(SPObject * /*object*/, guint /*flags*/)
{
  if ( !symbolSets[symbolSet->get_active_text()] ) {
    rebuild();
  }
}

void SymbolsDialog::selectionChanged(Inkscape::Selection *selection) {
  Glib::ustring symbol_id = selectedSymbolId();
  SPDocument* symbolDocument = selectedSymbols();
  SPObject* symbol = symbolDocument->getObjectById(symbol_id);

  if(symbol && !selection->includes(symbol)) {
      iconView->unselect_all();
  }
}

void SymbolsDialog::documentReplaced(SPDesktop *desktop, SPDocument *document)
{
  currentDesktop  = desktop;
  currentDocument = document;
  rebuild();
}

SPDocument* SymbolsDialog::selectedSymbols() {
  /* OK, we know symbol name... now we need to copy it to clipboard, bon chance! */
  Glib::ustring symbolSetString = symbolSet->get_active_text();

  SPDocument* symbolDocument = symbolSets[symbolSetString];
  if( !symbolDocument ) {
    // Symbol must be from Current Document (this method of checking should be language independent).
    return currentDocument;
  }
  return symbolDocument;
}

Glib::ustring SymbolsDialog::selectedSymbolId() {

#if WITH_GTKMM_3_0
  std::vector<Gtk::TreePath> iconArray = iconView->get_selected_items();
#else
  Gtk::IconView::ArrayHandle_TreePaths iconArray = iconView->get_selected_items();
#endif

  if( !iconArray.empty() ) {
    Gtk::TreeModel::Path const & path = *iconArray.begin();
    Gtk::ListStore::iterator row = store->get_iter(path);
    return (*row)[getColumns()->symbol_id];
  }
  return Glib::ustring("");
}

void SymbolsDialog::iconChanged() {

  Glib::ustring symbol_id = selectedSymbolId();
  SPDocument* symbolDocument = selectedSymbols();
  SPObject* symbol = symbolDocument->getObjectById(symbol_id);

  if( symbol ) {
    if( symbolDocument == currentDocument ) {
      // Select the symbol on the canvas so it can be manipulated
      currentDesktop->selection->set( symbol, false );
    }

    // Find style for use in <use>
    // First look for default style stored in <symbol>
    gchar const* style = symbol->getAttribute("inkscape:symbol-style");
    if( !style ) {
      // If no default style in <symbol>, look in documents.
      if( symbolDocument == currentDocument ) {
        style = style_from_use( symbol_id.c_str(), currentDocument );
      } else {
        style = symbolDocument->getReprRoot()->attribute("style");
      }
    }

    ClipboardManager *cm = ClipboardManager::get();
    cm->copySymbol(symbol->getRepr(), style, symbolDocument == currentDocument);
  }
}

#ifdef WITH_LIBVISIO
// Read Visio stencil files
SPDocument* read_vss( gchar* fullname, gchar* filename ) {

  RVNGFileStream input(fullname);

  if (!libvisio::VisioDocument::isSupported(&input)) {
    return NULL;
  }

  RVNGStringVector output;
#if WITH_LIBVISIO01
  librevenge::RVNGSVGDrawingGenerator generator(output, "svg");

  if (!libvisio::VisioDocument::parseStencils(&input, &generator)) {
#else
  if (!libvisio::VisioDocument::generateSVGStencils(&input, output)) {
#endif
    return NULL;
  }

  if (output.empty()) {
    return NULL;
  }

  Glib::ustring tmpSVGOutput;
  tmpSVGOutput += "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  tmpSVGOutput += "<svg\n";
  tmpSVGOutput += "  xmlns=\"http://www.w3.org/2000/svg\"\n";
  tmpSVGOutput += "  xmlns:svg=\"http://www.w3.org/2000/svg\"\n";
  tmpSVGOutput += "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n";
  tmpSVGOutput += "  version=\"1.1\"\n";
  tmpSVGOutput += "  style=\"fill:none;stroke:#000000;stroke-width:2\">\n";
  tmpSVGOutput += "  <title>";
  tmpSVGOutput += filename;
  tmpSVGOutput += "</title>\n";
  tmpSVGOutput += "  <defs>\n";

  // Create a string we can use for the symbol id (libvisio doesn't give us a name)
  std::string sanitized( filename );
  sanitized.erase( sanitized.find_last_of(".vss")-3 );
  sanitized.erase( std::remove_if( sanitized.begin(), sanitized.end(), ispunct ), sanitized.end() );
  std::replace( sanitized.begin(), sanitized.end(), ' ', '_' );
  // std::cout << filename << "   |" << sanitized << "|" << std::endl;

  // Each "symbol" is in it's own SVG file, we wrap with <symbol> and merge into one file.
  for (unsigned i=0; i<output.size(); ++i) {

    std::stringstream ss;
    ss << i;

    tmpSVGOutput += "    <symbol id=\"";
    tmpSVGOutput += sanitized;
    tmpSVGOutput += "_";
    tmpSVGOutput += ss.str();
    tmpSVGOutput += "\">\n";

    std::istringstream iss( output[i].cstr() );
    std::string line;
    while( std::getline( iss, line ) ) {
      // std::cout << line << std::endl;
      if( line.find( "svg:svg" ) == std::string::npos ) {
        tmpSVGOutput += line;
        tmpSVGOutput += "\n";
      }
    }

    tmpSVGOutput += "    </symbol>\n";
  }

  tmpSVGOutput += "  </defs>\n";
  tmpSVGOutput += "</svg>\n";
        
  return SPDocument::createNewDocFromMem( tmpSVGOutput.c_str(), strlen( tmpSVGOutput.c_str()), 0 );

}
#endif
                        
/* Hunts preference directories for symbol files */
void SymbolsDialog::get_symbols() {

  std::list<Glib::ustring> directories;

// \TODO optimize this

  if( Inkscape::IO::file_test( INKSCAPE_SYMBOLSDIR, G_FILE_TEST_EXISTS ) &&
      Inkscape::IO::file_test( INKSCAPE_SYMBOLSDIR, G_FILE_TEST_IS_DIR ) ) {
    directories.push_back( INKSCAPE_SYMBOLSDIR );
  }
  if( Inkscape::IO::file_test( Inkscape::Application::profile_path("symbols"), G_FILE_TEST_EXISTS ) &&
      Inkscape::IO::file_test( Inkscape::Application::profile_path("symbols"), G_FILE_TEST_IS_DIR ) ) {
    directories.push_back( Inkscape::Application::profile_path("symbols") );
  }

  std::list<Glib::ustring>::iterator it;
  for( it = directories.begin(); it != directories.end(); ++it ) {

    GError *err = 0;
    GDir *dir = g_dir_open( (*it).c_str(), 0, &err );
    if( dir ) {

      gchar *filename = 0;
      while( (filename = (gchar *)g_dir_read_name( dir ) ) != NULL) {

        gchar *fullname = g_build_filename((*it).c_str(), filename, NULL);

        if ( !Inkscape::IO::file_test( fullname, G_FILE_TEST_IS_DIR )
             && ( Glib::str_has_suffix(fullname, ".svg") || Glib::str_has_suffix(fullname, ".vss") ) ) {

          Glib::ustring fn( filename );
          Glib::ustring tag = fn.substr( fn.find_last_of(".") + 1 );

          SPDocument* symbol_doc = NULL;

#ifdef WITH_LIBVISIO
          if( tag.compare( "vss" ) == 0 ) {

            symbol_doc = read_vss( fullname, filename );
            if( symbol_doc ) {
              symbolSets[Glib::ustring(filename)]= symbol_doc;
              symbolSet->append(filename);
            }
          }
#endif
          // Try to read all remaining files as SVG
          if( !symbol_doc ) {

            symbol_doc = SPDocument::createNewDoc( fullname, FALSE );
            if( symbol_doc ) {

              const gchar *title = symbol_doc->getRoot()->title();

              // A user provided file may not have a title
              if( title != NULL ) {
                title = g_dpgettext2(NULL, "Symbol", title); // Translate
              } else {
                title = _("Unnamed Symbols");
              }

              symbolSets[Glib::ustring(title)] = symbol_doc;
              symbolSet->append(title);
            }
          }

        }
        g_free( fullname );
      }
      g_dir_close( dir );
    }
  }
}

GSList* SymbolsDialog::symbols_in_doc_recursive (SPObject *r, GSList *l)
{
  g_return_val_if_fail(r != NULL, l);

  // Stop multiple counting of same symbol
  if ( dynamic_cast<SPUse *>(r) ) {
    return l;
  }

  if ( dynamic_cast<SPSymbol *>(r) ) {
    l = g_slist_prepend (l, r);
  }

  for (SPObject *child = r->firstChild(); child; child = child->getNext()) {
    l = symbols_in_doc_recursive( child, l );
  }

  return l;
}

GSList* SymbolsDialog::symbols_in_doc( SPDocument* symbolDocument ) {

  GSList *l = NULL;
  l = symbols_in_doc_recursive (symbolDocument->getRoot(), l );
  l = g_slist_reverse( l );
  return l;
}

GSList* SymbolsDialog::use_in_doc_recursive (SPObject *r, GSList *l)
{ 

  if ( dynamic_cast<SPUse *>(r) ) {
    l = g_slist_prepend (l, r);
  }

  for (SPObject *child = r->firstChild(); child; child = child->getNext()) {
    l = use_in_doc_recursive( child, l );
  }

  return l;
}

GSList* SymbolsDialog::use_in_doc( SPDocument* useDocument ) {

  GSList *l = NULL;
  l = use_in_doc_recursive (useDocument->getRoot(), l );
  return l;
}

// Returns style from first <use> element found that references id.
// This is a last ditch effort to find a style.
gchar const* SymbolsDialog::style_from_use( gchar const* id, SPDocument* document) {

  gchar const* style = 0;
  GSList* l = use_in_doc( document );
  for( ; l != NULL; l = l->next ) {
    SPObject *obj = reinterpret_cast<SPObject *>(l->data);
    SPUse *use = dynamic_cast<SPUse *>(obj);
    if ( use ) {
      gchar const *href = use->getRepr()->attribute("xlink:href");
      if( href ) {
        Glib::ustring href2(href);
        Glib::ustring id2(id);
        id2 = "#" + id2;
        if( !href2.compare(id2) ) {
          style = use->getRepr()->attribute("style");
          break;
        }
      }
    }
  }
  return style;
}

void SymbolsDialog::add_symbols( SPDocument* symbolDocument ) {

  GSList* l = symbols_in_doc( symbolDocument );
  for( ; l != NULL; l = l->next ) {
    SPObject *obj = reinterpret_cast<SPObject *>(l->data);
    SPSymbol *symbol = dynamic_cast<SPSymbol *>(obj);
    if (symbol) {
      add_symbol( symbol );
    }
  }
}

void SymbolsDialog::add_symbol( SPObject* symbol ) {

  SymbolColumns* columns = getColumns();

  gchar const *id    = symbol->getRepr()->attribute("id");
  gchar const *title = symbol->title(); // From title element
  if( !title ) {
    title = id;
  }

  Glib::RefPtr<Gdk::Pixbuf> pixbuf = draw_symbol( symbol );

  if( pixbuf ) {
    Gtk::ListStore::iterator row = store->append();
    (*row)[columns->symbol_id]    = Glib::ustring( id );
    (*row)[columns->symbol_title] = Glib::Markup::escape_text(Glib::ustring( g_dpgettext2(NULL, "Symbol", title) ));
    (*row)[columns->symbol_image] = pixbuf;
  }

  delete columns;
}

/*
 * Returns image of symbol.
 *
 * Symbols normally are not visible. They must be referenced by a
 * <use> element.  A temporary document is created with a dummy
 * <symbol> element and a <use> element that references the symbol
 * element. Each real symbol is swapped in for the dummy symbol and
 * the temporary document is rendered.
 */
Glib::RefPtr<Gdk::Pixbuf>
SymbolsDialog::draw_symbol(SPObject *symbol)
{
  // Create a copy repr of the symbol with id="the_symbol"
  Inkscape::XML::Document *xml_doc = previewDocument->getReprDoc();
  Inkscape::XML::Node *repr = symbol->getRepr()->duplicate(xml_doc);
  repr->setAttribute("id", "the_symbol");

  // Replace old "the_symbol" in previewDocument by new.
  Inkscape::XML::Node *root = previewDocument->getReprRoot();
  SPObject *symbol_old = previewDocument->getObjectById("the_symbol");
  if (symbol_old) {
      symbol_old->deleteObject(false);
  }

  // First look for default style stored in <symbol>
  gchar const* style = repr->attribute("inkscape:symbol-style");
  if( !style ) {
    // If no default style in <symbol>, look in documents.
    if( symbol->document == currentDocument ) {
      gchar const *id = symbol->getRepr()->attribute("id");
      style = style_from_use( id, symbol->document );
    } else {
      style = symbol->document->getReprRoot()->attribute("style");
    }
  }
  // Last ditch effort to provide some default styling
  if( !style ) style = "fill:#bbbbbb;stroke:#808080";

  // This is for display in Symbols dialog only
  if( style ) repr->setAttribute( "style", style );

  // BUG: Symbols don't work if defined outside of <defs>. Causes Inkscape
  // crash when trying to read in such a file.
  root->appendChild(repr);
  //defsrepr->appendChild(repr);
  Inkscape::GC::release(repr);

  // Uncomment this to get the previewDocument documents saved (useful for debugging)
  // FILE *fp = fopen (g_strconcat(id, ".svg", NULL), "w");
  // sp_repr_save_stream(previewDocument->getReprDoc(), fp);
  // fclose (fp);

  // Make sure previewDocument is up-to-date.
  previewDocument->getRoot()->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
  previewDocument->ensureUpToDate();

  // Make sure we have symbol in previewDocument
  SPObject *object_temp = previewDocument->getObjectById( "the_use" );
  previewDocument->getRoot()->requestDisplayUpdate(SP_OBJECT_MODIFIED_FLAG);
  previewDocument->ensureUpToDate();

  SPItem *item = dynamic_cast<SPItem *>(object_temp);
  g_assert(item != NULL);
  unsigned psize = SYMBOL_ICON_SIZES[pack_size];

  Glib::RefPtr<Gdk::Pixbuf> pixbuf(NULL);
  // We could use cache here, but it doesn't really work with the structure
  // of this user interface and we've already cached the pixbuf in the gtklist

  // Find object's bbox in document.
  // Note symbols can have own viewport... ignore for now.
  //Geom::OptRect dbox = item->geometricBounds();
  Geom::OptRect dbox = item->documentVisualBounds();

  if (dbox) {
    /* Scale symbols to fit */
    double scale = 1.0;
    double width  = dbox->width();
    double height = dbox->height();

    if( width == 0.0 ) width = 1.0;
    if( height == 0.0 ) height = 1.0;

    if( fitSymbol->get_active() )
        scale = psize / std::max(width, height);
    else
      scale = pow( 2.0, scale_factor/2.0 ) * psize / 32.0;

    pixbuf = Glib::wrap(render_pixbuf(renderDrawing, scale, *dbox, psize));
  }

  return pixbuf;
}

/*
 * Return empty doc to render symbols in.
 * Symbols are by default not rendered so a <use> element is
 * provided.
 */
SPDocument* SymbolsDialog::symbols_preview_doc()
{
  // BUG: <symbol> must be inside <defs>
  gchar const *buffer =
"<svg xmlns=\"http://www.w3.org/2000/svg\""
"     xmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\""
"     xmlns:inkscape=\"http://www.inkscape.org/namespaces/inkscape\""
"     xmlns:xlink=\"http://www.w3.org/1999/xlink\">"
"  <defs id=\"defs\">"  
"    <symbol id=\"the_symbol\"/>"
"  </defs>"
"  <use id=\"the_use\" xlink:href=\"#the_symbol\"/>"
"</svg>";

  return SPDocument::createNewDocFromMem( buffer, strlen(buffer), FALSE );
}

void SymbolsDialog::setTargetDesktop(SPDesktop *desktop)
{
  if (this->currentDesktop != desktop) {
    this->currentDesktop = desktop;
    if( !symbolSets[symbolSet->get_active_text()] ) {
      // Symbol set is from Current document, update
      rebuild();
    }
  }
}

} //namespace Dialogs
} //namespace UI
} //namespace Inkscape

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-basic-offset:2
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=2:tabstop=8:softtabstop=2:fileencoding=utf-8:textwidth=99 :
