#ifndef SEEN_SP_CLIPPATH_H
#define SEEN_SP_CLIPPATH_H

/*
 * SVG <clipPath> implementation
 *
 * Authors:
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *   Abhishek Sharma
 *   Jon A. Cruz <jon@joncruz.org>
 *
 * Copyright (C) 2001-2002 authors
 * Copyright (C) 2001 Ximian, Inc.
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#define SP_TYPE_CLIPPATH (SPClipPath::sp_clippath_get_type())
#define SP_CLIPPATH(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SP_TYPE_CLIPPATH, SPClipPath))
#define SP_CLIPPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SP_TYPE_CLIPPATH, SPClipPathClass))
#define SP_IS_CLIPPATH(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SP_TYPE_CLIPPATH))
#define SP_IS_CLIPPATH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SP_TYPE_CLIPPATH))

class SPClipPathView;

#include "display/display-forward.h"
#include "libnr/nr-forward.h"
#include "sp-object-group.h"
#include "uri-references.h"
#include "xml/node.h"

class SPClipPath : public SPObjectGroup {
public:
    class Reference;

    unsigned int clipPathUnits_set : 1;
    unsigned int clipPathUnits : 1;

    SPClipPathView *display;
    static const gchar *create(GSList *reprs, SPDocument *document, Geom::Affine const* applyTransform);
    static GType sp_clippath_get_type(void);

    Inkscape::DrawingItem *show(Inkscape::Drawing &drawing, unsigned int key);
    void hide(unsigned int key);

    void setBBox(unsigned int key, NRRect *bbox);
    void getBBox(NRRect *bbox, Geom::Affine const &transform, unsigned const flags);

private:
    static void init(SPClipPath *clippath);

    static void build(SPObject *object, SPDocument *document, Inkscape::XML::Node *repr);
    static void release(SPObject * object);
    static void set(SPObject *object, unsigned int key, gchar const *value);
    static void childAdded(SPObject *object, Inkscape::XML::Node *child, Inkscape::XML::Node *ref);
    static void update(SPObject *object, SPCtx *ctx, guint flags);
    static void modified(SPObject *object, guint flags);
    static Inkscape::XML::Node *write(SPObject *object, Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags);

    friend class SPClipPathClass;
};

class SPClipPathClass {
public:
    SPObjectGroupClass parent_class;

private:
    static void sp_clippath_class_init(SPClipPathClass *klass);
    static SPObjectGroupClass *static_parent_class;

    friend class SPClipPath;
};

class SPClipPathReference : public Inkscape::URIReference {
public:
    SPClipPathReference(SPObject *obj) : URIReference(obj) {}
    SPClipPath *getObject() const {
        return (SPClipPath *)URIReference::getObject();
    }

protected:
    /**
     * If the owner element of this reference (the element with <... clippath="...">)
     * is a child of the clippath it refers to, return false.
     * \return false if obj is not a clippath or if obj is a parent of this
     *         reference's owner element.  True otherwise.
     */
    virtual bool _acceptObject(SPObject *obj) const {
        if (!SP_IS_CLIPPATH(obj)) {
            return false;
        }
        SPObject * const owner = this->getOwner();
        if (obj->isAncestorOf(owner)) {
            //XML Tree being used directly here while it shouldn't be...
            Inkscape::XML::Node * const owner_repr = owner->getRepr();
            //XML Tree being used directly here while it shouldn't be...
            Inkscape::XML::Node * const obj_repr = obj->getRepr();
            gchar const * owner_name = NULL;
            gchar const * owner_clippath = NULL;
            gchar const * obj_name = NULL;
            gchar const * obj_id = NULL;
            if (owner_repr != NULL) {
                owner_name = owner_repr->name();
                owner_clippath = owner_repr->attribute("clippath");
            }
            if (obj_repr != NULL) {
                obj_name = obj_repr->name();
                obj_id = obj_repr->attribute("id");
            }
            g_warning("Ignoring recursive clippath reference "
                      "<%s clippath=\"%s\"> in <%s id=\"%s\">",
                      owner_name, owner_clippath,
                      obj_name, obj_id);
            return false;
        }
        return true;
    }
};

#endif // SEEN_SP_CLIPPATH_H

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
