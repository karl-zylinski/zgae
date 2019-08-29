#pragma once
#include "gjk_epa_types.h"

bool gjk_intersect(C(GjkShape, s1), C(GjkShape, s2));
GjkEpaSolution gjk_epa_intersect_and_solve(C(GjkShape, s1), C(GjkShape, s2));