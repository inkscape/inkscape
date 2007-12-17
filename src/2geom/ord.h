
#ifndef __2GEOM_ORD__
#define __2GEOM_ORD__

namespace {

enum Cmp {
  LESS_THAN=-1,
  GREATER_THAN=1,
  EQUAL_TO=0
};

inline Cmp operator-(Cmp x) {
  switch(x) {
  case LESS_THAN:
    return GREATER_THAN;
  case GREATER_THAN:
    return LESS_THAN;
  case EQUAL_TO:
    return EQUAL_TO;
  }
}

template <typename T1, typename T2>
inline Cmp cmp(T1 const &a, T2 const &b) {
  if ( a < b ) {
    return LESS_THAN;
  } else if ( b < a ) {
    return GREATER_THAN;
  } else {
    return EQUAL_TO;
  }
}

}

#endif
