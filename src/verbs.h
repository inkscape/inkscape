#ifndef SEEN_SP_VERBS_H
#define SEEN_SP_VERBS_H
/*
 * Author:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Ted Gould <ted@gould.cx>
 *   David Yip <yipdw@rose-hulman.edu>
 *
 *  Copyright (C) 2006 Johan Engelen <johan@shouraizou.nl>
 *  Copyright (C) (date unspecified) Authors

 * This code is in public domain if done by Lauris
 * This code is GPL if done by Ted or David
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <cstring>
#include <string>
#include <glibmm/ustring.h>

struct SPAction;
class SPDocument;

namespace Inkscape {

class ActionContext;

namespace UI {
namespace View {
class View;
} // namespace View
} // namespace UI
} // namespace Inkscape

/**
 * This anonymous enum is used to provide a list of the Verbs
 * which are defined staticly in the verb files.  There may be
 * other verbs which are defined dynamically also.
 */
enum {
    /* Header */
    SP_VERB_INVALID,               /**< A dummy verb to represent doing something wrong. */
    SP_VERB_NONE,                  /**< A dummy verb to represent not having a verb. */
    /* File */
    SP_VERB_FILE_NEW,              /**< A new file in a new window. */
    SP_VERB_FILE_OPEN,             /**< Open a file. */
    SP_VERB_FILE_REVERT,           /**< Revert this file to its original state. */
    SP_VERB_FILE_SAVE,             /**< Save the current file with its saved filename */
    SP_VERB_FILE_SAVE_AS,          /**< Save the current file with a new filename */
    SP_VERB_FILE_SAVE_A_COPY,      /**< Save a copy of the current file */
    SP_VERB_FILE_PRINT,
    SP_VERB_FILE_VACUUM,
    SP_VERB_FILE_IMPORT,
//    SP_VERB_FILE_EXPORT,
    SP_VERB_FILE_IMPORT_FROM_OCAL, /**< Import the file from Open Clip Art Library */
//    SP_VERB_FILE_EXPORT_TO_OCAL, /**< Export the file to Open Clip Art  Library */
    SP_VERB_FILE_NEXT_DESKTOP,
    SP_VERB_FILE_PREV_DESKTOP,
    SP_VERB_FILE_CLOSE_VIEW,
    SP_VERB_FILE_QUIT,
    SP_VERB_FILE_TEMPLATES,
    /* Edit */
    SP_VERB_EDIT_UNDO,
    SP_VERB_EDIT_REDO,
    SP_VERB_EDIT_CUT,
    SP_VERB_EDIT_COPY,
    SP_VERB_EDIT_PASTE,
    SP_VERB_EDIT_PASTE_STYLE,
    SP_VERB_EDIT_PASTE_SIZE,
    SP_VERB_EDIT_PASTE_SIZE_X,
    SP_VERB_EDIT_PASTE_SIZE_Y,
    SP_VERB_EDIT_PASTE_SIZE_SEPARATELY,
    SP_VERB_EDIT_PASTE_SIZE_SEPARATELY_X,
    SP_VERB_EDIT_PASTE_SIZE_SEPARATELY_Y,
    SP_VERB_EDIT_PASTE_IN_PLACE,
    SP_VERB_EDIT_PASTE_LIVEPATHEFFECT,
    SP_VERB_EDIT_REMOVE_LIVEPATHEFFECT,
    SP_VERB_EDIT_REMOVE_FILTER,
    SP_VERB_EDIT_DELETE,
    SP_VERB_EDIT_DUPLICATE,
    SP_VERB_EDIT_CLONE,
    SP_VERB_EDIT_UNLINK_CLONE,
    SP_VERB_EDIT_RELINK_CLONE,
    SP_VERB_EDIT_CLONE_SELECT_ORIGINAL,
    SP_VERB_EDIT_CLONE_ORIGINAL_PATH_LPE,
    SP_VERB_EDIT_SELECTION_2_MARKER,
    SP_VERB_EDIT_SELECTION_2_GUIDES,
    SP_VERB_EDIT_TILE,
    SP_VERB_EDIT_UNTILE,
    SP_VERB_EDIT_SYMBOL,
    SP_VERB_EDIT_UNSYMBOL,
    SP_VERB_EDIT_CLEAR_ALL,
    SP_VERB_EDIT_SELECT_ALL,
    SP_VERB_EDIT_SELECT_ALL_IN_ALL_LAYERS,
    SP_VERB_EDIT_SELECT_SAME_FILL_STROKE,
    SP_VERB_EDIT_SELECT_SAME_FILL_COLOR,
    SP_VERB_EDIT_SELECT_SAME_STROKE_COLOR,
    SP_VERB_EDIT_SELECT_SAME_STROKE_STYLE,
    SP_VERB_EDIT_SELECT_SAME_OBJECT_TYPE,
    SP_VERB_EDIT_INVERT,
    SP_VERB_EDIT_INVERT_IN_ALL_LAYERS,
    SP_VERB_EDIT_SELECT_NEXT,
    SP_VERB_EDIT_SELECT_PREV,
    SP_VERB_EDIT_DESELECT,
    SP_VERB_EDIT_DELETE_ALL_GUIDES,
    SP_VERB_EDIT_GUIDES_TOGGLE_LOCK,
    SP_VERB_EDIT_GUIDES_AROUND_PAGE,
    SP_VERB_EDIT_NEXT_PATHEFFECT_PARAMETER,
    /* Selection */
    SP_VERB_SELECTION_TO_FRONT,
    SP_VERB_SELECTION_TO_BACK,
    SP_VERB_SELECTION_RAISE,
    SP_VERB_SELECTION_LOWER,
    SP_VERB_SELECTION_GROUP,
    SP_VERB_SELECTION_UNGROUP,
    SP_VERB_SELECTION_TEXTTOPATH,
    SP_VERB_SELECTION_TEXTFROMPATH,
    SP_VERB_SELECTION_REMOVE_KERNS,
    SP_VERB_SELECTION_UNION,
    SP_VERB_SELECTION_INTERSECT,
    SP_VERB_SELECTION_DIFF,
    SP_VERB_SELECTION_SYMDIFF,
    SP_VERB_SELECTION_CUT,
    SP_VERB_SELECTION_SLICE,
    SP_VERB_SELECTION_OFFSET,
    SP_VERB_SELECTION_OFFSET_SCREEN,
    SP_VERB_SELECTION_OFFSET_SCREEN_10,
    SP_VERB_SELECTION_INSET,
    SP_VERB_SELECTION_INSET_SCREEN,
    SP_VERB_SELECTION_INSET_SCREEN_10,
    SP_VERB_SELECTION_DYNAMIC_OFFSET,
    SP_VERB_SELECTION_LINKED_OFFSET,
    SP_VERB_SELECTION_OUTLINE,
    SP_VERB_SELECTION_SIMPLIFY,
    SP_VERB_SELECTION_REVERSE,

#if HAVE_POTRACE
    SP_VERB_SELECTION_TRACE,
#endif

    SP_VERB_SELECTION_PIXEL_ART,
    SP_VERB_SELECTION_CREATE_BITMAP,
    SP_VERB_SELECTION_COMBINE,
    SP_VERB_SELECTION_BREAK_APART,
    SP_VERB_SELECTION_ARRANGE, // Former SP_VERB_SELECTION_GRIDTILE
    /* Layer */
    SP_VERB_LAYER_NEW,
    SP_VERB_LAYER_RENAME,
    SP_VERB_LAYER_NEXT,
    SP_VERB_LAYER_PREV,
    SP_VERB_LAYER_MOVE_TO_NEXT,
    SP_VERB_LAYER_MOVE_TO_PREV,
    SP_VERB_LAYER_MOVE_TO,
    SP_VERB_LAYER_TO_TOP,
    SP_VERB_LAYER_TO_BOTTOM,
    SP_VERB_LAYER_RAISE,
    SP_VERB_LAYER_LOWER,
    SP_VERB_LAYER_DUPLICATE,
    SP_VERB_LAYER_DELETE,
    SP_VERB_LAYER_SOLO,
    SP_VERB_LAYER_SHOW_ALL,
    SP_VERB_LAYER_HIDE_ALL,
    SP_VERB_LAYER_LOCK_ALL,
    SP_VERB_LAYER_LOCK_OTHERS,
    SP_VERB_LAYER_UNLOCK_ALL,
    SP_VERB_LAYER_TOGGLE_LOCK,
    SP_VERB_LAYER_TOGGLE_HIDE,
    /* Object */
    SP_VERB_OBJECT_ROTATE_90_CW,
    SP_VERB_OBJECT_ROTATE_90_CCW,
    SP_VERB_OBJECT_FLATTEN,
    SP_VERB_OBJECT_TO_CURVE,
    SP_VERB_OBJECT_FLOW_TEXT,
    SP_VERB_OBJECT_UNFLOW_TEXT,
    SP_VERB_OBJECT_FLOWTEXT_TO_TEXT,
    SP_VERB_OBJECT_FLIP_HORIZONTAL,
    SP_VERB_OBJECT_FLIP_VERTICAL,
    SP_VERB_OBJECT_SET_MASK,
    SP_VERB_OBJECT_EDIT_MASK,
    SP_VERB_OBJECT_UNSET_MASK,
    SP_VERB_OBJECT_SET_CLIPPATH,
    SP_VERB_OBJECT_CREATE_CLIP_GROUP,
    SP_VERB_OBJECT_EDIT_CLIPPATH,
    SP_VERB_OBJECT_UNSET_CLIPPATH,
    /* Tag */
    SP_VERB_TAG_NEW,
    /* Tools */
    SP_VERB_CONTEXT_SELECT,
    SP_VERB_CONTEXT_NODE,
    SP_VERB_CONTEXT_TWEAK,
    SP_VERB_CONTEXT_SPRAY,
    SP_VERB_CONTEXT_RECT,
    SP_VERB_CONTEXT_3DBOX,
    SP_VERB_CONTEXT_ARC,
    SP_VERB_CONTEXT_STAR,
    SP_VERB_CONTEXT_SPIRAL,
    SP_VERB_CONTEXT_PENCIL,
    SP_VERB_CONTEXT_PEN,
    SP_VERB_CONTEXT_CALLIGRAPHIC,
    SP_VERB_CONTEXT_TEXT,
    SP_VERB_CONTEXT_GRADIENT,
    SP_VERB_CONTEXT_MESH,
    SP_VERB_CONTEXT_ZOOM,
    SP_VERB_CONTEXT_MEASURE,
    SP_VERB_CONTEXT_DROPPER,
    SP_VERB_CONTEXT_CONNECTOR,

#if HAVE_POTRACE
    SP_VERB_CONTEXT_PAINTBUCKET,
#endif

    SP_VERB_CONTEXT_LPE, /* not really a tool but used for editing LPE parameters on-canvas for example */
    SP_VERB_CONTEXT_ERASER,
    SP_VERB_CONTEXT_LPETOOL, /* note that this is very different from SP_VERB_CONTEXT_LPE above! */
    /* Tool preferences */
    SP_VERB_CONTEXT_SELECT_PREFS,
    SP_VERB_CONTEXT_NODE_PREFS,
    SP_VERB_CONTEXT_TWEAK_PREFS,
    SP_VERB_CONTEXT_SPRAY_PREFS,
    SP_VERB_CONTEXT_RECT_PREFS,
    SP_VERB_CONTEXT_3DBOX_PREFS,
    SP_VERB_CONTEXT_ARC_PREFS,
    SP_VERB_CONTEXT_STAR_PREFS,
    SP_VERB_CONTEXT_SPIRAL_PREFS,
    SP_VERB_CONTEXT_PENCIL_PREFS,
    SP_VERB_CONTEXT_PEN_PREFS,
    SP_VERB_CONTEXT_CALLIGRAPHIC_PREFS,
    SP_VERB_CONTEXT_TEXT_PREFS,
    SP_VERB_CONTEXT_GRADIENT_PREFS,
    SP_VERB_CONTEXT_MESH_PREFS,
    SP_VERB_CONTEXT_ZOOM_PREFS,
    SP_VERB_CONTEXT_MEASURE_PREFS,
    SP_VERB_CONTEXT_DROPPER_PREFS,
    SP_VERB_CONTEXT_CONNECTOR_PREFS,

#if HAVE_POTRACE
    SP_VERB_CONTEXT_PAINTBUCKET_PREFS,
#endif

    SP_VERB_CONTEXT_ERASER_PREFS,
    SP_VERB_CONTEXT_LPETOOL_PREFS,
    /* Zooming and desktop settings */
    SP_VERB_ZOOM_IN,
    SP_VERB_ZOOM_OUT,
    SP_VERB_TOGGLE_RULERS,
    SP_VERB_TOGGLE_SCROLLBARS,
    SP_VERB_TOGGLE_GRID,
    SP_VERB_TOGGLE_GUIDES,
    SP_VERB_TOGGLE_SNAPPING,
    SP_VERB_TOGGLE_COMMANDS_TOOLBAR,
    SP_VERB_TOGGLE_SNAP_TOOLBAR,
    SP_VERB_TOGGLE_TOOL_TOOLBAR,
    SP_VERB_TOGGLE_TOOLBOX,
    SP_VERB_TOGGLE_PALETTE,
    SP_VERB_TOGGLE_STATUSBAR,
    SP_VERB_ZOOM_NEXT,
    SP_VERB_ZOOM_PREV,
    SP_VERB_ZOOM_1_1,
    SP_VERB_ZOOM_1_2,
    SP_VERB_ZOOM_2_1,
    SP_VERB_FULLSCREEN,
    SP_VERB_FULLSCREENFOCUS,
    SP_VERB_FOCUSTOGGLE,
    SP_VERB_VIEW_NEW,
    SP_VERB_VIEW_NEW_PREVIEW,
    SP_VERB_VIEW_MODE_NORMAL,
    SP_VERB_VIEW_MODE_NO_FILTERS,
    SP_VERB_VIEW_MODE_OUTLINE,
    SP_VERB_VIEW_MODE_TOGGLE,
    SP_VERB_VIEW_COLOR_MODE_NORMAL,
    SP_VERB_VIEW_COLOR_MODE_GRAYSCALE,
//    SP_VERB_VIEW_COLOR_MODE_PRINT_COLORS_PREVIEW,
    SP_VERB_VIEW_COLOR_MODE_TOGGLE,
    SP_VERB_VIEW_CMS_TOGGLE,
    SP_VERB_VIEW_ICON_PREVIEW,
    SP_VERB_ZOOM_PAGE,
    SP_VERB_ZOOM_PAGE_WIDTH,
    SP_VERB_ZOOM_DRAWING,
    SP_VERB_ZOOM_SELECTION,
    /* Dialogs */
    SP_VERB_DIALOG_DISPLAY,
    SP_VERB_DIALOG_NAMEDVIEW,
    SP_VERB_DIALOG_METADATA,
    SP_VERB_DIALOG_FILL_STROKE,
    SP_VERB_DIALOG_GLYPHS,
    SP_VERB_DIALOG_SWATCHES,
    SP_VERB_DIALOG_SYMBOLS,
    SP_VERB_DIALOG_TRANSFORM,
    SP_VERB_DIALOG_ALIGN_DISTRIBUTE,
    SP_VERB_DIALOG_SPRAY_OPTION,
    SP_VERB_DIALOG_UNDO_HISTORY,
    SP_VERB_DIALOG_TEXT,
    SP_VERB_DIALOG_XML_EDITOR,
    SP_VERB_DIALOG_FIND,
    SP_VERB_DIALOG_FINDREPLACE,
    SP_VERB_DIALOG_SPELLCHECK,
    SP_VERB_DIALOG_DEBUG,
    SP_VERB_DIALOG_TOGGLE,
    SP_VERB_DIALOG_CLONETILER,
    SP_VERB_DIALOG_ATTR,
    SP_VERB_DIALOG_ITEM,
    SP_VERB_DIALOG_INPUT,
    SP_VERB_DIALOG_EXTENSIONEDITOR,
    SP_VERB_DIALOG_LAYERS,
    SP_VERB_DIALOG_OBJECTS,
    SP_VERB_DIALOG_TAGS,
    SP_VERB_DIALOG_LIVE_PATH_EFFECT,
    SP_VERB_DIALOG_FILTER_EFFECTS,
    SP_VERB_DIALOG_SVG_FONTS,
    SP_VERB_DIALOG_PRINT_COLORS_PREVIEW,
    SP_VERB_DIALOG_EXPORT,
    /* Help */
    SP_VERB_HELP_ABOUT_EXTENSIONS,
    SP_VERB_HELP_MEMORY,
    SP_VERB_HELP_ABOUT,
    //SP_VERB_SHOW_LICENSE,
    /* Tutorials */
    SP_VERB_TUTORIAL_BASIC,
    SP_VERB_TUTORIAL_SHAPES,
    SP_VERB_TUTORIAL_ADVANCED,

#if HAVE_POTRACE
    SP_VERB_TUTORIAL_TRACING,
#endif

    SP_VERB_TUTORIAL_TRACING_PIXELART,
    SP_VERB_TUTORIAL_CALLIGRAPHY,
    SP_VERB_TUTORIAL_INTERPOLATE,
    SP_VERB_TUTORIAL_DESIGN,
    SP_VERB_TUTORIAL_TIPS,
    /* Effects */
    SP_VERB_EFFECT_LAST,
    SP_VERB_EFFECT_LAST_PREF,
    /* Fit Canvas */
    SP_VERB_FIT_CANVAS_TO_SELECTION,
    SP_VERB_FIT_CANVAS_TO_DRAWING,
    SP_VERB_FIT_CANVAS_TO_SELECTION_OR_DRAWING,
    /* LockAndHide */
    SP_VERB_UNLOCK_ALL,
    SP_VERB_UNLOCK_ALL_IN_ALL_LAYERS,
    SP_VERB_UNHIDE_ALL,
    SP_VERB_UNHIDE_ALL_IN_ALL_LAYERS,
    /* Color management */
    SP_VERB_EDIT_LINK_COLOR_PROFILE,
    SP_VERB_EDIT_REMOVE_COLOR_PROFILE,
    /*Scripting*/
    SP_VERB_EDIT_ADD_EXTERNAL_SCRIPT,
    SP_VERB_EDIT_ADD_EMBEDDED_SCRIPT,
    SP_VERB_EDIT_EMBEDDED_SCRIPT,
    SP_VERB_EDIT_REMOVE_EXTERNAL_SCRIPT,
    SP_VERB_EDIT_REMOVE_EMBEDDED_SCRIPT,
    /* Alignment */
    SP_VERB_ALIGN_HORIZONTAL_RIGHT_TO_ANCHOR,
    SP_VERB_ALIGN_HORIZONTAL_LEFT,
    SP_VERB_ALIGN_HORIZONTAL_CENTER,
    SP_VERB_ALIGN_HORIZONTAL_RIGHT,
    SP_VERB_ALIGN_HORIZONTAL_LEFT_TO_ANCHOR,
    SP_VERB_ALIGN_VERTICAL_BOTTOM_TO_ANCHOR,
    SP_VERB_ALIGN_VERTICAL_TOP,
    SP_VERB_ALIGN_VERTICAL_CENTER,
    SP_VERB_ALIGN_VERTICAL_BOTTOM,
    SP_VERB_ALIGN_VERTICAL_TOP_TO_ANCHOR,
    SP_VERB_ALIGN_VERTICAL_HORIZONTAL_CENTER,

    /* Footer */
    SP_VERB_LAST
};

char *sp_action_get_title (const SPAction *action);

#include <map>
#include <vector>

namespace Inkscape {

/**
 * A class to represent things the user can do.  In many ways
 * these are 'action factories' as they are used to create
 * individual actions that are based on a given view.
 */
class Verb {
private:
    /** An easy to use defition of the table of verbs by code. */
    typedef std::map<unsigned int, Inkscape::Verb *> VerbTable;

    /** A table of all the dynamically created verbs. */
    static VerbTable _verbs;

    /** The table of statically created verbs which are mostly
               'base verbs'. */
    static Verb * _base_verbs[SP_VERB_LAST + 1];
    /* Plus one because there is an entry for SP_VERB_LAST */

    /** A string comparison function to be used in the Verb ID lookup
        to find the different verbs in the hash map. */
    struct ltstr {
        bool operator()(const char* s1, const char* s2) const {
            if ( (s1 == NULL) && (s2 != NULL) ) {
                return true;
            } else if (s1 == NULL || s2 == NULL) {
                return false;
            } else {
                return strcmp(s1, s2) < 0;
            }
        }
    };

    /** An easy to use definition of the table of verbs by ID. */
    typedef std::map<gchar const *, Verb *, ltstr> VerbIDTable;

    /** Quick lookup of verbs by ID */
    static VerbIDTable _verb_ids;

    /** A simple typedef to make using the action table easier. */
    typedef std::map<Inkscape::UI::View::View *, SPAction *> ActionTable;
    /** A list of all the actions that have been created for this
               verb.  It is referenced by the view that they are created for. */
    ActionTable * _actions;

    /** A unique textual ID for the verb. */
    char const * _id;

    /** The full name of the verb.  (shown on menu entries) */
    char const * _name;

    /** Tooltip for the verb. */
    char const * _tip;

    char * _full_tip; // includes shortcut

    unsigned int _shortcut;

    /** Name of the image that represents the verb. */
    char const * _image;

    /**
     * Unique numerical representation of the verb.  In most cases
     * it is a value from the anonymous enum at the top of this
     * file.
     */
    unsigned int  _code;

    /** Name of the group the verb belongs to. */
    char const * _group;

    /**
     * Whether this verb is set to default to sensitive or
     * insensitive when new actions are created.
     */
    bool _default_sensitive;

protected:

    /**
     * Allows for preliminary setting of the \c _default_sensitive
     * value without effecting existing actions.
     * This function is mostly used at initialization where there are
     * not actions to effect.  I can't think of another case where it
     * should be used.
     *
     * @param in_val New value.
     */
    bool set_default_sensitive (bool in_val) { return _default_sensitive = in_val; }

public:

    /** Accessor to get the \c _default_sensitive value. */
    bool get_default_sensitive (void) { return _default_sensitive; }

    /** Accessor to get the internal variable. */
    unsigned int get_code (void) { return _code; }

    /** Accessor to get the internal variable. */
    char const * get_id (void) { return _id; }

    /** Accessor to get the internal variable. */
    char const * get_name (void) { return _name; }

    /** Accessor to get the internal variable. */
    char const * get_short_tip (void) { return _tip; };

    /** Accessor to get the internal variable. */
    char const * get_tip (void) ;

    /** Accessor to get the internal variable. */
    char const * get_image (void) { return _image; }

    /** Get the verbs group */
    char const * get_group (void) { return _group; }

    /** Set the name after initialization. */
    char const * set_name (char const * name) { _name = name; return _name; }

    /** Set the tooltip after initialization. */
    char const * set_tip (char const * tip) { _tip = tip; return _tip; }


protected:
    SPAction *make_action_helper (Inkscape::ActionContext const & context, void (*perform_fun)(SPAction *, void *), void *in_pntr = NULL);
    virtual SPAction *make_action (Inkscape::ActionContext const & context);

public:

    /**
     * Inititalizes the Verb with the parameters.
     *
     * This function also sets \c _actions to NULL.
     *
     * @warning NO DATA IS COPIED BY CALLING THIS FUNCTION.
     *
     * In many respects this is very bad object oriented design, but it
     * is done for a reason.  All verbs today are of two types: 1) static
     * or 2) created for extension.  In the static case all of the
     * strings are constants in the code, and thus don't really need to
     * be copied.  In the extensions case the strings are identical to
     * the ones already created in the extension object, copying them
     * would be a waste of memory.
     *
     * @param code  Goes to \c _code.
     * @param id    Goes to \c _id.
     * @param name  Goes to \c _name.
     * @param tip   Goes to \c _tip.
     * @param image Goes to \c _image.
     */
    Verb(const unsigned int code,
         char const * id,
         char const * name,
         char const * tip,
         char const * image,
         char const * group) :
        _actions(0),
        _id(id),
        _name(name),
        _tip(tip),
        _full_tip(0),
        _shortcut(0),
        _image(image),
        _code(code),
        _group(group),
        _default_sensitive(true)
    {
        _verbs.insert(VerbTable::value_type(_code, this));
        _verb_ids.insert(VerbIDTable::value_type(_id, this));
    }
    Verb (char const * id, char const * name, char const * tip, char const * image, char const * group);
    virtual ~Verb (void);

    SPAction * get_action(Inkscape::ActionContext const & context);

private:
    static Verb * get_search (unsigned int code);
public:

    /**
     * A function to turn a code into a verb.
     *
     * This is an inline function to translate the codes which are
     * static quickly.  This should optimize into very quick code
     * everywhere which hard coded \c codes are used.  In the case
     * where the \c code is not static the \c get_search function
     * is used.
     *
     * @param  code  The code to be translated
     * @return A pointer to a verb object or a NULL if not found.
     */
    static Verb * get (unsigned int code) {
        if (code <= SP_VERB_LAST) {
            return _base_verbs[code];
        } else {
            return get_search(code);
        }
    }
    static Verb * getbyid (gchar const * id);
    
    /**
     * Print a message to stderr indicating that this verb needs a GUI to run
     */
    static bool ensure_desktop_valid(SPAction *action);

    static void delete_all_view (Inkscape::UI::View::View * view);
    void delete_view (Inkscape::UI::View::View * view);

    void sensitive (SPDocument * in_doc = NULL, bool in_sensitive = true);
    void name (SPDocument * in_doc = NULL, Glib::ustring in_name = "");

// Yes, multiple public, protected and private sections are bad. We'll clean that up later
protected:

    /**
     * Returns the size of the internal base verb array.
     *
     * This is an inline function intended for testing. This should normally not be used.
     * For testing, a subclass that returns this value can be created to verify that the
     * length matches the enum values, etc.
     *
     * @return The size in elements of the internal base array.
     */
    static int _getBaseListSize(void) {return G_N_ELEMENTS(_base_verbs);}

public:
    static void list (void);
    static std::vector<Inkscape::Verb *>getList (void);

}; /* Verb class */


}  /* Inkscape namespace */

#endif // SEEN_SP_VERBS_H

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
