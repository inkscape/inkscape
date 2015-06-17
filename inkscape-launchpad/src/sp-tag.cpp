/** \file
 * SVG <inkscape:tag> implementation
 * 
 * Authors:
 *   Theodore Janeczko
 *   Liam P. White
 *
 * Copyright (C) Theodore Janeczko 2012-2014 <flutterguy317@gmail.com>
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "attributes.h"
#include "sp-tag.h"
#include "xml/repr.h"
#include <cstring>

/*
 * Move this SPItem into or after another SPItem in the doc
 * \param  target - the SPItem to move into or after
 * \param  intoafter - move to after the target (false), move inside (sublayer) of the target (true)
 */
void SPTag::moveTo(SPObject *target, gboolean intoafter) {

    Inkscape::XML::Node *target_ref = ( target ? target->getRepr() : NULL );
    Inkscape::XML::Node *our_ref = getRepr();
    gboolean first = FALSE;

    if (target_ref == our_ref) {
        // Move to ourself ignore
        return;
    }

    if (!target_ref) {
        // Assume move to the "first" in the top node, find the top node
        target_ref = our_ref;
        while (target_ref->parent() != target_ref->root()) {
            target_ref = target_ref->parent();
        }
        first = TRUE;
    }

    if (intoafter) {
        // Move this inside of the target at the end
        our_ref->parent()->removeChild(our_ref);
        target_ref->addChild(our_ref, NULL);
    } else if (target_ref->parent() != our_ref->parent()) {
        // Change in parent, need to remove and add
        our_ref->parent()->removeChild(our_ref);
        target_ref->parent()->addChild(our_ref, target_ref);
    } else if (!first) {
        // Same parent, just move
        our_ref->parent()->changeOrder(our_ref, target_ref);
    }
}

/**
 * Reads the Inkscape::XML::Node, and initializes SPTag variables.  For this to get called,
 * our name must be associated with a repr via "sp_object_type_register".  Best done through
 * sp-object-repr.cpp's repr_name_entries array.
 */
void
SPTag::build(SPDocument *document, Inkscape::XML::Node *repr)
{
    readAttr( "inkscape:expanded" );
    SPObject::build(document, repr);
}

/**
 * Sets a specific value in the SPTag.
 */
void
SPTag::set(unsigned int key, gchar const *value)
{
    
    switch (key)
    {
        case SP_ATTR_INKSCAPE_EXPANDED:
            if ( value && !strcmp(value, "true") ) {
                setExpanded(true);
            }
            break;
        default:
                SPObject::set(key, value);
            break;
    }
}

void SPTag::setExpanded(bool isexpanded) {
    //if ( _expanded != isexpanded ){
        _expanded = isexpanded;
    //}
}

/**
 * Receives update notifications.
 */
void
SPTag::update(SPCtx *ctx, guint flags)
{
    if (flags & (SP_OBJECT_MODIFIED_FLAG | SP_OBJECT_STYLE_MODIFIED_FLAG |
                 SP_OBJECT_VIEWPORT_MODIFIED_FLAG)) {

        /* do something to trigger redisplay, updates? */

    }
    SPObject::update(ctx, flags);
}

/**
 * Writes its settings to an incoming repr object, if any.
 */
Inkscape::XML::Node *
SPTag::write(Inkscape::XML::Document *doc, Inkscape::XML::Node *repr, guint flags)
{
    if ((flags & SP_OBJECT_WRITE_BUILD) && !repr) {
        repr = doc->createElement("inkscape:tag");
    }
    
    // Inkscape-only object, not copied during an "plain SVG" dump:
    if (flags & SP_OBJECT_WRITE_EXT) {
        if (_expanded) {
            repr->setAttribute("inkscape:expanded", "true");
        } else {
            repr->setAttribute("inkscape:expanded", NULL);
        }
    }
    SPObject::write(doc, repr, flags);
    return repr;
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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
