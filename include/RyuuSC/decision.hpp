#pragma once
#include <vector>

#include "RyuuSC/types.hpp"

namespace RyuuSC {

// ─── Decision ──────────────────────────────────────────────────────────────────
/* 
 * A variable that holds all the decision making variables at any given period.
 */
struct Decision {
        std::vector<SkuID>		items;
        std::vector<std::vector<fInt>>	forecasts;
};

}
