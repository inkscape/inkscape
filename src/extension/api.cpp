namespace Inkscape {
namespace Extension {
namespace API {

doc    file_open (gchar * uri, Inkscape::Extension::Input * ext) { }
bool   file_save (SPDocument * doc, gchar * uri, Inkscape::Extension::Output * ext) { }
bool   print_doc (SPDOcument * doc) { }

array  getElements(string element_name) { }
string getElementName() { }
string getElementByID(string id) { }
string getElementID() { }
hash   getAttributes() { }
string getAttribute(string attribute) { }
bool   SetAttribute(string name, string value) { }
bool   SetAttributes(hash) { }
bool   hasChildren() { }
array  getChildren() { }
obj    getFirstChild() { }
obj    getLastChild() { }
array  getSiblings() { }
obj    getNextSibling() { }
obj    getPrevSibling() { }
int    getChildIndex() { }
obj    getChildAtIndex(int index) { }
obj    getParent() { }
array  getParents() { }
bool   isAncestor(obj) { }
bool   isDescendant(obj) { }
cdata  getCDATA() { }

/** looks up the numerical ID of the given verb name.  (E.g., it could
    pull it out of the SPVerbActionDef props[] static object) */
int    verb_find(string verb_name) { }

/** retrieves the Action object for the verb in the given view */
action verb_get_action(int verb, int view) { }

/** retrieves the view, given the action object */
view   action_get_view(int action) { }

/** invokes the Action for the verb.  It validates the parameters and
    calls the appropriate listener that will handle the action. */
void   action_perform(int action, int data, int pdata) { }

void   action_set_active(int action, int state, int data) { }

void   action_set_sensitive(int action, int state, int data) { }

void    prefs_set_int_attribute(string path, string attr, int value) { }

int     prefs_get_int_attribute(string path, string attr, gint def) { }

int     prefs_get_int_attribute_limited(string path, string attr, int def, int min, int max) { }

void    prefs_set_double_attribute(string path, string attr, double value) { }

double  prefs_get_double_attribute(string path, string attr, double def) { }

double  prefs_get_double_attribute_limited(string path, string attr, double def, double min, double max) { }

string  prefs_get_string_attribute(string path, string attr) { }

void    prefs_set_string_attribute(string path, string attr, string value) { }

void    prefs_set_recent_file(string uri, string name) { }

string[] prefs_get_recent_files() { }

/** returns a pointer to an instance of the desired stock object in
    the current doc, importing the object if necessary. */
obj get_stock_item(string urn) { }

/** adds a new stock SVG def to the Inkscape Application.  All
    currently loaded and new documents will have access to the item. */
void set_stock_item_from_svg(void) { }

/** This function gets the variable by the name 'name' with the data
    in 'data'.  It can be a boolean, integer or a string. */
get_param (gchar * name, bool data) { }
get_param (gchar * name, int data) { }
get_param (gchar * name, gchar * data) { }

/** This function sets the variable by the name 'name' with the data
    in 'data'.  It can be a boolean, integer or a string.  In the
    case of the string it is copied. */
set_param (gchar * name, bool data) { }
set_param (gchar * name, int data) { }
set_param (gchar * name, gchar * data) { }


} } } /* namespaces Inkscape::Extension::API */
