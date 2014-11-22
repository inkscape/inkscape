#ifndef EXTENSION_INTERNAL_LATEX_TEXT_RENDERER_H_SEEN
#define EXTENSION_INTERNAL_LATEX_TEXT_RENDERER_H_SEEN

/** \file
 * Declaration of LaTeXTextRenderer, used for rendering the accompanying LaTeX file when exporting to PDF/EPS/PS + LaTeX 
 */
/*
 * Authors:
 *  Johan Engelen <goejendaagh@zonnet.nl>
 *
 * Copyright (C) 2010 Authors
 * 
 * Licensed under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "extension/extension.h"
#include <2geom/affine.h>
#include <stack>

class SPItem;
class SPRoot;
class SPGroup;
class SPUse;
class SPText;
class SPFlowtext;

namespace Inkscape {
namespace Extension {
namespace Internal {

bool latex_render_document_text_to_file(SPDocument *doc, gchar const *filename,
                                        const gchar * const exportId, bool exportDrawing, bool exportCanvas, float bleedmargin_px,
                                        bool pdflatex);

class LaTeXTextRenderer {
public:
    LaTeXTextRenderer(bool pdflatex);
    virtual ~LaTeXTextRenderer();

    bool setTargetFile(gchar const *filename);

    /** Initializes the LaTeXTextRenderer according to the specified
    SPDocument. Important to set the boundingbox to the pdf boundingbox */
    bool setupDocument(SPDocument *doc, bool pageBoundingBox, float bleedmargin_px, SPItem *base);

    /** Traverses the object tree and invokes the render methods. */
    void renderItem(SPItem *item);

protected:
    enum LaTeXOmitTextPageState {
        EMPTY,
        GRAPHIC_ON_TOP,
        NEW_PAGE_ON_GRAPHIC
    };

    FILE * _stream;
    gchar * _filename;

    bool _pdflatex; /** true if ouputting for pdfLaTeX*/

    LaTeXOmitTextPageState _omittext_state;
    gulong _omittext_page;

    void push_transform(Geom::Affine const &transform);
    Geom::Affine const & transform();
    void pop_transform();
    std::stack<Geom::Affine> _transform_stack;

    void writePreamble();
    void writePostamble();

    void writeGraphicPage();

    void sp_item_invoke_render(SPItem *item);
    void sp_root_render(SPRoot *item);
    void sp_group_render(SPGroup *group);
    void sp_use_render(SPUse *use);
    void sp_text_render(SPText *text);
    void sp_flowtext_render(SPFlowtext *flowtext);
};

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

#endif /* !EXTENSION_INTERNAL_LATEX_TEXT_RENDERER_H_SEEN */

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
