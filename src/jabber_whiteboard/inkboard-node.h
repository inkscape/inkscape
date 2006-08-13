/**
 * Inkscape::Whiteboard::InkboardDocument - Inkboard document implementation
 *
 * Authors:
 * Dale Harvey <harveyd@gmail.com>
 *
 * Copyright (c) 2005 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef __INKSCAPE_WHITEBOARD_INKBOARDNODE_H__
#define __INKSCAPE_WHITEBOARD_INKBOARDNODE_H__

#include <glibmm.h>

#include "document.h"
#include "xml/document.h"
#include "xml/simple-node.h"

namespace Inkscape {

namespace Whiteboard {

class InkboardNode : public XML::SimpleNode {
public:
	
    explicit InkboardNode(int code, Inkscape::XML::NodeType type);

    XML::NodeType type() const
    {
	return this->nodetype;
    }

protected:

    /**
     * Copy constructor.
     * 
     * \param orig Instance to copy.
     */
    InkboardNode(InkboardNode const& orig, Inkscape::XML::NodeType type) :
    	XML::Node(), XML::SimpleNode(orig), version(0), nodetype(type) {}

	XML::SimpleNode* _duplicate() const
	{
		return new InkboardNode(*this,this->nodetype);
	}

private:

    unsigned int version;
    Inkscape::XML::NodeType nodetype;
};

}

}

#endif

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
