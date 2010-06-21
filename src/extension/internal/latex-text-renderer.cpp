#define EXTENSION_INTERNAL_LATEX_TEXT_RENDERER_CPP

/** \file
 * Rendering LaTeX file (pdf/eps/ps+latex output)
 *
 * The idea stems from GNUPlot's epslatex terminal output :-)
 */
/*
 * Authors:
 *   Johan Engelen <goejendaagh@zonnet.nl>
 *   Miklos Erdelyi <erdelyim@gmail.com>
 *
 * Copyright (C) 2006-2010 Authors
 *
 * Licensed under GNU GPL
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "latex-text-renderer.h"

#include <signal.h>
#include <errno.h>

#include "libnrtype/Layout-TNG.h"
#include <2geom/transforms.h>
#include <2geom/rect.h>

#include <glibmm/i18n.h>
#include "sp-item.h"
#include "sp-item-group.h"
#include "style.h"
#include "sp-root.h"
#include "sp-use.h"
#include "sp-text.h"
#include "sp-flowtext.h"
#include "sp-rect.h"
#include "text-editing.h"

#include <unit-constants.h>

#include "extension/system.h"

#include "io/sys.h"

namespace Inkscape {
namespace Extension {
namespace Internal {

/**
 * This method is called by the PDF, EPS and PS output extensions.
 * @param filename This should be the filename without extension to which the tex code should be written. Output goes to <filename>.tex.
 */
bool
latex_render_document_text_to_file( SPDocument *doc, gchar const *filename,
                                    const gchar * const exportId, bool exportDrawing, bool exportCanvas,
                                    bool pdflatex)
{
    sp_document_ensure_up_to_date(doc);

    SPItem *base = NULL;

    bool pageBoundingBox = true;
    if (exportId && strcmp(exportId, "")) {
        // we want to export the given item only
        base = SP_ITEM(doc->getObjectById(exportId));
        pageBoundingBox = exportCanvas;
    }
    else {
        // we want to export the entire document from root
        base = SP_ITEM(sp_document_root(doc));
        pageBoundingBox = !exportDrawing;
    }

    if (!base)
        return false;

    /* Create renderer */
    LaTeXTextRenderer *renderer = new LaTeXTextRenderer(pdflatex);

    bool ret = renderer->setTargetFile(filename);
    if (ret) {
        /* Render document */
        bool ret = renderer->setupDocument(doc, pageBoundingBox, base);
        if (ret) {
            renderer->renderItem(base);
        }
    }

    delete renderer;

    return ret;
}

LaTeXTextRenderer::LaTeXTextRenderer(bool pdflatex)
  : _stream(NULL),
    _filename(NULL),
    _pdflatex(pdflatex)
{
    push_transform(Geom::identity());
}

LaTeXTextRenderer::~LaTeXTextRenderer(void)
{
    if (_stream) {
        writePostamble();

        fclose(_stream);
    }

    /* restore default signal handling for SIGPIPE */
#if !defined(_WIN32) && !defined(__WIN32__)
    (void) signal(SIGPIPE, SIG_DFL);
#endif

    if (_filename) {
        g_free(_filename);
    }

    return;
}

/** This should create the output LaTeX file, and assign it to _stream.
 * @return Returns true when succesfull
 */
bool
LaTeXTextRenderer::setTargetFile(gchar const *filename) {
    if (filename != NULL) {
        while (isspace(*filename)) filename += 1;

        _filename = g_path_get_basename(filename);

        gchar *filename_ext = g_strdup_printf("%s.tex", filename);
        Inkscape::IO::dump_fopen_call(filename_ext, "K");
        FILE *osf = Inkscape::IO::fopen_utf8name(filename_ext, "w+");
        if (!osf) {
            fprintf(stderr, "inkscape: fopen(%s): %s\n",
                    filename_ext, strerror(errno));
            return false;
        }
        _stream = osf;
        g_free(filename_ext);
    }

    if (_stream) {
        /* fixme: this is kinda icky */
#if !defined(_WIN32) && !defined(__WIN32__)
        (void) signal(SIGPIPE, SIG_IGN);
#endif
    }

    fprintf(_stream, "%%%% Creator: Inkscape %s, www.inkscape.org\n", PACKAGE_STRING);
    fprintf(_stream, "%%%% PDF/EPS/PS + LaTeX output extension by Johan Engelen, 2010\n");
    fprintf(_stream, "%%%% Accompanies image file '%s' (pdf, eps, ps)\n", _filename);
    fprintf(_stream, "%%%%\n");
    /* flush this to test output stream as early as possible */
    if (fflush(_stream)) {
        if (ferror(_stream)) {
            g_print("Error %d on LaTeX file output stream: %s\n", errno,
                    g_strerror(errno));
        }
        g_print("Output to LaTeX file failed\n");
        /* fixme: should use pclose() for pipes */
        fclose(_stream);
        _stream = NULL;
        fflush(stdout);
        return false;
    }

    writePreamble();

    return true;
}

static char const preamble[] =
"%% To include the image in your LaTeX document, write\n"
"%%   \\input{<filename>.tex}\n"
"%%  instead of\n"
"%%   \\includegraphics{<filename>.pdf}\n"
"%% To scale the image, write\n"
"%%   \\def{\\svgwidth}{<desired width>}\n"
"%%   \\input{<filename>.tex}\n"
"%%  instead of\n"
"%%   \\includegraphics[width=<desired width>]{<filename>.pdf}\n"
"%%\n"
"%% Images with a different path to the parent latex file can\n"
"%% be accessed with the `import' package (which may need to be\n"
"%% installed) using\n"
"%%   \\usepackage{import}\n"
"%% in the preamble, and then including the image with\n"
"%%   \\import{<path to file>}{<filename>.tex}\n"
"%% Alternatively, one can specify\n"
"%%   \\graphicspath{{<path to file>/}}\n"
"%% \n"
"%% For more information, please see info/svg-inkscape on CTAN:\n"
"%%   http://tug.ctan.org/tex-archive/info/svg-inkscape\n"
"\n"
"\\begingroup\n"
"  \\makeatletter\n"
"  \\providecommand\\color[2][]{%\n"
"    \\errmessage{(Inkscape) Color is used for the text in Inkscape, but the package \'color.sty\' is not loaded}\n"
"    \\renewcommand\\color[2][]{}%\n"
"  }\n"
"  \\providecommand\\transparent[1]{%\n"
"    \\errmessage{(Inkscape) Transparency is used (non-zero) for the text in Inkscape, but the package \'transparent.sty\' is not loaded}\n"
"    \\renewcommand\\transparent[1]{}%\n"
"  }\n"
"  \\providecommand\\rotatebox[2]{#2}\n";

static char const postamble[] =
"  \\end{picture}%\n"
"\\endgroup\n";

void
LaTeXTextRenderer::writePreamble()
{
    fprintf(_stream, "%s", preamble);
}
void
LaTeXTextRenderer::writePostamble()
{
    fprintf(_stream, "%s", postamble);
}

void
LaTeXTextRenderer::sp_group_render(SPItem *item)
{
    SPGroup *group = SP_GROUP(item);

    GSList *l = g_slist_reverse(group->childList(false));
    while (l) {
        SPObject *o = SP_OBJECT (l->data);
        if (SP_IS_ITEM(o)) {
            renderItem (SP_ITEM (o));
        }
        l = g_slist_remove (l, o);
    }
}

void
LaTeXTextRenderer::sp_use_render(SPItem *item)
{
    bool translated = false;
    SPUse *use = SP_USE(item);

    if ((use->x._set && use->x.computed != 0) || (use->y._set && use->y.computed != 0)) {
        Geom::Matrix tp(Geom::Translate(use->x.computed, use->y.computed));
        push_transform(tp);
        translated = true;
    }

    if (use->child && SP_IS_ITEM(use->child)) {
        renderItem(SP_ITEM(use->child));
    }

    if (translated) {
        pop_transform();
    }
}

void
LaTeXTextRenderer::sp_text_render(SPItem *item)
{
    SPText *textobj = SP_TEXT (item);
    SPStyle *style = SP_OBJECT_STYLE (SP_OBJECT(item));

    gchar *str = sp_te_get_string_multiline(item);
    if (!str) {
        return;
    }

    // get position and alignment
    // Align vertically on the baseline of the font (retreived from the anchor point)
    // Align horizontally on anchorpoint
    gchar const *alignment = NULL;
    switch (style->text_anchor.computed) {
    case SP_CSS_TEXT_ANCHOR_START:
        alignment = "[lb]";
        break;
    case SP_CSS_TEXT_ANCHOR_END:
        alignment = "[rb]";
        break;
    case SP_CSS_TEXT_ANCHOR_MIDDLE:
    default:
        alignment = "[b]";
        break;
    }
    Geom::Point anchor = textobj->attributes.firstXY() * transform();
    Geom::Point pos(anchor);

    // determine color and transparency (for now, use rgb color model as it is most native to Inkscape)
    bool has_color = false; // if the item has no color set, don't force black color
    bool has_transparency = false;
    // TODO: how to handle ICC colors?
    // give priority to fill color
    guint32 rgba = 0;
    float opacity = SP_SCALE24_TO_FLOAT(style->opacity.value);
    if (style->fill.set && style->fill.isColor()) {
        has_color = true;
        rgba = style->fill.value.color.toRGBA32(1.);
        opacity *= SP_SCALE24_TO_FLOAT(style->fill_opacity.value);
    } else if (style->stroke.set && style->stroke.isColor()) {
        has_color = true;
        rgba = style->stroke.value.color.toRGBA32(1.);
        opacity *= SP_SCALE24_TO_FLOAT(style->stroke_opacity.value);
    }
    if (opacity < 1.0) {
        has_transparency = true;
    }

    // get rotation
    Geom::Matrix i2doc = sp_item_i2doc_affine(item);
    Geom::Matrix wotransl = i2doc.without_translation();
    double degrees = -180/M_PI * Geom::atan2(wotransl.xAxis());
    bool has_rotation = !Geom::are_near(degrees,0.);

    // write to LaTeX
    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed); // don't use scientific notation

    os << "    \\put(" << pos[Geom::X] << "," << pos[Geom::Y] << "){";
    if (has_color) {
        os << "\\color[rgb]{" << SP_RGBA32_R_F(rgba) << "," << SP_RGBA32_G_F(rgba) << "," << SP_RGBA32_B_F(rgba) << "}";
    }
    if (_pdflatex && has_transparency) {
        os << "\\transparent{" << opacity << "}";
    }
    if (has_rotation) {
        os << "\\rotatebox{" << degrees << "}{";
    }
    os << "\\makebox(0,0)" << alignment << "{";
    os << "\\smash{" << str << "}";  // smash the text, to be able to put the makebox coordinates at the baseline
    if (has_rotation) {
        os << "}"; // rotatebox end
    }
    os << "}"; //makebox end
    os << "}%\n"; // put end

    fprintf(_stream, "%s", os.str().c_str());
}

void
LaTeXTextRenderer::sp_flowtext_render(SPItem * item)
{
/*
Flowtext is possible by using a minipage! :)
Flowing in rectangle is possible, not in arb shape.
*/

    SPFlowtext *flowtext = SP_FLOWTEXT(item);
    SPStyle *style = SP_OBJECT_STYLE (SP_OBJECT(item));

    gchar *strtext = sp_te_get_string_multiline(item);
    if (!strtext) {
        return;
    }
    // replace carriage return with double slash
    gchar ** splitstr = g_strsplit(strtext, "\n", -1);
    gchar *str = g_strjoinv("\\\\ ", splitstr);
    g_free(strtext);
    g_strfreev(splitstr);

    SPItem *frame_item = flowtext->get_frame(NULL);
    if (!frame_item || !SP_IS_RECT(frame_item)) {
        g_warning("LaTeX export: non-rectangular flowed text shapes are not supported, skipping text.");
        return; // don't know how to handle non-rect frames yet. is quite uncommon for latex users i think
    }

    SPRect *frame = SP_RECT(frame_item);
    Geom::Rect framebox = sp_rect_get_rect(frame) * transform();

    // get position and alignment
    // Align on topleft corner.
    gchar const *alignment = "[lt]";
    gchar const *justification = "";
    switch (flowtext->layout.paragraphAlignment(flowtext->layout.begin())) {
    case Inkscape::Text::Layout::LEFT:
        justification = "\\raggedright ";
        break;
    case Inkscape::Text::Layout::RIGHT:
        justification = "\\raggedleft ";
        break;
    case Inkscape::Text::Layout::CENTER:
        justification = "\\centering ";
    case Inkscape::Text::Layout::FULL:
    default:
        // no need to add LaTeX code for standard justified output :)
        break;
    }
    Geom::Point pos(framebox.corner(3)); //topleft corner

    // determine color and transparency (for now, use rgb color model as it is most native to Inkscape)
    bool has_color = false; // if the item has no color set, don't force black color
    bool has_transparency = false;
    // TODO: how to handle ICC colors?
    // give priority to fill color
    guint32 rgba = 0;
    float opacity = SP_SCALE24_TO_FLOAT(style->opacity.value);
    if (style->fill.set && style->fill.isColor()) {
        has_color = true;
        rgba = style->fill.value.color.toRGBA32(1.);
        opacity *= SP_SCALE24_TO_FLOAT(style->fill_opacity.value);
    } else if (style->stroke.set && style->stroke.isColor()) {
        has_color = true;
        rgba = style->stroke.value.color.toRGBA32(1.);
        opacity *= SP_SCALE24_TO_FLOAT(style->stroke_opacity.value);
    }
    if (opacity < 1.0) {
        has_transparency = true;
    }

    // get rotation
    Geom::Matrix i2doc = sp_item_i2doc_affine(item);
    Geom::Matrix wotransl = i2doc.without_translation();
    double degrees = -180/M_PI * Geom::atan2(wotransl.xAxis());
    bool has_rotation = !Geom::are_near(degrees,0.);

    // write to LaTeX
    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed); // don't use scientific notation

    os << "    \\put(" << pos[Geom::X] << "," << pos[Geom::Y] << "){";
    if (has_color) {
        os << "\\color[rgb]{" << SP_RGBA32_R_F(rgba) << "," << SP_RGBA32_G_F(rgba) << "," << SP_RGBA32_B_F(rgba) << "}";
    }
    if (_pdflatex && has_transparency) {
        os << "\\transparent{" << opacity << "}";
    }
    if (has_rotation) {
        os << "\\rotatebox{" << degrees << "}{";
    }
    os << "\\makebox(0,0)" << alignment << "{";
    os << "\\begin{minipage}{" << framebox.width() << "\\unitlength}";
    os << justification;
    os << str;
    os << "\\end{minipage}";
    if (has_rotation) {
        os << "}"; // rotatebox end
    }
    os << "}"; //makebox end
    os << "}%\n"; // put end

    fprintf(_stream, "%s", os.str().c_str());
}

void
LaTeXTextRenderer::sp_root_render(SPItem *item)
{
    SPRoot *root = SP_ROOT(item);

    push_transform(root->c2p);
    sp_group_render(item);
    pop_transform();
}

void
LaTeXTextRenderer::sp_item_invoke_render(SPItem *item)
{
    // Check item's visibility
    if (item->isHidden()) {
        return;
    }

    if (SP_IS_ROOT(item)) {
        return sp_root_render(item);
    } else if (SP_IS_GROUP(item)) {
        return sp_group_render(item);
    } else if (SP_IS_USE(item)) {
        sp_use_render(item);
    } else if (SP_IS_TEXT(item)) {
        return sp_text_render(item);
    } else if (SP_IS_FLOWTEXT(item)) {
        return sp_flowtext_render(item);
    }
    // We are not interested in writing the other SPItem types to LaTeX
}

void
LaTeXTextRenderer::renderItem(SPItem *item)
{
    push_transform(item->transform);
    sp_item_invoke_render(item);
    pop_transform();
}

bool
LaTeXTextRenderer::setupDocument(SPDocument *doc, bool pageBoundingBox, SPItem *base)
{
// The boundingbox calculation here should be exactly the same as the one by CairoRenderer::setupDocument !

    if (!base)
        base = SP_ITEM(sp_document_root(doc));

    Geom::OptRect d;
    if (pageBoundingBox) {
        d = Geom::Rect( Geom::Point(0,0),
                        Geom::Point(sp_document_width(doc), sp_document_height(doc)) );
    } else {
        sp_item_invoke_bbox(base, d, sp_item_i2d_affine(base), TRUE, SPItem::RENDERING_BBOX);
    }
    if (!d) {
        g_message("LaTeXTextRenderer: could not retrieve boundingbox.");
        return false;
    }

    // scale all coordinates, such that the width of the image is 1, this is convenient for scaling the image in LaTeX
    double scale = 1/(d->width());
    double _width = d->width() * scale;
    double _height = d->height() * scale;
    push_transform( Geom::Scale(scale, scale) );

    if (!pageBoundingBox)
    {
        push_transform( Geom::Translate( - d->min() ) );
    }

    // flip y-axis
    push_transform( Geom::Scale(1,-1) * Geom::Translate(0, sp_document_height(doc)) );

    // write the info to LaTeX
    Inkscape::SVGOStringStream os;
    os.setf(std::ios::fixed); // no scientific notation

    // scaling of the image when including it in LaTeX

    os << "  \\ifx\\svgwidth\\undefined\n";
    os << "    \\setlength{\\unitlength}{" << d->width() * PT_PER_PX << "pt}\n";
    os << "  \\else\n";
    os << "    \\setlength{\\unitlength}{\\svgwidth}\n";
    os << "  \\fi\n";
    os << "  \\global\\let\\svgwidth\\undefined\n";
    os << "  \\makeatother\n";

    os << "  \\begin{picture}(" << _width << "," << _height << ")%\n";
    // strip pathname, as it is probably desired. Having a specific path in the TeX file is not convenient.
    os << "    \\put(0,0){\\includegraphics[width=\\unitlength]{" << _filename << "}}%\n";

    fprintf(_stream, "%s", os.str().c_str());

    return true;
}

Geom::Matrix const &
LaTeXTextRenderer::transform()
{
    return _transform_stack.top();
}

void
LaTeXTextRenderer::push_transform(Geom::Matrix const &tr)
{
    if(_transform_stack.size()){
        Geom::Matrix tr_top = _transform_stack.top();
        _transform_stack.push(tr * tr_top);
    } else {
        _transform_stack.push(tr);
    }
}

void
LaTeXTextRenderer::pop_transform()
{
    _transform_stack.pop();
}

}  /* namespace Internal */
}  /* namespace Extension */
}  /* namespace Inkscape */

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
