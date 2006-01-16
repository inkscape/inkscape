/* //////////////////////////////////////////////////////////////////////
//                               ftos.cc
//
// Copyright (c) 1996-2003 Bryce W. Harrington  [bryce at osdl dot org]
//
//-----------------------------------------------------------------------
// License:  This code may be used by anyone for any purpose
//           so long as the copyright notices and this license
//           statement remains attached.
//-----------------------------------------------------------------------
//
// string ftos(double val[, char mode[, int sigfig[, int precision[, int options]]]])
//
//  DESCRIPTION
//    This routine is intended to replace the typical use of sprintf for
//    converting floating point numbers into strings.
//
//    To one-up sprintf, an additional mode was created - 'h' mode -
//    which produces numbers in 'engineering notation' - exponents are
//    always shown in multiples of 3.  To non-engineers this mode is
//    probably irrelevant, but for engineers (and scientists) it is SOP.
//
//    One other new feature is an option to use 'x10^' instead of the
//    conventional 'E' for exponental notation.  This is entirely for
//    aesthetics since numbers in the 'x10^' form cannot be used as
//    inputs for most programs.
//
//    For most cases, the routine can simply be used with the defaults
//    and acceptable results will be produced.  No fill zeros or trailing
//    zeros are shown, and exponential notation is only used for numbers
//    greater than 1e6 or less than 1e-3.
//
//    The one area where sprintf may surpass this routine is in width control.
//    No provisions are made in this routine to restrict a number to a
//    certain number of digits (thus allowing the number to be constrained
//    to an 8 space column, for instance.)  Along with this, it does not
//    support pre-padding a number with zeros (e.g., '5' -> '0005') and will
//    not post-pad a number with spaces (i.e., allow left-justification.)
//
//    If width control is this important, then the user will probably want to
//    use the stdio routines, which really is well suited for outputting
//    columns of data with a brief amount of code.
//
//  PARAMETERS
//    val        - number to be converted
//    mode       - can be one of four possible values.  Default is 'g'
//
//                 e: Produces numbers in scientific notation.  One digit
//                    is shown on the left side of the decimal, the rest
//                    on the right, and the exponential is always shown.
//                    EXAMPLE:  1.04e-4
//
//                 f: Produces numbers with fixed format.  Number is shown
//                    exact, with no exponent.
//                    EXAMPLE:  0.000104
//
//                 g: If val is greater than 1e6 or less than 1e-3 it will
//                    be shown in 'e' format, otherwise 'f' format will be
//                    used.
//
//                 h: Produces numbers in engineering format.  Result is
//                    identical to 'f' format for numbers between 1 and
//                    1e3, otherwise, the number is shown such that it
//                    always begins with a nonzero digit on the left side
//                    (unless number equals zero), and the exponential is
//                    a multiple of 3.
//                    EXAMPLE:  104e-6
//
//                 If the mode is expressed as a capital letter (e.g., 'F')
//                 then the exponential part of the number will also be
//                 capitalized (e.g., '1E6' or '1X10^6'.)
//
//    sigfig     - the number of significant figures.  These are the digits
//                 that are "retained".  For example, the following numbers
//                 all have four sigfigs:
//                    1234       12.34      0.0001234       1.234e-10
//                 the last digit shown will be rounded in the standard
//                 manner (down if the next digit is less than 5, up otherwise.)
//
//    precision  - the number of digits to show to the right of the decimal.
//                 For example, all of the following numbers have precisions
//                 of 2:
//                    1234.00     12.34     0.00     1.23e-10   123.40e-12
//
//    options    - several options are allowed to control the look of the
//                 output.
//
//               FORCE_DECIMAL - require the decimal point to be shown for
//                 numbers that do not have any fractional digits (or that
//                 have a precision set to zero)
//                 EXAMPLE:  1.e6
//               FORCE_EXP_ZERO - pad the 10's zero in exponent if necessary
//                 EXAMPLE:  1e06
//               FORCE_HUNDRED_EXP_ZERO - pad the 100's zero in exponent if
//                 necessary.  Also pads 10's zero in exponent if necessary.
//                 EXAMPLE:  1e006
//               FORCE_EXP_PLUS - show the '+' in the exponent if exponent
//                 is used.
//                 EXAMPLE:  1e+6
//               FORCE_EXP - force the output to display the exponent
//                 EXAMPLE:  0e0
//               FORCE_X10 - use x10^ instead of E
//                 EXAMPLE:  1x10^6
//               FORCE_PLUS - force output of the '+' for positive numbers
//                 EXAMPLE:  +1e6
//
//                 Options can be combined using the usual OR method.  For
//                 example,
//
//                 ftos(123.456, 'f', -1, -1, FORCE_PLUS | FORCE_X10 | FORCE_EXP)
//
//                 gives "+123.456x10^0"
//
//  RETURN VALUE
//    The string representation of the number is returned from the routine.
//    The ANSI C++ Standard "string" class was used for several important
//    reasons.  First, because the string class manages it's own space, the
//    ftos routine does not need to concern itself with writing to unallocated
//    areas of memory or with handling memory reallocation internally.  Second,
//    it allows return of an object, not a pointer to an object; this may not
//    be as efficient, but it is cleaner and safer than the alternative.  Third,
//    the routine's return value can be directly assigned to a variable, i.e.
//        string var = ftos(3.1415);
//    which makes code much easier to comprehend and modify.
//
//    Internally, the ftos routine uses fairly typical string operators (=, +=,
//    +, etc.) which pretty much any other flavor of string class will define as
//    well.  Thus if one does not have access to the ANSI C++ Standard string
//    class, the user can substitute another with little difficulty.  (If the
//    alternate class is not named "string" then redefine "string" to whatever
//    you wish to use.  For example,
//        #define string CString
//
// November 1996 - Bryce Harrington
//    Created ftoa and ftos
//
// December 1996 - Bryce Harrington
//    Added engineering notation mode, added sigfig capability, added
//    significant debug code, added options, thoroughly debugged and
//    tested the code.
//
//
// June 1999 - Bryce Harrington
//    Modified to run on Linux for WorldForge
//
// March 2003 - Bryce Harrington
//    Removed DTAG() macros - use of fprintf(stderr,...) instead
//    Broke out round/itos/ftos into separate files
//    Removed curses bits
//
/////////////////////////////////////////////////////////////////////// */


// This is the routine used for converting a floating point into a string
// This may be included in stdlib.h on some systems and may conflict.
// Let me know your system & etc. so I can properly #ifdef this, but
// try commenting the following four lines out if you run into conflicts.
// extern "C" {
// char*
// ecvt (double val, size_t ndigit, int *decpt, int *sign);
// }

using namespace std;

#ifndef HAS_ECVT
#include <glib.h>
#endif


#include "ftos.h"


// This routine counts from the end of a string like '10229000' to find the index
// of the first non-'0' character (5 would be returned for the above number.)
int countDigs(char *p)
{
    int length =0;
    while (*(p+length)!='\0') length++;               // Count total length
    while (length>0 && *(p+length-1)=='0') length--;  // Scan backwards for a non-'0'
    return length;
}

// This routine determines how many digits make up the left hand
// side of the number if the abs value of the number is greater than 1, or the
// digits that make up the right hand side if the abs value of the number
// is between 0 and 1.  Returns 1 if v==0.  Return value is positive for numbers
// greater than or equal to 1, negative for numbers less than 0.1, and zero for
// numbers between 0.1 and 1.
int countLhsDigits(double v)
{
    if (v<0) v = -v;                   // Take abs value
    else if (v==0) return 1;           // Special case if v==0

    int n=0;
    for (; v<0.1; v*=10)               // Count digits on right hand side (l.t. 0.1)
        { n--; }
    for (; v>=1; v/=10)                // Count digits on left hand side (g.e. 1.0)
        { n++; }
    return n;
}

// This is the routine that does the work of converting the number into a string.
string ftos(double val, char mode, int sigfig, int precision, int options)
{
    // Parse the options to a more usable form
    // These options allow the user to control some of the ornaments on the
    // number that is output.  By default they are all false.  Turning them
    // on helps to "fix" the format of the number so it lines up in columns
    // better.
    // - require the decimal point to be shown for numbers that do not have
    //   any fractional digits (or that have a precision set to zero
    bool forceDecimal = (options & FORCE_DECIMAL);
    // - show the 10's and 100's zero in exponent
    bool forceExpZero = (options & FORCE_EXP_ZERO);
    bool forceHundredExpZero = (options & FORCE_HUNDRED_EXP_ZERO);
    // - show the '+' in the exponent if exponent is used
    bool forceExpPlus = (options & FORCE_EXP_PLUS);
    // - force the output to display the exponent
    bool forceExponent = (options & FORCE_EXP);
    // - use x10^ instead of E
    bool forcex10 = (options & FORCE_X10);
    // - force output of the '+' for positive numbers
    bool forcePlus = (options & FORCE_PLUS);

#ifdef DEBUG
    fprintf(stderr, "Options: ");
    fprintf(stderr, "  %4s = %s ", "x10", (forcex10            ? "on" : "off" ));
    fprintf(stderr, "  %4s = %s ", ".",   (forceDecimal        ? "on" : "off" ));
    fprintf(stderr, "  %4s = %s ", "e0",  (forceExpZero        ? "on" : "off" ));
    fprintf(stderr, "  %4s = %s ", "e00", (forceHundredExpZero ? "on" : "off" ));
    fprintf(stderr, "  %4s = %s ", "e+",  (forceExpPlus        ? "on" : "off" ));
    fprintf(stderr, "  %4s = %s ", "e",   (forceExponent       ? "on" : "off" ));
    fprintf(stderr, "  %4s = %s \n", "+#",  (forcePlus           ? "on" : "off" ));
#endif

    // - exponent usage
    bool useExponent = false;

    // Determine the case for the 'e' (if used)
    char E = (forcex10)? 'x' : 'e';
    if (g_ascii_isupper(mode)) {
        E = g_ascii_toupper(E);
        mode = g_ascii_tolower(mode);
    }

    // Determine how many decimals we're interested in
    int L = countLhsDigits(val);

#ifdef DEBUG
    fprintf(stderr, "*** L is %s\n", itos(L).c_str());
#endif

    int count = 0;
    if (sigfig==0)                     // bad input - don't want any sigfigs??!!
        return "";
    else if (precision>=0) {           // Use fixed number of decimal places
        count = precision;
        if (mode == 'e') count += 1;
        else if (mode == 'f') count += L;
        else if (mode == 'g') count += (L>6 || L<-3)? 1 : L;
        else if (mode == 'h') count += (L>0)? ((L-1)%3+1) : (L%3+3);
        if (sigfig>0) count = (sigfig > count)? count : sigfig;  // Use sigfig # if it means more decimal places
    }
    else if (sigfig>0)                 // Just use sigfigs
        count = sigfig;
    else                               // prec < 0 and sigfig < 0
        count = 10;
#ifdef DEBUG
    fprintf(stderr, "*** count is %s\n", itos(count).c_str());
#endif

    // Get number's string rep, sign, and exponent
    int sign = 0;
    int decimal=0;

#ifdef HAS_ECVT
    char *p = ecvt(val, count, &decimal, &sign);
#else
    char *p = (char *) g_strdup_printf("%.0f", val);
    // asprintf(&p, "%.0f", val);
#endif

#ifdef DEBUG
    fprintf(stderr, "*** string rep is %s\n", p);
    fprintf(stderr, "*** decimal is %s\n", itos(decimal).c_str());
    fprintf(stderr, "*** sign is %s\n", itos(sign).c_str());
#endif

    // Count the number of relevant digits in the resultant number
    int dig = countDigs(p);
    if (dig < sigfig) dig = sigfig;

#ifdef DEBUG
    fprintf(stderr, "*** digs is %s\n", itos(dig).c_str());
#endif

    // Determine number of digits to put on left side of the decimal point
    int lhs=0;
    // For 'g' mode, decide whether to use 'e' or 'f' format.
    if (mode=='g') mode = (decimal>6 || decimal<-3)? 'e' : 'f';
    switch (mode) {
        case 'e':
            lhs = 1;                   // only need one char on left side
            useExponent = true;        // force exponent use
            break;

        case 'f':
            lhs = (decimal<1)? 1 : decimal;
                                       // use one char on left for num < 1,
                                       // otherwise, use the number of decimal places.
            useExponent = false;       // don't want exponent for 'f' format
            break;

        case 'h':
            if (val==0.0)              // special case for if value is zero exactly.
                lhs = 0;               // this prevents code from returning '000.0'
            else
                lhs = (decimal<=0)? (decimal)%3 + 3  :  (decimal-1)%3+1;
            useExponent = !(lhs==decimal);   // only use exponent if we really need it
            break;

        default:
            return "**bad mode**";
    }

#ifdef DEBUG
    fprintf(stderr, "*** lhs is %s\n", itos(lhs).c_str());
#endif

    // Figure out the number of digits to show in the right hand side
    int rhs=0;
    if (precision>=0)
        rhs = precision;
    else if (val == 0.0)
        rhs = 0;
    else if (useExponent || decimal>0)
        rhs = dig-lhs;
    else
        rhs = dig-decimal;

    // can't use a negative rhs value, so turn it to zero if that is the case
    if (rhs<0) rhs = 0;

#ifdef DEBUG
    fprintf(stderr, "*** rhs is", itos(rhs).c_str());
#endif

    // Determine the exponent
    int exponent = decimal - lhs;
    if (val==0.0) exponent=0;          // prevent zero from getting an exponent
#ifdef DEBUG
    fprintf(stderr, "*** exponent is %s\n", itos(exponent).c_str());
#endif

    string ascii;

    // output the sign
    if (sign) ascii += "-";
    else if (forcePlus) ascii += "+";

    // output the left hand side
    if (!useExponent && decimal<=0)    // if fraction, put the 0 out front
        ascii += '0';
    else                               // is either exponential or >= 1, so write the lhs
        for (; lhs>0; lhs--)
            ascii += (*p)? *p++ : int('0'); // now fill in the numbers before decimal

#ifdef DEBUG
    fprintf(stderr, "*** ascii + sign + lhs is %s\n", ascii.c_str());
#endif

    // output the decimal point
    if (forceDecimal || rhs>0)
        ascii += '.';

    // output the right hand side
    if (!useExponent && rhs>0)         // first fill in zeros after dp and before numbers
        while (decimal++ <0 && rhs-->0)
            ascii += '0';
    for (; rhs>0 ; rhs--)              // now fill in the numbers after decimal
        ascii += (*p)? *p++ : int('0');

#ifdef DEBUG
    fprintf(stderr, "*** ascii + . + rhs is %s\n", ascii.c_str());
#endif

    if (forceExponent || useExponent)  // output the entire exponent if required
    {
        ascii += E;                    // output the E or X
        if (forcex10) ascii += "10^";  // if using 'x10^' format, output the '10^' part

        // output the exponent's sign
        if (exponent < 0) {            // Negative exponent
            exponent = -exponent;      // make exponent positive if needed
            ascii += '-';              // output negative sign
        }
        else if (forceExpPlus)         // We only want the '+' if it is asked for explicitly
            ascii += '+';

        // output the exponent
        if (forceHundredExpZero || exponent >= 100)
            ascii += ( (exponent/100) % 10 + '0' );
        if (forceHundredExpZero || forceExpZero || exponent >= 10)
            ascii += ( (exponent/10) % 10 + '0' );
        ascii += ( exponent % 10 + '0' );

#ifdef DEBUG
        fprintf(stderr, "*** ascii + exp is %s\n", ascii.c_str());
#endif
    }

#ifdef DEBUG
    fprintf(stderr, "*** End of ftos with ascii = ", ascii.c_str());
#endif
    /* finally, we can return */
    return ascii;
}

#ifdef TESTFTOS

int main()
{
  cout << "Normal (g): " << endl;
  cout << "1.0   = " << ftos(1.0)   << endl;
  cout << "42    = " << ftos(42)    << endl;
  cout << "3.141 = " << ftos(3.141) << endl;
  cout << "0.01  = " << ftos(0.01)  << endl;
  cout << "1.0e7 = " << ftos(1.0e7) << endl;
  cout << endl;

  cout << "Scientific (e): " << endl;
  cout << "1.0   = " << ftos(1.0,   'e')   << endl;
  cout << "42    = " << ftos(42,    'e')   << endl;
  cout << "3.141 = " << ftos(3.141, 'e') << endl;
  cout << "0.01  = " << ftos(0.01,  'e')  << endl;
  cout << "1.0e7 = " << ftos(1.0e7, 'e') << endl;
  cout << endl;

  cout << "Fixed (f): " << endl;
  cout << "1.0   = " << ftos(1.0,   'f')   << endl;
  cout << "42    = " << ftos(42,    'f')   << endl;
  cout << "3.141 = " << ftos(3.141, 'f') << endl;
  cout << "0.01  = " << ftos(0.01,  'f')  << endl;
  cout << "1.0e7 = " << ftos(1.0e7, 'f') << endl;
  cout << endl;

  cout << "Engineering (h): " << endl;
  cout << "1.0   = " << ftos(1.0,   'h')   << endl;
  cout << "42    = " << ftos(42,    'h')    << endl;
  cout << "3.141 = " << ftos(3.141, 'h') << endl;
  cout << "0.01  = " << ftos(0.01,  'h')  << endl;
  cout << "1.0e7 = " << ftos(1.0e7, 'h') << endl;
  cout << endl;

  cout << "Sigfigs: " << endl;
  cout << "2 sf = " << ftos(1234, 'g', 2) << "  "
       << ftos(12.34,     'g', 2) << "  "
       << ftos(0,         'g', 2) << "  "
       << ftos(123.4e-11, 'g', 2) << endl;
  cout << "4 sf = " << ftos(1234, 'g', 4) << "  "
       << ftos(12.34,     'g', 4) << "  "
       << ftos(0,         'g', 4) << "  "
       << ftos(123.4e-11, 'g', 4) << endl;
  cout << "8 sf = " << ftos(1234, 'g', 8) << "  "
       << ftos(12.34,     'g', 8) << "  "
       << ftos(0,         'g', 8) << "  "
       << ftos(123.4e-11, 'g', 8) << endl;
  cout << endl;

  cout << "x10 mode: " << endl;
  cout << "1234 = " << ftos(1234, 'e', 4, -1, FORCE_X10 | FORCE_EXP) << endl;
  cout << "1.01e10 = " << ftos(1.01e10, 'h', -1, -1, FORCE_X10 | FORCE_EXP) << endl;
  cout << endl;

  cout << "itos tests..." << endl;
  cout << "42   = " << itos(42) << endl;
  cout << endl;

  return 0;
}

#endif // TESTFTOS
