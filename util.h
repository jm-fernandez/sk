#ifndef UTIL_H__
#define UTIL_H__

#include <stdbool.h>
#include <stdlib.h>

// square
typedef struct square_t_ {
  int x;
  int y;
  int sx;
  int sy;
} square_t;


static inline bool square_empty(const square_t* square)
{
  return (square->sx == 0) || (square->sy == 0);
}

static inline square_t square_intersection(const square_t* left, const square_t* right)
{
  const int new_x = max(left->x, right->x);
  const int new_y = max(left->y, right->y);
  const int lxe = left->x + left->sx;
  const int rxe = right->x + right->sx;
  const int new_xe = min(lxe, rxe);
  const int lye = left->y + left->sy;
  const int rye = right->y + right->sy;
  const int new_ye = min(lye, rye);
  const int new_sx = new_xe - new_x;
  const int new_sy = new_ye - new_y;
  square_t result = {0};

  result.x = new_x;
  result.y = new_y;
  result.sx = new_sx > 0 ? new_sx : 0;
  result.sy = new_sy > 0 ? new_sy : 0;
  return result;
}

static inline bool square_contained(const square_t* left, const square_t* right)
{
  const int lxe = left->x + left->sx;
  const int rxe = right->x + right->sx;
  const int lye = left->y + left->sy;
  const int rye = right->y + right->sy;

  return (left->x <= right->x) &&
    (left->y <= right->y) &&
    (rxe <= lxe) &&
    (rye <= lye);
}

#endif // UTIL_H__
