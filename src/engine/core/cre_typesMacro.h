#ifndef CRE_TYPESMACRO_H
#define CRE_TYPESMACRO_H

#include "cre_types.h"
#include "raylib.h"

[[nodiscard]] inline Vector2 R_VEC(creVec2 v) { return Vector2{v.x, v.y}; }

[[nodiscard]] inline Color R_COL(creColor c) {
  return Color{c.r, c.g, c.b, c.a};
}

[[nodiscard]] inline Rectangle R_REC(creRectangle r) {
  return Rectangle{r.x, r.y, r.width, r.height};
}
#endif
