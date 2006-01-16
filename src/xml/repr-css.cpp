/*
 *   bulia byak <buliabyak@users.sf.net>
*/ 
 
#define SP_REPR_CSS_C


#include <glibmm/ustring.h>

#include "xml/repr.h"
#include "xml/simple-node.h"

using Inkscape::Util::List;
using Inkscape::XML::AttributeRecord;
using Inkscape::XML::SimpleNode;
using Inkscape::XML::Node;
using Inkscape::XML::NodeType;

struct SPCSSAttrImpl : public SimpleNode, public SPCSSAttr {
public:
    SPCSSAttrImpl() : SimpleNode(g_quark_from_static_string("css")) {}

    NodeType type() const { return Inkscape::XML::ELEMENT_NODE; }

protected:
    SimpleNode *_duplicate() const { return new SPCSSAttrImpl(*this); }
};

static void sp_repr_css_add_components(SPCSSAttr *css, Node *repr, gchar const *attr);

SPCSSAttr *
sp_repr_css_attr_new()
{
    return new SPCSSAttrImpl();
}

void
sp_repr_css_attr_unref(SPCSSAttr *css)
{
    g_assert(css != NULL);
    Inkscape::GC::release((Node *) css);
}

SPCSSAttr *sp_repr_css_attr(Node *repr, gchar const *attr)
{
    g_assert(repr != NULL);
    g_assert(attr != NULL);

    SPCSSAttr *css = sp_repr_css_attr_new();
    sp_repr_css_add_components(css, repr, attr);
    return css;
}

SPCSSAttr *sp_repr_css_attr_inherited(Node *repr, gchar const *attr)
{
    g_assert(repr != NULL);
    g_assert(attr != NULL);

    SPCSSAttr *css = sp_repr_css_attr_new();

    sp_repr_css_add_components(css, repr, attr);
    Node *current = sp_repr_parent(repr);

    while (current) {
        sp_repr_css_add_components(css, current, attr);
        current = sp_repr_parent(current);
    }

    return css;
}

static void
sp_repr_css_add_components(SPCSSAttr *css, Node *repr, gchar const *attr)
{
    g_assert(css != NULL);
    g_assert(repr != NULL);
    g_assert(attr != NULL);

    char const *data = repr->attribute(attr);
    sp_repr_css_attr_add_from_string(css, data);
}

char const *
sp_repr_css_property(SPCSSAttr *css, gchar const *name, gchar const *defval)
{
    g_assert(css != NULL);
    g_assert(name != NULL);

    char const *attr = ((Node *)css)->attribute(name);
    return ( attr == NULL
             ? defval
             : attr );
}

bool
sp_repr_css_property_is_unset(SPCSSAttr *css, gchar const *name)
{
    g_assert(css != NULL);
    g_assert(name != NULL);

    char const *attr = ((Node *)css)->attribute(name);
    return (attr && !strcmp(attr, "inkscape:unset"));
}


void
sp_repr_css_set_property(SPCSSAttr *css, gchar const *name, gchar const *value)
{
    g_assert(css != NULL);
    g_assert(name != NULL);

    sp_repr_set_attr((Node *) css, name, value);
}

void
sp_repr_css_unset_property(SPCSSAttr *css, gchar const *name)
{
    g_assert(css != NULL);
    g_assert(name != NULL);

    sp_repr_set_attr((Node *) css, name, "inkscape:unset");
}

double
sp_repr_css_double_property(SPCSSAttr *css, gchar const *name, double defval)
{
    g_assert(css != NULL);
    g_assert(name != NULL);

    return sp_repr_get_double_attribute((Node *) css, name, defval);
}

gchar *
sp_repr_css_write_string(SPCSSAttr *css)
{
    Glib::ustring buffer;

    for ( List<AttributeRecord const> iter = css->attributeList() ;
          iter ; ++iter )
    {
        if (iter->value && !strcmp(iter->value, "inkscape:unset")) {
            continue;
        }

        buffer.append(g_quark_to_string(iter->key));
        buffer.push_back(':');
        buffer.append(iter->value);
        if (rest(iter)) {
            buffer.push_back(';');
        }
    }

    return (buffer.empty() ? NULL : g_strdup (buffer.c_str()));
}

void
sp_repr_css_set(Node *repr, SPCSSAttr *css, gchar const *attr)
{
    g_assert(repr != NULL);
    g_assert(css != NULL);
    g_assert(attr != NULL);

    gchar *value = sp_repr_css_write_string(css);

    repr->setAttribute(attr, value);

    if (value) g_free (value);
}

void
sp_repr_css_print(SPCSSAttr *css)
{
    for ( List<AttributeRecord const> iter = css->attributeList() ;
          iter ; ++iter )
    {
        g_print(g_quark_to_string(iter->key));
        g_print(":\t");
        g_print(iter->value);
        g_print("\n");
    }
}

void
sp_repr_css_merge(SPCSSAttr *dst, SPCSSAttr *src)
{
    g_assert(dst != NULL);
    g_assert(src != NULL);

    dst->mergeFrom(src, "");
}

void
sp_repr_css_attr_add_from_string(SPCSSAttr *css, gchar const *data)
{
    if (data != NULL) {
        char *new_str = g_strdup(data);
        char **token = g_strsplit(new_str, ";", 0);
        for (char **ctoken = token; *ctoken != NULL; ctoken++) {
            char *current = g_strstrip(*ctoken);
            char *key = current;
            char *val;
            for (val = key; *val != '\0'; val++)
                if (*val == ':') break;
            if (*val == '\0') break;
            *val++ = '\0';
            key = g_strstrip(key);
            val = g_strstrip(val);
            if (*val == '\0') break;

            /* fixme: CSS specifies that later declarations override earlier ones with the same
               key.  (Ref: http://www.w3.org/TR/REC-CSS2/cascade.html#cascading-order point 4.)
               Either add a precondition that there are no key duplicates in the string, or get rid
               of the below condition (documenting the change that data[] will override existing
               values in *css), or process the list in reverse order. */
            if (!css->attribute(key))
                sp_repr_set_attr((Node *) css, key, val);
        }
        g_strfreev(token);
        g_free(new_str);
    }
}

void
sp_repr_css_change(Node *repr, SPCSSAttr *css, gchar const *attr)
{
    g_assert(repr != NULL);
    g_assert(css != NULL);
    g_assert(attr != NULL);

    SPCSSAttr *current = sp_repr_css_attr(repr, attr);
    sp_repr_css_merge(current, css);
    sp_repr_css_set(repr, current, attr);

    sp_repr_css_attr_unref(current);
}

void
sp_repr_css_change_recursive(Node *repr, SPCSSAttr *css, gchar const *attr)
{
    g_assert(repr != NULL);
    g_assert(css != NULL);
    g_assert(attr != NULL);

    sp_repr_css_change(repr, css, attr);

    for (Node *child = repr->firstChild(); child != NULL; child = child->next()) {
        sp_repr_css_change_recursive(child, css, attr);
    }
}


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
