#ifndef INK_EXTENSION_H
#define INK_EXTENSION_H

/** \file
 * Frontend to certain, possibly pluggable, actions.
 */

/*
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2002-2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include <ostream>
#include <fstream>
#include <vector>
#include <glibmm/ustring.h>
#include "xml/repr.h"
#include <sigc++/signal.h>

namespace Gtk {
#if WITH_GTKMM_3_0
	class Grid;
#else
	class Table;
#endif

	class VBox;
	class Widget;
}

/** The key that is used to identify that the I/O should be autodetected */
#define SP_MODULE_KEY_AUTODETECT "autodetect"
/** This is the key for the SVG input module */
#define SP_MODULE_KEY_INPUT_SVG "org.inkscape.input.svg"
#define SP_MODULE_KEY_INPUT_SVGZ "org.inkscape.input.svgz"
/** Specifies the input module that should be used if none are selected */
#define SP_MODULE_KEY_INPUT_DEFAULT SP_MODULE_KEY_AUTODETECT
/** The key for outputing standard W3C SVG */
#define SP_MODULE_KEY_OUTPUT_SVG "org.inkscape.output.svg.plain"
#define SP_MODULE_KEY_OUTPUT_SVGZ "org.inkscape.output.svgz.plain"
/** This is an output file that has SVG data with the Sodipodi namespace extensions */
#define SP_MODULE_KEY_OUTPUT_SVG_INKSCAPE "org.inkscape.output.svg.inkscape"
#define SP_MODULE_KEY_OUTPUT_SVGZ_INKSCAPE "org.inkscape.output.svgz.inkscape"
/** Which output module should be used? */
#define SP_MODULE_KEY_OUTPUT_DEFAULT SP_MODULE_KEY_AUTODETECT

/** Defines the key for Postscript printing */
#define SP_MODULE_KEY_PRINT_PS    "org.inkscape.print.ps"
#define SP_MODULE_KEY_PRINT_CAIRO_PS    "org.inkscape.print.ps.cairo"
#define SP_MODULE_KEY_PRINT_CAIRO_EPS    "org.inkscape.print.eps.cairo"
/** Defines the key for PDF printing */
#define SP_MODULE_KEY_PRINT_PDF    "org.inkscape.print.pdf"
#define SP_MODULE_KEY_PRINT_CAIRO_PDF    "org.inkscape.print.pdf.cairo"
/** Defines the key for LaTeX printing */
#define SP_MODULE_KEY_PRINT_LATEX    "org.inkscape.print.latex"
/** Defines the key for printing with GNOME Print */
#define SP_MODULE_KEY_PRINT_GNOME "org.inkscape.print.gnome"

/** Mime type for SVG */
#define MIME_SVG "image/svg+xml"

/** Name of the extension error file */
#define EXTENSION_ERROR_LOG_FILENAME  "extension-errors.log"


#define INKSCAPE_EXTENSION_URI   "http://www.inkscape.org/namespace/inkscape/extension"
#define INKSCAPE_EXTENSION_NS_NC "extension"
#define INKSCAPE_EXTENSION_NS    "extension:"

class SPDocument;

namespace Inkscape {
namespace Extension {

class Dependency;
class ExpirationTimer;
class ExpirationTimer;
class Parameter;

namespace Implementation
{
class Implementation;
}


/** The object that is the basis for the Extension system.  This object
    contains all of the information that all Extension have.  The
    individual items are detailed within. This is the interface that
    those who want to _use_ the extensions system should use.  This
    is most likely to be those who are inside the Inkscape program. */
class Extension {
public:
    /** An enumeration to identify if the Extension has been loaded or not. */
    typedef enum {
        STATE_LOADED,      /**< The extension has been loaded successfully */
        STATE_UNLOADED,    /**< The extension has not been loaded */
        STATE_DEACTIVATED  /**< The extension is missing something which makes it unusable */
    } state_t;
    static std::vector<const gchar *> search_path; /**< A vector of paths to search for extensions */

private:
    gchar     *id;                        /**< The unique identifier for the Extension */
    gchar     *name;                      /**< A user friendly name for the Extension */
    gchar     *_help;                     /**< A string that contains a help text for the user */
    state_t    _state;                    /**< Which state the Extension is currently in */
    std::vector<Dependency *>  _deps;     /**< Dependencies for this extension */
    static std::ofstream error_file;      /**< This is the place where errors get reported */
    bool silent;
    bool _gui;

protected:
    Inkscape::XML::Node *repr;            /**< The XML description of the Extension */
    Implementation::Implementation * imp; /**< An object that holds all the functions for making this work */
    ExpirationTimer * timer;              /**< Timeout to unload after a given time */

public:
                  Extension    (Inkscape::XML::Node * in_repr,
                                Implementation::Implementation * in_imp);
    virtual      ~Extension    (void);

    void          set_state    (state_t in_state);
    state_t       get_state    (void);
    bool          loaded       (void);
    virtual bool  check        (void);
    Inkscape::XML::Node *      get_repr     (void);
    gchar *       get_id       (void);
    gchar *       get_name     (void);
    /** \brief  Gets the help string for this extension */
    gchar const * get_help     (void) { return _help; }
    bool          is_silent (void);
    void          deactivate   (void);
    bool          deactivated  (void);
    void          printFailure (Glib::ustring reason);
    Implementation::Implementation * get_imp (void) { return imp; };

/* Parameter Stuff */
private:
    GSList * parameters; /**< A table to store the parameters for this extension.
                              This only gets created if there are parameters in this
                              extension */

public:
    /** \brief  A function to get the number of parameters that
                the extension has.
        \return The number of parameters. */
    unsigned int param_count ( ) { return parameters == NULL ? 0 :
                                              g_slist_length(parameters); };
    /** \brief  A function to get the number of parameters that
                are visible to the user that the extension has.
        \return The number of visible parameters.

        \note Currently this just calls param_count as visible isn't implemented
              but in the future it'll do something different.  Please call
              the appropriate function in code so that it'll work in the
              future.
    */
    unsigned int param_visible_count ( );

public:
    /** An error class for when a parameter is called on a type it is not */
    class param_wrong_type {};
    class param_not_color_param {};
    class param_not_enum_param {};
    class param_not_optiongroup_param {};
    class param_not_string_param {};
    class param_not_float_param {};
    class param_not_int_param {};
    class param_not_bool_param {};

    /** An error class for when a parameter is looked for that just
     * simply doesn't exist */
    class param_not_exist {};

    /** An error class for when a filename already exists, but the user
     * doesn't want to overwrite it */
    class no_overwrite {};

private:
    void             make_param       (Inkscape::XML::Node * paramrepr);
    
    /**
     * This function looks through the linked list for a parameter
     * structure with the name of the passed in name.
     *
     * This is an inline function that is used by all the get_param and
     * set_param functions to find a param_t in the linked list with
     * the passed in name.
     *
     * This function can throw a 'param_not_exist' exception if the
     * name is not found.
     *
     * The first thing that this function checks is if the list is NULL.
     * It could be NULL because there are no parameters for this extension
     * or because all of them have been checked.  If the list
     * is NULL then the 'param_not_exist' exception is thrown.
     *
     * @param name The name to search for.
     * @return Parameter structure with a name of 'name'.
     */
     Parameter *get_param(const gchar * name);

     Parameter const *get_param(const gchar * name) const;

public:
    bool             get_param_bool   (const gchar * name,
                                       const SPDocument *   doc = NULL,
                                       const Inkscape::XML::Node * node = NULL);

    int              get_param_int    (const gchar * name,
                                       const SPDocument *   doc = NULL,
                                       const Inkscape::XML::Node * node = NULL);

    float            get_param_float  (const gchar * name,
                                       const SPDocument *   doc = NULL,
                                       const Inkscape::XML::Node * node = NULL);

    /**
     * Gets a parameter identified by name with the string placed in value.
     * It isn't duplicated into the value string. Look up in the parameters list,
     * then execute the function on that found parameter.
     *
     * @param name The name of the parameter to get.
     * @param doc The document to look in for document specific parameters.
     * @param node The node to look in for a specific parameter.
     * @return A constant pointer to the string held by the parameters.
     */
    gchar const *get_param_string(gchar const *name,
                                  SPDocument const *doc = NULL,
                                  Inkscape::XML::Node const *node = NULL) const;

    guint32          get_param_color  (const gchar * name,
                                       const SPDocument *   doc = NULL,
                                       const Inkscape::XML::Node * node = NULL) const;

    const gchar *    get_param_enum   (const gchar * name,
                                       const SPDocument *   doc = NULL,
                                       const Inkscape::XML::Node * node = NULL) const;

    gchar const *get_param_optiongroup( gchar const * name,
                                        SPDocument const *   doc = 0,
                                        Inkscape::XML::Node const * node = 0) const;

    bool             get_param_enum_contains(gchar const * name,
                                             gchar const * value,
                                             SPDocument  * doc = 0x0,
                                             Inkscape::XML::Node * node = 0x0) const;

    bool             set_param_bool   (const gchar * name,
                                       bool          value,
                                       SPDocument *   doc = NULL,
                                       Inkscape::XML::Node *       node = NULL);

    int              set_param_int    (const gchar * name,
                                       int           value,
                                       SPDocument *   doc = NULL,
                                       Inkscape::XML::Node *       node = NULL);

    float            set_param_float  (const gchar * name,
                                       float         value,
                                       SPDocument *   doc = NULL,
                                       Inkscape::XML::Node *       node = NULL);

    const gchar *    set_param_string (const gchar * name,
                                       const gchar * value,
                                       SPDocument *   doc = NULL,
                                       Inkscape::XML::Node *       node = NULL);

    gchar const * set_param_optiongroup(gchar const * name,
                                        gchar const * value,
                                        SPDocument * doc = 0,
                                        Inkscape::XML::Node * node = 0);

    gchar const *    set_param_enum   (gchar const * name,
                                       gchar const * value,
                                       SPDocument * doc = 0x0,
                                       Inkscape::XML::Node * node = 0x0);

    guint32          set_param_color  (const gchar * name,
                                       guint32 color,
                                       SPDocument *   doc = NULL,
                                       Inkscape::XML::Node *       node = NULL);

    /* Error file handling */
public:
    static void      error_file_open  (void);
    static void      error_file_close (void);

public:
    Gtk::Widget *    autogui (SPDocument * doc, Inkscape::XML::Node * node, sigc::signal<void> * changeSignal = NULL);
    void paramListString (std::list <std::string> & retlist);
    void set_gui(bool s) { _gui = s; }

    /* Extension editor dialog stuff */
public:
    Gtk::VBox *    get_info_widget(void);
    Gtk::VBox *    get_help_widget(void);
    Gtk::VBox *    get_params_widget(void);
protected:
#if WITH_GTKMM_3_0
    inline static void add_val(Glib::ustring labelstr, Glib::ustring valuestr, Gtk::Grid * table, int * row);
#else
    inline static void add_val(Glib::ustring labelstr, Glib::ustring valuestr, Gtk::Table * table, int * row);
#endif
};



/*

This is a prototype for how collections should work.  Whoever gets
around to implementing this gets to decide what a 'folder' and an
'item' really is.  That is the joy of implementing it, eh?

class Collection : public Extension {

public:
    folder  get_root (void);
    int     get_count (folder);
    thumbnail get_thumbnail(item);
    item[]  get_items(folder);
    folder[]  get_folders(folder);
    metadata get_metadata(item);
    image   get_image(item);

};
*/

}  /* namespace Extension */
}  /* namespace Inkscape */

#endif // INK_EXTENSION_H

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
