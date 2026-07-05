#pragma once

#include "RyuuSC/book.hpp"
#include "RyuuSC/decision.hpp"

namespace RyuuSC::Forecast {

struct ForecastAlgoBase {
	virtual ~ForecastAlgoBase() = default;
	[[nodiscard]] virtual Decision operator()(const Book* book) const noexcept = 0;
	[[nodiscard]] virtual std::string_view() name() const noexcept = 0;
};

}
