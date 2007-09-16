#ifndef __2GEOM_SHAPE_H
#define __2GEOM_SHAPE_H

#include <vector>
#include <set>

#include "region.h"

//TODO: BBOX optimizations

namespace Geom {

enum {
  BOOLOP_JUST_A  = 1,
  BOOLOP_JUST_B  = 2,
  BOOLOP_BOTH    = 4,
  BOOLOP_NEITHER = 8
};

enum {
  BOOLOP_NULL         = 0,
  BOOLOP_INTERSECT    = BOOLOP_BOTH,
  BOOLOP_SUBTRACT_A_B = BOOLOP_JUST_B,
  BOOLOP_IDENTITY_A   = BOOLOP_JUST_A | BOOLOP_BOTH,
  BOOLOP_SUBTRACT_B_A = BOOLOP_JUST_A,
  BOOLOP_IDENTITY_B   = BOOLOP_JUST_B | BOOLOP_BOTH,
  BOOLOP_EXCLUSION    = BOOLOP_JUST_A | BOOLOP_JUST_B,
  BOOLOP_UNION        = BOOLOP_JUST_A | BOOLOP_JUST_B | BOOLOP_BOTH
};

class Shape {
    Regions content;
    mutable bool fill;
    //friend Shape shape_region_boolean(bool rev, Shape const & a, Region const & b);
    friend CrossingSet crossings_between(Shape const &a, Shape const &b);
    friend Shape shape_boolean(bool rev, Shape const &, Shape const &, CrossingSet const &);
    friend Shape boolop(Shape const &a, Shape const &b, unsigned);
    friend Shape boolop(Shape const &a, Shape const &b, unsigned, CrossingSet const &);
    friend void add_to_shape(Shape &s, Path const &p, bool);
  public:
    Shape() : fill(true) {}
    explicit Shape(Region const & r) {
        content = Regions(1, r);
        fill = r.fill;
    }
    explicit Shape(Regions const & r) : content(r) { update_fill(); }
    explicit Shape(bool f) : fill(f) {}
    Shape(Regions const & r, bool f) : content(r), fill(f) {}
    
    Regions getContent() const { return content; }
    bool isFill() const { return fill; }
    
    unsigned size() const { return content.size(); }
    const Region &operator[](unsigned ix) const { return content[ix]; }
    
    Shape inverse() const;
    Shape operator*(Matrix const &m) const;
    
    bool contains(Point const &p) const;
    
    bool inside_invariants() const;  //semi-slow & easy to violate : checks that the insides are inside, the outsides are outside
    bool region_invariants() const; //semi-slow                    : checks for self crossing
    bool cross_invariants() const; //slow                          : checks that everything is disjoint
    bool invariants() const;      //vera slow (combo, checks the above)

  private:
    std::vector<unsigned> containment_list(Point p) const;
    void update_fill() const {
        unsigned ix = outer_index(content);
        if(ix < size())
            fill = content[ix].fill;
        else if(size() > 0)
            fill = content.front().fill;
        else
            fill = true;
    }
};

inline CrossingSet crossings_between(Shape const &a, Shape const &b) { return crossings(paths_from_regions(a.content), paths_from_regions(b.content)); }

Shape shape_boolean(bool rev, Shape const &, Shape const &, CrossingSet const &);
Shape shape_boolean(bool rev, Shape const &, Shape const &);

//unsigned pick_coincident(unsigned ix, unsigned jx, bool &rev, std::vector<Path> const &ps, CrossingSet const &crs);
//void outer_crossing(unsigned &ix, unsigned &jx, bool & dir, std::vector<Path> const & ps, CrossingSet const & crs);
void crossing_dual(unsigned &i, unsigned &j, CrossingSet const & crs);
unsigned crossing_along(double t, unsigned ix, unsigned jx, bool dir, Crossings const & crs);

Shape boolop(Shape const &, Shape const &, unsigned flags);
Shape boolop(Shape const &, Shape const &, unsigned flags, CrossingSet &);

Shape sanitize(std::vector<Path> const &ps);

Shape stopgap_cleaner(std::vector<Path> const &ps);

inline std::vector<Path> desanitize(Shape const & s) {
    return paths_from_regions(s.getContent());
}

}

#endif
