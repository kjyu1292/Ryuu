#pragma once

#include "RyuuSC/book.hpp"
#include "RyuuSC/decision.hpp"

#include "Ryuu/Forecast/forecast_base.hpp"

namespace RyuuSC::Forecast {
	
struct MovingAverage final : ForecastAlgoBase {
	std::size_t			window;
	
	[[nodiscard]] Decision operator()(const Book& book) const noexcept override {
		Decision d;
		/* */
		return d;
	}

	[[nodiscard]] std::string_view name() const noexcept override 
		{ return "MA"; }
};

}
