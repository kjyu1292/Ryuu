#pragma once
#include <vector>

#include "RyuuSC/types.hpp"

namespace RyuuSC {

// ─── Book ─────────────────────────────────────────────────────────────────────
/*
 * The book acts as a ledger that holds all the historical data that needs
 * analyzing.
 */
struct Book {
        std::vector<SkuID>         	items;
        std::vector<std::vector<fInt>>	quantity;
        std::vector<fInt>		value;
};

}
