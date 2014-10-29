#ifndef SEEN_SP_ITEM_FLOWREGION_H
#define SEEN_SP_ITEM_FLOWREGION_H

/*
 */

#include "sp-item.h"

#define SP_FLOWREGION(obj) (dynamic_cast<SPFlowregion*>((SPObject*)obj))
#define SP_IS_FLOWREGION(obj) (dynamic_cast<const SPFlowregion*>((SPObject*)obj) != NULL)

#define SP_FLOWREGIONEXCLUDE(obj) (dynamic_cast<SPFlowregionExclude*>((SPObject*)obj))
#define SP_IS_FLOWREGIONEXCLUDE(obj) (dynamic_cast<const SPFlowregionExclude*>((SPObject*)obj) != NULL)

class Path;
class Shape;
class flow_dest;
class FloatLigne;

class SPFlowregion : public SPItem {
public:
	SPFlowregion();
	virtual ~SPFlowregion();

	std::vector<Shape*>     computed;
	
	void             UpdateComputed(void);

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node *child);
	virtual void update(SPCtx *ctx, unsigned int flags);
	virtual void modified(guint flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
	virtual const char* displayName() const;
};

class SPFlowregionExclude : public SPItem {
public:
	SPFlowregionExclude();
	virtual ~SPFlowregionExclude();

	Shape            *computed;
	
	void             UpdateComputed(void);

	virtual void child_added(Inkscape::XML::Node* child, Inkscape::XML::Node* ref);
	virtual void remove_child(Inkscape::XML::Node *child);
	virtual void update(SPCtx *ctx, unsigned int flags);
	virtual void modified(guint flags);
	virtual Inkscape::XML::Node* write(Inkscape::XML::Document *xml_doc, Inkscape::XML::Node *repr, unsigned int flags);
	virtual const char* displayName() const;
};

#endif
