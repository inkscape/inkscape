#ifndef __INK_SVG_PROFILE_H__
#define __INK_SVG_PROFILE_H__
/*
 * A class for managing which SVG Profiles are used.
 *
 * Authors:
 *   Ted Gould <ted@gould.cx>
 *
 * Copyright (C) 2004 Authors
 *
 * Released under GNU GPL, read the file 'COPYING' for more information
 */

namespace Inkscape {

/**
    \brief  A class that contains information on which SVG profiles are
            marked for the object.

    The basic functionality here is to allow marking of which profiles
    are supported or required by some other object.  The basic
    implementation of that is a bitfield, a data type that has bits
    to be able to be set depending on whether or not that particular
    feature is available.  This implementation is a little more complex
    for a coulple of reasons.

    The first reason for making it be more complex is to make it into
    a nice C++ friendly class, where the actual bits aren't required
    by the calling application.  This is accomplished by use of the
    \c SvgProfileType enum, and having the various fields in there.

    The second reason that this is more complex is that it is reasonable
    that the enum will grow to be greater than the native integer of
    a given system.  As more profiles and distinctions are made, there
    will have to be more entries in the enum.  In order to combat this,
    the \c BitField class was created.  This creates consistent operations
    independent of how many integers are used to represent the particular
    bitfield.
 
    The entire class has been optimized for inlining and compiler reduction
    of code.  All entires should allow being put in a structure or other
    static allocation.  For most operations, simple and/or instructions
    in the processor is required after optimization.

    Adding additional profiles to the class is as easy as adding entires
    into the enum.  Adding additional aggregates requires adding to the
    enum, and adding the aggregate into the constructor for the class.
*/
class SvgProfile {
public:
    /** \brief  The enum listing all the different profiles supported, and
                some aggregate ones supported.

        \note  This \c enum should not be used for anything other than passing
               data into the constructor of \c SvgProfile.  The optimization
               of the constructor is best when it is passed a static value.
               So, passing this enum through a function, and then constructing
               the \c SvgProfile would be less than ideal.  Plus, as far as
               memory goes, today, they take up the same amount of memory.
    */
    enum SvgProfileType {
        BASIC_OPERATION = 0,  /**< This describes a feature that is part
                                   of the basic functionality of Inkscape
                                   itself.  This would be like save or open. */
        SVG_BASE_1_0,         /**< The base SVG spec version 1.0 */
        SVG_BASE_1_1,         /**< The base SVG spec version 1.1 */
        SVG_BASE_1_2,         /**< The base SVG spec version 1.2 */
        SVG_BASE_2_0,         /**< The base SVG spec version 2.0 */

        SVG_MOBILE_TINY_1_1,  /**< The mobile SVG spec Tiny profile version 1.1 */
        SVG_MOBILE_BASIC_1_1, /**< The mobile SVG spec Basic profile version 1.1 */
        SVG_MOBILE_TINY_1_2,  /**< The mobile SVG spec Tiny profile version 1.2 */
        SVG_MOBILE_BASIC_1_2, /**< The mobile SVG spec Basic profile version 1.2 */

        SVG_PRINT_1_1,        /**< */

        PROFILE_UNIQUE_CNT,   /**< A marker to separate between the entires
                                   that are unique, and those which are
                                   aggregates of them. */

        SVG_BASE_ALL,         /**< Every version of the SVG Base spec */
        SVG_MOBILE_ALL,       /**< Every version and profile in SVG Mobile */
        SVG_TINY_ALL,         /**< Every version of the tiny profile in SVG Mobile */
        SVG_BASIC_ALL,        /**< Every version of the basic profile in SVG Mobile */
        ALL                   /**< All, everything, basically doesn't tell you anything at all */
    };

private:
    /** \brief  The core of the \c SvgProfile class, this implements a
                bitfield which can be several integers large with standard
                operations independent of size. */
    class BitField {
        /** \brief A quick way identify the number of bits in an integer. */
        #define  BITS_IN_INT (sizeof(int) * 8)
        /** \brief The size of the array which is being used. */
        #define  ARRAY_SIZE  (((PROFILE_UNIQUE_CNT - 1) / BITS_IN_INT) + 1)

            /** \brief The actuall array holding the bitfield. */
            unsigned int bits[ARRAY_SIZE];

        public:
            /** \brief Constructor for the bitfield, it clears the \c bits
                       array by setting things to zero. */
            inline BitField(void) {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    bits[i] = 0;
                }
            }

            /** \brief Constructs a bitfield by passing in another array
                       describing how the bits should look.  The function
                       just copies the array into \c bits. */
            inline BitField(unsigned int in_bits[ARRAY_SIZE]) {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    bits[i] = in_bits[i];
                }
            }

            /** \brief The equals operator, but it doesn't do quite that.
                       This function checks to see if there are bits that
                       are similarly high in both bitfields. */
            inline bool operator == (const BitField &in_field) const {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    if (bits[i] & in_field.bits[i] != 0)
                        return true;
                }
                return false;
            }

            /** \brief A convience function to set a particular bit in the
                       bitfield

                This function first find which integer the bit is in by
                dividing by \c BITS_IN_INT and then which bit in the
                integer by getting the modulus.  The selected integer is
                the \c |= with a \c 1 shifted left by the possition.
            */
            inline void set (const unsigned int pos) {
                unsigned int array_pos = pos / BITS_IN_INT;
                unsigned int bit_pos   = pos % BITS_IN_INT;
                bits[array_pos] |= 1 << bit_pos;
            }

            /** \brief Does a bitwise \c OR on two bitfields.  It does
                       this for the entire \c bits array. */
            inline BitField operator | (const BitField &other) const {
                unsigned int local_bits[ARRAY_SIZE];

                for (int i = 0; i < ARRAY_SIZE; i++) {
                    local_bits[i] = bits[i] | other.bits[i];
                }

                return BitField(local_bits);
            }

            /** \brief  Causes one \c BitField to take on the values
                        stored in a different bitfield. */
            inline BitField & operator = (const BitField &other) {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    bits[i] = other.bits[i];
                }

                return *this;
            }

            /** \brief  Does a logical \c OR of the bitfield with another
                        bitfield.  It does this by using \c |=. */
            inline BitField & operator |= (const BitField &other) {
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    bits[i] |= other.bits[i];
                }
                return *this;    
            }

        #undef BITS_IN_INT
        #undef ARRAY_SIZE
    };
    
    /** \brief The actual data stored on the profile. */
    BitField _profile;

    /**
        \brief  Create an SvgProfile with an already created bitfield
        \param  in_field  The bitfield that should be used in the profile

        This function just copies the incoming bitfield into the one
        which is allocated for this.
    */
    inline SvgProfile (const BitField &in_field) {
        _profile = in_field;
    }

public:
    /** \brief A constructor for \c SvgProfile which sets up the bitfield
               based on the \c SvgProfileType getting passed in.

        This function has basically two different modes of operation
        depending on whether the requested value is a pure profile or
        and aggregate.  If it is pure, then that bit is set in \c _profile
        and the function exits.  Otherwise a case statement is used to
        determine which aggregate is called, and then setting all of
        the bits for that aggregate.
    */
    inline SvgProfile (SvgProfileType type) {
        if (type < PROFILE_UNIQUE_CNT) {
            _profile.set(type);
        } else {
            /* Okay, so this could be done by OR'ing a bunch of these
               together, but I thought that would reduce the chance of
               the compiler actually figuring it all out and optimizing
               everything.  This is already getting pretty complex. */
            switch (type) {
                case SVG_BASE_ALL:
                    _profile.set(SVG_BASE_1_0);
                    _profile.set(SVG_BASE_1_1);
                    _profile.set(SVG_BASE_1_2);
                    _profile.set(SVG_BASE_2_0);
                    break;
                case SVG_BASIC_ALL:
                    _profile.set(SVG_MOBILE_BASIC_1_1);
                    _profile.set(SVG_MOBILE_BASIC_1_2);
                    break;
                case SVG_TINY_ALL:
                    _profile.set(SVG_MOBILE_TINY_1_1);
                    _profile.set(SVG_MOBILE_TINY_1_2);
                    break;
                case SVG_MOBILE_ALL:
                    _profile.set(SVG_MOBILE_BASIC_1_1);
                    _profile.set(SVG_MOBILE_BASIC_1_2);
                    _profile.set(SVG_MOBILE_TINY_1_1);
                    _profile.set(SVG_MOBILE_TINY_1_2);
                    break;
                case ALL:
                    _profile.set(SVG_BASE_1_0);
                    _profile.set(SVG_BASE_1_1);
                    _profile.set(SVG_BASE_1_2);
                    _profile.set(SVG_BASE_2_0);

                    _profile.set(SVG_MOBILE_BASIC_1_1);
                    _profile.set(SVG_MOBILE_BASIC_1_2);

                    _profile.set(SVG_MOBILE_TINY_1_1);
                    _profile.set(SVG_MOBILE_TINY_1_2);
                    break;
            };
        }
        return;    
    };

    /** \brief A function to check equality of two \c SvgProfiles.
               It doesn't quite check equality though, it more ensures
               that there is a profile supported by both objects.  This
               would be similar to: (a & b) != 0 in a standard bitfield
               impelemtation.  But is done this way for simplicity. */
    inline bool operator == (const SvgProfile &in_profile) const {
        return _profile == in_profile._profile;
    };

    /** \brief A function allow combining of \c SvgProfiles with each
               other into a larger \c SvgProfile. */
    inline SvgProfile operator | (const SvgProfile &other) const {
        return SvgProfile(_profile | other._profile);
    }

    /** \brief A quick way to add additional profiles to the currently
               allocated object. */
    inline SvgProfile & operator |= (const SvgProfile &other) {
        _profile |= other._profile;
        return *this;
    }
};


}  /* namespace Inkscape */

#endif /* __INK_SVG_PROFILE_H__ */

/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
