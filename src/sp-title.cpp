/*
 * SVG <title> implementation
 *
 * Authors:
 *   Jeff Schiller <codedread@gmail.com>
 *
 * Copyright (C) 2008 Jeff Schiller
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "sp-title.h"
#include "xml/repr.h"

SPTitle::SPTitle() : SPObject() {
}

SPTitle::~SPTitle() {
}

Inkscape::XML::Node* SPTitle::write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, guint flags) {
	SPTitle* object = this;

    if (!repr) {
        repr = object->getRepr()->duplicate(xml_doc);
    }

    SPObject::write(xml_doc, repr, flags);

    return repr;
}

