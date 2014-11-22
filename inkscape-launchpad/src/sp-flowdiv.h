#ifndef SEEN_SP_ITEM_FLOWDIV_H
#define SEEN_SP_ITEM_FLOWDIV_H

/*
 */

#include "sp-object.h"
#include "sp-item.h"

#define SP_FLOWDIV(obj) (dynamic_cast<SPFlowdiv*>((SPObject*)obj))
#define SP_IS_FLOWDIV(obj) (dynamic_cast<const SPFlowdiv*>((SPObject*)obj) != NULL)

#define SP_FLOWTSPAN(obj) (dynamic_cast<SPFlowtspan*>((SPObject*)obj))
#define SP_IS_FLOWTSPAN(obj) (dynamic_cast<const SPFlowtspan*>((SPObject*)obj) != NULL)

#define SP_FLOWPARA(obj) (dynamic_cast<SPFlowpara*>((SPObject*)obj))
#define SP_IS_FLOWPARA(obj) (dynamic_cast<const SPFlowpara*>((SPObject*)obj) != NULL)

#define SP_FLOWLINE(obj) (dynamic_cast<SPFlowline*>((SPObject*)obj))
#define SP_IS_FLOWLINE(obj) (dynamic_cast<const SPFlowline*>((SPObject*)obj) != NULL)

#define SP_FLOWREGIONBREAK(obj) (dynamic_cast<SPFlowregionbreak*>((SPObject*)obj))
#define SP_IS_FLOWREGIONBREAK(obj) (dynamic_cast<const SPFlowregionbreak*>((SPObject*)obj) != NULL)

// these 3 are derivatives of SPItem to get the automatic style handling
class SPFlowdiv : public SPItem {
public:
	SPFlowdiv();
	virtual ~SPFlowdiv();

protected:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

class SPFlowtspan : public SPItem {
public:
	SPFlowtspan();
	virtual ~SPFlowtspan();

protected:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

class SPFlowpara : public SPItem {
public:
	SPFlowpara();
	virtual ~SPFlowpara();

protected:
	virtual void build(SPDocument *document, Inkscape::XML::Node *repr);
	virtual void release();
	virtual void update(SPCtx* ctx, unsigned int flags);
	virtual void modified(unsigned int flags);

	virtual void set(unsigned int key, char const* value);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

// these do not need any style
class SPFlowline : public SPObject {
public:
	SPFlowline();
	virtual ~SPFlowline();

protected:
	virtual void release();
	virtual void modified(unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

class SPFlowregionbreak : public SPObject {
public:
	SPFlowregionbreak();
	virtual ~SPFlowregionbreak();

protected:
	virtual void release();
	virtual void modified(unsigned int flags);

	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
};

#endif
