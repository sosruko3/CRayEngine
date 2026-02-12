#ifndef CRE_TYPESMACRO_H
#define CRE_TYPESMACRO_H

#include "cre_types.h"
#include "raylib.h"

// creVec2 -> Vector2
#define R_VEC(v) ((Vector2){ (v).x, (v).y })
// creColor -> Color
#define R_COL(c) ((Color){ (c).r, (c).g, (c).b, (c).a })
// creRectangle -> Rectangle
#define R_REC(r) ((Rectangle){ (r).x, (r).y, (r).width, (r).height })

#endif
