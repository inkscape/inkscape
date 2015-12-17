#ifndef INKSCAPE_TEXT_TAG_ATTRIBUTES_H
#define INKSCAPE_TEXT_TAG_ATTRIBUTES_H

#include <vector>
#include <glib.h>
#include "libnrtype/Layout-TNG.h"
#include "svg/svg-length.h"

namespace Inkscape {
namespace XML {
class Node;
}
}


/** \brief contains and manages the attributes common to all types of text tag

The five attributes x, y, dx, dy and rotate (todo: textlength, lengthadjust)
are permitted on all of text, tspan and textpath elements so we need a class
to abstract the management of those attributes from the actual type of the
element.
*/
class TextTagAttributes {
public:
    TextTagAttributes() {}
    TextTagAttributes(Inkscape::Text::Layout::OptionalTextTagAttrs const &attrs)
        : attributes(attrs) {}

    /// Fill in all the fields of #attributes from the given node.
    void readFrom(Inkscape::XML::Node const *node);

    /** Process the parameters from the set() function of SPObject.
        Returns true if \a key was a recognised attribute. */
    bool readSingleAttribute(unsigned key, gchar const *value, SPStyle const *style, Geom::Rect const *viewport);

    /// Write out all the contents of #attributes to the given node.
    void writeTo(Inkscape::XML::Node *node) const;

    /// Update relative values
    void update( double em, double ex, double w, double h );

    /** For tspan role=line elements we should not use the set x,y
    coordinates since that would overrule the values calculated by the
    text layout engine, however if there are more than one element in
    the x or y vectors we can presume that the user set them and hence
    they should be copied. This function detects that condition so the
    \a use_xy parameter to mergeInto() can be set correctly. */
    bool singleXYCoordinates() const;

    /** Returns false if all of the vectors are zero length. */
    bool anyAttributesSet() const;

    /** Implements the rules for overlaying the contents of the class
    (treated as the child object) on top of previously existing
    attributes from \a parent_attrs using the rules described in
    SVG 1.1 section 10.5. \a parent_attrs_offset can be used to require
    that only fields from \a parent_attrs starting at that index will
    be used. Basically, the algorithm is that if a child attribute
    exists that will be used, otherwise the parent attribute will be used,
    otherwise the vector will end. textLength is never merged with parent. */
    void mergeInto(Inkscape::Text::Layout::OptionalTextTagAttrs *output, Inkscape::Text::Layout::OptionalTextTagAttrs const &parent_attrs, unsigned parent_attrs_offset, bool copy_xy, bool copy_dxdyrotate) const;

    /** Deletes all the values from all the vectors beginning at
    \a start_index and extending for \a n fields. This is what you want
    to do when deleting characters from the corresponding text. */
    void erase(unsigned start_index, unsigned n);

    /** Inserts \a n new values in all the stored vectors at \a
    start_index. This is what you want to do when inserting characters
    in the corresponding text. If a vector is shorter than \a start_index
    it will not be extended (the defaults are fine). dx, dy and rotate
    will be extended with zero values, x and y will be extended with
    linearly interpolated values. TODO: The inserted values should probably
    be unset but sp_svg_length_list_read() can't cope with that. */
    void insert(unsigned start_index, unsigned n);

    /** Divides the stored attributes into two, at the given index. The
    first section (0..index-1) stay in this object, the second section
    (index..end) go in \a second. This function is generally used when
    line breaking. */
    void split(unsigned index, TextTagAttributes *second);

    /** Overwrites all the attributes contained in this object with the
    given parameters by putting \a first at the beginning, then the
    contents of \a second after \a second_index. */
    void join(TextTagAttributes const &first, TextTagAttributes const &second, unsigned second_index);

    /** Applies the given transformation to the stored coordinates. Pairs
    of x and y coordinates are multiplied by the matrix and the dx and dy
    vectors are multiplied by the given parameters. rotate is not altered.
    If \a extend_zero_length is true, then if the x or y vectors are empty
    they will be made length 1 in order to store the newly calculated
    position. */
    void transform(Geom::Affine const &matrix, double scale_x, double scale_y, bool extend_zero_length = false);

    /** Gets current value of dx vector at \a index. */
    double getDx(unsigned index);

    /** Gets current value of dy vector at \a index. */
    double getDy(unsigned index);

    /** Adds the given value to the dx vector at the given
    \a index. The vector is extended if necessary. */
    void addToDx(unsigned index, double delta);

    /** Adds the given value to the dy vector at the given
    \a index. The vector is extended if necessary. */
    void addToDy(unsigned index, double delta);

    /** Adds the given values to the dx and dy vectors at the given
    \a index. The vectors are extended if necessary. */
    void addToDxDy(unsigned index, Geom::Point const &adjust);

    /** Gets current value of rotate vector at \a index. */
    double getRotate(unsigned index);

    /** Adds the given value to the rotate vector at the given \a index. The
    vector is extended if necessary. Delta is measured in degrees, clockwise
    positive. */
    void addToRotate(unsigned index, double delta);

    /** Sets rotate vector at the given \a index. The vector is extended if
    necessary. Angle is measured in degrees, clockwise positive. */
    void setRotate(unsigned index, double angle);

    /** Returns the first coordinates in the x and y vectors. If either
    is zero length, 0.0 is used for that coordinate. */
    Geom::Point firstXY() const;

    /** Sets the first coordinates in the x and y vectors. */
    void setFirstXY(Geom::Point &point);

    SVGLength *getTextLength() { return &(attributes.textLength); }
    int getLengthAdjust() { return attributes.lengthAdjust; }

private:
    /// This holds the actual values.
    Inkscape::Text::Layout::OptionalTextTagAttrs attributes;

    /** Does the reverse of readSingleAttribute(), converting a vector<> to
    its SVG string representation and writing it in to \a node. Used by
    writeTo(). */
    static void writeSingleAttributeVector(Inkscape::XML::Node *node, gchar const *key, std::vector<SVGLength> const &attr_vector);

    /** Writes a single length value to \a node. Used by
    writeTo(). */
    static void writeSingleAttributeLength(Inkscape::XML::Node *node, gchar const *key, const SVGLength &length);

    /** Does mergeInto() for one member of #attributes. If \a overlay_list
    is NULL then it does a simple copy of parent elements, starting at
    \a parent_offset. */
    static void mergeSingleAttribute(std::vector<SVGLength> *output_list, std::vector<SVGLength> const &parent_list, unsigned parent_offset, std::vector<SVGLength> const *overlay_list = NULL);

    /// Does the work for erase().
    static void eraseSingleAttribute(std::vector<SVGLength> *attr_vector, unsigned start_index, unsigned n);

    /// Does the work for insert().
    static void insertSingleAttribute(std::vector<SVGLength> *attr_vector, unsigned start_index, unsigned n, bool is_xy);

    /// Does the work for split().
    static void splitSingleAttribute(std::vector<SVGLength> *first_vector, unsigned index, std::vector<SVGLength> *second_vector, bool trimZeros);

    /// Does the work for join().
    static void joinSingleAttribute(std::vector<SVGLength> *dest_vector, std::vector<SVGLength> const &first_vector, std::vector<SVGLength> const &second_vector, unsigned second_index);
};


#endif /* !INKSCAPE_TEXT_TAG_ATTRIBUTES_H */

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
