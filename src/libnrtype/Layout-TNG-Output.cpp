/*
 * Inkscape::Text::Layout - text layout engine output functions
 *
 * Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 * Copyright (C) 2005 Richard Hughes
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#include <glib/gmem.h>
#include "Layout-TNG.h"
#include "display/nr-arena-glyphs.h"
#include "style.h"
#include "print.h"
#include "extension/print.h"
#include "livarot/Path.h"
#include "libnr/nr-matrix-fns.h"
#include "libnr/nr-scale-matrix-ops.h"
#include "libnr/nr-convert2geom.h"
#include "font-instance.h"
#include "svg/svg-length.h"
#include "extension/internal/cairo-render-context.h"
#include "display/curve.h"
#include <2geom/pathvector.h>

namespace Inkscape {
    namespace Extension {
        namespace Internal {
            class CairoRenderContext;
            class CairoGlyphInfo;
        }
    }
}

using Inkscape::Extension::Internal::CairoRenderContext;
using Inkscape::Extension::Internal::CairoGlyphInfo;

namespace Inkscape {
namespace Text {

void Layout::_clearOutputObjects()
{
    _paragraphs.clear();
    _lines.clear();
    _chunks.clear();
    for (std::vector<Span>::iterator it_span = _spans.begin() ; it_span != _spans.end() ; it_span++)
        if (it_span->font) it_span->font->Unref();
    _spans.clear();
    _characters.clear();
    _glyphs.clear();
    _path_fitted = NULL;
}

void Layout::LineHeight::max(LineHeight const &other)
{
    if (other.ascent > ascent)  ascent  = other.ascent;
    if (other.descent > descent) descent = other.descent;
    if (other.leading > leading) leading = other.leading;
}

void Layout::_getGlyphTransformMatrix(int glyph_index, Geom::Matrix *matrix) const
{
    Span const &span = _glyphs[glyph_index].span(this);
    double sin_rotation = sin(_glyphs[glyph_index].rotation);
    double cos_rotation = cos(_glyphs[glyph_index].rotation);
    (*matrix)[0] = span.font_size * cos_rotation;
    (*matrix)[1] = span.font_size * sin_rotation;
    (*matrix)[2] = span.font_size * sin_rotation;
    (*matrix)[3] = -span.font_size * cos_rotation;
    if (span.block_progression == LEFT_TO_RIGHT || span.block_progression == RIGHT_TO_LEFT) {
        (*matrix)[4] = _lines[_chunks[span.in_chunk].in_line].baseline_y + _glyphs[glyph_index].y;
        (*matrix)[5] = _chunks[span.in_chunk].left_x + _glyphs[glyph_index].x;
    } else {
        (*matrix)[4] = _chunks[span.in_chunk].left_x + _glyphs[glyph_index].x;
        (*matrix)[5] = _lines[_chunks[span.in_chunk].in_line].baseline_y + _glyphs[glyph_index].y;
    }
}

void Layout::show(NRArenaGroup *in_arena, NRRect const *paintbox) const
{
    int glyph_index = 0;
    for (unsigned span_index = 0 ; span_index < _spans.size() ; span_index++) {
        if (_input_stream[_spans[span_index].in_input_stream_item]->Type() != TEXT_SOURCE) continue;
        InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_input_stream[_spans[span_index].in_input_stream_item]);
        NRArenaGlyphsGroup *nr_group = NRArenaGlyphsGroup::create(in_arena->arena);
        nr_arena_item_add_child(in_arena, nr_group, NULL);
        nr_arena_item_unref(nr_group);

        nr_arena_glyphs_group_set_style(nr_group, text_source->style);
        while (glyph_index < (int)_glyphs.size() && _characters[_glyphs[glyph_index].in_character].in_span == span_index) {
            if (_characters[_glyphs[glyph_index].in_character].in_glyph != -1) {
                Geom::Matrix glyph_matrix;
                _getGlyphTransformMatrix(glyph_index, &glyph_matrix);
                nr_arena_glyphs_group_add_component(nr_group, _spans[span_index].font, _glyphs[glyph_index].glyph, glyph_matrix);
            }
            glyph_index++;
        }
        nr_arena_glyphs_group_set_paintbox(NR_ARENA_GLYPHS_GROUP(nr_group), paintbox);
    }
    nr_arena_item_request_update(NR_ARENA_ITEM(in_arena), NR_ARENA_ITEM_STATE_ALL, FALSE);
}

void Layout::getBoundingBox(NRRect *bounding_box, Geom::Matrix const &transform, int start, int length) const
{
    for (unsigned glyph_index = 0 ; glyph_index < _glyphs.size() ; glyph_index++) {
        if (_characters[_glyphs[glyph_index].in_character].in_glyph == -1) continue;
        if (start != -1 && (int) _glyphs[glyph_index].in_character < start) continue;
        if (length != -1) {
            if (start == -1)
                start = 0;
            if ((int) _glyphs[glyph_index].in_character > start + length) continue;
        }
        // this could be faster
        Geom::Matrix glyph_matrix;
        _getGlyphTransformMatrix(glyph_index, &glyph_matrix);
        Geom::Matrix total_transform = glyph_matrix;
        total_transform *= transform;
        if(_glyphs[glyph_index].span(this).font) {
	    boost::optional<Geom::Rect> glyph_rect = _glyphs[glyph_index].span(this).font->BBox(_glyphs[glyph_index].glyph);
            if (glyph_rect) {
	        Geom::Point bmi = glyph_rect->min(), bma = glyph_rect->max();
	        Geom::Point tlp(bmi[0],bmi[1]), trp(bma[0],bmi[1]), blp(bmi[0],bma[1]), brp(bma[0],bma[1]);
                tlp *= total_transform;
                trp *= total_transform;
                blp *= total_transform;
                brp *= total_transform;
                *glyph_rect = Geom::Rect(tlp,trp);
                glyph_rect->expandTo(blp);
                glyph_rect->expandTo(brp);
                if ( (glyph_rect->min())[0] < bounding_box->x0 ) bounding_box->x0=(glyph_rect->min())[0];
                if ( (glyph_rect->max())[0] > bounding_box->x1 ) bounding_box->x1=(glyph_rect->max())[0];
                if ( (glyph_rect->min())[1] < bounding_box->y0 ) bounding_box->y0=(glyph_rect->min())[1];
                if ( (glyph_rect->max())[1] > bounding_box->y1 ) bounding_box->y1=(glyph_rect->max())[1];
            }
	}
    }
}

void Layout::print(SPPrintContext *ctx,
                   NRRect const *pbox, NRRect const *dbox, NRRect const *bbox,
                   Geom::Matrix const &ctm) const
{
    if (_input_stream.empty()) return;

    Geom::Matrix ctm_2geom(ctm);
    Direction block_progression = _blockProgression();
    bool text_to_path = ctx->module->textToPath();
    for (unsigned glyph_index = 0 ; glyph_index < _glyphs.size() ; ) {
        if (_characters[_glyphs[glyph_index].in_character].in_glyph == -1) {
            // invisible glyphs
            unsigned same_character = _glyphs[glyph_index].in_character;
            while (_glyphs[glyph_index].in_character == same_character)
                glyph_index++;
            continue;
        }
        Geom::Matrix glyph_matrix;
        Span const &span = _spans[_characters[_glyphs[glyph_index].in_character].in_span];
        InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_input_stream[span.in_input_stream_item]);
        if (text_to_path || _path_fitted) {
            Geom::PathVector const * pv = span.font->PathVector(_glyphs[glyph_index].glyph);
            if (pv) {
                _getGlyphTransformMatrix(glyph_index, &glyph_matrix);
                Geom::PathVector temp_pv = (*pv) * glyph_matrix;
                if (!text_source->style->fill.isNone())
                    sp_print_fill(ctx, temp_pv, &ctm_2geom, text_source->style, pbox, dbox, bbox);
                if (!text_source->style->stroke.isNone())
                    sp_print_stroke(ctx, temp_pv, &ctm_2geom, text_source->style, pbox, dbox, bbox);
            }
            glyph_index++;
        } else {
            Geom::Point g_pos(0,0);    // all strings are output at (0,0) because we do the translation using the matrix
            glyph_matrix = Geom::Scale(1.0, -1.0) * (Geom::Matrix)Geom::Rotate(_glyphs[glyph_index].rotation);
            if (block_progression == LEFT_TO_RIGHT || block_progression == RIGHT_TO_LEFT) {
                glyph_matrix[4] = span.line(this).baseline_y + span.baseline_shift;
                // since we're outputting character codes, not glyphs, we want the character x
                glyph_matrix[5] = span.chunk(this).left_x + span.x_start + _characters[_glyphs[glyph_index].in_character].x;
            } else {
                glyph_matrix[4] = span.chunk(this).left_x + span.x_start + _characters[_glyphs[glyph_index].in_character].x;
                glyph_matrix[5] = span.line(this).baseline_y + span.baseline_shift;
            }
            Glib::ustring::const_iterator span_iter = span.input_stream_first_character;
            unsigned char_index = _glyphs[glyph_index].in_character;
            unsigned original_span = _characters[char_index].in_span;
            while (char_index && _characters[char_index - 1].in_span == original_span) {
                char_index--;
                span_iter++;
            }

            // try to output as many characters as possible in one go by detecting kerning and stopping when we encounter it
            Glib::ustring span_string;
            double char_x = _characters[_glyphs[glyph_index].in_character].x;
            unsigned this_span_index = _characters[_glyphs[glyph_index].in_character].in_span;
            do {
                span_string += *span_iter;
                span_iter++;

                unsigned same_character = _glyphs[glyph_index].in_character;
                while (glyph_index < _glyphs.size() && _glyphs[glyph_index].in_character == same_character) {
                    char_x += _glyphs[glyph_index].width;
                    glyph_index++;
                }
            } while (glyph_index < _glyphs.size()
                     && _path_fitted == NULL
                     && _characters[_glyphs[glyph_index].in_character].in_span == this_span_index
                     && fabs(char_x - _characters[_glyphs[glyph_index].in_character].x) < 1e-5);
            sp_print_bind(ctx, glyph_matrix, 1.0);
            sp_print_text(ctx, span_string.c_str(), g_pos, text_source->style);
            sp_print_release(ctx);
        }
    }
}

#ifdef HAVE_CAIRO_PDF
void Layout::showGlyphs(CairoRenderContext *ctx) const
{
    if (_input_stream.empty()) return;
    
    bool clip_mode = false;//(ctx->getRenderMode() == CairoRenderContext::RENDER_MODE_CLIP);
    std::vector<CairoGlyphInfo> glyphtext;

    for (unsigned glyph_index = 0 ; glyph_index < _glyphs.size() ; ) {
        if (_characters[_glyphs[glyph_index].in_character].in_glyph == -1) {
            // invisible glyphs
            unsigned same_character = _glyphs[glyph_index].in_character;
            while (_glyphs[glyph_index].in_character == same_character)
                glyph_index++;
            continue;
        }
        Span const &span = _spans[_characters[_glyphs[glyph_index].in_character].in_span];
        InputStreamTextSource const *text_source = static_cast<InputStreamTextSource const *>(_input_stream[span.in_input_stream_item]);

        Geom::Matrix glyph_matrix;
        _getGlyphTransformMatrix(glyph_index, &glyph_matrix);
        if (clip_mode) {
            Geom::PathVector const *pathv = span.font->PathVector(_glyphs[glyph_index].glyph);
            if (pathv) {
                Geom::PathVector pathv_trans = (*pathv) * glyph_matrix;
                SPStyle const *style = text_source->style;
                ctx->renderPathVector(pathv_trans, style, NULL);
            }
            glyph_index++;
            continue;
        }

        Geom::Matrix font_matrix;
        if (_path_fitted == NULL) {
            font_matrix = glyph_matrix;
            font_matrix[4] = 0;
            font_matrix[5] = 0;
        } else {
            font_matrix.setIdentity();
        }

        Glib::ustring::const_iterator span_iter = span.input_stream_first_character;
        unsigned char_index = _glyphs[glyph_index].in_character;
        unsigned original_span = _characters[char_index].in_span;
        while (char_index && _characters[char_index - 1].in_span == original_span) {
            char_index--;
            span_iter++;
        }

        // try to output as many characters as possible in one go
        Glib::ustring span_string;
        unsigned this_span_index = _characters[_glyphs[glyph_index].in_character].in_span;
        unsigned int first_index = glyph_index;
        glyphtext.clear();
        do {
            span_string += *span_iter;
            span_iter++;

            unsigned same_character = _glyphs[glyph_index].in_character;
            while (glyph_index < _glyphs.size() && _glyphs[glyph_index].in_character == same_character) {
                if (glyph_index != first_index)
                    _getGlyphTransformMatrix(glyph_index, &glyph_matrix);

                CairoGlyphInfo info;
                info.index = _glyphs[glyph_index].glyph;
                if (_path_fitted == NULL) {
	            info.x = glyph_matrix[4];
                    info.y = glyph_matrix[5];
                } else {
                    info.x = 0;
                    info.y = 0;                    
                }
                glyphtext.push_back(info);

                glyph_index++;
            }
        } while (glyph_index < _glyphs.size()
                 && _path_fitted == NULL
                 && NR::transform_equalp(font_matrix, glyph_matrix, NR_EPSILON)
                 && _characters[_glyphs[glyph_index].in_character].in_span == this_span_index);
         
        // remove vertical flip
        font_matrix[3] *= -1.0;

        SPStyle const *style = text_source->style;
        float opacity = SP_SCALE24_TO_FLOAT(style->opacity.value);
        
        if (_path_fitted) {
            ctx->pushState();
            ctx->transform(&glyph_matrix);
        } else if (opacity != 1.0) {
            ctx->pushState();
            ctx->setStateForStyle(style);
            ctx->pushLayer();
        }
        if (glyph_index - first_index > 0)
            ctx->renderGlyphtext(span.font->pFont, &font_matrix, glyphtext, style);
        if (_path_fitted)
            ctx->popState();
        else if (opacity != 1.0) {
            ctx->popLayer();
            ctx->popState();
        }
    }
}
#endif

// these functions are for dumpAsText() only. No need to translate
static char const *direction_to_text(Layout::Direction d)
{
    switch (d) {
        case Layout::LEFT_TO_RIGHT: return "ltr";
        case Layout::RIGHT_TO_LEFT: return "rtl";
        case Layout::TOP_TO_BOTTOM: return "ttb";
        case Layout::BOTTOM_TO_TOP: return "btt";
    }
    return "???";
}

static char const *style_to_text(PangoStyle s)
{
    switch (s) {
        case PANGO_STYLE_NORMAL: return "upright";
        case PANGO_STYLE_ITALIC: return "italic";
        case PANGO_STYLE_OBLIQUE: return "oblique";
    }
    return "???";
}

static char const *weight_to_text(PangoWeight w)
{
    switch (w) {
        case PANGO_WEIGHT_ULTRALIGHT: return "ultralight";
        case PANGO_WEIGHT_LIGHT     : return "light";
        case PANGO_WEIGHT_SEMIBOLD  : return "semibold";
        case PANGO_WEIGHT_NORMAL    : return "normalweight";
        case PANGO_WEIGHT_BOLD      : return "bold";
        case PANGO_WEIGHT_ULTRABOLD : return "ultrabold";
        case PANGO_WEIGHT_HEAVY     : return "heavy";
    }
    return "???";
}

Glib::ustring Layout::dumpAsText() const
{
    Glib::ustring result;

    for (unsigned span_index = 0 ; span_index < _spans.size() ; span_index++) {
        char line[256];
        snprintf(line, sizeof(line), "==== span %d\n", span_index);
        result += line;
        snprintf(line, sizeof(line), "  in para %d (direction=%s)\n", _lines[_chunks[_spans[span_index].in_chunk].in_line].in_paragraph,
                 direction_to_text(_paragraphs[_lines[_chunks[_spans[span_index].in_chunk].in_line].in_paragraph].base_direction));
        result += line;
        snprintf(line, sizeof(line), "  in source %d (type=%d, cookie=%p)\n", _spans[span_index].in_input_stream_item,
                 _input_stream[_spans[span_index].in_input_stream_item]->Type(),
                 _input_stream[_spans[span_index].in_input_stream_item]->source_cookie);
        result += line;
        snprintf(line, sizeof(line), "  in line %d (baseline=%f, shape=%d)\n", _chunks[_spans[span_index].in_chunk].in_line,
                 _lines[_chunks[_spans[span_index].in_chunk].in_line].baseline_y,
                 _lines[_chunks[_spans[span_index].in_chunk].in_line].in_shape);
        result += line;
        snprintf(line, sizeof(line), "  in chunk %d (x=%f, baselineshift=%f)\n", _spans[span_index].in_chunk, _chunks[_spans[span_index].in_chunk].left_x, _spans[span_index].baseline_shift);
        result += line;
        if (_spans[span_index].font) {
            snprintf(line, sizeof(line), "    font '%s' %f %s %s\n", pango_font_description_get_family(_spans[span_index].font->descr), _spans[span_index].font_size, style_to_text(pango_font_description_get_style(_spans[span_index].font->descr)), weight_to_text(pango_font_description_get_weight(_spans[span_index].font->descr)));
            result += line;
        }
        snprintf(line, sizeof(line), "    x_start = %f, x_end = %f\n", _spans[span_index].x_start, _spans[span_index].x_end);
        result += line;
        snprintf(line, sizeof(line), "    line height: ascent %f, descent %f leading %f\n", _spans[span_index].line_height.ascent, _spans[span_index].line_height.descent, _spans[span_index].line_height.leading);
        result += line;
        snprintf(line, sizeof(line), "    direction %s, block-progression %s\n", direction_to_text(_spans[span_index].direction), direction_to_text(_spans[span_index].block_progression));
        result += line;
        result += "    ** characters:\n";
        Glib::ustring::const_iterator iter_char = _spans[span_index].input_stream_first_character;
        // very inefficent code. what the hell, it's only debug stuff.
        for (unsigned char_index = 0 ; char_index < _characters.size() ; char_index++) {
            if (_characters[char_index].in_span != span_index) continue;
            if (_input_stream[_spans[span_index].in_input_stream_item]->Type() != TEXT_SOURCE) {
                snprintf(line, sizeof(line), "      %d: control x=%f flags=%03x glyph=%d\n", char_index, _characters[char_index].x, *(unsigned*)&_characters[char_index].char_attributes, _characters[char_index].in_glyph);
            } else {
                snprintf(line, sizeof(line), "      %d: '%c' x=%f flags=%03x glyph=%d\n", char_index, *iter_char, _characters[char_index].x, *(unsigned*)&_characters[char_index].char_attributes, _characters[char_index].in_glyph);
                iter_char++;
            }
            result += line;
        }
        result += "    ** glyphs:\n";
        for (unsigned glyph_index = 0 ; glyph_index < _glyphs.size() ; glyph_index++) {
            if (_characters[_glyphs[glyph_index].in_character].in_span != span_index) continue;
            snprintf(line, sizeof(line), "      %d: %d (%f,%f) rot=%f cx=%f char=%d\n", glyph_index, _glyphs[glyph_index].glyph, _glyphs[glyph_index].x, _glyphs[glyph_index].y, _glyphs[glyph_index].rotation, _glyphs[glyph_index].width, _glyphs[glyph_index].in_character);
            result += line;
        }
        result += "\n";
    }
    result += "EOT\n";
    return result;
}

void Layout::fitToPathAlign(SVGLength const &startOffset, Path const &path)
{
    double offset = 0.0;

    if (startOffset._set) {
        if (startOffset.unit == SVGLength::PERCENT)
            offset = startOffset.computed * const_cast<Path&>(path).Length();
        else
            offset = startOffset.computed;
    }

    switch (_paragraphs.front().alignment) {
        case CENTER:
            offset -= _getChunkWidth(0) * 0.5;
            break;
        case RIGHT:
            offset -= _getChunkWidth(0);
            break;
        default:
            break;
    }

    if (_characters.empty()) {
        int unused = 0;
        Path::cut_position *point_otp = const_cast<Path&>(path).CurvilignToPosition(1, &offset, unused);
        if (offset >= 0.0 && point_otp != NULL && point_otp[0].piece >= 0) {
            NR::Point point;
            NR::Point tangent;
            const_cast<Path&>(path).PointAndTangentAt(point_otp[0].piece, point_otp[0].t, point, tangent);
            _empty_cursor_shape.position = point;
            _empty_cursor_shape.rotation = atan2(tangent[Geom::Y], tangent[Geom::X]);
        }
    }

    for (unsigned char_index = 0 ; char_index < _characters.size() ; ) {
        int next_cluster_glyph_index;
        unsigned next_cluster_char_index;
        double character_advance;
        Span const &span = _characters[char_index].span(this);

        for (next_cluster_char_index = char_index + 1 ; next_cluster_char_index < _characters.size() ; next_cluster_char_index++) {
            if (_characters[next_cluster_char_index].in_glyph != -1 && _characters[next_cluster_char_index].char_attributes.is_cursor_position)
                break;
        }

        if (next_cluster_char_index == _characters.size()) {
            next_cluster_glyph_index = _glyphs.size();
            character_advance = 0.0;   // arbitrary because we're not going to advance
        } else {
            next_cluster_glyph_index = _characters[next_cluster_char_index].in_glyph;
            character_advance =   (_glyphs[next_cluster_glyph_index].x + _glyphs[next_cluster_glyph_index].chunk(this).left_x)
                - (_glyphs[_characters[char_index].in_glyph].x + span.chunk(this).left_x);
        }

        double start_offset = offset + span.x_start + _characters[char_index].x;
        double cluster_width = 0.0;
        for (int glyph_index = _characters[char_index].in_glyph ; glyph_index < next_cluster_glyph_index ; glyph_index++)
            cluster_width += _glyphs[glyph_index].width;
        if (span.direction == RIGHT_TO_LEFT)
            start_offset -= cluster_width;
        double end_offset = start_offset + cluster_width;

        int unused = 0;
        double midpoint_offset = (start_offset + end_offset) * 0.5;
        // as far as I know these functions are const, they're just not marked as such
        Path::cut_position *midpoint_otp = const_cast<Path&>(path).CurvilignToPosition(1, &midpoint_offset, unused);
        if (midpoint_offset >= 0.0 && midpoint_otp != NULL && midpoint_otp[0].piece >= 0) {
            NR::Point midpoint;
            NR::Point tangent;
            const_cast<Path&>(path).PointAndTangentAt(midpoint_otp[0].piece, midpoint_otp[0].t, midpoint, tangent);

            if (start_offset >= 0.0 && end_offset >= 0.0) {
                Path::cut_position *start_otp = const_cast<Path&>(path).CurvilignToPosition(1, &start_offset, unused);
                if (start_otp != NULL && start_otp[0].piece >= 0) {
                    Path::cut_position *end_otp = const_cast<Path&>(path).CurvilignToPosition(1, &end_offset, unused);
                    if (end_otp != NULL && end_otp[0].piece >= 0) {
                        bool on_same_subpath = true;
                        for (size_t i = 0 ; i < path.pts.size() ; i++) {
                            if (path.pts[i].piece <= start_otp[0].piece) continue;
                            if (path.pts[i].piece >= end_otp[0].piece) break;
                            if (path.pts[i].isMoveTo == polyline_moveto) {
                                on_same_subpath = false;
                                break;
                            }
                        }
                        if (on_same_subpath) {
                            // both points were on the same subpath (without this test the angle is very weird)
                            NR::Point startpoint, endpoint;
                            const_cast<Path&>(path).PointAt(start_otp[0].piece, start_otp[0].t, startpoint);
                            const_cast<Path&>(path).PointAt(end_otp[0].piece, end_otp[0].t, endpoint);
                            if (endpoint != startpoint) {
                                tangent = endpoint - startpoint;
                                tangent.normalize();
                            } else {
                                tangent = Geom::Point (0,0);
                            }
                        }
                        g_free(end_otp);
                    }
                    g_free(start_otp);
                }
            }

            double rotation = atan2(tangent[1], tangent[0]);
            for (int glyph_index = _characters[char_index].in_glyph ; glyph_index < next_cluster_glyph_index ; glyph_index++) {
                double tangent_shift = -cluster_width * 0.5 + _glyphs[glyph_index].x - (_characters[char_index].x + span.x_start);
                double normal_shift = _glyphs[glyph_index].y;
                if (span.direction == RIGHT_TO_LEFT)
                    tangent_shift += cluster_width;
                _glyphs[glyph_index].x = midpoint[0] - span.chunk(this).left_x + tangent[0] * tangent_shift - tangent[1] * normal_shift;
                _glyphs[glyph_index].y = midpoint[1] - _lines.front().baseline_y + tangent[1] * tangent_shift + tangent[0] * normal_shift;
                _glyphs[glyph_index].rotation += rotation;
            }
        } else {  // outside the bounds of the path: hide the glyphs
            _characters[char_index].in_glyph = -1;
        }
        g_free(midpoint_otp);

        char_index = next_cluster_char_index;
    }

    for (unsigned span_index = 0 ; span_index < _spans.size() ; span_index++) {
        _spans[span_index].x_start += offset;
        _spans[span_index].x_end += offset;
    }

    _path_fitted = &path;
}

SPCurve *Layout::convertToCurves(iterator const &from_glyph, iterator const &to_glyph) const
{
    GSList *cc = NULL;

    for (int glyph_index = from_glyph._glyph_index ; glyph_index < to_glyph._glyph_index ; glyph_index++) {
        Geom::Matrix glyph_matrix;
        Span const &span = _glyphs[glyph_index].span(this);
        _getGlyphTransformMatrix(glyph_index, &glyph_matrix);

        Geom::PathVector const * pathv = span.font->PathVector(_glyphs[glyph_index].glyph);
        if (pathv) {
            Geom::PathVector pathv_trans = (*pathv) * glyph_matrix;
            SPCurve *c = new SPCurve(pathv_trans);
            if (c) cc = g_slist_prepend(cc, c);
        }
    }
    cc = g_slist_reverse(cc);

    SPCurve *curve;
    if ( cc ) {
        curve = SPCurve::concat(cc);
    } else {
        curve = new SPCurve();
    }

    while (cc) {
        /* fixme: This is dangerous, as we are mixing art_alloc and g_new */
        reinterpret_cast<SPCurve *>(cc->data)->unref();
        cc = g_slist_remove(cc, cc->data);
    }

    return curve;
}

void Layout::transform(Geom::Matrix const &transform)
{
    // this is all massively oversimplified
    // I can't actually think of anybody who'll want to use it at the moment, so it'll stay simple
    for (unsigned glyph_index = 0 ; glyph_index < _glyphs.size() ; glyph_index++) {
        Geom::Point point(_glyphs[glyph_index].x, _glyphs[glyph_index].y);
        point *= transform;
        _glyphs[glyph_index].x = point[0];
        _glyphs[glyph_index].y = point[1];
    }
}

}//namespace Text
}//namespace Inkscape


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
