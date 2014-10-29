/*
 * Generic auxiliary routines for 3D axes
 *
 * Authors:
 *   Maximilian Albert <Anhalter42@gmx.de>
 *
 * Copyright (C) 2007 authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

#ifndef SEEN_AXIS_MANIP_H
#define SEEN_AXIS_MANIP_H

#include <cassert>
#include <string>
#include <utility>

namespace Proj {

enum VPState {
    VP_FINITE = 0,
    VP_INFINITE
};

// The X-/Y-/Z-axis corresponds to the first/second/third digit
// in binary representation, respectively.
enum Axis {
    X = 0,
    Y = 1,
    Z = 2,
    W = 3,
    NONE
};

extern Axis axes[4];

inline char const*
string_from_axis(Proj::Axis axis) {
    switch (axis) {
    case X: return "X"; break;
    case Y: return "Y"; break;
    case Z: return "Z"; break;
    case W: return "W"; break;
    case NONE: return "NONE"; break;
    }
    return "";
}

} // namespace Proj

namespace Box3D {

const double epsilon = 1e-6;

// The X-/Y-/Z-axis corresponds to the first/second/third digit
// in binary representation, respectively.
enum Axis {
    X = 1,
    Y = 2,
    Z = 4,
    XY = 3,
    XZ = 5,
    YZ = 6,
    XYZ = 7,
    NONE = 0
};

// We use the fourth bit in binary representation
// to indicate whether a face is front or rear.
enum FrontOrRear { // find a better name
    FRONT = 0,
    REAR = 8
};

// converts X, Y, Z respectively to 0, 1, 2 (for use as array indices, e.g)
inline int axis_to_int(Box3D::Axis axis) {
    switch (axis) {
    case Box3D::X:
        return 0;
    case Box3D::Y:
        return 1;
    case Box3D::Z:
        return 2;
    case Box3D::NONE:
        return -1;
    default:
        assert(false);
        return -1; // help compiler's flow analysis (-Werror=return-value)
    }
}

inline Proj::Axis toProj(Box3D::Axis axis) {
    switch (axis) {
    case Box3D::X:
        return Proj::X;
    case Box3D::Y:
        return Proj::Y;
    case Box3D::Z:
        return Proj::Z;
    case Box3D::NONE:
        return Proj::NONE;
    default:
        assert(false);
        return Proj::NONE; // help compiler's flow analysis (-Werror=return-value)
    }
}

extern Axis axes[3];
extern Axis planes[3];
extern FrontOrRear face_positions [2];

} // namespace Box3D

namespace Proj {

inline Box3D::Axis toAffine(Proj::Axis axis) {
    switch (axis) {
    case Proj::X:
        return Box3D::X;
    case Proj::Y:
        return Box3D::Y;
    case Proj::Z:
        return Box3D::Z;
    case Proj::NONE:
        return Box3D::NONE;
    default:
        assert(false);
        return Box3D::NONE; // help compiler's flow analysis (-Werror=return-value)
    }
}

} // namespace Proj

namespace Box3D {

/* 
 * Identify the axes X, Y, Z with the numbers 0, 1, 2.
 * A box's face is identified by the axis perpendicular to it.
 * For a rear face, add 3.
 */
// Given a bit sequence that unambiguously specifies the face of a 3D box,
// return a number between 0 and 5 corresponding to that particular face
// (which is normally used to index an array). Return -1 if the bit sequence
// does not specify a face. A face can either be given by its plane (e.g, XY)
// or by the axis that is orthogonal to it (e.g., Z).
inline int face_to_int (unsigned int face_id) {
    switch (face_id) {
      case 1:  return 0;
      case 2:  return 1;
      case 4:  return 2;
      case 3:  return 2;
      case 5:  return 1;
      case 6:  return 0;

      case 9:  return 3;
      case 10: return 4;
      case 12: return 5;
      case 11: return 5;
      case 13: return 4;
      case 14: return 3;

    default: return -1;
    }
}

inline int int_to_face (unsigned id) {
    switch (id) {
    case 0: return Box3D::YZ ^ Box3D::FRONT;
    case 1: return Box3D::XZ ^ Box3D::FRONT;
    case 2: return Box3D::XY ^ Box3D::FRONT;
    case 3: return Box3D::YZ ^ Box3D::REAR;
    case 4: return Box3D::XZ ^ Box3D::REAR;
    case 5: return Box3D::XY ^ Box3D::REAR;
    }
    return Box3D::NONE; // should not be reached
}

inline bool is_face_id (unsigned int face_id) {
    return !((face_id & 0x7) == 0x7);
}

/**
inline gint opposite_face (guint face_id) {
    return face_id + (((face_id % 2) == 0) ? 1 : -1);
}
**/

inline unsigned int number_of_axis_directions (Box3D::Axis axis) {
    unsigned int num = 0;
    if (axis & Box3D::X) num++;
    if (axis & Box3D::Y) num++;
    if (axis & Box3D::Z) num++;

    return num;
}

inline bool is_plane (Box3D::Axis plane) {
    return (number_of_axis_directions (plane) == 2);
}

inline bool is_single_axis_direction (Box3D::Axis dir) {
    // tests whether dir is nonzero and a power of 2
    return (!(dir & (dir - 1)) && dir);
}

/**
 * Given two axis directions out of {X, Y, Z} or the corresponding plane, return the remaining one
 * We don't check if 'plane' really specifies a plane (i.e., if it consists of precisely two directions).
 */
inline Box3D::Axis third_axis_direction (Box3D::Axis dir1, Box3D::Axis dir2) {
    return (Box3D::Axis) ((dir1 + dir2) ^ 0x7);
}
inline Box3D::Axis third_axis_direction (Box3D::Axis plane) {
    return (Box3D::Axis) (plane ^ 0x7);
}

/* returns the first/second axis direction occuring in the (possibly compound) expression 'dirs' */
inline Box3D::Axis extract_first_axis_direction (Box3D::Axis dirs) {
    if (dirs & Box3D::X) return Box3D::X;
    if (dirs & Box3D::Y) return Box3D::Y;
    if (dirs & Box3D::Z) return Box3D::Z;
    return Box3D::NONE;
}
inline Box3D::Axis extract_second_axis_direction (Box3D::Axis dirs) {
    return extract_first_axis_direction ((Box3D::Axis) (dirs ^ extract_first_axis_direction(dirs)));
}

inline Box3D::Axis orth_plane_or_axis (Box3D::Axis axis) {
    return (Box3D::Axis) (Box3D::XYZ ^ axis);
}

/* returns an axis direction perpendicular to the ones occuring in the (possibly compound) expression 'dirs' */
inline Box3D::Axis get_perpendicular_axis_direction (Box3D::Axis dirs) {
    if (!(dirs & Box3D::X)) return Box3D::X;
    if (!(dirs & Box3D::Y)) return Box3D::Y;
    if (!(dirs & Box3D::Z)) return Box3D::Z;
    return Box3D::NONE;
}

char * string_from_axes (Box3D::Axis axis);
std::pair <Axis, Axis> get_remaining_axes (Axis axis);

} // namespace Box3D

#endif /* !SEEN_AXIS_MANIP_H */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
