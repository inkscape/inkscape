#ifndef SP_ERASER_CONTEXT_H_SEEN
#define SP_ERASER_CONTEXT_H_SEEN

/*
 * Handwriting-like drawing mode
 *
 * Authors:
 *   Mitsuru Oka <oka326@parkcity.ne.jp>
 *   Lauris Kaplinski <lauris@kaplinski.com>
 *
 * The original dynadraw code:
 *   Paul Haeberli <paul@sgi.com>
 *
 * Copyright (C) 1998 The Free Software Foundation
 * Copyright (C) 1999-2002 authors
 * Copyright (C) 2001-2002 Ximian, Inc.
 * Copyright (C) 2008 Jon A. Cruz
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#include "common-context.h"

#define SP_TYPE_ERASER_CONTEXT (sp_eraser_context_get_type())
#define SP_ERASER_CONTEXT(o) (GTK_CHECK_CAST((o), SP_TYPE_ERASER_CONTEXT, SPEraserContext))
#define SP_ERASER_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_CAST((k), SP_TYPE_ERASER_CONTEXT, SPEraserContextClass))
#define SP_IS_ERASER_CONTEXT(o) (GTK_CHECK_TYPE((o), SP_TYPE_ERASER_CONTEXT))
#define SP_IS_ERASER_CONTEXT_CLASS(k) (GTK_CHECK_CLASS_TYPE((k), SP_TYPE_ERASER_CONTEXT))

class SPEraserContext;
class SPEraserContextClass;

#define ERC_MIN_PRESSURE      0.0
#define ERC_MAX_PRESSURE      1.0
#define ERC_DEFAULT_PRESSURE  1.0

#define ERC_MIN_TILT         -1.0
#define ERC_MAX_TILT          1.0
#define ERC_DEFAULT_TILT      0.0

struct SPEraserContext : public SPCommonContext {
};

struct SPEraserContextClass : public SPEventContextClass{};

GType sp_eraser_context_get_type(void);

#endif // SP_ERASER_CONTEXT_H_SEEN

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
