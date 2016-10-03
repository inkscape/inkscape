/*
 * Authors:
 *   Dmitry Zhulanov <dmitry.zhulanov@gmail.com>
 *
 * Copyright (C) 2016 Authors
 *
 * Released under GNU GPL v2, read the file 'COPYING' for more information
 *
 * Format of xverbs.yaml
 * using: $ inkscape -B xverbs.yaml
 *
 * verbose: yes # only "verbose: yes" enable logging
 * run:
 *  # open document to process
 *  - xverb-id: XFileOpen, gfx_sources/loading_screen/sandclock_atlas.svg
 *  - xverb-id: XUndoLabel, fresh_document # set label for UndoToLabel xverb works
 *  # note: if something wrong with undo labels use verb EditUndo instead of XUndoLabel and UndoToLabel at all
 *
 *  # select element to handle
 *  - xverb-id: XSelectElement, top_sand
 *
 *  # verbs
 *  - verb-id: EditInvertInAllLayers
 *  - verb-id: EditDelete
 *  - verb-id: FitCanvasToDrawing
 *
 *  # save element to separated svg document
 *  - xverb-id: XFileSaveAs, output/thegame/linux/data/gfx/loading_screen/top_sand.svg
 *
 *  # also save png preview
 *  - xverb-id: XFileExportPNG, output/thegame/linux/data/gfx_preview/loading_screen/top_sand.png
 *
 *  # return to the fresh_state of document
 *  - xverb-id: UndoToLabel, fresh_document
 *
 *  # do any other handling
 *
 *  # inkscape have a lot of useful verbs
 *  - verb-id: FileQuit
 */
#ifdef WITH_YAML
#include <ui/view/view.h>
#include <desktop.h>
#include <helper/action.h>
#include <helper/action-context.h>
#include <selection.h>
#include <verbs.h>
#include <inkscape.h>

#include <document.h>

#include <glibmm/i18n.h>

#include "main-cmdlinexact.h"

#include "yaml.h"

#include "extension/system.h"
#include "file.h"
#include <glib.h>
#include "sp-root.h"
#include "document-undo.h"
#include "util/units.h"
#include "sp-namedview.h"
#include "resource-manager.h"
#include "ui/dialog/font-substitution.h"
#include "extension/db.h"
#include "preferences.h"
#include "helper/png-write.h"
#include <document-undo.h>
#include <ui/view/view-widget.h>
#include <ui/interface.h>
#include <verbs.h>

#define DPI_BASE Inkscape::Util::Quantity::convert(1, "in", "px")

namespace
{
bool s_verbose = false;

bool createDirForFilename( const std::string &filename )
{
    size_t found = filename.find_last_of("/\\");
    std::string output_directory = filename.substr(0,found);

    if( output_directory == filename )
        return true;

    if (g_mkdir_with_parents(output_directory.c_str(), 0755)) {
        printf("Can't create directory %s\n", output_directory.c_str());
        fflush(stdout);

        return false;
    }

    return true;
}

std::vector<std::string> vectorFromString(const std::string &csv)
{
    std::vector<std::string> result;

    std::string delimiters = ",";

    // Skip delimiters at beginning.
    std::string::size_type lastPos = csv.find_first_not_of(delimiters, 0);

    // Find first non-delimiter.
    std::string::size_type pos = csv.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos) {
        // Found a token, add it to the vector.
        std::string token = csv.substr(lastPos, pos - lastPos);
        token.erase(0, token.find_first_not_of(' '));       //prefixing spaces
        token.erase(token.find_last_not_of(' ')+1);         //surfixing spaces
        result.push_back(token);

        // Skip delimiters.
        lastPos = csv.find_first_not_of(delimiters, pos);

        // Find next non-delimiter.
        pos = csv.find_first_of(delimiters, lastPos);
    }

    return result;
}

void xFileOpen( const Glib::ustring &uri )
{
    if (s_verbose) {
        printf("open %s\n", uri.c_str());
        fflush(stdout);
    }

    SPDesktop *desktop = SP_ACTIVE_DESKTOP;
    if (desktop) {
        SPDocument *old_document = desktop->getDocument();
        desktop->setWaitingCursor();
        Inkscape::DocumentUndo::clearRedo(old_document);
    }

    SPDocument *doc = NULL;
    Inkscape::Extension::Extension *key = NULL;
    try {
        doc = Inkscape::Extension::open(key, uri.c_str());
    } catch (std::exception &e) {
        doc = NULL;
        std::string exeption_mgs = e.what();
        printf("Error: open %s:%s\n",uri.c_str(), exeption_mgs.c_str() );
        fflush(stdout);
    }

    // Set viewBox if it doesn't exist
    if (!doc->getRoot()->viewBox_set
            && (doc->getRoot()->width.unit != SVGLength::PERCENT)
            && (doc->getRoot()->height.unit != SVGLength::PERCENT)) {
        doc->setViewBox(Geom::Rect::from_xywh(0, 0, doc->getWidth().value(doc->getDisplayUnit()), doc->getHeight().value(doc->getDisplayUnit())));
    }

    desktop->change_document(doc);
    doc->emitResizedSignal(doc->getWidth().value("px"), doc->getHeight().value("px"));
    if(desktop)
        desktop->clearWaitingCursor();

    doc->virgin = FALSE;

    // everyone who cares now has a reference, get rid of our`s
    doc->doUnref();

    // resize the window to match the document properties
    sp_namedview_window_from_document(desktop);
    sp_namedview_update_layers_from_document(desktop);

    if ( INKSCAPE.use_gui() ) {
        // Perform a fixup pass for hrefs.
        if ( Inkscape::ResourceManager::getManager().fixupBrokenLinks(doc) ) {
            Glib::ustring msg = _("Broken links have been changed to point to existing files.");
            desktop->showInfoDialog(msg);
        }

        // Check for font substitutions
        Inkscape::UI::Dialog::FontSubstitution::getInstance().checkFontSubstitutions(doc);
    }
}

void xFileSaveAs( Inkscape::ActionContext const & context, const Glib::ustring &uri )
{
    SPDocument *doc = context.getDocument();
    if (s_verbose) {
        printf("save as %s\n", uri.c_str());
        fflush(stdout);
    }

    if( createDirForFilename( uri )) {
        Inkscape::Extension::save(
            Inkscape::Extension::db.get("org.inkscape.output.svg.inkscape"),
            doc, uri.c_str(), false, false, true, Inkscape::Extension::FILE_SAVE_METHOD_SAVE_AS);
        if (s_verbose) {
            printf("save done: %s\n", uri.c_str() );
            fflush(stdout);
        }
    }
    else {
        printf("can't create dirs for filename %s\n", uri.c_str() );
        fflush(stdout);
    }
}

void xFileExportPNG( Inkscape::ActionContext const & context, const Glib::ustring &uri )
{
    if (s_verbose) {
        printf("export png %s\n", uri.c_str());
        fflush(stdout);
    }

    SPDocument *doc = context.getDocument();

    gdouble dpi = 200.0;
    SPDesktop *desktop = SP_ACTIVE_DESKTOP;

    Inkscape::Preferences *prefs = Inkscape::Preferences::get();
    dpi = prefs->getDouble("/dialogs/export/defaultxdpi/value", DPI_BASE);

    gdouble width = doc->getWidth().value(doc->getDisplayUnit());
    gdouble height = doc->getHeight().value(doc->getDisplayUnit());

    gdouble bmwidth = (width) * dpi / DPI_BASE;
    gdouble bmheight = (height) * dpi / DPI_BASE;

    int png_width = (int)(0.5 + bmwidth);
    int png_height = (int)(0.5 + bmheight);

    SPNamedView *nv = desktop->getNamedView();

    ExportResult status = sp_export_png_file(doc, uri.c_str(),
                          Geom::Rect(Geom::Point(0,0), Geom::Point(width, height)), png_width, png_height, dpi, dpi,
                          nv->pagecolor, 0, 0, TRUE);
}

void xSelectElement( Inkscape::ActionContext const & context, const Glib::ustring &uri )
{
    if (context.getDocument() == NULL || context.getSelection() == NULL) {
        return;
    }

    if (s_verbose) {
        printf("select element: %s\n", uri.c_str());
        fflush(stdout);
    }

    SPDocument * doc = context.getDocument();
    SPObject * obj = doc->getObjectById(uri);

    if (obj == NULL) {
        printf(_("Unable to find node ID: '%s'\n"), uri.c_str());
        fflush(stdout);
        return;
    }

    Inkscape::Selection * selection = context.getSelection();
    selection->add(obj);

    if (s_verbose) {
        printf("select done %s\n", uri.c_str());
        fflush(stdout);
    }
}

} // end of unnamed namespace

namespace Inkscape {

CmdLineXAction::CmdLineXAction (gchar const * arg, xaction_args_values_map_t &values_map):
    CmdLineAction(true, arg), _values_map(values_map) {
    this->arg = (char *)arg;
    return;
}

bool
CmdLineXAction::isExtended() {
    return true;
}

void
CmdLineXAction::doItX (ActionContext const & context) {
    (void)(context);

    if( arg == "XFileSaveAs")
        xFileSaveAs( context, _values_map["filename"] );
    else if (arg == "XFileOpen")
        xFileOpen( _values_map["filename"] );
    else if (arg == "XFileExportPNG")
        xFileExportPNG( context, _values_map["png_filename"] );
    else if (arg == "XSelectElement")
        xSelectElement( context, _values_map["element-id"] );
    else {
        printf("unknown xverb: %s", arg.c_str());
        fflush(stdout);
    }

    return;
}

enum parser_state_t { HANDLING_ROOT,
                      HANDLING_VERBOSE, // options
                      HANDLING_RUN, HANDLING_RUN_LIST, HANDLING_RUN_LIST_ENTRY
                    }; // run entries

struct verb_info_t
{
    bool xverb;
    std::vector<std::string> args;
};

typedef std::list<verb_info_t> verbs_list_t;

static verbs_list_t
parseVerbsYAMLFile(gchar const *yaml_filename)
{
    verbs_list_t verbs_list;

    FILE *fh = fopen(yaml_filename, "r");
    if(fh == NULL) {
        printf("Failed to open file!\n");
        fflush(stdout);
        return verbs_list;
    }

    yaml_parser_t parser;
    if(!yaml_parser_initialize(&parser)) {
        printf("Failed to initialize parser!\n");
        fflush(stdout);
        return verbs_list;
    }

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    parser_state_t state = HANDLING_ROOT;

    bool handling_key = false;
    bool handling_value = false;

    std::string key;

    // parse
    yaml_token_t token;
    do {
        yaml_parser_scan(&parser, &token);
        switch(token.type)
        {
            // avoid "warning: enumeration value", "-Wswitch"
        case YAML_NO_TOKEN:
            break;
        case YAML_STREAM_START_TOKEN:
            break;
        case YAML_STREAM_END_TOKEN:
            break;
        case YAML_VERSION_DIRECTIVE_TOKEN:
            break;
        case YAML_TAG_DIRECTIVE_TOKEN:
            break;
        case YAML_DOCUMENT_START_TOKEN:
            break;
        case YAML_DOCUMENT_END_TOKEN:
            break;
        case YAML_FLOW_SEQUENCE_START_TOKEN:
            break;
        case YAML_FLOW_SEQUENCE_END_TOKEN:
            break;
        case YAML_FLOW_MAPPING_START_TOKEN:
            break;
        case YAML_FLOW_MAPPING_END_TOKEN:
            break;
        case YAML_FLOW_ENTRY_TOKEN:
            break;
        case YAML_ALIAS_TOKEN:
            break;
        case YAML_ANCHOR_TOKEN:
            break;
        case YAML_TAG_TOKEN:
            break;

            /* Token types (read before actual token) */
        case YAML_KEY_TOKEN:
            handling_key = true;
            handling_value = false;
            break;
        case YAML_VALUE_TOKEN:
            handling_key = false;
            handling_value = true;
            break;

            /* Block delimeters */
        case YAML_BLOCK_SEQUENCE_START_TOKEN:
            if( state == HANDLING_ROOT ) {
                if( key == "run" )
                    state = HANDLING_RUN;
            }
            break;
        case YAML_BLOCK_ENTRY_TOKEN:
            if( state == HANDLING_RUN )
                state = HANDLING_RUN_LIST;
            else if( state == HANDLING_RUN_LIST )
                state = HANDLING_RUN_LIST_ENTRY;
            else if( state == HANDLING_VERBOSE )
                state = HANDLING_ROOT;
            break;
        case YAML_BLOCK_END_TOKEN:
            if( state == HANDLING_RUN_LIST_ENTRY )
                state = HANDLING_RUN_LIST;
            else if( state == HANDLING_RUN_LIST )
                state = HANDLING_RUN;
            else if( state == HANDLING_VERBOSE )
                state = HANDLING_ROOT;
            else if( state == HANDLING_RUN )
                state = HANDLING_ROOT;
            break;

            /* Data */
        case YAML_BLOCK_MAPPING_START_TOKEN:
            break;
        case YAML_SCALAR_TOKEN:
            if( handling_key )
                key = (char *)token.data.scalar.value;
            else if ( handling_value ) {
                if(state == HANDLING_RUN_LIST) {
                    if(key == "xverb-id") {
                        verb_info_t verb;
                        verb.xverb = true;
                        verb.args = vectorFromString((char *)token.data.scalar.value);
                        if ((verb.args.size() > 1) && Verb::getbyid((char *)token.data.scalar.value, false))
                            verb.xverb = false;
                        verbs_list.push_back(verb);
                    }
                    else if(key == "verb-id") {
                        verb_info_t verb;
                        verb.xverb = false;
                        verb.args = vectorFromString((char *)token.data.scalar.value);
                        verbs_list.push_back(verb);
                    }
                    else {
                        printf("unknown verb type [%s]\n", key.c_str());
                        fflush(stdout);
                    }
                }
                else if(state == HANDLING_ROOT) {
                    std::string value = (char *)token.data.scalar.value;
                    if( (key == "verbose") && (value == "yes") )
                        s_verbose = true;
                }
            }
            break;
        }
    } while(token.type != YAML_STREAM_END_TOKEN);

    /* Cleanup */
    yaml_token_delete(&token);
    yaml_parser_delete(&parser);
    fclose(fh);

    return verbs_list;
}

void
CmdLineXAction::createActionsFromYAML( gchar const *yaml_filename )
{
    verbs_list_t verbs_list = parseVerbsYAMLFile(yaml_filename);

    typedef std::map<std::string,int> undo_labels_map_t;
    undo_labels_map_t undo_labels_map;
    int undo_counter = 0;

    verbs_list_t::iterator iter = verbs_list.begin();
    for (; iter != verbs_list.end(); ++iter) {
        verb_info_t &verb = *iter;
        std::string &verb_word = verb.args[0];
        if (s_verbose)
            printf("handle %s and args count is %d\n", verb_word.c_str(), (int)verb.args.size());

        if (verb_word == "XFileOpen") {
            if( verb.args.size() < 2 )
            {
                printf("bad arguments for XFileOpen\n");
                continue;
            }

            xaction_args_values_map_t values_map;
            values_map["filename"] = verb.args[1];
            new CmdLineXAction(verb_word.c_str(), values_map);
        } else if (verb_word == "XFileSaveAs")
        {
            if (verb.args.size() < 2) {
                printf("bad arguments for XFileSaveAs\n");
                continue;
            }

            xaction_args_values_map_t values_map;
            values_map["filename"] = verb.args[1];
            new CmdLineXAction(verb_word.c_str(), values_map);
        } else if (verb_word == "XUndoLabel") {
            if (verb.args.size() < 2) {
                printf("bad arguments for XUndoLabel\n");
                continue;
            }
            undo_labels_map[verb.args[1]] = undo_counter;
        } else if (verb_word == "UndoToLabel") {
            if (verb.args.size() < 2) {
                printf("bad arguments for UndoToLabel\n");
                continue;
            }

            undo_labels_map_t::iterator iter = undo_labels_map.find(verb.args[1]);
            if(iter != undo_labels_map.end()) {
                int counter = undo_counter - iter->second;
                if( counter > 0 ) {
                    for(int i = 0; i < counter; ++i)
                        new CmdLineAction(true, "EditUndo");
                    undo_counter -= counter;
                }
            }
        } else if (verb_word == "XSelectElement") {
            if (verb.args.size() < 2) {
                printf("bad arguments for XSelectElement\n");
                continue;
            }
            ++undo_counter;

            xaction_args_values_map_t values_map;
            values_map["element-id"] = verb.args[1];
            new CmdLineXAction(verb_word.c_str(), values_map);
        } else if (verb_word == "XFileExportPNG") {
            if (verb.args.size() < 2) {
                printf("bad arguments for XFileExportPNG\n");
                continue;
            }

            xaction_args_values_map_t values_map;
            std::string &png_filename = verb.args[1];
            values_map["png_filename"] = png_filename;
            if(createDirForFilename( png_filename ))
                new CmdLineXAction(verb_word.c_str(), values_map);
        }
        else if(!verb.xverb) {
            ++undo_counter;
            new CmdLineAction(true, verb.args[0].c_str());
        }
        else if( Verb::getbyid(verb.args[0].c_str()) != NULL )
        {
            ++undo_counter;
            new CmdLineAction(true, verb.args[0].c_str());
        }
        else {
            printf("Unhadled xverb %s\n", verb.args[0].c_str());
            fflush(stdout);
        }
    }

    fflush(stdout);
}


} // Inkscape

#endif // WITH_YAML

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
