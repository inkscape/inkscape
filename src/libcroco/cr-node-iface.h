#ifndef __CR_NODE_IFACE_H__
#define __CR_NODE_IFACE_H__

#include <glib.h>

G_BEGIN_DECLS

typedef gconstpointer CRXMLNodePtr ;
typedef struct _CRNodeIface CRNodeIface ;

struct _CRNodeIface {
	/* Names based on DOM. */
	CRXMLNodePtr (*getParentNode)(CRXMLNodePtr);
	CRXMLNodePtr (*getFirstChild)(CRXMLNodePtr);
	CRXMLNodePtr (*getNextSibling)(CRXMLNodePtr);
	CRXMLNodePtr (*getPrevSibling)(CRXMLNodePtr);
	char const *(*getLocalName)(CRXMLNodePtr);
	char *(*getProp)(CRXMLNodePtr, char const *);

	/* Others. */
	void (*freePropVal)(void *);
	gboolean (*isElementNode)(CRXMLNodePtr);

#if 0
	char const *getLang(CRXMLNodePtr);
	/* todo: Make it easy to have the default xml rules for lang.  Maybe interpret NULL
	   like this.  Or provide a cr_get_xml_lang(CRNodeIface const *, CRXMLNodePtr) function. */
#endif
};

G_END_DECLS


#endif/*__CR_NODE_IFACE_H__*/
