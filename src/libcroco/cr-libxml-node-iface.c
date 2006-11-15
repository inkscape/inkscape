#include <libxml/tree.h>
#include <string.h>
#include "cr-libxml-node-iface.h"

static CRXMLNodePtr
libxml_getParentNode(CRXMLNodePtr cnode)
{
	xmlNode const *xnode = (xmlNode const *) cnode;
	return xnode->parent;
}

static CRXMLNodePtr
libxml_getFirstChild(CRXMLNodePtr cnode)
{
	xmlNode const *xnode = (xmlNode const *) cnode;
	return xnode->children;
}

static CRXMLNodePtr
libxml_getNextSibling(CRXMLNodePtr cnode)
{
	xmlNode const *xnode = (xmlNode const *) cnode;
	return xnode->next;
}

static CRXMLNodePtr
libxml_getPrevSibling(CRXMLNodePtr cnode)
{
	xmlNode const *xnode = (xmlNode const *) cnode;
	return xnode->prev;
}

static char const *
local_part(char const *const qname)
{
	char const *ret = strrchr(qname, ':');
	if (ret)
		return ++ret;
	else
		return qname;
}

static char const *
libxml_getLocalName(CRXMLNodePtr cnode)
{
	xmlNode const *xnode = (xmlNode const *) cnode;
	return local_part((char *)xnode->name);
}

static char *
libxml_getProp(CRXMLNodePtr cnode, char const *cprop)
{
	xmlNodePtr xnode = (xmlNodePtr) cnode;
	xmlChar const *xprop = (xmlChar const *) cprop;
	return (char *)xmlGetProp(xnode, xprop);
}

static gboolean
libxml_isElementNode(CRXMLNodePtr cnode)
{
	xmlNode const *xnode = (xmlNode const *) cnode;
	return xnode->type == XML_ELEMENT_NODE;
}

static void
libxml_freePropVal(void *const cval)
{
	xmlFree(cval);
}

CRNodeIface const cr_libxml_node_iface = {
	libxml_getParentNode,
	libxml_getFirstChild,
	libxml_getNextSibling,
	libxml_getPrevSibling,
	libxml_getLocalName,
	libxml_getProp,  /* fixme: Check whether we want xmlGetNoNsProp instead. */

	libxml_freePropVal,
	libxml_isElementNode
};
