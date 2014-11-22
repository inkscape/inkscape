/*
 * Inkscape::Text::Layout - text layout engine
 *
 * Authors:
 *   Richard Hughes <cyreve@users.sf.net>
 *
 * Copyright (C) 2005 Richard Hughes
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */
#ifndef __LAYOUT_TNG_H__
#define __LAYOUT_TNG_H__

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <2geom/d2.h>
#include <2geom/affine.h>
#include <glibmm/ustring.h>
#include <pango/pango-break.h>
#include <algorithm>
#include <vector>
#include <boost/optional.hpp>

#ifdef HAVE_CAIRO_PDF
namespace Inkscape {
        namespace Extension {
                namespace Internal {
                        class CairoRenderContext;
                }
        }
}

using Inkscape::Extension::Internal::CairoRenderContext;
#endif

class SPStyle;
class Shape;
struct SPPrintContext;
class SVGLength;
class Path;
class SPCurve;
class font_instance;
typedef struct _PangoFontDescription PangoFontDescription;

namespace Inkscape {
class DrawingGroup;

namespace Text {

/** \brief Generates the layout for either wrapped or non-wrapped text and stores the result

Use this class for all your text output needs. It takes text with formatting
markup as input and turns that into the glyphs and their necessary positions.
It stores the glyphs internally, but maintains enough information to both
retrieve your own rendering information if you wish and to perform visual
text editing where the output refers back to where it came from.

Usage:
-# Construct
-# Set the text using appendText() and appendControlCode()
-# If you want text wrapping, call appendWrapShape() a few times
-# Call calculateFlow()
-# You can go several directions from here, but the most interesting
   things start with creating a Layout::iterator with begin() or end().

Terminology, in descending order of size:
- Flow: Not often used, but when it is it means all the text
- Shape: A Shape object which is used to represent one of the regions inside
  which to flow the text. Can overlap with...
- Paragraph: Err...A paragraph. Contains one or more...
- Line: An entire horizontal line with a common baseline. Contains one or
  more...
- Chunk: You only get more than one of these when a shape is sufficiently
  complex that the text has to flow either side of some obstruction in
  the middle. A chunk is the base unit for wrapping. Contains one or more...
- Span: A convenient subset of a chunk with the same font, style,
  directionality, block progression and input stream. Fill and outline
  need not be constant because that's a later rendering stage.
- This is where it gets weird because a span will contain one or more 
  elements of both of the following, which can overlap with each other in
  any way:
  - Character: a single Unicode codepoint from an input stream. Many arabic
    characters contain multiple glyphs
  - Glyph: a rendering primitive for font engines. A ligature glyph will
    represent multiple characters.

Other terminology:
- Input stream: An object representing a single call to appendText() or
  appendControlCode().
- Control code: Metadata in the text stream to signify items that occupy
  real space (unlike style changes) but don't belong in the text string.
  Paragraph breaks are in this category. See Layout::TextControlCode.
- SVG1.1: The W3C Recommendation "Scalable Vector Graphics (SVG) 1.1"
  http://www.w3.org/TR/SVG11/
- 'left', 'down', etc: These terms are generally used to mean what they
  mean in left-to-right, top-to-bottom text but rotated or reflected for
  the current directionality. Thus, the 'width' of a ttb line is actually
  its height, and the (internally stored) y coordinate of a glyph is
  actually its x coordinate. Confusing to the reader but much simpler in
  the code. All public methods use real x and y.

Comments:
- There's a strong emphasis on international support in this class, but
  that's primarily because once you can display all the insane things
  required by various languages, simple things like styling text are
  almost trivial.
- There are a few places (appendText() is one) where pointers are held to
  caller-owned objects and used for quite a long time. This is messy but
  is safe for our usage scenario and in many cases the cost of copying the
  objects is quite high.
- "Why isn't foo here?": Ask yourself if it's possible to implement foo
  externally using iterators. However this may not mean that it doesn't
  belong as a member, though.
- I've used floats rather than doubles to store relative distances in some
  places (internal only) where it would save significant amounts of memory.
  The SVG spec allows you to do this as long as intermediate calculations
  are done double. Very very long lines might not finish precisely where
  you want, but that's to be expected with any typesetting. Also,
  SVGLength only uses floats.
- If you look at the six arrays for holding the output data you'll realise
  that there's no O(1) way to drill down from a paragraph to find its
  starting glyph. This was a conscious decision to reduce complexity and
  to save memory. Drilling down isn't actually that slow because a binary
  chop will work nicely. Add this to the realisation that most of the
  times you do this will be in response to user actions and hence you only
  need to be faster than the user and I think the design makes sense.
- There are a massive number of functions acting on Layout::iterator. A
  large number are trivial and will be inline, but is it really necessary
  to have all these, especially when some can be implemented by the caller
  using the others?
- The separation of methods between Layout and Layout::iterator is a
  bit arbitrary, because many methods could go in either. I've used the STL
  model where the iterator itself can only move around; the base class is
  required to do anything interesting.
- I use Pango internally, not Pangomm. The reason for this is lots of
  Pangomm methods take Glib::ustrings as input and then output byte offsets
  within the strings. There's simply no way to use byte offsets with
  ustrings without some very entertaining reinterpret_cast<>s. The Pangomm
  docs seem to be lacking quite a lot of things mentioned in the Pango
  docs, too.
*/
class Layout {
public:
    class iterator;
    friend class iterator;
    class Calculator;
    friend class Calculator;
    class ScanlineMaker;
    class InfiniteScanlineMaker;
    class ShapeScanlineMaker;

    Layout();
    virtual ~Layout();

    /** Used to specify any particular text direction required. Used for
    both the 'direction' and 'block-progression' CSS attributes. */
    enum Direction {LEFT_TO_RIGHT, RIGHT_TO_LEFT, TOP_TO_BOTTOM, BOTTOM_TO_TOP};

    /** Display alignment for shapes. See appendWrapShape(). */
    enum DisplayAlign {DISPLAY_ALIGN_BEFORE, DISPLAY_ALIGN_CENTER, DISPLAY_ALIGN_AFTER};

    /** The optional attributes which can be applied to a SVG text or
    related tag. See appendText(). See SVG1.1 section 10.4 for the
    definitions of all these members. See sp_svg_length_list_read() for
    the standard way to make these vectors. It is the responsibility of
    the caller to deal with the inheritance of these values using its
    knowledge of the parse tree. */
    struct OptionalTextTagAttrs {
        std::vector<SVGLength> x;
        std::vector<SVGLength> y;
        std::vector<SVGLength> dx;
        std::vector<SVGLength> dy;
        std::vector<SVGLength> rotate;
    };

    /** Control codes which can be embedded in the text to be flowed. See
    appendControlCode(). */
    enum TextControlCode {
        PARAGRAPH_BREAK,    /// forces the flow to move on to the next line
        SHAPE_BREAK,        /// forces the flow to ignore the remainder of the current shape (from #flow_inside_shapes) and continue at the top of the one after.
        ARBITRARY_GAP       /// inserts an arbitrarily-sized hole in the flow in line with the current text.
    };

    /** For expressing paragraph alignment. These values are rotated in the
    case of vertical text, but are not dependent on whether the paragraph is
    rtl or ltr, thus LEFT is always either left or top. */
    enum Alignment {LEFT, CENTER, RIGHT, FULL};

    /** The CSS spec allows line-height:normal to be whatever the user agent
    thinks will look good. This is our value, as a multiple of font-size. */
    static const double LINE_HEIGHT_NORMAL;

    // ************************** describing the stuff to flow *************************

    /** \name Input
      Methods for describing the text you want to flow, its style, and the
      shapes to flow in to.
    */
    //@{

    /** Empties everything stored in this class and resets it to its
    original state, like when it was created. All iterators on this
    object will be invalidated (but can be revalidated using
    validateIterator(). */
    void clear();

    /** Queries whether any calls have been made to appendText() or
    appendControlCode() since the object was last cleared. */
    bool inputExists() const
        {return !_input_stream.empty();}

    bool _input_truncated;
    bool inputTruncated() const
        {return _input_truncated;}

    /** adds a new piece of text to the end of the current list of text to
    be processed. This method can only add text of a consistent style.
    To add lots of different styles, call it lots of times.
     \param text  The text. \b Note: only a \em pointer is stored. Do not
                  mess with the text until after you have called
                  calculateFlow().
     \param style The font style. Layout will hold a reference to this
                  object for the duration of its ownership, ie until you
                  call clear() or the class is destroyed. Must not be NULL.
     \param source_cookie  This pointer is treated as opaque by Layout
                  but will be passed through the flowing process intact so
                  that callers can use it to refer to the original object
                  that generated a particular glyph. See Layout::iterator.
                  Implementation detail: currently all callers put an
                  SPString in here.
     \param optional_attributes  A structure containing additional options
                  for this text. See OptionalTextTagAttrs. The values are
                  copied to internal storage before this method returns.
     \param optional_attributes_offset  It is convenient for callers to be
                  able to use the same \a optional_attributes structure for
                  several sequential text fields, in which case the vectors
                  will need to be offset. This parameter causes the <i>n</i>th
                  element of all the vectors to be read as if it were the
                  first.
     \param text_begin  Used for selecting only a substring of \a text
                  to process.
     \param text_end    Used for selecting only a substring of \a text
                  to process.
    */
    void appendText(Glib::ustring const &text, SPStyle *style, void *source_cookie, OptionalTextTagAttrs const *optional_attributes, unsigned optional_attributes_offset, Glib::ustring::const_iterator text_begin, Glib::ustring::const_iterator text_end);
    inline void appendText(Glib::ustring const &text, SPStyle *style, void *source_cookie, OptionalTextTagAttrs const *optional_attributes = NULL, unsigned optional_attributes_offset = 0)
        {appendText(text, style, source_cookie, optional_attributes, optional_attributes_offset, text.begin(), text.end());}

    /** Control codes are metadata in the text stream to signify items
    that occupy real space (unlike style changes) but don't belong in the
    text string. See TextControlCode for the types available.

    A control code \em cannot be the first item in the input stream. Use
    appendText() with an empty string to set up the paragraph properties.
     \param code    A member of the TextFlowControlCode enumeration.
     \param width   The width in pixels that this item occupies.
     \param ascent  The number of pixels above the text baseline that this
                    control code occupies.
     \param descent The number of pixels below the text baseline that this
                    control code occupies.
     \param source_cookie This pointer is treated as opaque by Layout
                  but will be passed through the flowing process intact so
                  that callers can use it to refer to the original object
                  that generated a particular area. See Layout::iterator.
                  Implementation detail: currently all callers put an
                  SPObject in here.
    Note that for some control codes (eg tab) the values of the \a width,
    \a ascender and \a descender are implied by the surrounding text (and
    in the case of tabs, the values set in tab_stops) so the values you pass
    here are ignored.
    */
    void appendControlCode(TextControlCode code, void *source_cookie, double width = 0.0, double ascent = 0.0, double descent = 0.0);

    /** Stores another shape inside which to flow the text. If this method
    is never called then no automatic wrapping is done and lines will
    continue to infinity if necessary. Text can be flowed inside multiple
    shapes in sequence, like with frames in a DTP package. If the text flows
    past the end of the last shape all remaining text is ignored.

      \param shape  The Shape to use next in the flow. The storage for this
                    is managed by the caller, and need only be valid for
                    the duration of the call to calculateFlow().
      \param display_align   The vertical alignment of the text within this
                    shape. See XSL1.0 section 7.13.4. The behaviour of
                    settings other than DISPLAY_ALIGN_BEFORE when using
                    non-rectangular shapes is undefined.
    */
    void appendWrapShape(Shape const *shape, DisplayAlign display_align = DISPLAY_ALIGN_BEFORE);

    //@}

    // ************************** doing the actual flowing *************************

    /** \name Processing
      The method to do the actual work of converting text into glyphs.
    */
    //@{

    /** Takes all the stuff you set with the members above here and creates
    a load of glyphs for use with the members below here. All iterators on
    this object will be invalidated (but can be fixed with validateIterator().
    The implementation just creates a new Layout::Calculator and calls its
    Calculator::Calculate() method, so if you want more details on the
    internals, go there.
      \return  false on failure.
    */
    bool calculateFlow();

    //@}

    // ************************** operating on the output glyphs *************************

    /** \name Output
      Methods for reading and interpreting the output glyphs. See also
      Layout::iterator.
    */
    //@{

    /** Returns true if there are some glyphs in this object, ie whether
    computeFlow() has been called on a non-empty input since the object was
    created or the last call to clear(). */
    inline bool outputExists() const
        {return !_characters.empty();}

    /** Adds all the output glyphs to \a in_arena using the given \a paintbox.
     \param in_arena  The arena to add the glyphs group to
     \param paintbox  The current rendering tile
    */
    void show(DrawingGroup *in_arena, Geom::OptRect const &paintbox) const;

    /** Calculates the smallest rectangle completely enclosing all the
    glyphs.
      \param bounding_box  Where to store the box
      \param transform     The transform to be applied to the entire object
                           prior to calculating its bounds.
    */
    Geom::OptRect bounds(Geom::Affine const &transform, int start = -1, int length = -1) const;

    /** Sends all the glyphs to the given print context.
     \param ctx   I have
     \param pbox  no idea
     \param dbox  what these
     \param bbox  parameters
     \param ctm   do yet
    */
    void print(SPPrintContext *ctx, Geom::OptRect const &pbox, Geom::OptRect const &dbox, Geom::OptRect const &bbox, Geom::Affine const &ctm) const;

#ifdef HAVE_CAIRO_PDF    
    /** Renders all the glyphs to the given Cairo rendering context.
     \param ctx   The Cairo rendering context to be used
     */
    void showGlyphs(CairoRenderContext *ctx) const;
#endif

    /** Returns the font family of the indexed span */
    Glib::ustring getFontFamily(unsigned span_index) const;

    /** debug and unit test method. Creates a textual representation of the
    contents of this object. The output is designed to be both human-readable
    and comprehensible when diffed with a known-good dump. */
    Glib::ustring dumpAsText() const;

    /** Moves all the glyphs in the structure so that the baseline of all
    the characters sits neatly along the path specified. If the text has
    more than one line the results are undefined. The 'align' means to
    use the SVG align method as documented in SVG1.1 section 10.13.2.
    NB: njh has suggested that it would be cool if we could flow from
    shape to path and back again. This is possible, so this method will be
    removed at some point.
    A pointer to \a path is retained by the class for use by the cursor
    positioning functions. */
    void fitToPathAlign(SVGLength const &startOffset, Path const &path);

    /** Convert the specified range of characters into their bezier 
    outlines.
    */
    SPCurve* convertToCurves(iterator const &from_glyph, iterator const &to_glyph) const;
    inline SPCurve* convertToCurves() const;

    /** Apply the given transform to all the output presently stored in
    this object. This only transforms the glyph positions, The glyphs
    themselves will not be transformed. */
    void transform(Geom::Affine const &transform);

    //@}

    // **********

    /** \name Output (Iterators)
      Methods for operating with the Layout::iterator class. The method
      names ending with 'Index' return 0-based offsets of the number of
      items since the beginning of the flow.
    */
    //@{

    /** Returns an iterator pointing at the first glyph of the flowed output.
    The first glyph is also the first character, line, paragraph, etc. */
    inline iterator begin() const;

    /** Returns an iterator pointing just past the end of the last glyph,
    which is also just past the end of the last chunk, span, etc, etc. */
    inline iterator end() const;

    /** Returns an iterator pointing at the given character index. This
    index should be related to the result from a prior call to
    iteratorToCharIndex(). */
    inline iterator charIndexToIterator(int char_index) const;

    /** Returns the character index from the start of the flow represented
    by the given iterator. This number isn't very useful, except for when
    editing text it will stay valid across calls to computeFlow() and will
    change in predictable ways when characters are added and removed. It's
    also useful when transitioning old code. */
    inline int iteratorToCharIndex(iterator const &it) const;

    /** Checks the validity of the given iterator over the current layout.
    If it points to a position out of the bounds for this layout it will
    be corrected to the nearest valid position. If you pass an iterator
    belonging to a different layout it will be converted to one for this
    layout. */
    inline void validateIterator(iterator *it) const;

    /** Returns an iterator pointing to the cursor position for a mouse
    click at the given coordinates. */
    iterator getNearestCursorPositionTo(double x, double y) const;
    inline iterator getNearestCursorPositionTo(Geom::Point const &point) const;

    /** Returns an iterator pointing to the letter whose bounding box contains
    the given coordinates. end() if the point is not over any letter. The
    iterator will \em not point at the specific glyph within the character. */
    iterator getLetterAt(double x, double y) const;
    inline iterator getLetterAt(Geom::Point &point) const;

    /* Returns an iterator pointing to the character in the output which
    was created from the given input. If the character at the given byte
    offset was removed (soft hyphens, for example) the next character after
    it is returned. If no input was added with the given cookie, end() is
    returned. If more than one input has the same cookie, the first will
    be used regardless of the value of \a text_iterator. If
    \a text_iterator is out of bounds, the first or last character belonging
    to the given input will be returned accordingly.
    iterator sourceToIterator(void *source_cookie, Glib::ustring::const_iterator text_iterator) const;
 */
 
    /** Returns an iterator pointing to the first character in the output
    which was created from the given source. If \a source_cookie is invalid,
    end() is returned. If more than one input has the same cookie, the
    first one will be used. */
    iterator sourceToIterator(void *source_cookie) const;

    // many functions acting on iterators, most of which are obvious
    // also most of them don't check that \a it != end(). Be careful.

    /** Returns the bounding box of the given glyph, and its rotation.
    The centre of rotation is the horizontal centre of the box at the
    text baseline. */
    Geom::OptRect glyphBoundingBox(iterator const &it, double *rotation) const;

    /** Returns the zero-based line number of the character pointed to by
    \a it. */
    inline unsigned lineIndex(iterator const &it) const;

    /** Returns the zero-based number of the shape which contains the
    character pointed to by \a it. */
    inline unsigned shapeIndex(iterator const &it) const;

    /** Returns true if the character at \a it is a whitespace, as defined
    by Pango. This is not meant to be used for picking out words from the
    output, use iterator::nextStartOfWord() and friends instead. */
    inline bool isWhitespace(iterator const &it) const;

    /** Returns the unicode character code of the character pointed to by
    \a it. If \a it == end() the result is undefined. */
    inline int characterAt(iterator const &it) const;

    /** Discovers where the character pointed to by \a it came from, by
    retrieving the cookie that was passed to the call to appendText() or
    appendControlCode() which generated that output. If \a it == end()
    then NULL is returned as the cookie. If the character was generated
    from a call to appendText() then the optional \a text_iterator
    parameter is set to point to the actual character, otherwise
    \a text_iterator is unaltered. */
    // TODO FIXME a void* cookie is a very unsafe design, and C++ makes it unnecessary.  
    void getSourceOfCharacter(iterator const &it, void **source_cookie, Glib::ustring::iterator *text_iterator = NULL) const;

    /** For latin text, the left side of the character, on the baseline */
    Geom::Point characterAnchorPoint(iterator const &it) const;

    /** For left aligned text, the leftmost end of the baseline
    For rightmost text, the rightmost... you probably got it by now ;-)*/
    boost::optional<Geom::Point> baselineAnchorPoint() const;

    Geom::Path baseline() const;

    /** This is that value to apply to the x,y attributes of tspan role=line
    elements, and hence it takes alignment into account. */
    Geom::Point chunkAnchorPoint(iterator const &it) const;

    /** Returns the box extents (not ink extents) of the given character.
    The centre of rotation is at the horizontal centre of the box on the
    text baseline. */
    Geom::Rect characterBoundingBox(iterator const &it, double *rotation = NULL) const;

    /** Basically uses characterBoundingBox() on all the characters from
    \a start to \a end and returns the union of these boxes. The return value
    is a list of zero or more quadrilaterals specified by a group of four
    points for each, thus size() is always a multiple of four. */
    std::vector<Geom::Point> createSelectionShape(iterator const &it_start, iterator const &it_end, Geom::Affine const &transform) const;

    /** Returns true if \a it points to a character which is a valid cursor
    position, as defined by Pango. */
    inline bool isCursorPosition(iterator const &it) const;

    /** Gets the ideal cursor shape for a given iterator. The result is
    undefined if \a it is not at a valid cursor position.
      \param it        The location in the output
      \param position  The pixel location of the centre of the 'bottom' of
                       the cursor.
      \param height    The height in pixels of the surrounding text
      \param rotation  The angle to draw from \a position. Radians, zero up,
                       increasing clockwise.
    */
    void queryCursorShape(iterator const &it, Geom::Point &position, double &height, double &rotation) const;

    /** Returns true if \a it points to a character which is a the start of
    a word, as defined by Pango. */
    inline bool isStartOfWord(iterator const &it) const;

    /** Returns true if \a it points to a character which is a the end of
    a word, as defined by Pango. */
    inline bool isEndOfWord(iterator const &it) const;

    /** Returns true if \a it points to a character which is a the start of
    a sentence, as defined by Pango. */
    inline bool isStartOfSentence(iterator const &it) const;

    /** Returns true if \a it points to a character which is a the end of
    a sentence, as defined by Pango. */
    inline bool isEndOfSentence(iterator const &it) const;

    /** Returns the zero-based number of the paragraph containing the
    character pointed to by \a it. */
    inline unsigned paragraphIndex(iterator const &it) const;

    /** Returns the actual alignment used for the paragraph containing
    the character pointed to by \a it. This means that the CSS 'start'
    and 'end' are correctly translated into LEFT or RIGHT according to
    the paragraph's directionality. For vertical text, LEFT is top
    alignment and RIGHT is bottom. */
    inline Alignment paragraphAlignment(iterator const &it) const;

    /** Returns kerning information which could cause the current output
    to be exactly reproduced if the letter and word spacings were zero and
    full justification was not used. The x and y arrays are not used, but
    they are cleared. The dx applied to the first character in a chunk
    will always be zero. If the region between \a from and \a to crosses
    a line break then the results may be surprising, and are undefined.
    Trailing zeros on the returned arrays will be trimmed. */
    void simulateLayoutUsingKerning(iterator const &from, iterator const &to, OptionalTextTagAttrs *result) const;

    //@}

    /// it's useful for this to be public so that ScanlineMaker can use it
    struct LineHeight {
        double ascent;
        double descent;
        double leading;
        inline double total() const {return ascent + descent + leading;}
        inline void setZero() {ascent = descent = leading = 0.0;}
        inline LineHeight& operator*=(double x) {ascent *= x; descent *= x; leading *= x; return *this;}
        void max(LineHeight const &other);   /// makes this object contain the largest of all three members between this object and other
        inline double getAscent() const {return ascent; }
        inline double getDescent() const {return descent; }
        inline double getLeading() const {return leading; }
    };

    /// see _enum_converter()
    struct EnumConversionItem {
        int input, output;
    };

private:
    /** Erases all the stuff set by the owner as input, ie #_input_stream
    and #_input_wrap_shapes. */
    void _clearInputObjects();

    /** Erases all the stuff output by computeFlow(). Glyphs and things. */
    void _clearOutputObjects();

    static const gunichar UNICODE_SOFT_HYPHEN;

    // ******************* input flow

    enum InputStreamItemType {TEXT_SOURCE, CONTROL_CODE};

    class InputStreamItem {
    public:
        virtual ~InputStreamItem() {}
        virtual InputStreamItemType Type() =0;
        void *source_cookie;
    };

    /** Represents a text item in the input stream. See #_input_stream.
    Most of the members are copies of the values passed to appendText(). */
    class InputStreamTextSource : public InputStreamItem {
    public:
        virtual InputStreamItemType Type() {return TEXT_SOURCE;}
        virtual ~InputStreamTextSource();
        Glib::ustring const *text;    /// owned by the caller
        Glib::ustring::const_iterator text_begin, text_end;
        int text_length;    /// in characters, from text_start to text_end only
        SPStyle *style;
        /** These vectors can (often will) be shorter than the text
        in this source, but never longer. */
        std::vector<SVGLength> x;
        std::vector<SVGLength> y;
        std::vector<SVGLength> dx;
        std::vector<SVGLength> dy;
        std::vector<SVGLength> rotate;
        
        // a few functions for some of the more complicated style accesses
        float styleComputeFontSize() const;
        /// The return value must be freed with pango_font_description_free()
        PangoFontDescription *styleGetFontDescription() const;
        font_instance *styleGetFontInstance() const;
        Direction styleGetBlockProgression() const;
        Alignment styleGetAlignment(Direction para_direction, bool try_text_align) const;
    };

    /** Represents a control code item in the input stream. See
    #_input_streams. All the members are copies of the values passed to
    appendControlCode(). */
    class InputStreamControlCode : public InputStreamItem {
    public:
        virtual InputStreamItemType Type() {return CONTROL_CODE;}
        TextControlCode code;
        double ascent;
        double descent;
        double width;
    };

    /** This is our internal storage for all the stuff passed to the
    appendText() and appendControlCode() functions. */
    std::vector<InputStreamItem*> _input_stream;

    /** The parameters to appendText() are allowed to be a little bit
    complex. This copies them to be the right length and starting at zero.
    We also don't want to write five bits of identical code just with
    different variable names. */
    static void _copyInputVector(std::vector<SVGLength> const &input_vector, unsigned input_offset, std::vector<SVGLength> *output_vector, size_t max_length);

    /** There are a few cases where we have different sets of enums meaning
    the same thing, eg Pango font styles vs. SPStyle font styles. These need
    converting. */
    static int _enum_converter(int input, EnumConversionItem const *conversion_table, unsigned conversion_table_size);

    /** The overall block-progression of the whole flow. */
    inline Direction _blockProgression() const
        {return static_cast<InputStreamTextSource*>(_input_stream.front())->styleGetBlockProgression();}

    /** so that LEFT_TO_RIGHT == RIGHT_TO_LEFT but != TOP_TO_BOTTOM */
    static bool _directions_are_orthogonal(Direction d1, Direction d2);

    /** If the output is empty callers still want to be able to call
    queryCursorShape() and get a valid answer so, while #_input_wrap_shapes
    can still be considered valid, we need to precompute the cursor shape
    for this case. */
    void _calculateCursorShapeForEmpty();

    struct CursorShape {
        Geom::Point position;
        double height;
        double rotation;
    } _empty_cursor_shape;

    // ******************* input shapes

    struct InputWrapShape {
        Shape const *shape;        /// as passed to Layout::appendWrapShape()
        DisplayAlign display_align;   /// as passed to Layout::appendWrapShape()
    };
    std::vector<InputWrapShape> _input_wrap_shapes;

    // ******************* output

    /** as passed to fitToPathAlign() */
    Path const *_path_fitted;

    struct Glyph;
    struct Character;
    struct Span;
    struct Chunk;
    struct Line;
    struct Paragraph;

    struct Glyph {
        int glyph;
        unsigned in_character;
        float x;         /// relative to the start of the chunk
        float y;         /// relative to the current line's baseline
        float rotation;  /// absolute, modulo any object transforms, which we don't know about
        float width;
        inline Span const & span(Layout const *l) const {return l->_spans[l->_characters[in_character].in_span];}
        inline Chunk const & chunk(Layout const *l) const {return l->_chunks[l->_spans[l->_characters[in_character].in_span].in_chunk];}
        inline Line const & line(Layout const *l) const {return l->_lines[l->_chunks[l->_spans[l->_characters[in_character].in_span].in_chunk].in_line];}
    };
    struct Character {
        unsigned in_span;
        float x;      /// relative to the start of the *span* (so we can do block-progression)
        PangoLogAttr char_attributes;
        int in_glyph;   /// will be -1 if this character has no visual representation
        inline Span const & span(Layout const *l) const {return l->_spans[in_span];}
        inline Chunk const & chunk(Layout const *l) const {return l->_chunks[l->_spans[in_span].in_chunk];}
        inline Line const & line(Layout const *l) const {return l->_lines[l->_chunks[l->_spans[in_span].in_chunk].in_line];}
        inline Paragraph const & paragraph(Layout const *l) const {return l->_paragraphs[l->_lines[l->_chunks[l->_spans[in_span].in_chunk].in_line].in_paragraph];}
        // to get the advance width of a character, subtract the x values if it's in the middle of a span, or use span.x_end if it's at the end
    };
    struct Span {
        unsigned in_chunk;
        font_instance *font;
        float font_size;
        float x_start;   /// relative to the start of the chunk
        float x_end;     /// relative to the start of the chunk
        inline float width() const {return std::abs(x_start - x_end);}
        LineHeight line_height;
        double baseline_shift;  /// relative to the line's baseline
        Direction direction;     /// See CSS3 section 3.2. Either rtl or ltr
        Direction block_progression;  /// See CSS3 section 3.2. The direction in which lines go.
        unsigned in_input_stream_item;
        Glib::ustring::const_iterator input_stream_first_character;
        inline Chunk const & chunk(Layout const *l) const {return l->_chunks[in_chunk];}
        inline Line const & line(Layout const *l) const {return l->_lines[l->_chunks[in_chunk].in_line];}
        inline Paragraph const & paragraph(Layout const *l) const {return l->_paragraphs[l->_lines[l->_chunks[in_chunk].in_line].in_paragraph];}
    };
    struct Chunk {
        unsigned in_line;
        double left_x;
    };
    struct Line {
        unsigned in_paragraph;
        double baseline_y;
        unsigned in_shape;
    };
    struct Paragraph {
        Direction base_direction;    /// can be overridden by child Span objects
        Alignment alignment;
    };
    std::vector<Paragraph> _paragraphs;
    std::vector<Line> _lines;
    std::vector<Chunk> _chunks;
    std::vector<Span> _spans;
    std::vector<Character> _characters;
    std::vector<Glyph> _glyphs;

    /** gets the overall matrix that transforms the given glyph from local
    space to world space. */
    void _getGlyphTransformMatrix(int glyph_index, Geom::Affine *matrix) const;

    // loads of functions to drill down the object tree, all of them
    // annoyingly similar and all of them requiring predicate functors.
    // I'll be buggered if I can find a way to make it work with
    // functions or with a templated functor, so macros it is.
#define EMIT_PREDICATE(name, object_type, index_generator)                  \
    class name {                                                            \
        Layout const * const _flow;                                         \
    public:                                                                 \
        inline name(Layout const *flow) : _flow(flow) {}                    \
        inline bool operator()(object_type const &object, unsigned index)   \
            {return index_generator < index;}                               \
    }
// end of macro
    EMIT_PREDICATE(PredicateLineToSpan,        Span,      _flow->_chunks[object.in_chunk].in_line);
    EMIT_PREDICATE(PredicateLineToCharacter,   Character, _flow->_chunks[_flow->_spans[object.in_span].in_chunk].in_line);
    EMIT_PREDICATE(PredicateSpanToCharacter,   Character, object.in_span);
    EMIT_PREDICATE(PredicateSourceToCharacter, Character, _flow->_spans[object.in_span].in_input_stream_item);

    inline unsigned _lineToSpan(unsigned line_index) const
        {return std::lower_bound(_spans.begin(), _spans.end(), line_index, PredicateLineToSpan(this)) - _spans.begin();}
    inline unsigned _lineToCharacter(unsigned line_index) const
        {return std::lower_bound(_characters.begin(), _characters.end(), line_index, PredicateLineToCharacter(this)) - _characters.begin();}
    inline unsigned _spanToCharacter(unsigned span_index) const
        {return std::lower_bound(_characters.begin(), _characters.end(), span_index, PredicateSpanToCharacter(this)) - _characters.begin();}
    inline unsigned _sourceToCharacter(unsigned source_index) const
        {return std::lower_bound(_characters.begin(), _characters.end(), source_index, PredicateSourceToCharacter(this)) - _characters.begin();}

    /** given an x coordinate and a line number, returns an iterator
    pointing to the closest cursor position on that line to the
    coordinate. */
    iterator _cursorXOnLineToIterator(unsigned line_index, double local_x) const;

    /** calculates the width of a chunk, which is the largest x
    coordinate (start or end) of the spans contained within it. */
    double _getChunkWidth(unsigned chunk_index) const;
};

/** \brief Holds a position within the glyph output of Layout.

Used to access the output of a Layout, query information and generally
move around in it. See Layout for a glossary of the names of functions.

I'm not going to document all the methods because most of their names make
their function self-evident.

A lot of the functions would do the same thing in a naive implementation
for latin-only text, for example nextCharacter(), nextCursorPosition() and
cursorRight(). Generally it's fairly obvious which one you should use in a
given situation, but sometimes you might need to put some thought in to it.

All the methods return false if the requested action would have caused the
current position to move out of bounds. In this case the position is moved
to either begin() or end(), depending on which direction you were going.

Note that some characters do not have a glyph representation (eg line
breaks), so if you try using prev/nextGlyph() from one of these you're
heading for a crash.
*/
class Layout::iterator {
public:
    friend class Layout;
    // this is just so you can create uninitialised iterators - don't actually try to use one
    iterator() :
        _parent_layout(NULL),
        _glyph_index(-1),
        _char_index(0),
        _cursor_moving_vertically(false),
        _x_coordinate(0.0){}
    // no copy constructor required, the default does what we want
    bool operator== (iterator const &other) const
        {return _glyph_index == other._glyph_index && _char_index == other._char_index;}
    bool operator!= (iterator const &other) const
        {return _glyph_index != other._glyph_index || _char_index != other._char_index;}

    /* mustn't compare _glyph_index in these operators because for characters
    that don't have glyphs (line breaks, elided soft hyphens, etc), the glyph
    index is -1 which makes them not well-ordered. To be honest, interating by
    glyphs is not very useful and should be avoided. */
    bool operator< (iterator const &other) const
        {return _char_index < other._char_index;}
    bool operator<= (iterator const &other) const
        {return _char_index <= other._char_index;}
    bool operator> (iterator const &other) const
        {return _char_index > other._char_index;}
    bool operator>= (iterator const &other) const
        {return _char_index >= other._char_index;}

    /* **** visual-oriented methods **** */

    //glyphs
    inline bool prevGlyph();
    inline bool nextGlyph();

    //span
    bool prevStartOfSpan();
    bool thisStartOfSpan();
    bool nextStartOfSpan();

    //chunk
    bool prevStartOfChunk();
    bool thisStartOfChunk();
    bool nextStartOfChunk();

    //line
    bool prevStartOfLine();
    bool thisStartOfLine();
    bool nextStartOfLine();
    bool thisEndOfLine();

    //shape
    bool prevStartOfShape();
    bool thisStartOfShape();
    bool nextStartOfShape();

    /* **** text-oriented methods **** */

    //characters
    inline bool nextCharacter();
    inline bool prevCharacter();

    bool nextCursorPosition();
    bool prevCursorPosition();
    bool nextLineCursor(int n = 1);
    bool prevLineCursor(int n = 1);

    //words
    bool nextStartOfWord();
    bool prevStartOfWord();
    bool nextEndOfWord();
    bool prevEndOfWord();

    //sentences
    bool nextStartOfSentence();
    bool prevStartOfSentence();
    bool nextEndOfSentence();
    bool prevEndOfSentence();

    //paragraphs
    bool prevStartOfParagraph();
    bool thisStartOfParagraph();
    bool nextStartOfParagraph();
    //no endOfPara methods because that's just the previous char

    //sources
    bool prevStartOfSource();
    bool thisStartOfSource();
    bool nextStartOfSource();

    //logical cursor movement
    bool cursorUp(int n = 1);
    bool cursorDown(int n = 1);
    bool cursorLeft();
    bool cursorRight();

    //logical cursor movement (by word or paragraph)
    bool cursorUpWithControl();
    bool cursorDownWithControl();
    bool cursorLeftWithControl();
    bool cursorRightWithControl();

private:
    Layout const *_parent_layout;
    int _glyph_index;      /// index into Layout::glyphs, or -1
    unsigned _char_index;       /// index into Layout::character
    bool _cursor_moving_vertically;
    /** for cursor up/down movement we must maintain the x position where
    we started so the cursor doesn't 'drift' left or right with the repeated
    quantization to character boundaries. */
    double _x_coordinate;

    inline iterator(Layout const *p, unsigned c, int g)
        : _parent_layout(p), _glyph_index(g), _char_index(c), _cursor_moving_vertically(false), _x_coordinate(0.0) {}
    inline iterator(Layout const *p, unsigned c)
        : _parent_layout(p), _glyph_index(p->_characters[c].in_glyph), _char_index(c), _cursor_moving_vertically(false), _x_coordinate(0.0) {}
    // no dtor required
    void beginCursorUpDown();  /// stores the current x coordinate so that the cursor won't drift. See #_x_coordinate

    /** moves forward or backwards one cursor position according to the
    directionality of the current paragraph, but ignoring block progression.
    Helper for the cursor*() functions. */
    bool _cursorLeftOrRightLocalX(Direction direction);

    /** moves forward or backwards by until the next character with
    is_word_start according to the directionality of the current paragraph,
    but ignoring block progression. Helper for the cursor*WithControl()
    functions. */
    bool _cursorLeftOrRightLocalXByWord(Direction direction);
};

// ************************** inline methods

inline SPCurve* Layout::convertToCurves() const
    {return convertToCurves(begin(), end());}

inline Layout::iterator Layout::begin() const
    {return iterator(this, 0, 0);}

inline Layout::iterator Layout::end() const
    {return iterator(this, _characters.size(), _glyphs.size());}

inline Layout::iterator Layout::charIndexToIterator(int char_index) const
{
    if (char_index < 0) return begin();
    if (char_index >= (int)_characters.size()) return end();
    return iterator(this, char_index);
}

inline int Layout::iteratorToCharIndex(Layout::iterator const &it) const
    {return it._char_index;}

inline void Layout::validateIterator(Layout::iterator *it) const
{
    it->_parent_layout = this;
    if (it->_char_index >= _characters.size()) {
        it->_char_index = _characters.size();
        it->_glyph_index = _glyphs.size();
    } else
        it->_glyph_index = _characters[it->_char_index].in_glyph;
}

inline Layout::iterator Layout::getNearestCursorPositionTo(Geom::Point const &point) const
    {return getNearestCursorPositionTo(point[0], point[1]);}

inline Layout::iterator Layout::getLetterAt(Geom::Point &point) const
    {return getLetterAt(point[0], point[1]);}

inline unsigned Layout::lineIndex(iterator const &it) const
    {return it._char_index == _characters.size() ? _lines.size() - 1 : _characters[it._char_index].chunk(this).in_line;}

inline unsigned Layout::shapeIndex(iterator const &it) const
    {return it._char_index == _characters.size() ? _input_wrap_shapes.size() - 1 : _characters[it._char_index].line(this).in_shape;}

inline bool Layout::isWhitespace(iterator const &it) const
    {return it._char_index == _characters.size() || _characters[it._char_index].char_attributes.is_white;}

inline int Layout::characterAt(iterator const &it) const
{
    void *unused;
    Glib::ustring::iterator text_iter;
    getSourceOfCharacter(it, &unused, &text_iter);
    return *text_iter;
}

inline bool Layout::isCursorPosition(iterator const &it) const
    {return it._char_index == _characters.size() || _characters[it._char_index].char_attributes.is_cursor_position;}

inline bool Layout::isStartOfWord(iterator const &it) const
    {return it._char_index != _characters.size() && _characters[it._char_index].char_attributes.is_word_start;}

inline bool Layout::isEndOfWord(iterator const &it) const
    {return it._char_index == _characters.size() || _characters[it._char_index].char_attributes.is_word_end;}

inline bool Layout::isStartOfSentence(iterator const &it) const
    {return it._char_index != _characters.size() && _characters[it._char_index].char_attributes.is_sentence_start;}

inline bool Layout::isEndOfSentence(iterator const &it) const
    {return it._char_index == _characters.size() || _characters[it._char_index].char_attributes.is_sentence_end;}

inline unsigned Layout::paragraphIndex(iterator const &it) const
    {return it._char_index == _characters.size() ? _paragraphs.size() - 1 : _characters[it._char_index].line(this).in_paragraph;}

inline Layout::Alignment Layout::paragraphAlignment(iterator const &it) const
    {return _paragraphs[paragraphIndex(it)].alignment;}

inline bool Layout::iterator::nextGlyph()
{
    _cursor_moving_vertically = false;
    if (_glyph_index >= (int)_parent_layout->_glyphs.size() - 1) {
        if (_glyph_index == (int)_parent_layout->_glyphs.size()) return false;
        _char_index = _parent_layout->_characters.size();
        _glyph_index = _parent_layout->_glyphs.size();
    }
    else _char_index = _parent_layout->_glyphs[++_glyph_index].in_character;
    return true;
}

inline bool Layout::iterator::prevGlyph()
{
    _cursor_moving_vertically = false;
    if (_glyph_index == 0) return false;
    _char_index = _parent_layout->_glyphs[--_glyph_index].in_character;
    return true;
}

inline bool Layout::iterator::nextCharacter()
{
    _cursor_moving_vertically = false;
    if (_char_index + 1 >= _parent_layout->_characters.size()) {
        if (_char_index == _parent_layout->_characters.size()) return false;
        _char_index = _parent_layout->_characters.size();
        _glyph_index = _parent_layout->_glyphs.size();
    }
    else _glyph_index = _parent_layout->_characters[++_char_index].in_glyph;
    return true;
}

inline bool Layout::iterator::prevCharacter()
{
    _cursor_moving_vertically = false;
    if (_char_index == 0) return false;
    _glyph_index = _parent_layout->_characters[--_char_index].in_glyph;
    return true;
}

}//namespace Text
}//namespace Inkscape

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
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :
