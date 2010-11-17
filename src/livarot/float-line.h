#ifndef INKSCAPE_LIVAROT_FLOAT_LINE_H
#define INKSCAPE_LIVAROT_FLOAT_LINE_H

/** \file
 * Coverage with floating-point boundaries.
 */

#include <vector>
#include "livarot/LivarotDefs.h"

class IntLigne;
class BitLigne;

/// A coverage portion ("run") with floating point boundaries.
struct float_ligne_run {
    float st;
    float en;
    float vst;
    float ven;
    float pente;   ///< (ven-vst)/(en-st)
};

/**
 * A floating-point boundary.
 * 
 * Each float_ligne_bord is a boundary of some coverage.
 * The Flatten() function will extract non-overlapping runs and produce an 
 * array of float_ligne_run. The float_ligne_bord are stored in an array, but 
 * linked like a doubly-linked list.
 * 
 * The idea behind that is that a given edge produces one float_ligne_bord at 
 * the beginning of Scan() and possibly another in AvanceEdge() and 
 * DestroyEdge(); but that second float_ligne_bord will not be far away in 
 * the list from the first, so it's faster to salvage the index of the first 
 * float_ligne_bord and try to insert the second from that salvaged position.
 */
struct float_ligne_bord {
    float pos;    ///< position of the boundary
    bool start;   ///< is the beginning of the coverage portion?
    float val;    ///< amount of coverage (ie vst if start==true, and ven if start==false)
    float pente;  ///< (ven-vst)/(en-st)
    int other;    ///< index, in the array of float_ligne_bord, of the other boundary associated to this one
    int s_prev;   ///< index of the previous bord in the doubly-linked list
    int s_next;   ///< index of the next bord in the doubly-linked list
    int pend_ind; ///< bords[i].pend_ind is the index of the float_ligne_bord that is the start of the
                  ///< coverage portion being scanned (in the Flatten() )  
    int pend_inv; ///< inverse of pend_ind, for faster handling of insertion/removal in the "pending" array
};

/**
 * Coverage with floating-point boundaries.
 *
 * The goal is to salvage exact coverage info in the sweepline performed by 
 * Scan() or QuickScan(), then clean up a bit, convert floating point bounds
 * to integer bounds, because pixel have integer bounds, and then raster runs 
 * of the type:
 * \verbatim
   position on the (pixel) line:                st         en
                                                |          |
   coverage value (0=empty, 1=full)            vst   ->   ven   \endverbatim
 */
class FloatLigne {
public:
    std::vector<float_ligne_bord> bords; ///< vector of coverage boundaries
    std::vector<float_ligne_run> runs;   ///< vector of runs

    /// first boundary in the doubly-linked list
    int s_first;
    /// last boundary in the doubly-linked list
    int s_last;

    FloatLigne();
    virtual ~FloatLigne();

    void Reset();
    
    int AddBord(float spos, float sval, float epos, float eval, int guess = -1);
    int AddBord(float spos, float sval, float epos, float eval, float pente, int guess = -1);
    int AddBordR(float spos, float sval, float epos, float eval, float pente, int guess = -1);
    int AppendBord(float spos, float sval, float epos, float eval, float pente);
    
    void Flatten();

    void Affiche();

    void Max(FloatLigne *a, float tresh, bool addIt);
    
    void Min(FloatLigne *a, float tresh, bool addIt);
    
    void Split(FloatLigne *a, float tresh, FloatLigne *over);
    
    void Over(FloatLigne *a, float tresh);
	
    void Copy(IntLigne *a);
    void Copy(FloatLigne *a);

    float RemainingValAt(float at, int pending);
  
    static int CmpBord(float_ligne_bord const &d1, float_ligne_bord const &d2) {
        if ( d1.pos == d2.pos ) {
            if ( d1.start && !(d2.start) ) {
                return 1;
            }
            if ( !(d1.start) && d2.start ) {
                return -1;
            }
            return 0;
        }
        
        return (( d1.pos < d2.pos ) ? -1 : 1);
    };

    int AddRun(float st, float en, float vst, float ven, float pente);

private:
    void InsertBord(int no, float p, int guess);
    int AddRun(float st, float en, float vst, float ven);

    inline float ValAt(float at, float ps, float pe, float vs, float ve) {
        return ((at - ps) * ve + (pe - at) * vs) / (pe - ps);
    };
};

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
