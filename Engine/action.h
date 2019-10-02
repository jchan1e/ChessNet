#ifndef ACTION_H
#define ACTION_H

#include "boardstate.h"

struct Action {
  int i1;
  int j1;
  int i2;
  int j2;
};

inline bool operator==(const Action& lhs, const Action& rhs){
  return lhs.i1 == rhs.i1 &&
         lhs.j1 == rhs.j1 &&
         lhs.i2 == rhs.i2 &&
         lhs.j2 == rhs.j2;
}

#endif
