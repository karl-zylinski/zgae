#pragma once
#include "gjk_epa_types.h"

bool gjk_intersect(const GjkShape& s1, const GjkShape& s2);
GjkEpaSolution gjk_epa_intersect_and_solve(const GjkShape& s1, const GjkShape& s2);
